#include "kernel.h"

// --- from compact.cu ---
extern "C"
void ccc_loop1(
  const int *  imaterial,
  const int *  nextfrac,
  const double *  rho_compact,
  const double *  rho_compact_list, 
  const double *  Vf_compact_list,
  const double *  V,
  double *  rho_ave_compact,
  int sizex, int sizey,
  int *  mmc_index)
{
    #pragma HLS INTERFACE m_axi port=imaterial offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=nextfrac offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=rho_compact offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=rho_compact_list offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=Vf_compact_list offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=V offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=rho_ave_compact offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=sizex
    #pragma HLS INTERFACE s_axilite port=sizey
    #pragma HLS INTERFACE m_axi port=mmc_index offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int i = _tid_x + _bid_x * BLOCK_DIM_X;
                    int j = _tid_y + _bid_y * BLOCK_DIM_Y;
                    if (i >= sizex || j >= sizey) return;
                    #ifdef FUSED
                    double ave = 0.0;
                    int ix = imaterial[i+sizex*j];

                    if (ix <= 0) {
                    // condition is 'ix >= 0', this is the equivalent of
                    // 'until ix < 0' from the paper
                    #ifdef LINKED
                    for (ix = -ix; ix >= 0; ix = nextfrac[ix]) {
                    ave += rho_compact_list[ix] * Vf_compact_list[ix];
                    }
                    #else
                    for (int idx = mmc_index[-ix]; idx < mmc_index[-ix+1]; idx++) {
                    ave += rho_compact_list[idx] * Vf_compact_list[idx];
                    }
                    #endif
                    rho_ave_compact[i+sizex*j] = ave/V[i+sizex*j];
                    }
                    else {
                    #endif
                    // We use a distinct output array for averages.
                    // In case of a pure cell, the average density equals to the total.
                    rho_ave_compact[i+sizex*j] = rho_compact[i+sizex*j] / V[i+sizex*j];
                    #ifdef FUSED
                    }
                    #endif

                }
            }
        }
    }
}
extern "C"

void ccc_loop1_2(
  const double *  rho_compact_list,
  const double *   Vf_compact_list,
  const double *   V,
  double *  rho_ave_compact,
  const int *  mmc_index,
  const int  mmc_cells,
  const int *  mmc_i,
  const int *  mmc_j,
  int sizex, int sizey)
{
    #pragma HLS INTERFACE m_axi port=rho_compact_list offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Vf_compact_list offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=V offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=rho_ave_compact offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=mmc_index offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=mmc_cells
    #pragma HLS INTERFACE m_axi port=mmc_i offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=mmc_j offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=sizex
    #pragma HLS INTERFACE s_axilite port=sizey
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int c = _tid_x + _bid_x * BLOCK_DIM_X;
                    if (c >= mmc_cells) return;
                    double ave = 0.0;
                    for (int m = mmc_index[c]; m < mmc_index[c+1]; m++) {
                    ave +=  rho_compact_list[m] * Vf_compact_list[m];
                    }
                    rho_ave_compact[mmc_i[c]+sizex*mmc_j[c]] = ave/V[mmc_i[c]+sizex*mmc_j[c]];

                }
            }
        }
    }
}
extern "C"

void ccc_loop2(
  const int *  imaterial,
  const int *  matids,
  const int *  nextfrac,
  const double *  rho_compact,
  const double *  rho_compact_list, 
  const double *  t_compact,
  const double *  t_compact_list, 
  const double *   Vf_compact_list,
  const double *  n,
  double *   p_compact,
  double *  p_compact_list,
  int sizex, int sizey,
  int *  mmc_index)
{
    #pragma HLS INTERFACE m_axi port=imaterial offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=matids offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=nextfrac offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=rho_compact offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=rho_compact_list offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=t_compact offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=t_compact_list offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=Vf_compact_list offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=n offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=p_compact offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=p_compact_list offset=slave bundle=gmem10
    #pragma HLS INTERFACE s_axilite port=sizex
    #pragma HLS INTERFACE s_axilite port=sizey
    #pragma HLS INTERFACE m_axi port=mmc_index offset=slave bundle=gmem11
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int i = _tid_x + _bid_x * BLOCK_DIM_X;
                    int j = _tid_y + _bid_y * BLOCK_DIM_Y;
                    if (i >= sizex || j >= sizey) return;

                    int ix = imaterial[i+sizex*j];
                    if (ix <= 0) {
                    #ifdef FUSED
                    // NOTE: I think the paper describes this algorithm (Alg. 9) wrong.
                    // The solution below is what I believe to good.

                    // condition is 'ix >= 0', this is the equivalent of
                    // 'until ix < 0' from the paper
                    #ifdef LINKED
                    for (ix = -ix; ix >= 0; ix = nextfrac[ix]) {
                    double nm = n[matids[ix]];
                    p_compact_list[ix] = (nm * rho_compact_list[ix] * t_compact_list[ix]) / Vf_compact_list[ix];
                    }
                    #else
                    for (int idx = mmc_index[-ix]; idx < mmc_index[-ix+1]; idx++) {
                    double nm = n[matids[idx]];
                    p_compact_list[idx] = (nm * rho_compact_list[idx] * t_compact_list[idx]) / Vf_compact_list[idx];
                    }
                    #endif
                    #endif
                    }
                    else {
                    // NOTE: HACK: we index materials from zero, but zero can be a list index
                    int mat = ix - 1;
                    // NOTE: There is no division by Vf here, because the fractional volume is 1.0 in the pure cell case.
                    p_compact[i+sizex*j] = n[mat] * rho_compact[i+sizex*j] * t_compact[i+sizex*j];;
                    }

                }
            }
        }
    }
}
extern "C"

void ccc_loop2_2(
  const int *  matids,
  const double *  rho_compact_list, 
  const double *  t_compact_list,
  const double *  Vf_compact_list,
  const double *  n,
  double *  p_compact_list,
  int *  mmc_index,
  int mmc_cells)
{
    #pragma HLS INTERFACE m_axi port=matids offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=rho_compact_list offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=t_compact_list offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=Vf_compact_list offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=n offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=p_compact_list offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=mmc_index offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=mmc_cells
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int idx = _tid_x + _bid_x * BLOCK_DIM_X;
                    if (idx >= mmc_cells) return;
                    double nm = n[matids[idx]];
                    p_compact_list[idx] = (nm * rho_compact_list[idx] * t_compact_list[idx]) / Vf_compact_list[idx];

                }
            }
        }
    }
}
