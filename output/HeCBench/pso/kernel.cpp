#include "kernel.h"

// --- from kernel_gpu.cu ---
float fitness_function(float x[])
{
  float y1 = F(x[0]);
  float yn = F(x[DIM-1]);
  float res = powf(sinf(phi*y1), 2.f) + powf(yn-1, 2.f);

  for(int i = 0; i < DIM-1; i++)
  {
    float y = F(x[i]);
    float yp = F(x[i+1]);
    res += powf(y-1.f, 2.f) * (1.f + 10.f * powf(sinf(phi*yp), 2.f));
  }

  return res;
}
extern "C"

void kernelUpdateParticle(float * positions,
                          float * velocities,
                          const float * pBests,
                          const float * gBest,
                          const int p,
                          const float rp,
                          const float rg)
{
    #pragma HLS INTERFACE m_axi port=positions offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=velocities offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=pBests offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=gBest offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=p
    #pragma HLS INTERFACE s_axilite port=rp
    #pragma HLS INTERFACE s_axilite port=rg
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i=_bid_x*BLOCK_DIM_X+_tid_x;
            if (i >= p*DIM) return;

            velocities[i]=OMEGA*velocities[i]+
            c1*rp*(pBests[i]-positions[i])+
            c2*rg*(gBest[i%DIM]-positions[i]);
            positions[i]+=velocities[i];

        }
    }
}
extern "C"

void kernelUpdatePBest(const float * positions,
                             float * pBests,
                             float * gBest,
                       const int p)
{
    #pragma HLS INTERFACE m_axi port=positions offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pBests offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=gBest offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=p
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i=_bid_x*BLOCK_DIM_X+_tid_x;
            if (i >= p) return;
            i = i*DIM;

            float tempParticle1[DIM];
            float tempParticle2[DIM];

            for(int j=0;j<DIM;j++)
            {
            tempParticle1[j]=positions[i+j];
            tempParticle2[j]=pBests[i+j];
            }

            if(fitness_function(tempParticle1)<fitness_function(tempParticle2))
            {
            for(int j=0;j<DIM;j++)
            pBests[i+j]=tempParticle1[j];

            if(fitness_function(tempParticle1)<130.f) //fitness_function(gBest))
            {
            for(int j=0;j<DIM;j++) {
            atomicAdd(gBest+j,tempParticle1[j]);
            }
            }
            }

        }
    }
}
