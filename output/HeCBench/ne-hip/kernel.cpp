#include "kernel.h"

// --- from main.cu ---
inline float3 normalize(const float3 &v)
{
  float invLen = rsqrtf(dot(v, v));
  return v * invLen;
}

inline float3 cross(const float3 &a, const float3 &b)
{
  return float3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

inline float length(const float3 &v)
{
  return sqrtf(dot(v, v));
}

inline float4 normalEstimate(const float3 *points, int idx, int width, int height) 
{
  float3 query_pt = points[idx];
  if (isnan(query_pt.z))
    return float4 (0.f,0.f,0.f,0.f);

  int xIdx = idx % width;
  int yIdx = idx / width;

  // are we at a border? are our neighbor valid points?
  bool west_valid  = (xIdx > 1)        && !isnan (points[idx-1].z) &&     fabsf (points[idx-1].z - query_pt.z) < 200.f;
  bool east_valid  = (xIdx < width-1)  && !isnan (points[idx+1].z) &&     fabsf (points[idx+1].z - query_pt.z) < 200.f;
  bool north_valid = (yIdx > 1)        && !isnan (points[idx-width].z) && fabsf (points[idx-width].z - query_pt.z) < 200.f;
  bool south_valid = (yIdx < height-1) && !isnan (points[idx+width].z) && fabsf (points[idx+width].z - query_pt.z) < 200.f;

  float3 horiz, vert;
  if (west_valid & east_valid)
    horiz = points[idx+1] - points[idx-1];
  if (west_valid & !east_valid)
    horiz = points[idx] - points[idx-1];
  if (!west_valid & east_valid)
    horiz = points[idx+1] - points[idx];
  if (!west_valid & !east_valid)
    return float4 (0.f,0.f,0.f,1.f);

  if (south_valid & north_valid)
    vert = points[idx-width] - points[idx+width];
  if (south_valid & !north_valid)
    vert = points[idx] - points[idx+width];
  if (!south_valid & north_valid)
    vert = points[idx-width] - points[idx];
  if (!south_valid & !north_valid)
    return float4 (0.f,0.f,0.f,1.f);

  float3 normal = cross (horiz, vert);

  float curvature = length (normal);
  curvature = fabsf(horiz.z) > 0.04f || fabsf(vert.z) > 0.04f ||
    !west_valid || !east_valid || !north_valid || !south_valid;

  float3 mc = normalize (normal);
  if ( dot (query_pt, mc) > 0.f )
    mc = mc * -1.f;
  return float4 (mc.x, mc.y, mc.z, curvature);
}
extern "C"

void ne (
  const float3 * points,
        float4 * normal_points,
  const int width,
  const int height,
  const int numPts)
{
    #pragma HLS INTERFACE m_axi port=points offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=normal_points offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=numPts
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int idx = _bid_x * BLOCK_DIM_X + _tid_x;
            if (idx < numPts)
            normal_points[idx] = normalEstimate(points, idx, width, height);

        }
    }
}
