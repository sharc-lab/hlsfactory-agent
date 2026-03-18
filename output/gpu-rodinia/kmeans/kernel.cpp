#include "kernel.h"

// --- from kmeans_cuda_kernel.cu ---
extern "C"
void invert_mapping(float *input,			/* original */
							   float *output,			/* inverted */
							   int npoints,				/* npoints */
							   int nfeatures)			/* nfeatures */
{
	int point_id = _tid_x + BLOCK_DIM_X*_bid_x;	/* id of thread */
	int i;

	if(point_id < npoints){
		for(i=0;i<nfeatures;i++)
			output[point_id + npoints*i] = input[point_id*nfeatures + i];
	}
	return;
}
extern "C"

void kmeansPoint(float  *features,			/* in: [npoints*nfeatures] */
            int     nfeatures,
            int     npoints,
            int     nclusters,
            int    *membership,
			float  *clusters,
			float  *block_clusters,
			int    *block_deltas) 
{
    #pragma HLS INTERFACE m_axi port=features offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=nfeatures offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=npoints
    #pragma HLS INTERFACE s_axilite port=nclusters
    #pragma HLS INTERFACE m_axi port=membership offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=clusters offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=block_clusters offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=block_deltas offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=deltas complete dim=1
    #pragma HLS ARRAY_PARTITION variable=new_center_ids complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1


                // block ID
                const unsigned int block_id = GRID_DIM_X*_bid_y+_bid_x;
                // point/thread ID
                const unsigned int point_id = block_id*BLOCK_DIM_X*BLOCK_DIM_Y + _tid_x;

                int  index = -1;

                if (point_id < npoints)
                {
                int i, j;
                float min_dist = FLT_MAX;
                float dist;													/* distance square between a point to cluster center */

                /* find the cluster center id with min distance to pt */
                for (i=0; i<nclusters; i++) {
                int cluster_base_index = i*nfeatures;					/* base index of cluster centers for inverted array */
                float ans=0.0;												/* Euclidean distance sqaure */

                for (j=0; j < nfeatures; j++)
                {
                int addr = point_id + j*npoints;					/* appropriate index of data point */
                float diff = (tex1Dfetch(t_features,addr) -
                c_clusters[cluster_base_index + j]);	/* distance between a data point to cluster centers */
                ans += diff*diff;									/* sum of squares */
                }
                dist = ans;

                /* see if distance is smaller than previous ones:
                if so, change minimum distance and save index of cluster center */
                if (dist < min_dist) {
                min_dist = dist;
                index    = i;
                }
                }
                }


                #ifdef GPU_DELTA_REDUCTION
                // count how many points are now closer to a different cluster center
                int deltas[THREADS_PER_BLOCK];
                if(_tid_x < THREADS_PER_BLOCK) {
                deltas[_tid_x] = 0;
                }
                #endif
                if (point_id < npoints)
                {
                #ifdef GPU_DELTA_REDUCTION
                /* if membership changes, increase delta by 1 */
                if (membership[point_id] != index) {
                deltas[_tid_x] = 1;
                }
                #endif
                /* assign the membership to object point_id */
                membership[point_id] = index;
                }

                #ifdef GPU_DELTA_REDUCTION
                // make sure all the deltas have finished writing to shared memory

                // now let's count them
                // primitve reduction follows
                unsigned int threadids_participating = THREADS_PER_BLOCK / 2;
                for(;threadids_participating > 1; threadids_participating /= 2) {
                if(_tid_x < threadids_participating) {
                deltas[_tid_x] += deltas[_tid_x + threadids_participating];
                }
                }
                if(_tid_x < 1)	{deltas[_tid_x] += deltas[_tid_x + 1];}
                // propagate number of changes to global counter
                if(_tid_x == 0) {
                block_deltas[_bid_y * GRID_DIM_X + _bid_x] = deltas[0];
                //printf("original id: %d, modified: %d\n", _bid_y*GRID_DIM_X+_bid_x, _bid_x);

                }

                #endif

                #ifdef GPU_NEW_CENTER_REDUCTION
                int center_id = _tid_x / nfeatures;
                int dim_id = _tid_x - nfeatures*center_id;

                int new_center_ids[THREADS_PER_BLOCK];

                new_center_ids[_tid_x] = index;

                /***
                determine which dimension calculte the sum for
                mapping of threads is
                center0[dim0,dim1,dim2,...]center1[dim0,dim1,dim2,...]...
                ***/

                int new_base_index = (point_id - _tid_x)*nfeatures + dim_id;
                float accumulator = 0.f;

                if(_tid_x < nfeatures * nclusters) {
                // accumulate over all the elements of this threadblock
                for(int i = 0; i< (THREADS_PER_BLOCK); i++) {
                float val = tex1Dfetch(t_features_flipped,new_base_index+i*nfeatures);
                if(new_center_ids[i] == center_id)
                accumulator += val;
                }

                // now store the sum for this threadblock
                /***
                mapping to global array is
                block0[center0[dim0,dim1,dim2,...]center1[dim0,dim1,dim2,...]...]block1[...]...
                ***/
                block_clusters[(_bid_y*GRID_DIM_X + _bid_x) * nclusters * nfeatures + _tid_x] = accumulator;
                }
                #endif


            }
        }
    }
}
