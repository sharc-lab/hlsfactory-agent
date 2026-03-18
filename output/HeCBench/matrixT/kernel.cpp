#include "kernel.h"

// --- from main.cu ---
extern "C"
void copy(
        float * odata,
  const float * idata,
  int width, int height)
{
    #pragma HLS INTERFACE m_axi port=odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int xIndex = _bid_x * TILE_DIM + _tid_x;
                    int yIndex = _bid_y * TILE_DIM + _tid_y;

                    int index  = xIndex + width*yIndex;

                    for (int i=0; i<TILE_DIM; i+=BLOCK_ROWS)
                    {
                    odata[index+i*width] = idata[index+i*width];
                    }

                }
            }
        }
    }
}
extern "C"

void copySharedMem(
        float * odata,
  const float * idata,
  int width, int height)
{
    #pragma HLS INTERFACE m_axi port=odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // Handle to thread block group

                    float tile[TILE_DIM][TILE_DIM];

                    int xIndex = _bid_x * TILE_DIM + _tid_x;
                    int yIndex = _bid_y * TILE_DIM + _tid_y;

                    int index  = xIndex + width*yIndex;

                    for (int i=0; i<TILE_DIM; i+=BLOCK_ROWS)
                    {
                    if (xIndex < width && yIndex < height)
                    {
                    tile[_tid_y][_tid_x] = idata[index];
                    }
                    }

                    for (int i=0; i<TILE_DIM; i+=BLOCK_ROWS)
                    {
                    if (xIndex < height && yIndex < width)
                    {
                    odata[index] = tile[_tid_y][_tid_x];
                    }
                    }

                }
            }
        }
    }
}
extern "C"

void transposeNaive(
        float * odata,
  const float * idata,
  int width, int height)
{
    #pragma HLS INTERFACE m_axi port=odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int xIndex = _bid_x * TILE_DIM + _tid_x;
                    int yIndex = _bid_y * TILE_DIM + _tid_y;

                    int index_in  = xIndex + width * yIndex;
                    int index_out = yIndex + height * xIndex;

                    for (int i=0; i<TILE_DIM; i+=BLOCK_ROWS)
                    {
                    odata[index_out+i] = idata[index_in+i*width];
                    }

                }
            }
        }
    }
}
extern "C"

void transposeCoalesced(
        float * odata,
  const float * idata,
  int width, int height)
{
    #pragma HLS INTERFACE m_axi port=odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // Handle to thread block group

                    float tile[TILE_DIM][TILE_DIM];

                    int xIndex = _bid_x * TILE_DIM + _tid_x;
                    int yIndex = _bid_y * TILE_DIM + _tid_y;
                    int index_in = xIndex + (yIndex)*width;

                    xIndex = _bid_y * TILE_DIM + _tid_x;
                    yIndex = _bid_x * TILE_DIM + _tid_y;
                    int index_out = xIndex + (yIndex)*height;

                    for (int i=0; i<TILE_DIM; i+=BLOCK_ROWS)
                    {
                    tile[_tid_y+i][_tid_x] = idata[index_in+i*width];
                    }

                    for (int i=0; i<TILE_DIM; i+=BLOCK_ROWS)
                    {
                    odata[index_out+i*height] = tile[_tid_x][_tid_y+i];
                    }

                }
            }
        }
    }
}
extern "C"

void transposeNoBankConflicts(
        float * odata,
  const float * idata,
  int width, int height)
{
    #pragma HLS INTERFACE m_axi port=odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // Handle to thread block group

                    float tile[TILE_DIM][TILE_DIM+1];

                    int xIndex = _bid_x * TILE_DIM + _tid_x;
                    int yIndex = _bid_y * TILE_DIM + _tid_y;
                    int index_in = xIndex + (yIndex)*width;

                    xIndex = _bid_y * TILE_DIM + _tid_x;
                    yIndex = _bid_x * TILE_DIM + _tid_y;
                    int index_out = xIndex + (yIndex)*height;

                    for (int i=0; i<TILE_DIM; i+=BLOCK_ROWS)
                    {
                    tile[_tid_y+i][_tid_x] = idata[index_in+i*width];
                    }

                    for (int i=0; i<TILE_DIM; i+=BLOCK_ROWS)
                    {
                    odata[index_out+i*height] = tile[_tid_x][_tid_y+i];
                    }

                }
            }
        }
    }
}
extern "C"

void transposeDiagonal(
        float * odata,
  const float * idata,
  int width, int height)
{
    #pragma HLS INTERFACE m_axi port=odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // Handle to thread block group

                    float tile[TILE_DIM][TILE_DIM+1];

                    int blockIdx_x, blockIdx_y;

                    // do diagonal reordering
                    if (width == height)
                    {
                    blockIdx_y = _bid_x;
                    blockIdx_x = (_bid_x+_bid_y)%GRID_DIM_X;
                    }
                    else
                    {
                    int bid = _bid_x + GRID_DIM_X*_bid_y;
                    blockIdx_y = bid%GRID_DIM_Y;
                    blockIdx_x = ((bid/GRID_DIM_Y)+blockIdx_y)%GRID_DIM_X;
                    }

                    // from here on the code is same as previous kernel except blockIdx_x replaces _bid_x
                    // and similarly for y

                    int xIndex = blockIdx_x * TILE_DIM + _tid_x;
                    int yIndex = blockIdx_y * TILE_DIM + _tid_y;
                    int index_in = xIndex + (yIndex)*width;

                    xIndex = blockIdx_y * TILE_DIM + _tid_x;
                    yIndex = blockIdx_x * TILE_DIM + _tid_y;
                    int index_out = xIndex + (yIndex)*height;

                    for (int i=0; i<TILE_DIM; i+=BLOCK_ROWS)
                    {
                    tile[_tid_y+i][_tid_x] = idata[index_in+i*width];
                    }

                    for (int i=0; i<TILE_DIM; i+=BLOCK_ROWS)
                    {
                    odata[index_out+i*height] = tile[_tid_x][_tid_y+i];
                    }

                }
            }
        }
    }
}
extern "C"

void transposeFineGrained(
        float * odata,
  const float * idata,
  int width, int height)
{
    #pragma HLS INTERFACE m_axi port=odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // Handle to thread block group

                    float block[TILE_DIM][TILE_DIM+1];

                    int xIndex = _bid_x * TILE_DIM + _tid_x;
                    int yIndex = _bid_y * TILE_DIM + _tid_y;
                    int index = xIndex + (yIndex)*width;

                    for (int i=0; i < TILE_DIM; i += BLOCK_ROWS)
                    {
                    block[_tid_y+i][_tid_x] = idata[index+i*width];
                    }

                    for (int i=0; i < TILE_DIM; i += BLOCK_ROWS)
                    {
                    odata[index+i*height] = block[_tid_x][_tid_y+i];
                    }

                }
            }
        }
    }
}
extern "C"

void transposeCoarseGrained(
        float * odata,
  const float * idata,
  int width, int height)
{
    #pragma HLS INTERFACE m_axi port=odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // Handle to thread block group

                    float block[TILE_DIM][TILE_DIM+1];

                    int xIndex = _bid_x * TILE_DIM + _tid_x;
                    int yIndex = _bid_y * TILE_DIM + _tid_y;
                    int index_in = xIndex + (yIndex)*width;

                    xIndex = _bid_y * TILE_DIM + _tid_x;
                    yIndex = _bid_x * TILE_DIM + _tid_y;
                    int index_out = xIndex + (yIndex)*height;

                    for (int i=0; i<TILE_DIM; i += BLOCK_ROWS)
                    {
                    block[_tid_y+i][_tid_x] = idata[index_in+i*width];
                    }

                    for (int i=0; i<TILE_DIM; i += BLOCK_ROWS)
                    {
                    odata[index_out+i*height] = block[_tid_y+i][_tid_x];
                    }

                }
            }
        }
    }
}
