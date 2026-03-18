#include "kernel.h"

// --- from components.cu ---
void storeComponents(float *d_r, float *d_g, float *d_b, float r, float g, float b, int pos)
{
    d_r[pos] = (r/255.0f) - 0.5f;
    d_g[pos] = (g/255.0f) - 0.5f;
    d_b[pos] = (b/255.0f) - 0.5f;
}

void storeComponents(int *d_r, int *d_g, int *d_b, int r, int g, int b, int pos)
{
    d_r[pos] = r - 128;
    d_g[pos] = g - 128;
    d_b[pos] = b - 128;
} 

void storeComponent(float *d_c, float c, int pos)
{
    d_c[pos] = (c/255.0f) - 0.5f;
}

void storeComponent(int *d_c, int c, int pos)
{
    d_c[pos] = c - 128;
}
extern "C"

void c_CopySrcToComponents(T *d_r, T *d_g, T *d_b, 
                                  unsigned char * d_src, 
                                  int pixels)
{
    #pragma HLS INTERFACE m_axi port=d_r offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_g offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_b offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_src offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=pixels
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sData complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sData complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int x  = _tid_x;
            int gX = BLOCK_DIM_X*_bid_x;

            unsigned char sData[THREADS*3];

            /* Copy data to shared mem by 4bytes
            other checks are not necessary, since
            d_src buffer is aligned to sharedDataSize */
            if ( (x*4) < THREADS*3 ) {
            float *s = (float *)d_src;
            float *d = (float *)sData;
            d[x] = s[((gX*3)>>2) + x];
            }

            T r, g, b;

            int offset = x*3;
            r = (T)(sData[offset]);
            g = (T)(sData[offset+1]);
            b = (T)(sData[offset+2]);

            int globalOutputPosition = gX + x;
            if (globalOutputPosition < pixels) {
            storeComponents(d_r, d_g, d_b, r, g, b, globalOutputPosition);
            }

        }
    }
}
extern "C"

void c_CopySrcToComponent(T *d_c, unsigned char * d_src, int pixels)
{
    #pragma HLS INTERFACE m_axi port=d_c offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_src offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=pixels
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sData complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sData complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int x  = _tid_x;
            int gX = BLOCK_DIM_X*_bid_x;

            unsigned char sData[THREADS];

            /* Copy data to shared mem by 4bytes
            other checks are not necessary, since
            d_src buffer is aligned to sharedDataSize */
            if ( (x*4) < THREADS) {
            float *s = (float *)d_src;
            float *d = (float *)sData;
            d[x] = s[(gX>>2) + x];
            }

            T c;

            c = (T)(sData[x]);

            int globalOutputPosition = gX + x;
            if (globalOutputPosition < pixels) {
            storeComponent(d_c, c, globalOutputPosition);
            }

        }
    }
}
