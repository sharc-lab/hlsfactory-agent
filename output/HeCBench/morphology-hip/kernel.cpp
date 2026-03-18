#include "kernel.h"

// --- from morphology.cu ---
inline unsigned char elementOp(unsigned char lhs, unsigned char rhs)
{
}

void reversedScan(
    const unsigned char*  buffer,
          unsigned char*  opArray,
    const int selSize,
    const int tid)
{
  opArray[tid] = buffer[tid];

  for (int offset = 1; offset < selSize; offset *= 2) {
    if (tid <= selSize - 1 - offset) {
      opArray[tid] = elementOp<opType>(opArray[tid], opArray[tid + offset]);
    }
  }
}

void scan(
    const unsigned char*  buffer,
          unsigned char*  opArray,
    const int selSize,
    const int tid)
{
  opArray[tid] = buffer[tid];

  for (int offset = 1; offset < selSize; offset *= 2) {
    if (tid >= offset) {
      opArray[tid] = elementOp<opType>(opArray[tid], opArray[tid - offset]);
    }
  }
}

void twoWayScan(
    const unsigned char*  buffer,
          unsigned char*  opArray,
    const int selSize,
    const int tid)
{
  opArray[tid] = buffer[tid];
  opArray[tid + selSize] = buffer[tid + selSize];

  for (int offset = 1; offset < selSize; offset *= 2) {
    if (tid >= offset) {
      opArray[tid + selSize - 1] = 
        elementOp<opType>(opArray[tid + selSize - 1], opArray[tid + selSize - 1 - offset]);
    }
    if (tid <= selSize - 1 - offset) {
      opArray[tid] = elementOp<opType>(opArray[tid], opArray[tid + offset]);
    }
  }
}
extern "C"

void vhgw_horiz(
          unsigned char*  dst,
    const unsigned char*  src,
    const int width,
    const int height,
    const int selSize
    )
{
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=selSize
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sMem complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sMem complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    unsigned char sMem[4096];
                    unsigned char* buffer = sMem;
                    unsigned char* opArray = buffer + 2 * selSize;

                    const int tidx = _tid_x + _bid_x * BLOCK_DIM_X;
                    const int tidy = _tid_y + _bid_y * BLOCK_DIM_Y;

                    if (tidx >= width || tidy >= height) return;

                    buffer[_tid_x] = src[tidy * width + tidx];
                    if (tidx + selSize < width) {
                    buffer[_tid_x + selSize] = src[tidy * width + tidx + selSize];
                    }

                    twoWayScan<opType>(buffer, opArray, selSize, _tid_x);

                    if (tidx + selSize/2 < width - selSize/2) {
                    dst[tidy * width + tidx + selSize/2] =
                    elementOp<opType>(opArray[_tid_x], opArray[_tid_x + selSize - 1]);
                    }

                }
            }
        }
    }
}
extern "C"

void vhgw_vert(
          unsigned char*  dst,
    const unsigned char*  src,
    const int width,
    const int height,
    const int selSize)
{
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=selSize
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sMem complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sMem complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    unsigned char sMem[4096];
                    unsigned char* buffer = sMem;
                    unsigned char* opArray = buffer + 2 * selSize;

                    const int tidx = _tid_x + _bid_x * BLOCK_DIM_X;
                    const int tidy = _tid_y + _bid_y * BLOCK_DIM_Y;
                    if (tidy >= height || tidx >= width) {
                    return;
                    }

                    buffer[_tid_y] = src[tidy * width + tidx];
                    if (tidy + selSize < height) {
                    buffer[_tid_y + selSize] = src[(tidy + selSize) * width + tidx];
                    }

                    twoWayScan<opType>(buffer, opArray, selSize, _tid_y);

                    if (tidy + selSize/2 < height - selSize/2) {
                    dst[(tidy + selSize/2) * width + tidx] =
                    elementOp<opType>(opArray[_tid_y], opArray[_tid_y + selSize - 1]);
                    }

                    if (tidy < selSize/2 || tidy >= height - selSize/2) {
                    dst[tidy * width + tidx] = borderValue<opType>();
                    }

                }
            }
        }
    }
}
