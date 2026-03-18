#include "kernel.h"

// --- from main.cu ---
float4 rgbaUintToFloat4(const unsigned int uiPackedRGBA)
{
    float4 rgba;
    rgba.x = uiPackedRGBA & 0xff;
    rgba.y = (uiPackedRGBA >> 8) & 0xff;
    rgba.z = (uiPackedRGBA >> 16) & 0xff;
    rgba.w = (uiPackedRGBA >> 24) & 0xff;
    return rgba;
}

unsigned int rgbaFloat4ToUint(const float4 rgba)
{
    unsigned int uiPackedRGBA = 0U;
    uiPackedRGBA |= 0x000000FF & (unsigned int)rgba.x;
    uiPackedRGBA |= 0x0000FF00 & (((unsigned int)rgba.y) << 8);
    uiPackedRGBA |= 0x00FF0000 & (((unsigned int)rgba.z) << 16);
    uiPackedRGBA |= 0xFF000000 & (((unsigned int)rgba.w) << 24);
    return uiPackedRGBA;
}
extern "C"

void Transpose(const unsigned int* uiDataIn, 
                     unsigned int* uiDataOut, 
               const size_t iWidth, const size_t iHeight)
{
    #pragma HLS INTERFACE m_axi port=uiDataIn offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=uiDataOut offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=iWidth
    #pragma HLS INTERFACE s_axilite port=iHeight
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=uiLocalBuff complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // read the matrix tile into LMEM
                    size_t xIndex = _bid_x * BLOCK_DIM_X + _tid_x;
                    size_t yIndex = _bid_y * BLOCK_DIM_Y + _tid_y;
                    unsigned int uiLocalBuff[16*17];

                    if((xIndex < iWidth) && (yIndex < iHeight))
                    {
                    //uiLocalBuff[get_local_id(1) * (get_local_size(0) + 1) + get_local_id(0)] = uiDataIn[(yIndex * iWidth) + xIndex];
                    uiLocalBuff[_tid_y * (BLOCK_DIM_X + 1) + _tid_x] = uiDataIn[(yIndex * iWidth) + xIndex];
                    }

                    // Synchronize the read into LMEM

                    // write the transposed matrix tile to global memory
                    xIndex = _bid_y * BLOCK_DIM_Y + _tid_x;
                    yIndex = _bid_x * BLOCK_DIM_X + _tid_y;
                    if((xIndex < iHeight) && (yIndex < iWidth))
                    {
                    uiDataOut[(yIndex * iHeight) + xIndex] =
                    //uiLocalBuff[get_local_id(0) * (get_local_size(1) + 1) + get_local_id(1)];
                    uiLocalBuff[_tid_x * (BLOCK_DIM_Y + 1) + _tid_y];
                    }

                }
            }
        }
    }
}
extern "C"

void SimpleRecursiveRGBA(
  const unsigned int* uiDataIn,
        unsigned int* uiDataOut,
  const size_t iWidth, const size_t iHeight, const float a)
{
    #pragma HLS INTERFACE m_axi port=uiDataIn offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=uiDataOut offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=iWidth
    #pragma HLS INTERFACE s_axilite port=iHeight
    #pragma HLS INTERFACE s_axilite port=a
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // compute X pixel location and check in-bounds
                    size_t X = _bid_x * BLOCK_DIM_X + _tid_x;
                    if (X >= iWidth) return;

                    // advance global pointers to correct column for this work item and x position
                    uiDataIn += X;
                    uiDataOut += X;

                    // start forward filter pass
                    float4 yp = rgbaUintToFloat4(*uiDataIn);  // previous output
                    for (int Y = 0; Y < iHeight; Y++)
                    {
                    float4 xc = rgbaUintToFloat4(*uiDataIn);
                    float4 yc = xc + (yp - xc) * float4(a, a, a, a);
                    *uiDataOut = rgbaFloat4ToUint(yc);
                    yp = yc;
                    uiDataIn += iWidth;     // move to next row
                    uiDataOut += iWidth;    // move to next row
                    }

                    // reset global pointers to point to last element in column for this work item and x position
                    uiDataIn -= iWidth;
                    uiDataOut -= iWidth;

                    // start reverse filter pass: ensures response is symmetrical
                    yp = rgbaUintToFloat4(*uiDataIn);
                    for (int Y = iHeight - 1; Y > -1; Y--)
                    {
                    float4 xc = rgbaUintToFloat4(*uiDataIn);
                    float4 yc = xc + (yp - xc) * float4(a, a, a, a);
                    *uiDataOut = rgbaFloat4ToUint((rgbaUintToFloat4(*uiDataOut) + yc) * 0.5f);
                    yp = yc;
                    uiDataIn -= iWidth;   // move to previous row
                    uiDataOut -= iWidth;  // move to previous row
                    }

                }
            }
        }
    }
}
extern "C"

void RecursiveRGBA(
  const unsigned int* uiDataIn, 
  unsigned int* uiDataOut, 
  const size_t iWidth, const size_t iHeight, 
  const float a0, const float a1, 
  const float a2, const float a3, 
  const float b1, const float b2, 
  const float coefp, const float coefn)
{
    #pragma HLS INTERFACE m_axi port=uiDataIn offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=uiDataOut offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=iWidth
    #pragma HLS INTERFACE s_axilite port=iHeight
    #pragma HLS INTERFACE s_axilite port=a0
    #pragma HLS INTERFACE s_axilite port=a1
    #pragma HLS INTERFACE s_axilite port=a2
    #pragma HLS INTERFACE s_axilite port=a3
    #pragma HLS INTERFACE s_axilite port=b1
    #pragma HLS INTERFACE s_axilite port=b2
    #pragma HLS INTERFACE s_axilite port=coefp
    #pragma HLS INTERFACE s_axilite port=coefn
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // compute X pixel location and check in-bounds
                    size_t X = _bid_x * BLOCK_DIM_X + _tid_x;
                    if (X >= iWidth) return;

                    // advance global pointers to correct column for this work item and x position
                    uiDataIn += X;
                    uiDataOut += X;

                    // start forward filter pass
                    float4 xp = float4(0.0f, 0.0f, 0.0f, 0.0f);  // previous input
                    float4 yp = float4(0.0f, 0.0f, 0.0f, 0.0f);  // previous output
                    float4 yb = float4(0.0f, 0.0f, 0.0f, 0.0f);  // previous output by 2

                    #ifdef CLAMP_TO_EDGE
                    xp = rgbaUintToFloat4(*uiDataIn);
                    yb = xp * float4(coefp,coefp,coefp,coefp);
                    yp = yb;
                    #endif

                    for (int Y = 0; Y < iHeight; Y++)
                    {
                    float4 xc = rgbaUintToFloat4(*uiDataIn);
                    float4 yc = (xc * a0) + (xp * a1) - (yp * b1) - (yb * b2);
                    *uiDataOut = rgbaFloat4ToUint(yc);
                    xp = xc;
                    yb = yp;
                    yp = yc;
                    uiDataIn += iWidth;     // move to next row
                    uiDataOut += iWidth;    // move to next row
                    }

                    // reset global pointers to point to last element in column for this work item and x position
                    uiDataIn -= iWidth;
                    uiDataOut -= iWidth;

                    // start reverse filter pass: ensures response is symmetrical
                    float4 xn = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    float4 xa = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    float4 yn = float4(0.0f, 0.0f, 0.0f, 0.0f);
                    float4 ya = float4(0.0f, 0.0f, 0.0f, 0.0f);

                    #ifdef CLAMP_TO_EDGE
                    xn = rgbaUintToFloat4(*uiDataIn);
                    xa = xn;
                    yn = xn * float4(coefn,coefn,coefn,coefn);
                    ya = yn;
                    #endif

                    for (int Y = iHeight - 1; Y > -1; Y--)
                    {
                    float4 xc = rgbaUintToFloat4(*uiDataIn);
                    float4 yc = (xn * a2) + (xa * a3) - (yn * b1) - (ya * b2);
                    xa = xn;
                    xn = xc;
                    ya = yn;
                    yn = yc;
                    *uiDataOut = rgbaFloat4ToUint(rgbaUintToFloat4(*uiDataOut) + yc);
                    uiDataIn -= iWidth;   // move to previous row
                    uiDataOut -= iWidth;  // move to previous row
                    }

                }
            }
        }
    }
}
