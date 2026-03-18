#include "kernel.h"

// --- from kernels.cu ---
extern "C"
void degrid_kernel(CmplxType* __restrict out, 
              const CmplxType* __restrict in, 
              const size_t npts,
              const CmplxType* __restrict img, 
              const size_t img_dim,
              const CmplxType* __restrict gcf)
{
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=npts
    #pragma HLS INTERFACE m_axi port=img offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=img_dim
    #pragma HLS INTERFACE m_axi port=gcf offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                const int blockIdx_x = _bid_x;
                const int blockDim_x = BLOCK_DIM_X;
                const int threadIdx_x = _tid_x;
                const int gridDim_x = GRID_DIM_X;
                const int blockDim_y = BLOCK_DIM_Y;
                const int threadIdx_y = _tid_y;

                for (int n = 32*blockIdx_x; n < npts; n += 32*gridDim_x) {
                for (int q = threadIdx_y; q < 32; q += blockDim_y) {
                CmplxType inn = in[n+q];
                const int sub_x = floorf(GCF_GRID*(inn.x-floorf(inn.x)));
                const int sub_y = floorf(GCF_GRID*(inn.y-floorf(inn.y)));
                const int main_x = floorf(inn.x);
                const int main_y = floorf(inn.y);
                CmplxType sum = {0,0};
                for(int a = threadIdx_x-GCF_DIM/2; a < GCF_DIM/2; a += blockDim_x)
                for(int b = -GCF_DIM/2; b < GCF_DIM/2; b++)
                {
                auto r1 = img[main_x+a+img_dim*(main_y+b)].x;
                auto i1 = img[main_x+a+img_dim*(main_y+b)].y;
                if (main_x+a < 0 || main_y+b < 0 ||
                main_x+a >= img_dim  || main_y+b >= img_dim) {
                r1 = i1 = 0;
                }
                auto r2 = gcf[GCF_DIM*GCF_DIM*(GCF_GRID*sub_y+sub_x) + GCF_DIM*b+a].x;
                auto i2 = gcf[GCF_DIM*GCF_DIM*(GCF_GRID*sub_y+sub_x) + GCF_DIM*b+a].y;
                sum.x += r1*r2 - i1*i2;
                sum.y += r1*i2 + r2*i1;
                }

                for(int s = blockDim_x < 16 ? blockDim_x : 16; s>0;s/=2) {
                sum.x += __shfl_down(sum.x,s);
                sum.y += __shfl_down(sum.y,s);
                }
                if (threadIdx_x == 0) {
                out[n+q] = sum;
                }
                }
                }

            }
        }
    }
}
