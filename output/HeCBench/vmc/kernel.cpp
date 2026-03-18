#include "kernel.h"

// --- from vmc2.cu ---
inline float EXP(float x) {return expf(x);}

inline double EXP(double x) {return exp(x);}

inline float SQRT(float x) {return sqrtf(x);}

inline double SQRT(double x) {return sqrt(x);}

float LCG_random(unsigned int * seed) {
  const unsigned int m = 2147483648;
  const unsigned int a = 26757677;
  const unsigned int c = 1;
  *seed = (a * (*seed) + c) % m;
  return (float) (*seed) / (float) m;
}

void LCG_random_init(unsigned int * seed) {
  const unsigned int m = 2147483648;
  const unsigned int a = 26757677;
  const unsigned int c = 1;
  *seed = (a * (*seed) + c) % m;
}
extern "C"

void SumWithinBlocks(const int n, const FLOAT* data, FLOAT* blocksums) {
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=blocksums offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sdata complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int nthread = BLOCK_DIM_X*GRID_DIM_X;
            int i = BLOCK_DIM_X * _bid_x + _tid_x;
            FLOAT sdata[512];  // max threads

            // Every thread in every block computes partial sum over rest of vector
            FLOAT st=ZERO;
            while (i < n) {
            st += data[i];
            i+=nthread;
            }
            sdata[_tid_x] = st;

            // Now do binary tree sum within a block
            int tid = _tid_x;
            for (unsigned int s=128; s>0; s>>=1) {
            if (tid<s && (tid+s)<BLOCK_DIM_X) {
            sdata[tid] += sdata[tid + s];
            }
            }
            if (tid==0) blocksums[_bid_x] = sdata[0];

        }
    }
}

inline void compute_distances(FLOAT x1, FLOAT y1, FLOAT z1, FLOAT x2, FLOAT y2, FLOAT z2,
    FLOAT& r1, FLOAT& r2, FLOAT& r12) {
  r1 = SQRT(x1*x1 + y1*y1 + z1*z1);
  r2 = SQRT(x2*x2 + y2*y2 + z2*z2);
  FLOAT xx = x1-x2;
  FLOAT yy = y1-y2;
  FLOAT zz = z1-z2;
  r12 = SQRT(xx*xx + yy*yy + zz*zz);
}

inline FLOAT wave_function(FLOAT x1, FLOAT y1, FLOAT z1, FLOAT x2, FLOAT y2, FLOAT z2) {
  FLOAT r1, r2, r12;
  compute_distances(x1, y1, z1, x2, y2, z2, r1, r2, r12);

  return (ONE + HALF*r12)*EXP(-TWO*(r1 + r2));
}
extern "C"

void initran(unsigned int seed, unsigned int* states) {
    #pragma HLS INTERFACE s_axilite port=seed
    #pragma HLS INTERFACE m_axi port=states offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = BLOCK_DIM_X * _bid_x + _tid_x;
            states[i] = seed ^ i;
            LCG_random_init(&states[i]);

        }
    }
}
extern "C"

void zero_stats(int Npoint, FLOAT* stats) {
    #pragma HLS INTERFACE s_axilite port=Npoint
    #pragma HLS INTERFACE m_axi port=stats offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = BLOCK_DIM_X * _bid_x + _tid_x;
            stats[0*Npoint+i] = ZERO; // r1
            stats[1*Npoint+i] = ZERO; // r2
            stats[2*Npoint+i] = ZERO; // r12
            stats[3*Npoint+i] = ZERO; // accept count

        }
    }
}
extern "C"

void initialize(FLOAT*  x1,
                           FLOAT*  y1,
                           FLOAT*  z1,
                           FLOAT*  x2,
                           FLOAT*  y2,
                           FLOAT*  z2,
                           FLOAT*  psi,
                           unsigned int*  states)
{
    #pragma HLS INTERFACE m_axi port=x1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=z1 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=x2 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=y2 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=z2 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=psi offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=states offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = BLOCK_DIM_X * _bid_x + _tid_x;
            x1[i] = (LCG_random(states+i) - HALF)*FOUR;
            y1[i] = (LCG_random(states+i) - HALF)*FOUR;
            z1[i] = (LCG_random(states+i) - HALF)*FOUR;
            x2[i] = (LCG_random(states+i) - HALF)*FOUR;
            y2[i] = (LCG_random(states+i) - HALF)*FOUR;
            z2[i] = (LCG_random(states+i) - HALF)*FOUR;
            psi[i] = wave_function(x1[i], y1[i], z1[i], x2[i], y2[i], z2[i]);

        }
    }
}
extern "C"

void propagate(const int Npoint, const int nstep,
                          FLOAT*   X1,
                          FLOAT*   Y1,
                          FLOAT*   Z1,
                          FLOAT*   X2,
                          FLOAT*   Y2,
                          FLOAT*   Z2,
                          FLOAT*   P,
                          FLOAT*   stats,
                          unsigned int*   states)
{
    #pragma HLS INTERFACE s_axilite port=Npoint
    #pragma HLS INTERFACE s_axilite port=nstep
    #pragma HLS INTERFACE m_axi port=X1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Y1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=Z1 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=X2 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=Y2 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=Z2 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=P offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=stats offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=states offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = BLOCK_DIM_X * _bid_x + _tid_x;
            FLOAT x1 = X1[i];
            FLOAT y1 = Y1[i];
            FLOAT z1 = Z1[i];
            FLOAT x2 = X2[i];
            FLOAT y2 = Y2[i];
            FLOAT z2 = Z2[i];
            FLOAT p = P[i];

            for (int step=0; step<nstep; step++) {
            FLOAT x1new = x1 + (LCG_random(states+i)-HALF)*DELTA;
            FLOAT y1new = y1 + (LCG_random(states+i)-HALF)*DELTA;
            FLOAT z1new = z1 + (LCG_random(states+i)-HALF)*DELTA;
            FLOAT x2new = x2 + (LCG_random(states+i)-HALF)*DELTA;
            FLOAT y2new = y2 + (LCG_random(states+i)-HALF)*DELTA;
            FLOAT z2new = z2 + (LCG_random(states+i)-HALF)*DELTA;
            FLOAT pnew = wave_function(x1new, y1new, z1new, x2new, y2new, z2new);

            if (pnew*pnew > p*p*LCG_random(states+i)) {
            stats[3*Npoint+i]++; //naccept ++;
            p = pnew;
            x1 = x1new;
            y1 = y1new;
            z1 = z1new;
            x2 = x2new;
            y2 = y2new;
            z2 = z2new;
            }

            FLOAT r1, r2, r12;
            compute_distances(x1, y1, z1, x2, y2, z2, r1, r2, r12);

            stats[0*Npoint+i] += r1;
            stats[1*Npoint+i] += r2;
            stats[2*Npoint+i] += r12;
            }
            X1[i] = x1;
            Y1[i] = y1;
            Z1[i] = z1;
            X2[i] = x2;
            Y2[i] = y2;
            Z2[i] = z2;
            P[i] = p;

        }
    }
}
