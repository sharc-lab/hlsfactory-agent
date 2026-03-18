#include "kernel.h"

// --- from sobol_gpu.cu ---
extern "C"
void sobolGPU_kernel(unsigned n_vectors, unsigned n_dimensions,
                     unsigned * d_directions,
                     float * d_output)
{
    #pragma HLS INTERFACE s_axilite port=n_vectors
    #pragma HLS INTERFACE s_axilite port=n_dimensions
    #pragma HLS INTERFACE m_axi port=d_directions offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_output offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=v complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                // Handle to thread block group

                unsigned int v[n_directions];

                // Offset into the correct dimension as specified by the
                // block y coordinate
                d_directions += n_directions * _bid_y;
                d_output += n_vectors * _bid_y;

                // Copy the direction numbers for this dimension into shared
                // memory - there are only 32 direction numbers so only the
                // first 32 (n_directions) threads need participate.
                if (_tid_x < n_directions)
                {
                v[_tid_x] = d_directions[_tid_x];
                }

                // Set initial index (i.e. which vector this thread is
                // computing first) and stride (i.e. step to the next vector
                // for this thread)
                int i0     = _tid_x + _bid_x * BLOCK_DIM_X;
                int stride = GRID_DIM_X * BLOCK_DIM_X;

                // Get the gray code of the index
                // c.f. Numerical Recipes in C, chapter 20
                // http://www.nrbook.com/a/bookcpdf/c20-2.pdf
                unsigned int g = i0 ^ (i0 >> 1);

                // Initialisation for first point x[i0]
                // In the Bratley and Fox paper this is equation (*), where
                // we are computing the value for x[n] without knowing the
                // value of x[n-1].
                unsigned int X = 0;
                unsigned int mask;

                for (unsigned int k = 0 ; k < __ffs(stride) - 1 ; k++)
                {
                // We want X ^= g_k * v[k], where g_k is one or zero.
                // We do this by setting a mask with all bits equal to
                // g_k. In reality we keep shifting g so that g_k is the
                // LSB of g. This way we avoid multiplication.
                mask = - (g & 1);
                X ^= mask & v[k];
                g = g >> 1;
                }

                if (i0 < n_vectors)
                {
                d_output[i0] = (float)X * k_2powneg32;
                }

                // Now do rest of points, using the stride
                // Here we want to generate x[i] from x[i-stride] where we
                // don't have any of the x in between, therefore we have to
                // revisit the equation (**), this is easiest with an example
                // so assume stride is 16.
                // From x[n] to x[n+16] there will be:
                //   8 changes in the first bit
                //   4 changes in the second bit
                //   2 changes in the third bit
                //   1 change in the fourth
                //   1 change in one of the remaining bits
                //
                // What this means is that in the equation:
                //   x[n+1] = x[n] ^ v[p]
                //   x[n+2] = x[n+1] ^ v[q] = x[n] ^ v[p] ^ v[q]
                //   ...
                // We will apply xor with v[1] eight times, v[2] four times,
                // v[3] twice, v[4] once and one other direction number once.
                // Since two xors cancel out, we can skip even applications
                // and just apply xor with v[4] (i.e. log2(16)) and with
                // the current applicable direction number.
                // Note that all these indices count from 1, so we need to
                // subtract 1 from them all to account for C arrays counting
                // from zero.
                unsigned int v_log2stridem1 = v[__ffs(stride) - 2];
                unsigned int v_stridemask = stride - 1;

                for (unsigned int i = i0 + stride ; i < n_vectors ; i += stride)
                {
                // x[i] = x[i-stride] ^ v[b] ^ v[c]
                //  where b is log2(stride) minus 1 for C array indexing
                //  where c is the index of the rightmost zero bit in i,
                //  not including the bottom log2(stride) bits, minus 1
                //  for C array indexing
                // In the Bratley and Fox paper this is equation (**)
                X ^= v_log2stridem1 ^ v[__ffs(~((i - stride) | v_stridemask)) - 1];
                d_output[i] = (float)X * k_2powneg32;
                }

            }
        }
    }
}
