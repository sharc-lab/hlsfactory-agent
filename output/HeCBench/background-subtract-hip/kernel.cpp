#include "kernel.h"

// --- from main.cu ---
extern "C"
void findMovingPixels(
  const size_t imgSize,
  const unsigned char * Img,
  const unsigned char * Img1,
  const unsigned char * Img2,
  const unsigned char * Tn,
        unsigned char * Mp) // moving pixel map
{
  size_t i = BLOCK_DIM_X * _bid_x + _tid_x;
  if (i >= imgSize) return;
  if ( abs(Img[i] - Img1[i]) > Tn[i] || abs(Img[i] - Img2[i]) > Tn[i] )
    Mp[i] = 255;
  else {
    Mp[i] = 0;
  }
}
extern "C"

void updateBackground(
  const size_t imgSize,
  const unsigned char * Img,
  const unsigned char * Mp,
        unsigned char * Bn)
{
    #pragma HLS INTERFACE s_axilite port=imgSize
    #pragma HLS INTERFACE m_axi port=Img offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Mp offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=Bn offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t i = BLOCK_DIM_X * _bid_x + _tid_x;
            if (i >= imgSize) return;
            if ( Mp[i] == 0 ) Bn[i] = 0.92 * Bn[i] + 0.08 * Img[i];

        }
    }
}
extern "C"

void updateThreshold(
  const size_t imgSize,
  const unsigned char * Img,
  const unsigned char * Mp,
  const unsigned char * Bn,
        unsigned char * Tn)
{
    #pragma HLS INTERFACE s_axilite port=imgSize
    #pragma HLS INTERFACE m_axi port=Img offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Mp offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=Bn offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=Tn offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t i = BLOCK_DIM_X * _bid_x + _tid_x;
            if (i >= imgSize) return;
            if (Mp[i] == 0) {
            float th = 0.92 * Tn[i] + 0.24 * (Img[i] - Bn[i]);
            Tn[i] = fmaxf(th, 20.f);
            }

        }
    }
}
extern "C"

void merge(
  const size_t imgSize,
  const unsigned char * Img,
  const unsigned char * Img1,
  const unsigned char * Img2,
        unsigned char * Tn,
        unsigned char * Bn)
{
    #pragma HLS INTERFACE s_axilite port=imgSize
    #pragma HLS INTERFACE m_axi port=Img offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Img1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=Img2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=Tn offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=Bn offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t i = BLOCK_DIM_X * _bid_x + _tid_x;
            if (i >= imgSize) return;
            if ( abs(Img[i] - Img1[i]) <= Tn[i] && abs(Img[i] - Img2[i]) <= Tn[i] ) {
            // update background
            Bn[i] = 0.92 * Bn[i] + 0.08 * Img[i];

            // update threshold
            float th = 0.92 * Tn[i] + 0.24 * (Img[i] - Bn[i]);
            Tn[i] = fmaxf(th, 20.f);
            }

        }
    }
}
