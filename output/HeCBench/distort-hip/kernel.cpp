#include "kernel.h"

// --- from distort.cu ---
inline float getRadialX(float x, float y, const struct Properties* prop)
{
  x = (x * prop->xscale + prop->xshift);
  y = (y * prop->yscale + prop->yshift);
  float result = x + ((x - prop->centerX) * prop->K *
    ((x - prop->centerX) * (x - prop->centerX) + (y - prop->centerY) * (y - prop->centerY)));
  return result;
}

inline float getRadialY(float x, float y, const struct Properties* prop)
{
  x = (x * prop->xscale + prop->xshift);
  y = (y * prop->yscale + prop->yshift);
  float result = y + ((y - prop->centerY) * prop->K * 
    ((x - prop->centerX) * (x - prop->centerX) + (y - prop->centerY) * (y - prop->centerY)));
  return result;
}

inline void sampleImageTest(const uchar3* src, float idx0, float idx1,
                            uchar3& result, const struct Properties* prop)
{
  // out-of-bound check
  if((idx0 < 0) || (idx1 < 0) || (idx0 > prop->height - 1) || (idx1 > prop->width - 1))
  {
    result.x = 0;
    result.y = 0;
    result.z = 0;
    return;
  }

  int idx0_floor = (int)floorf(idx0);
  int idx0_ceil = (int)ceilf(idx0);
  int idx1_floor = (int)floorf(idx1);
  int idx1_ceil = (int)ceilf(idx1);

  uchar3 s1 = src[(idx0_floor * prop->width) + idx1_floor];
  uchar3 s2 = src[(idx0_floor * prop->width) + idx1_ceil];
  uchar3 s3 = src[(idx0_ceil * prop->width) + idx1_ceil];
  uchar3 s4 = src[(idx0_ceil * prop->width) + idx1_floor];

  float x = idx0 - idx0_floor;
  float y = idx1 - idx1_floor;

  result.x = s1.x * (1.f - x) * (1.f - y) + s2.x * (1.f - x) * y + s3.x * x * y + s4.x * x * (1.f - y);
  result.y = s1.y * (1.f - x) * (1.f - y) + s2.y * (1.f - x) * y + s3.y * x * y + s4.y * x * (1.f - y);
  result.z = s1.z * (1.f - x) * (1.f - y) + s2.z * (1.f - x) * y + s3.z * x * y + s4.z * x * (1.f - y);
}
extern "C"

void barrel_distort (
  const uchar3 * src,
        uchar3 * dst,
  const struct Properties * prop)
{
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=prop offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=prop offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int h = _bid_y * BLOCK_DIM_Y + _tid_y;
                    int w = _bid_x * BLOCK_DIM_X + _tid_x;
                    if (w < prop->width && h < prop->height) {
                    float x = getRadialX((float)w, (float)h, prop);
                    float y = getRadialY((float)w, (float)h, prop);
                    uchar3 temp;
                    sampleImageTest(src, y, x, temp, prop);
                    dst[(h * prop->width) + w] = temp;
                    }

                }
            }
        }
    }
 * BLOCK_DIM_Y + _tid_y;
                    int w = _bid_x * BLOCK_DIM_X + _tid_x;
                    if (w < prop->width && h < prop->height) {
                    float x = getRadialX((float)w, (float)h, prop);
                    float y = getRadialY((float)w, (float)h, prop);
                    uchar3 temp;
                    sampleImageTest(src, y, x, temp, prop);
                    dst[(h * prop->width) + w] = temp;
                    }

                }
            }
        }
    }
}
