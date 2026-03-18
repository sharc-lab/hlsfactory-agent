#include "kernel.h"

// --- from noise.cu ---
inline float lerp(float a, float b, float t)
{
  return a + t*(b-a);
}

inline float dot(float2 a, float2 b)
{
  return a.x * b.x + a.y * b.y;
}

__inline__ float smooth(float t) {
  return t * t * t * (t * (t * 6.f - 15.f) + 10.f);
}

float noiseAt(float x, float y, int seed) {

  // Get top-left corner indices
  const int ix = static_cast<int>(x),
            iy = static_cast<int>(y);

  // Weights
  const float wx = x - ix,
              wy = y - iy;

  // Get gradients at cell corners
  const int ix0 = ix & 255, iy0 = iy & 255;
  const int ix1 = (ix0 + 1) & 255, iy1 = (iy0 + 1) & 255;
  const int h0 = _hash[ix0], h1 = _hash[ix1];
  const int iTL = (_hash[h0 + iy0] + seed) % N_GRADIENTS,
            iTR = (_hash[h1 + iy0] + seed) % N_GRADIENTS,
            iBL = (_hash[h0 + iy1] + seed) % N_GRADIENTS,
            iBR = (_hash[h1 + iy1] + seed) % N_GRADIENTS;
  const float2 gTopLeft  = float2(gradientX[iTL], gradientY[iTL]);
  const float2 gTopRight = float2(gradientX[iTR], gradientY[iTR]);
  const float2 gBotLeft  = float2(gradientX[iBL], gradientY[iBL]);
  const float2 gBotRight = float2(gradientX[iBR], gradientY[iBR]);

  // Calculate dots between distance and gradient vectors
  const float dTopLeft  = dot(gTopLeft,  float2(wx,     wy));
  const float dTopRight = dot(gTopRight, float2(wx - 1, wy));
  const float dBotLeft  = dot(gBotLeft,  float2(wx,     wy - 1));
  const float dBotRight = dot(gBotRight, float2(wx - 1, wy - 1));

  // Calculate the smoothed distance between given point and the top-left corner
  const float tx = smooth(wx), ty = smooth(wy);

  // Interpolate with the other corners
  const float leftInterp  = lerp(dTopLeft, dBotLeft, ty);
  const float rightInterp = lerp(dTopRight, dBotRight, ty);

  return (lerp(leftInterp, rightInterp, tx) + 1.0) * 0.5;
}

float sumOctaves(float x, float y, NoiseParams params) {
  float frequency = 1;
  float sum = noiseAt(x * frequency , y * frequency, params.seed);
  float amplitude = 1;
  float range = 1;
  for (int i = 1; i < params.octaves; i++) {
    frequency *= params.lacunarity;
    amplitude *= params.persistence;
    range += amplitude;
    sum += amplitude * noiseAt(x * frequency, y * frequency, params.seed);
  }
  return sum / range;
}
extern "C"

void perlin(int yStart, int height, NoiseParams params, uint8_t *outPixels) {
    #pragma HLS INTERFACE s_axilite port=yStart
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=params
    #pragma HLS INTERFACE m_axi port=outPixels offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

  // Pixel coordinates
  const auto px = CUID(x);
  const auto py = CUID(y) + yStart;

  if (px >= WIN_WIDTH || py >= yStart + height) return;

  auto noise = sumOctaves(px / params.ppu, py / params.ppu, params);

  // Convert noise to pixel
  const auto baseIdx = 4 * LIN(px, py, WIN_WIDTH);

  const auto val = noise * 255;

  outPixels[baseIdx + 0] = val;
  outPixels[baseIdx + 1] = val;
  outPixels[baseIdx + 2] = val;
  outPixels[baseIdx + 3] = 255;
}
