#include "kernel.h"

// --- from kernels.cu ---
unsigned TausStep(unsigned &z, int S1, int S2, int S3, unsigned M)
{
  unsigned b = (((z << S1) ^ z) >> S2);
  return z = (((z & M) << S3) ^ b);
}

unsigned LCGStep(unsigned &z)
{
  return z = (1664525 * z + 1013904223);
}

float getRandomValueTauswortheUniform(unsigned &z1, unsigned &z2, unsigned &z3, unsigned &z4)
{
  unsigned taus = TausStep(z1, 13, 19, 12, 4294967294U) ^ 
                  TausStep(z2, 2, 25, 4, 4294967288U) ^ TausStep(z3, 3, 11, 17, 4294967280U);
  unsigned lcg = LCGStep(z4);

  return 2.3283064365387e-10f * (taus ^ lcg);  // taus+
}

void boxMuller(float u1, float u2, float &uo1, float &uo2)
{
  float z1 = sqrtf(-2.0f * logf(u1));
  float s1 = sinf(2.0f * PI * u2);
  float s2 = cosf(2.0f * PI * u2);
  uo1 = z1 * s1;
  uo2 = z1 * s2;
}

float getRandomValueTausworthe(unsigned &z1, unsigned &z2, unsigned &z3, 
                                          unsigned &z4, float &temporary, unsigned phase)
{
  if (phase & 1)
  {
    // Return second value of pair
    return temporary;
  }
  else
  {
    float t1, t2, t3;
    // Phase is even, generate pair, return first of values, store second
    t1 = getRandomValueTauswortheUniform(z1, z2, z3, z4);
    t2 = getRandomValueTauswortheUniform(z1, z2, z3, z4);
    boxMuller(t1, t2, t3, temporary);
    return t3;
  }
}

float tausworthe_lookback_sim(
    unsigned T, float VOL_0, float EPS_0, 
    float A_0, float A_1, float A_2, float S_0,
    float MU, unsigned &z1, unsigned &z2,
    unsigned &z3, unsigned &z4, float* path)
{
  float temp_random_value;
  float vol = VOL_0, eps = EPS_0;
  float s = S_0;
  int base = _tid_x;

  for (unsigned t = 0; t < T; t++)
  {
    // store the current asset price
    path[base] = s;
    base += LOOKBACK_TAUSWORTHE_NUM_THREADS;

    // time-varying volatility in the GARCH model
    vol = sqrtf(A_0 + A_1 * vol * vol + A_2 * eps * eps);

    // size of next asset movement depends in part on the size of the most recent movement
    eps = getRandomValueTausworthe(z1, z2, z3, z4, temp_random_value, t) * vol;
    // s may become infinite
    eps = fmaxf(fminf(eps, 1.f), -1.f);

    // next price
    s *= expf(MU + eps);
  }

  // Look back at path to find payoff
  float sum = 0;
  for (unsigned t = 0; t < T; t++)
  {
    base -= LOOKBACK_TAUSWORTHE_NUM_THREADS;
    sum += fmaxf(path[base] - s, 0.f);
  }
  return sum;
}
extern "C"

void tausworthe_lookback(
    unsigned num_cycles,
    const unsigned int * seedValues,
    float * simulationResultsMean,
    float * simulationResultsVariance,
    const float * g_VOL_0,
    const float * g_EPS_0,
    const float * g_A_0,
    const float * g_A_1,
    const float * g_A_2,
    const float * g_S_0,
    const float * g_MU)
{
    #pragma HLS INTERFACE s_axilite port=num_cycles
    #pragma HLS INTERFACE m_axi port=seedValues offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=simulationResultsMean offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=simulationResultsVariance offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g_VOL_0 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=g_EPS_0 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=g_A_0 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=g_A_1 offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=g_A_2 offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=g_S_0 offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=g_MU offset=slave bundle=gmem9
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=path complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            float path[LOOKBACK_TAUSWORTHE_NUM_THREADS*LOOKBACK_MAX_T];

            unsigned address = _bid_x * BLOCK_DIM_X + _tid_x;

            // Initialise tausworth with seeds
            unsigned z1 = seedValues[address];
            unsigned z2 = seedValues[address +     TAUSWORTHE_TOTAL_NUM_THREADS];
            unsigned z3 = seedValues[address + 2 * TAUSWORTHE_TOTAL_NUM_THREADS];
            unsigned z4 = seedValues[address + 3 * TAUSWORTHE_TOTAL_NUM_THREADS];

            float VOL_0, EPS_0, A_0, A_1, A_2, S_0, MU;
            VOL_0 = g_VOL_0[address];
            EPS_0 = g_EPS_0[address];
            A_0 = g_A_0[address];
            A_1 = g_A_1[address];
            A_2 = g_A_2[address];
            S_0 = g_S_0[address];
            MU = g_MU[address];

            float mean = 0, variance = 0;
            for (unsigned i = 1; i <= LOOKBACK_PATHS_PER_SIM; i++)
            {
            // simulate a path for num_cyles cyles
            float res = tausworthe_lookback_sim(num_cycles, VOL_0, EPS_0,
            A_0, A_1, A_2, S_0,
            MU,
            z1, z2, z3, z4,  // rng state variables
            path);

            // update mean and variance in a numerically stable way
            float delta = res - mean;
            mean += delta / i;
            variance += delta * (res - mean);
            }

            simulationResultsMean[address] = mean;
            simulationResultsVariance[address] = variance / (LOOKBACK_PATHS_PER_SIM - 1);

        }
    }
}
