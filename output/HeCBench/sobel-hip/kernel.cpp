#include "kernel.h"

// --- from kernels.cu ---
inline uchar4 convert_uchar4(float4 v) {
  uchar4 res;
  res.x = (uchar) ((v.x > 255.f) ? 255.f : (v.x < 0.f ? 0.f : v.x));
  res.y = (uchar) ((v.y > 255.f) ? 255.f : (v.y < 0.f ? 0.f : v.y));
  res.z = (uchar) ((v.z > 255.f) ? 255.f : (v.z < 0.f ? 0.f : v.z));
  res.w = (uchar) ((v.w > 255.f) ? 255.f : (v.w < 0.f ? 0.f : v.w));
  return res;
}
extern "C"

void sobel_filter(const uchar4* inputImage, 
                        uchar4* outputImage, 
                  const uint width,
                  const uint height)
{
    #pragma HLS INTERFACE m_axi port=inputImage offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=outputImage offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    uint x = BLOCK_DIM_X * _bid_x + _tid_x;
                    uint y = BLOCK_DIM_Y * _bid_y + _tid_y;

                    /* Read each texel component and calculate the filtered value using neighbouring texel components */
                    if( x >= 1 && x < (width-1) && y >= 1 && y < height - 1)
                    {
                    int c = x + y * width;
                    float4 i00 = convert_float4(inputImage[c - 1 - width]);
                    float4 i01 = convert_float4(inputImage[c - width]);
                    float4 i02 = convert_float4(inputImage[c + 1 - width]);

                    float4 i10 = convert_float4(inputImage[c - 1]);
                    float4 i12 = convert_float4(inputImage[c + 1]);

                    float4 i20 = convert_float4(inputImage[c - 1 + width]);
                    float4 i21 = convert_float4(inputImage[c + width]);
                    float4 i22 = convert_float4(inputImage[c + 1 + width]);

                    const float4 two = float4(2.f, 2.f, 2.f, 2.f);

                    float4 Gx = i00 + two * i10 + i20 - i02 - two * i12 - i22;

                    float4 Gy = i00 - i20  + two * i01 - two * i21 + i02 - i22;

                    /* taking root of sums of squares of Gx and Gy */
                    outputImage[c] = convert_uchar4(float4(sqrtf(Gx.x*Gx.x + Gy.x*Gy.x)/2.f,
                    sqrtf(Gx.y*Gx.y + Gy.y*Gy.y)/2.f,
                    sqrtf(Gx.z*Gx.z + Gy.z*Gy.z)/2.f,
                    sqrtf(Gx.w*Gx.w + Gy.w*Gy.w)/2.f));
                    }

                }
            }
        }
    }
}
