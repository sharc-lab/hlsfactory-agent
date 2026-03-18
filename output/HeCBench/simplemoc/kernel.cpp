#include "kernel.h"

// --- from main.cu ---
extern "C"
void att (
  const int* QSR_id_acc,
  const int* FAI_id_acc,
  float* fine_flux_acc,
  float* fine_source_acc,
  float* sigT_acc,
  float* state_flux_acc,
  float* v_acc,
  const int fine_axial_intervals,
  const int egroups,
  const int segments )
{
    #pragma HLS INTERFACE m_axi port=QSR_id_acc offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=FAI_id_acc offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=fine_flux_acc offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fine_source_acc offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=sigT_acc offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=state_flux_acc offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=v_acc offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=fine_axial_intervals
    #pragma HLS INTERFACE s_axilite port=egroups
    #pragma HLS INTERFACE s_axilite port=segments
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int gid = _bid_x*BLOCK_DIM_X+_tid_x;
            if (gid >= segments) return;

            const float dz = 0.1f;
            const float zin = 0.3f;
            const float weight = 0.5f;
            const float mu = 0.9f;
            const float mu2 = 0.3f;
            const float ds = 0.7f;

            int QSR_id = QSR_id_acc[gid];
            int FAI_id = FAI_id_acc[gid];

            // load fine source region flux vector
            int offset = QSR_id * fine_axial_intervals * egroups;

            float *FSR_flux = fine_flux_acc + offset + FAI_id * egroups;

            float* q0 = v_acc;
            float* q1 = v_acc + egroups;
            float* q2 = v_acc + egroups * 2;
            float* sigT = v_acc + egroups * 3;
            float* tau = v_acc + egroups * 4;
            float* sigT2 = v_acc + egroups * 5;
            float* expVal = v_acc + egroups * 6;
            float* reuse = v_acc + egroups * 7;
            float* flux_integral = v_acc + egroups * 8;
            float* tally = v_acc + egroups * 9;
            float* t1 = v_acc + egroups * 10;
            float* t2 = v_acc + egroups * 11;
            float* t3 = v_acc + egroups * 12;
            float* t4 = v_acc + egroups * 13;

            if( FAI_id == 0 )
            {
            float * f2 = fine_source_acc + offset + FAI_id*egroups;
            float * f3 = fine_source_acc + offset + (FAI_id+1)*egroups;
            // cycle over energy groups
            for( int g = 0; g < egroups; g++)
            {
            // load neighboring sources
            const float y2 = f2[g];
            const float y3 = f3[g];

            // do linear "fitting"
            const float c0 = y2;
            const float c1 = (y3 - y2) / dz;

            // calculate q0, q1, q2
            q0[g] = c0 + c1*zin;
            q1[g] = c1;
            q2[g] = 0;
            }
            }
            else if ( FAI_id == fine_axial_intervals - 1 )
            {
            float * f1 = fine_source_acc + offset + (FAI_id-1)*egroups;
            float * f2 = fine_source_acc + offset + FAI_id*egroups;

            for( int g = 0; g < egroups; g++)
            {
            // load neighboring sources
            const float y1 = f1[g];
            const float y2 = f2[g];

            // do linear "fitting"
            const float c0 = y2;
            const float c1 = (y2 - y1) / dz;

            // calculate q0, q1, q2
            q0[g] = c0 + c1*zin;
            q1[g] = c1;
            q2[g] = 0;
            }
            }
            else
            {
            float * f1 = fine_source_acc + offset + (FAI_id-1)*egroups;
            float * f2 = fine_source_acc + offset + FAI_id*egroups;
            float * f3 = fine_source_acc + offset + (FAI_id+1)*egroups;
            // cycle over energy groups
            for( int g = 0; g < egroups; g++)
            {
            // load neighboring sources
            const float y1 = f1[g];
            const float y2 = f2[g];
            const float y3 = f3[g];

            // do quadratic "fitting"
            const float c0 = y2;
            const float c1 = (y1 - y3) / (2.f*dz);
            const float c2 = (y1 - 2.f*y2 + y3) / (2.f*dz*dz);

            // calculate q0, q1, q2
            q0[g] = c0 + c1*zin + c2*zin*zin;
            q1[g] = c1 + 2.f*c2*zin;
            q2[g] = c2;
            }
            }

            // cycle over energy groups
            offset = QSR_id * egroups;
            for( int g = 0; g < egroups; g++)
            {
            // load total cross section
            sigT[g] = sigT_acc[offset + g];

            // calculate common values for efficiency
            tau[g] = sigT[g] * ds;
            sigT2[g] = sigT[g] * sigT[g];

            expVal[g] = 1.f - exp( -tau[g] ); // exp is faster on many architectures
            reuse[g] = tau[g] * (tau[g] - 2.f) + 2.f * expVal[g] / (sigT[g] * sigT2[g]);

            // add contribution to new source flux
            flux_integral[g] = (q0[g] * tau[g] + (sigT[g] * state_flux_acc[g] - q0[g])
            * expVal[g]) / sigT2[g] + q1[g] * mu * reuse[g] + q2[g] * mu2
            * (tau[g] * (tau[g] * (tau[g] - 3.f) + 6.f) - 6.f * expVal[g])
            / (3.f * sigT2[g] * sigT2[g]);

            tally[g] = weight * flux_integral[g];
            FSR_flux[g] += tally[g];
            t1[g] = q0[g] * expVal[g] / sigT[g];
            t2[g] = q1[g] * mu * (tau[g] - expVal[g]) / sigT2[g];
            t3[g] = q2[g] * mu2 * reuse[g];
            t4[g] = state_flux_acc[g] * (1.f - expVal[g]);
            state_flux_acc[g] = t1[g]+t2[g]+t3[g]+t4[g];
            }

        }
    }
}
