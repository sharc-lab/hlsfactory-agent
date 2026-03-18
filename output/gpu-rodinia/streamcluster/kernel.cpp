#include "kernel.h"

// --- from streamcluster_cuda.cu ---
float d_dist(int p1, int p2, int num, int dim, float *coord_d)
{
	float retval = 0.0;
	for(int i = 0; i < dim; i++){
		float tmp = coord_d[(i*num)+p1] - coord_d[(i*num)+p2];
		retval += tmp * tmp;
	}
	return retval;
}
extern "C"

void kernel_compute_cost(int num, int dim, long x, Point *p, int K, int stride,
					float *coord_d, float *work_mem_d, int *center_table_d, bool *switch_membership_d)
{
    #pragma HLS INTERFACE s_axilite port=num
    #pragma HLS INTERFACE s_axilite port=dim
    #pragma HLS INTERFACE s_axilite port=x
    #pragma HLS INTERFACE m_axi port=p offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=K
    #pragma HLS INTERFACE s_axilite port=stride
    #pragma HLS INTERFACE m_axi port=coord_d offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=work_mem_d offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=center_table_d offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=switch_membership_d offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                // block ID and global thread ID
                const int bid  = _bid_x + GRID_DIM_X * _bid_y;
                const int tid = BLOCK_DIM_X * bid + _tid_x;

                if(tid < num)
                {
                float *lower = &work_mem_d[tid*stride];

                // cost between this point and point[x]: euclidean distance multiplied by weight
                float x_cost = d_dist(tid, x, num, dim, coord_d) * p[tid].weight;

                // if computed cost is less then original (it saves), mark it as to reassign
                if ( x_cost < p[tid].cost )
                {
                switch_membership_d[tid] = 1;
                lower[K] += x_cost - p[tid].cost;
                }
                // if computed cost is larger, save the difference
                else
                {
                lower[center_table_d[p[tid].assign]] += p[tid].cost - x_cost;
                }
                }

            }
        }
    }
}
