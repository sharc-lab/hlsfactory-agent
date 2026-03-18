#include "kernel.h"

// --- from kernels.cu ---
extern "C"
void reaction_gray_scott(
    const float * fx,
    const float * fy,
    float * drx,
    float * dry,
    const unsigned int ncells,
    const float d_c1,
    const float d_c2)
{
    #pragma HLS INTERFACE m_axi port=fx offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fy offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=drx offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=dry offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=ncells
    #pragma HLS INTERFACE s_axilite port=d_c1
    #pragma HLS INTERFACE s_axilite port=d_c2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int index = _bid_x * BLOCK_DIM_X + _tid_x;
                    int stride = BLOCK_DIM_X * GRID_DIM_X;

                    for(int i = index; i < ncells; i += stride) {
                    float r = fx[i] * fy[i] * fy[i];
                    drx[i] = -r + d_c1 * (1.f - fx[i]);
                    dry[i] = r - (d_c1 + d_c2) * fy[i];
                    }

                }
            }
        }
    }
}
extern "C"

void derivative_x2_pbc(
    const float * f,
    float * df,
    const unsigned int mx,
    const unsigned int my,
    const unsigned int pencils)
{
    #pragma HLS INTERFACE m_axi port=f offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=df offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=mx
    #pragma HLS INTERFACE s_axilite port=my
    #pragma HLS INTERFACE s_axilite port=pencils
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const int offset = 1;
                    float s_f[4096]; // 2-wide halo

                    int i   = _tid_x;
                    int j   = _bid_x * BLOCK_DIM_Y + _tid_y;
                    int k   = _bid_y;
                    int si  = i + offset;  // local i for shared memory access + halo offset
                    int sj  = _tid_y; // local j for shared memory access

                    int globalIdx = k * mx * my + j * mx + i;

                    s_f[sj * (mx + 2 * offset) + si] = f[globalIdx];

                    // fill in periodic images in shared memory array
                    if (i < offset) {
                    s_f[sj * (mx + 2 * offset) + si - offset]  = s_f[sj * (mx + 2 * offset) + si + mx - offset];
                    s_f[sj * (mx + 2 * offset) + si + mx] = s_f[sj * (mx + 2 * offset) + si];
                    }

                    df[globalIdx] = s_f[sj * (mx + 2 * offset) + si + 1] - 2.f * s_f[sj * (mx + 2 * offset) + si] + s_f[sj * (mx + 2 * offset) + si - 1];

                }
            }
        }
    }
}
extern "C"

void derivative_x2_zeroflux(
    const float * f,
    float * df,
    const unsigned int mx,
    const unsigned int my)
{
    #pragma HLS INTERFACE m_axi port=f offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=df offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=mx
    #pragma HLS INTERFACE s_axilite port=my
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float s_f[4096];

                    int i   = _tid_x;
                    int j   = _bid_x * BLOCK_DIM_Y + _tid_y;
                    int k   = _bid_y;
                    int sj  = _tid_y; // local j for shared memory access

                    int globalIdx = k * mx * my + j * mx + i;

                    s_f[sj * mx + i] = f[globalIdx];

                    if(i == 0) {
                    df[globalIdx] = s_f[sj * mx + i + 1] - s_f[sj * mx + i];
                    } else if(i == (mx - 1)) {
                    df[globalIdx] = s_f[sj * mx + i - 1] - s_f[sj * mx + i];
                    } else {
                    df[globalIdx] = s_f[sj * mx + i + 1] - 2.f * s_f[sj * mx + i] + s_f[sj * mx + i - 1];
                    }

                }
            }
        }
    }
}
extern "C"

void derivative_y2_pbc(
    const float * f,
    float * df,
    const unsigned int mx,
    const unsigned int my,
    const unsigned int pencils)
{
    #pragma HLS INTERFACE m_axi port=f offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=df offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=mx
    #pragma HLS INTERFACE s_axilite port=my
    #pragma HLS INTERFACE s_axilite port=pencils
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const int offset = 1;
                    float s_f[4096]; // 2-wide halo

                    int i  = _bid_x * BLOCK_DIM_X + _tid_x;
                    int j  = _tid_y;
                    int k  = _bid_y;
                    int si = _tid_x;
                    int sj = j + offset;

                    int globalIdx = k * mx * my + j * mx + i;

                    s_f[sj * pencils + si] = f[globalIdx];

                    // fill in periodic images in shared memory array
                    if (j < offset) {
                    s_f[(sj - offset) * pencils + si]  = s_f[(sj + my - offset) * pencils + si];
                    s_f[(sj + my) * pencils + si] = s_f[sj * pencils + si];
                    }

                    df[globalIdx] = s_f[(sj+1) * pencils + si] - 2.f * s_f[sj * pencils + si] + s_f[(sj-1) * pencils + si];

                }
            }
        }
    }
}
extern "C"

void derivative_y2_zeroflux(
    const float * f,
    float * df,
    const unsigned int mx,
    const unsigned int my,
    const unsigned int pencils)
{
    #pragma HLS INTERFACE m_axi port=f offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=df offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=mx
    #pragma HLS INTERFACE s_axilite port=my
    #pragma HLS INTERFACE s_axilite port=pencils
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float s_f[4096];

                    int i  = _bid_x * BLOCK_DIM_X + _tid_x;
                    int j  = _tid_y;
                    int k  = _bid_y;
                    int si = _tid_x;

                    int globalIdx = k * mx * my + j * mx + i;

                    s_f[j * pencils + si] = f[globalIdx];

                    if(j == 0) {
                    df[globalIdx] = s_f[(j+1) * pencils + si] - s_f[j * pencils + si];
                    } else if(j == (my - 1)) {
                    df[globalIdx] = s_f[(j-1) * pencils + si] - s_f[j * pencils + si];
                    } else {
                    df[globalIdx] = s_f[(j+1) * pencils + si] - 2.f * s_f[j * pencils + si] + s_f[(j-1) * pencils + si];
                    }

                }
            }
        }
    }
}
extern "C"

void derivative_z2_pbc(
    const float * f,
    float * df,
    const unsigned int mx,
    const unsigned int my,
    const unsigned int mz,
    const unsigned int pencils)
{
    #pragma HLS INTERFACE m_axi port=f offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=df offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=mx
    #pragma HLS INTERFACE s_axilite port=my
    #pragma HLS INTERFACE s_axilite port=mz
    #pragma HLS INTERFACE s_axilite port=pencils
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const int offset = 1;
                    float s_f[4096]; // 2-wide halo

                    int i  = _bid_x * BLOCK_DIM_X + _tid_x;
                    int j  = _bid_y;
                    int k  = _tid_y;
                    int si = _tid_x;
                    int sk = k + offset; // halo offset

                    int globalIdx = k * mx * my + j * mx + i;

                    s_f[sk * pencils + si] = f[globalIdx];

                    // fill in periodic images in shared memory array
                    if (k < offset) {
                    s_f[(sk - offset) * pencils + si]  = s_f[(sk + mz - offset) * pencils + si];
                    s_f[(sk + mz) * pencils + si] = s_f[sk * pencils + si];
                    }

                    df[globalIdx] = s_f[(sk+1) * pencils + si] - 2.f * s_f[sk * pencils + si] + s_f[(sk-1) * pencils + si];

                }
            }
        }
    }
}
extern "C"

void derivative_z2_zeroflux(
    const float * f,
    float * df,
    const unsigned int mx,
    const unsigned int my,
    const unsigned int mz,
    const unsigned int pencils)
{
    #pragma HLS INTERFACE m_axi port=f offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=df offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=mx
    #pragma HLS INTERFACE s_axilite port=my
    #pragma HLS INTERFACE s_axilite port=mz
    #pragma HLS INTERFACE s_axilite port=pencils
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_f complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float s_f[4096]; // 2-wide halo

                    int i  = _bid_x * BLOCK_DIM_X + _tid_x;
                    int j  = _bid_y;
                    int k  = _tid_y;
                    int si = _tid_x;

                    int globalIdx = k * mx * my + j * mx + i;

                    s_f[k * pencils + si] = f[globalIdx];

                    if(k == 0) {
                    df[globalIdx] = s_f[(k+1) * pencils + si] - s_f[k * pencils + si];
                    } else if(k == (mz - 1)) {
                    df[globalIdx] = s_f[(k-1) * pencils + si] - s_f[k * pencils + si];
                    } else {
                    df[globalIdx] = s_f[(k+1) * pencils + si] - 2.f * s_f[k * pencils + si] + s_f[(k-1) * pencils + si];
                    }

                }
            }
        }
    }
}
extern "C"

void construct_laplacian(
    float * df,
    const float * dfx,
    const float * dfy,
    const float * dfz,
    const unsigned int ncells,
    const float d_diffcon)
{
    #pragma HLS INTERFACE m_axi port=df offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dfx offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dfy offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=dfz offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=ncells
    #pragma HLS INTERFACE s_axilite port=d_diffcon
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int index = _bid_x * BLOCK_DIM_X + _tid_x;
                    int stride = BLOCK_DIM_X * GRID_DIM_X;

                    for(int i = index; i < ncells; i += stride) {
                    df[i] = d_diffcon * (dfx[i] + dfy[i] + dfz[i]);
                    }

                }
            }
        }
    }
}
extern "C"

void update(
    float * x,
    float * y,
    const float * ddx,
    const float * ddy,
    const float * drx,
    const float * dry,
    const unsigned int ncells,
    const float d_dt)
{
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=ddx offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=ddy offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=drx offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=dry offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=ncells
    #pragma HLS INTERFACE s_axilite port=d_dt
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int index = _bid_x * BLOCK_DIM_X + _tid_x;
                    int stride = BLOCK_DIM_X * GRID_DIM_X;

                    for(int i = index; i < ncells; i += stride) {
                    x[i] += (ddx[i] + drx[i]) * d_dt;
                    y[i] += (ddy[i] + dry[i]) * d_dt;
                    }

                }
            }
        }
    }
}
