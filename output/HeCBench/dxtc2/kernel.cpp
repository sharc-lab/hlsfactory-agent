#include "kernel.h"

// --- from main.cu ---
inline float3 firstEigenVector(float matrix[6]) {
  // 8 iterations seems to be more than enough.

  float3 v = float3(1.0f, 1.0f, 1.0f);

  for (int i = 0; i < 8; i++) {
    float x = v.x * matrix[0] + v.y * matrix[1] + v.z * matrix[2];
    float y = v.x * matrix[1] + v.y * matrix[3] + v.z * matrix[4];
    float z = v.x * matrix[2] + v.y * matrix[4] + v.z * matrix[5];
    float m = max(max(x, y), z);
    float iv = 1.0f / m;
    v = float3(x * iv, y * iv, z * iv);
  }

  return v;
}

inline void swap(T &a, T &b) {
  T tmp = a;
  a = b;
  b = tmp;
}

inline float3 roundAndExpand(float3 v, ushort *w) {
  v.x = rintf(__saturatef(v.x) * 31.0f);
  v.y = rintf(__saturatef(v.y) * 63.0f);
  v.z = rintf(__saturatef(v.z) * 31.0f);

  *w = ((ushort)v.x << 11) | ((ushort)v.y << 5) | (ushort)v.z;
  v.x *= 0.03227752766457f;  // approximate integer bit expansion.
  v.y *= 0.01583151765563f;
  v.z *= 0.03227752766457f;
  return v;
}

static float evalPermutation4(const float3 *colors, uint permutation,
                                         ushort *start, ushort *end,
                                         float3 color_sum) {
// Compute endpoints using least squares.
#if USE_TABLES
  float3 alphax_sum = float3(0.0f, 0.0f, 0.0f);

  int akku = 0;

  // Compute alpha & beta for this permutation.
  for (int i = 0; i < 16; i++) {
    const uint bits = permutation >> (2 * i);

    alphax_sum += alphaTable4[bits & 3] * colors[i];
    akku += prods4[bits & 3];
  }

  float alpha2_sum = float(akku >> 16);
  float beta2_sum = float((akku >> 8) & 0xff);
  float alphabeta_sum = float((akku >> 0) & 0xff);
  float3 betax_sum = (9.0f * color_sum) - alphax_sum;
#else
  float alpha2_sum = 0.0f;
  float beta2_sum = 0.0f;
  float alphabeta_sum = 0.0f;
  float3 alphax_sum = float3(0.0f, 0.0f, 0.0f);

  // Compute alpha & beta for this permutation.
  for (int i = 0; i < 16; i++) {
    const uint bits = permutation >> (2 * i);

    float beta = (bits & 1);

    if (bits & 2) {
      beta = (1 + beta) * (1.0f / 3.0f);
    }

    float alpha = 1.0f - beta;

    alpha2_sum += alpha * alpha;
    beta2_sum += beta * beta;
    alphabeta_sum += alpha * beta;
    alphax_sum += alpha * colors[i];
  }

  float3 betax_sum = color_sum - alphax_sum;
#endif

  // alpha2, beta2, alphabeta and factor could be precomputed for each
  // permutation, but it's faster to recompute them.
  const float factor =
      1.0f / (alpha2_sum * beta2_sum - alphabeta_sum * alphabeta_sum);

  float3 a = (alphax_sum * beta2_sum - betax_sum * alphabeta_sum) * factor;
  float3 b = (betax_sum * alpha2_sum - alphax_sum * alphabeta_sum) * factor;

  // Round a, b to the closest 5-6-5 color and expand...
  a = roundAndExpand(a, start);
  b = roundAndExpand(b, end);

  // compute the error
  float3 e = a * a * alpha2_sum + b * b * beta2_sum +
             2.0f * (a * b * alphabeta_sum - a * alphax_sum - b * betax_sum);

  return (0.111111111111f) * (e.x + e.y + e.z);
}

static float evalPermutation3(const float3 *colors, uint permutation,
                                         ushort *start, ushort *end,
                                         float3 color_sum) {
// Compute endpoints using least squares.
#if USE_TABLES
  float3 alphax_sum = float3(0.0f, 0.0f, 0.0f);

  int akku = 0;

  // Compute alpha & beta for this permutation.
  for (int i = 0; i < 16; i++) {
    const uint bits = permutation >> (2 * i);

    alphax_sum += alphaTable3[bits & 3] * colors[i];
    akku += prods3[bits & 3];
  }

  float alpha2_sum = float(akku >> 16);
  float beta2_sum = float((akku >> 8) & 0xff);
  float alphabeta_sum = float((akku >> 0) & 0xff);
  float3 betax_sum = (4.0f * color_sum) - alphax_sum;
#else
  float alpha2_sum = 0.0f;
  float beta2_sum = 0.0f;
  float alphabeta_sum = 0.0f;
  float3 alphax_sum = float3(0.0f, 0.0f, 0.0f);

  // Compute alpha & beta for this permutation.
  for (int i = 0; i < 16; i++) {
    const uint bits = permutation >> (2 * i);

    float beta = (bits & 1);

    if (bits & 2) {
      beta = 0.5f;
    }

    float alpha = 1.0f - beta;

    alpha2_sum += alpha * alpha;
    beta2_sum += beta * beta;
    alphabeta_sum += alpha * beta;
    alphax_sum += alpha * colors[i];
  }

  float3 betax_sum = color_sum - alphax_sum;
#endif

  const float factor =
      1.0f / (alpha2_sum * beta2_sum - alphabeta_sum * alphabeta_sum);

  float3 a = (alphax_sum * beta2_sum - betax_sum * alphabeta_sum) * factor;
  float3 b = (betax_sum * alpha2_sum - alphax_sum * alphabeta_sum) * factor;

  // Round a, b to the closest 5-6-5 color and expand...
  a = roundAndExpand(a, start);
  b = roundAndExpand(b, end);

  // compute the error
  float3 e = a * a * alpha2_sum + b * b * beta2_sum +
             2.0f * (a * b * alphabeta_sum - a * alphax_sum - b * betax_sum);

  return (0.25f) * (e.x + e.y + e.z);
}

void saveBlockDXT1(ushort start, ushort end, uint permutation,
                              int xrefs[16], uint2 *result, int blockOffset) {
  const int bid = _bid_x + blockOffset;

  if (start == end) {
    permutation = 0;
  }

  // Reorder permutation.
  uint indices = 0;

  for (int i = 0; i < 16; i++) {
    int ref = xrefs[i];
    indices |= ((permutation >> (2 * ref)) & 3) << (2 * i);
  }

  // Write endpoints.
  result[bid].x = (end << 16) | start;

  // Write palette indices.
  result[bid].y = indices;
}
extern "C"

void compress(const uint *permutations, const uint *image,
                         uint2 *result, int blockOffset) {
    #pragma HLS INTERFACE m_axi port=permutations offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=image offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=blockOffset
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=colors complete dim=1
    #pragma HLS ARRAY_PARTITION variable=xrefs complete dim=1
    #pragma HLS ARRAY_PARTITION variable=dps complete dim=1
    #pragma HLS ARRAY_PARTITION variable=covariance complete dim=1
    #pragma HLS ARRAY_PARTITION variable=errors complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            // Handle to thread block group

            const int idx = _tid_x;
            const int bid = _bid_x + blockOffset;

            float3 sums;
            float3 colors[16];
            int xrefs[16];
            float dps[16];
            float covariance[16 * 6];

            if (idx < 16) {
            // Read color and copy to shared mem.
            uint c = image[(bid)*16 + idx];

            colors[idx].x = ((c >> 0) & 0xFF) * (1.0f / 255.0f);
            colors[idx].y = ((c >> 8) & 0xFF) * (1.0f / 255.0f);
            colors[idx].z = ((c >> 16) & 0xFF) * (1.0f / 255.0f);
            }

            if (idx == 0) {
            sums = colors[0];
            for (int i = 1; i < 16; i++)
            sums += colors[i];
            }

            if (idx < 16) {
            // Sort colors along the best fit line.
            float3 diff = colors[idx] - sums * (1.0f / 16.0f);
            covariance[6 * idx + 0] = diff.x * diff.x;  // 0, 6, 12, 2, 8, 14, 4, 10, 0
            covariance[6 * idx + 1] = diff.x * diff.y;
            covariance[6 * idx + 2] = diff.x * diff.z;
            covariance[6 * idx + 3] = diff.y * diff.y;
            covariance[6 * idx + 4] = diff.y * diff.z;
            covariance[6 * idx + 5] = diff.z * diff.z;
            }

            for (int d = 8; d > 0; d >>= 1) {
            if (idx < d) {
            covariance[6 * idx + 0] += covariance[6 * (idx + d) + 0];
            covariance[6 * idx + 1] += covariance[6 * (idx + d) + 1];
            covariance[6 * idx + 2] += covariance[6 * (idx + d) + 2];
            covariance[6 * idx + 3] += covariance[6 * (idx + d) + 3];
            covariance[6 * idx + 4] += covariance[6 * (idx + d) + 4];
            covariance[6 * idx + 5] += covariance[6 * (idx + d) + 5];
            }

            }

            if (idx < 16) {
            // Compute first eigen vector.
            float3 axis = firstEigenVector(covariance);

            dps[idx] = colors[idx].x * axis.x +
            colors[idx].y * axis.y +
            colors[idx].z * axis.z;
            }

            if (idx < 16) {
            int rank = 0;
            #pragma unroll
            for (int i = 0; i < 16; i++) {
            rank += (dps[i] < dps[idx]);
            }

            xrefs[idx] = rank;
            }


            // Resolve elements with the same index.
            for (int i = 0; i < 15; i++) {
            if (idx < 16 && idx > i && xrefs[idx] == xrefs[i]) {
            ++xrefs[idx];
            }

            }

            if (idx < 16) {
            colors[xrefs[idx]] = colors[idx];
            }

            ushort bestStart, bestEnd;
            uint bestPermutation;

            float errors[NUM_THREADS];

            evalAllPermutations(colors, permutations, bestStart, bestEnd, bestPermutation,
            errors, sums, cta);

            // Use a parallel reduction to find minimum error.
            const int minIdx = findMinError(errors, cta);

            // Only write the result of the winner thread.
            if (idx == minIdx) {
            saveBlockDXT1(bestStart, bestEnd, bestPermutation, xrefs, result,
            blockOffset);
            }

        }
    }
}
