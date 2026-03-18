#include "kernel.h"

// --- from main.cu ---
T parallel_prefix_sum(const int n, const int *ind, const T *w)
{

  T sum = 0.0;
  T last;

  int mn =(((n+BLOCK_DIM_X-1)/BLOCK_DIM_X)*BLOCK_DIM_X); //n in multiple of BLOCK_DIM_X
  for (int i=_tid_x; i<mn; i+=BLOCK_DIM_X) {
    //All threads (especially the last one) must always participate
    //in the shfl instruction, otherwise their sum will be undefined.
    //So, the loop stopping condition is based on multiple of n in loop increments,
    //so that all threads enter into the loop and inside we make sure we do not
    //read out of bounds memory checking for the actual size n.

    //check if the thread is valid
    bool valid  = i<n;

    //Notice that the last thread is used to propagate the prefix sum.
    //For all the threads, in the first iteration the last is 0, in the following
    //iterations it is the value at the last thread of the previous iterations.

    //get the value of the last thread
    last = 0;

    //if you are valid read the value from memory, otherwise set your value to 0
    sum = (valid) ? w[ind[i]] : 0.0;

    //do prefix sum (of size warpSize=BLOCK_DIM_X =< 32)
    for (int j=1; j<BLOCK_DIM_X; j*=2) {
      T v = __shfl_up_sync(mask, sum, j, BLOCK_DIM_X);
      if (_tid_x >= j) sum += v;
    }
    //shift by last
    sum += last;
    //notice that no __threadfence or __syncthreads are needed in this implementation
  }
  //get the value of the last thread (to all threads)
  last = 0;

  return last;
}
extern "C"

void jaccard_row_sum(const int n,
                const int * csrPtr,
                const int * csrInd,
                const T * w,
                      T * work)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=csrPtr offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=csrInd offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=w offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=work offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            for (int row=_tid_y+_bid_y*BLOCK_DIM_Y; row<n; row+=GRID_DIM_Y*BLOCK_DIM_Y) {
                            int start = csrPtr[row];
                            int end   = csrPtr[row+1];
                            int length= end-start;
                            //compute row sums
                            if (weighted) {
                            T sum = parallel_prefix_sum(length, csrInd + start, w);
                            if (_tid_x == 0) work[row] = sum;
                            }
                            else {
                            work[row] = (T)length;
                            }
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void jaccard_is(const int n, const int e,
           const int * csrPtr,
           const int * csrInd,
           const T * v,
           const T * work,
                 T * weight_i,
                 T * weight_s)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=e
    #pragma HLS INTERFACE m_axi port=csrPtr offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=csrInd offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=work offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=weight_i offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=weight_s offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            for (int row=_tid_z+_bid_z*BLOCK_DIM_Z; row<n; row+=GRID_DIM_Z*BLOCK_DIM_Z) {
                            for (int j=csrPtr[row]+_tid_y+_bid_y*BLOCK_DIM_Y;
                            j<csrPtr[row+1]; j+=GRID_DIM_Y*BLOCK_DIM_Y) {
                            int col = csrInd[j];
                            //find which row has least elements (and call it reference row)
                            int Ni = csrPtr[row+1] - csrPtr[row];
                            int Nj = csrPtr[col+1] - csrPtr[col];
                            int ref= (Ni < Nj) ? row : col;
                            int cur= (Ni < Nj) ? col : row;

                            //compute new sum weights
                            weight_s[j] = work[row] + work[col];

                            //compute new intersection weights
                            //search for the element with the same column index in the reference row
                            for (int i=csrPtr[ref]+_tid_x+_bid_x*BLOCK_DIM_X; i<csrPtr[ref+1]; i+=GRID_DIM_X*BLOCK_DIM_X) {
                            int match  =-1;
                            int ref_col = csrInd[i];
                            T ref_val = weighted ? v[ref_col] : (T)1.0;

                            //binary search (column indices are sorted within each row)
                            int left = csrPtr[cur];
                            int right= csrPtr[cur+1]-1;
                            while(left <= right){
                            int middle = (left+right)>>1;
                            int cur_col= csrInd[middle];
                            if (cur_col > ref_col) {
                            right=middle-1;
                            }
                            else if (cur_col < ref_col) {
                            left=middle+1;
                            }
                            else {
                            match = middle;
                            break;
                            }
                            }

                            //if the element with the same column index in the reference row has been found
                            if (match != -1){
                            (weight_i[j] += ref_val);
                            }
                            }
                            }
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void jaccard_is_opt(const int n, const int e,
               const int * csrPtr,
               const int * csrInd,
               const T * v,
               const T * work,
                     T * weight_i,
                     T * weight_s)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=e
    #pragma HLS INTERFACE m_axi port=csrPtr offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=csrInd offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=work offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=weight_i offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=weight_s offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            for (int row=_tid_z+_bid_z*BLOCK_DIM_Z; row<n; row+=GRID_DIM_Z*BLOCK_DIM_Z) {
                            for (int j=csrPtr[row]+_tid_y+_bid_y*BLOCK_DIM_Y;
                            j<csrPtr[row+1]; j+=GRID_DIM_Y*BLOCK_DIM_Y) {
                            int col = csrInd[j];
                            //find which row has least elements (and call it reference row)
                            int Ni = csrPtr[row+1] - csrPtr[row];
                            int Nj = csrPtr[col+1] - csrPtr[col];
                            int ref= (Ni < Nj) ? row : col;
                            int cur= (Ni < Nj) ? col : row;

                            //compute new sum weights
                            weight_s[j] = work[row] + work[col];

                            //compute new intersection weights
                            //search for the element with the same column index in the reference row
                            if (_tid_x == 0) {
                            T local_sum = 0;
                            int i_ptr = csrPtr[ref];      // pointer in reference row
                            int j_ptr = csrPtr[cur];        // pointer in current row
                            int ref_end = csrPtr[ref+1];
                            int cur_end = csrPtr[cur+1];

                            // Two-pointer merge for intersection of the two sorted lists
                            while (i_ptr < ref_end && j_ptr < cur_end) {
                            int ref_col = csrInd[i_ptr];
                            int cur_col = csrInd[j_ptr];
                            if (ref_col == cur_col) {
                            T ref_val = weighted ? v[ref_col] : (T)1.0;
                            local_sum += ref_val;
                            i_ptr++;
                            j_ptr++;
                            } else if (ref_col < cur_col) {
                            i_ptr++;
                            } else {
                            j_ptr++;
                            }
                            }
                            // perform a single atomic update per this j index
                            if (local_sum != 0)
                            (weight_i[j] += local_sum);
                            }
                            }
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void jaccard_jw(const int e,
    const T * csrVal,
    const T gamma,
    const T * weight_i,
    const T * weight_s,
          T * weight_j)
{
    #pragma HLS INTERFACE s_axilite port=e
    #pragma HLS INTERFACE m_axi port=csrVal offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=gamma
    #pragma HLS INTERFACE m_axi port=weight_i offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=weight_s offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=weight_j offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            for (int j=_tid_x+_bid_x*BLOCK_DIM_X; j<e; j+=GRID_DIM_X*BLOCK_DIM_X) {
                            T Wi =  weight_i[j];
                            T Ws =  weight_s[j];
                            weight_j[j] = (gamma*csrVal[j])* (Wi/(Ws-Wi));
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void fill(const int e, T* w, const T value)
{
    #pragma HLS INTERFACE s_axilite port=e
    #pragma HLS INTERFACE m_axi port=w offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=value
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            for (int j=_tid_x+_bid_x*BLOCK_DIM_X; j<e; j+=GRID_DIM_X*BLOCK_DIM_X) {
                            // e.g. w[0] is the weight of a non-zeron element when csr_ind[i] equals 0.
                            // So multiple non-zero elements on different rows of a matrix may share
                            // the same weight value
                            w[j] = weighted ? (T)(j+1)/e : value;
                            }

                        }
                    }
                }
            }
        }
    }
}
