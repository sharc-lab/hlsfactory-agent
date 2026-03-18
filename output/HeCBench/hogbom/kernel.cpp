#include "kernel.h"

// --- from kernels.cu ---
  Position(int _x, int _y) : x(_x), y(_y) { };

static Position idxToPos(const size_t idx, const int width)
{
  const int y = idx / width;
  const int x = idx % width;
  return Position(x, y);
}

static size_t posToIdx(const int width, const Position& pos)
{
  return (pos.y * width) + pos.x;
}
extern "C"

void k_findPeak(
  const float * image, 
  const size_t size,
  Peak * absPeak)
{
    #pragma HLS INTERFACE m_axi port=image offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE m_axi port=absPeak offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=maxVal complete dim=1
    #pragma HLS ARRAY_PARTITION variable=maxPos complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float maxVal[findPeakWidth];
                    size_t maxPos[findPeakWidth];
                    float val = 0.f;
                    size_t pos = 0;

                    for (size_t idx = _tid_x + _bid_x * BLOCK_DIM_X;
                    idx < size; idx += findPeakWidth * findPeakNBlocks) {
                    if (fabsf(image[idx]) > fabsf(val)) {
                    val = image[idx];
                    pos = idx;
                    }
                    }
                    maxVal[_tid_x] = val;
                    maxPos[_tid_x] = pos;

                    if (_tid_x == 0) {
                    val = 0.f, pos = 0;
                    for (int i = 0; i < findPeakWidth; ++i) {
                    if (fabsf(maxVal[i]) > fabsf(val)) {
                    val = maxVal[i];
                    pos = maxPos[i];
                    }
                    }
                    absPeak[_bid_x].val = val;
                    absPeak[_bid_x].pos = pos;
                    }

                }
            }
        }
    }
}
extern "C"

void k_subtractPSF(
    const float * d_psf,
    const int psfWidth,
          float * d_residual,
    const int residualWidth,
    const int startx, const int starty,
    const int stopx, const int stopy,
    const int diffx, const int diffy,
    const float absPeakVal, const float gain)
{
    #pragma HLS INTERFACE m_axi port=d_psf offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=psfWidth
    #pragma HLS INTERFACE m_axi port=d_residual offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=residualWidth
    #pragma HLS INTERFACE s_axilite port=startx
    #pragma HLS INTERFACE s_axilite port=starty
    #pragma HLS INTERFACE s_axilite port=stopx
    #pragma HLS INTERFACE s_axilite port=stopy
    #pragma HLS INTERFACE s_axilite port=diffx
    #pragma HLS INTERFACE s_axilite port=diffy
    #pragma HLS INTERFACE s_axilite port=absPeakVal
    #pragma HLS INTERFACE s_axilite port=gain
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const int x = startx + _tid_x + _bid_x * BLOCK_DIM_X;
                    const int y = starty + _tid_y + _bid_y * BLOCK_DIM_Y;

                    // thread blocks are of size 16, but the workload is not always a multiple of 16
                    if (x <= stopx && y <= stopy) {
                    d_residual[posToIdx(residualWidth, Position(x, y))] -= gain * absPeakVal
                    * d_psf[posToIdx(psfWidth, Position(x - diffx, y - diffy))];
                    }

                }
            }
        }
    }
}
