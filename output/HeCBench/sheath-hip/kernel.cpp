#include "kernel.h"

// --- from main.cu ---
extern "C"
void scatterParticle(const Particle* __restrict particles, float*__restrict den, long N)
{
    #pragma HLS INTERFACE m_axi port=particles offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=den offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            /*get particle id*/
            long p = _bid_x * BLOCK_DIM_X + _tid_x;
            if (p < N && particles[p].alive)
            {
            double lc = XtoL(particles[p].x);
            scatter(lc, 1.f, den);
            }

        }
    }
}
extern "C"

void pushParticle(Particle*__restrict particles, const double*__restrict ef, double qm, long N)
{
    #pragma HLS INTERFACE m_axi port=particles offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=ef offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=qm
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            /*get particle id*/
            long p = _bid_x * BLOCK_DIM_X + _tid_x;

            if (p < N && particles[p].alive)
            {
            /*grab pointer to this particle*/
            Particle* part = &particles[p];

            /*compute particle node position*/
            double lc = XtoL(part->x);

            /*gather electric field onto particle position*/
            double part_ef = gather(lc, ef);

            /*advance velocity*/
            part->v += DT * qm * part_ef;

            /*advance position*/
            part->x += DT * part->v;

            /*remove particles leaving the domain*/
            if (part->x < X0 || part->x >= XMAX)
            part->alive = false;
            }

        }
    }
}
extern "C"

void rewindParticle(Particle*__restrict particles, const double*__restrict ef, double qm, long N)
{
    #pragma HLS INTERFACE m_axi port=particles offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=ef offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=qm
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            /*get particle id*/
            long p = _bid_x * BLOCK_DIM_X + _tid_x;

            if (p < N && particles[p].alive)
            {
            /*grab pointer to this particle*/
            Particle* part = &particles[p];

            /*compute particle node position*/
            double lc = XtoL(part->x);

            /*gather electric field onto particle position*/
            double part_ef = gather(lc, ef);

            /*advance velocity*/
            part->v -= 0.5 * DT * qm * part_ef;
            }

        }
    }
}

double XtoL(double pos)
{
  double li = (pos - 0) / DX;
  return li;
}

void scatter(double lc, float value, float* field)
{
  int i    = (int)lc;
  float di = lc - i;
  atomicAdd(&(field[i]), value * (1 - di));
  atomicAdd(&(field[i + 1]), value * (di));
}

double gather(double lc, const double* field)
{
  int i     = (int)lc;
  double di = lc - i;

  /*gather field value onto particle position*/
  double val = field[i] * (1 - di) + field[i + 1] * (di);
  return val;
}
