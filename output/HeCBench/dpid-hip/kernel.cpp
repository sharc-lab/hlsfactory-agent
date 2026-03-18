#include "kernel.h"

// --- from kernels.cu ---
void normalize(float4& var) {
  var.x /= var.w;
  var.y /= var.w;
  var.z /= var.w;
  var.w = 1.f;
}

void add(float4& output, const uchar3& color, const float factor) {
  output.x += color.x * factor;  
  output.y += color.y * factor;  
  output.z += color.z * factor;  
  output.w += factor;
}

float lambda(const Params p, const float dist) {
  if(p.lambda == 0.f)
    return 1.f;
  else if(p.lambda == 1.f)
    return dist;
  return powf(dist, p.lambda);
}

  inline Local(const Params& p) {
    sx      = fmaxf( PX    * p.pWidth, 0.f);
    ex      = fminf((PX+1) * p.pWidth, (float)p.iWidth);
    sy      = fmaxf( PY    * p.pHeight, 0.f);
    ey      = fminf((PY+1) * p.pHeight, (float)p.iHeight);

    sxr      = (uint32_t)floorf(sx);
    syr      = (uint32_t)floorf(sy);
    exr      = (uint32_t)ceilf(ex);
    eyr      = (uint32_t)ceilf(ey);
    xCount    = exr - sxr;
    yCount    = eyr - syr;
    pixelCount  = xCount * yCount;
  }

float contribution(const Local& l, float f, const uint32_t x, const uint32_t y) {
  if(x < l.sx)    f *= 1.f - (l.sx - x);
  if((x+1.f) > l.ex)  f *= 1.f - ((x+1.f) - l.ex);
  if(y < l.sy)    f *= 1.f - (l.sy - y);
  if((y+1.f) > l.ey)  f *= 1.f - ((y+1.f) - l.ey);
  return f;
}

float4 __shfl_down(const float4 var, const uint32_t srcLane, const uint32_t width = 32) {
  float4 output;
  output.x = __shfl_down(var.x, srcLane, width);
  output.y = __shfl_down(var.y, srcLane, width);
  output.z = __shfl_down(var.z, srcLane, width);
  output.w = __shfl_down(var.w, srcLane, width);
  return output;
}

void reduce(float4& value) {
  value += __shfl_down(value, 16);
  value += __shfl_down(value, 8);
  value += __shfl_down(value, 4);
  value += __shfl_down(value, 2);
  value += __shfl_down(value, 1);
}

float distance(const float4& avg, const uchar3& color) {
  const float x = avg.x - color.x;
  const float y = avg.y - color.y;
  const float z = avg.z - color.z;
  return sqrtf(x * x + y * y + z * z) / 441.6729559f; // L2-Norm / sqrt(255^2 * 3)
}
extern "C"

void kernelGuidance(const uchar3*  input,
                          uchar3*  patches, const Params p)
{
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=patches offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=p
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                if(PX >= p.oWidth || PY >= p.oHeight) return;

                // init
                const Local l(p);
                float4 color = float4(0.f, 0.f, 0.f, 0.f);

                // iterate pixels
                for(uint32_t i = WTHREAD; i < l.pixelCount; i += WSIZE) {
                const uint32_t x = l.sxr + (i % l.xCount);
                const uint32_t y = l.syr + (i / l.xCount);

                float f = contribution(l, 1.f, x, y);

                const uchar3& pixel = input[x + y * p.iWidth];
                color += float4(pixel.x * f, pixel.y * f, pixel.z * f, f);
                }

                // reduce warps
                reduce(color);

                // store results
                if((TX % 32) == 0) {
                normalize(color);
                patches[PX + PY * p.oWidth] = make_uchar3(color.x, color.y, color.z);
                }

            }
        }
    }
}

float4 calcAverage(const Params& p, const uchar3*  patches) {
  const float corner = 1.0;
  const float edge   = 2.0;
  const float center = 4.0;

  // calculate average color
  float4 avg = float4(0.f, 0.f, 0.f, 0.f);

  // TOP
  if(PY > 0) {
    if(PX > 0) 
      add(avg, patches[(PX - 1) + (PY - 1) * p.oWidth], corner);

    add(avg, patches[(PX) + (PY - 1) * p.oWidth], edge);

    if((PX+1) < p.oWidth)
      add(avg, patches[(PX + 1) + (PY - 1) * p.oWidth], corner);
  }

  // LEFT
  if(PX > 0) 
    add(avg, patches[(PX - 1) + (PY) * p.oWidth], edge);

  // CENTER
  add(avg, patches[(PX) + (PY) * p.oWidth], center);

  // RIGHT
  if((PX+1) < p.oWidth)
    add(avg, patches[(PX + 1) + (PY) * p.oWidth], edge);

  // BOTTOM
  if((PY+1) < p.oHeight) {
    if(PX > 0) 
      add(avg, patches[(PX - 1) + (PY + 1) * p.oWidth], corner);

    add(avg, patches[(PX) + (PY + 1) * p.oWidth], edge);

    if((PX+1) < p.oWidth)
      add(avg, patches[(PX + 1) + (PY + 1) * p.oWidth], corner);
  }

  normalize(avg);

  return avg;
}
extern "C"

void kernelDownsampling(const uchar3*  input,
                        const uchar3*  patches,
                        const Params p,
                              uchar3*  output)
{
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=patches offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=p
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                if(PX >= p.oWidth || PY >= p.oHeight) return;

                // init
                const Local l(p);
                const float4 avg = calcAverage(p, patches);

                float4 color = float4(0.f, 0.f, 0.f, 0.f);

                // iterate pixels
                for(uint32_t i = WTHREAD; i < l.pixelCount; i += WSIZE) {
                const uint32_t x = l.sxr + (i % l.xCount);
                const uint32_t y = l.syr + (i / l.xCount);

                const uchar3& pixel = input[x + y * p.iWidth];
                float f = distance(avg, pixel);

                f = lambda(p, f);
                f = contribution(l, f, x, y);

                add(color, pixel, f);
                }

                // reduce warp
                reduce(color);

                if(WTHREAD == 0) {
                uchar3& ref = output[PX + PY * p.oWidth];

                if(color.w == 0.0f)
                ref = make_uchar3((unsigned char)avg.x, (unsigned char)avg.y, (unsigned char)avg.z);
                else {
                normalize(color);
                ref = make_uchar3((unsigned char)color.x, (unsigned char)color.y, (unsigned char)color.z);
                }
                }

            }
        }
    }
}
