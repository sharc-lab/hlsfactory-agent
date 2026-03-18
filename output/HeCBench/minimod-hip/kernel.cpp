#include "kernel.h"

// --- from minimig.cu ---
extern "C"
void target_inner_3d_kernel(
    llint nx, llint ny, llint nz, int ldimx, int ldimy, int ldimz,
    llint x3, llint x4, llint y3, llint y4, llint z3, llint z4,
    llint lx, llint ly, llint lz,
    float hdx_2, float hdy_2, float hdz_2,
    float coef0,
    float coefx_1, float coefx_2, float coefx_3, float coefx_4,
    float coefy_1, float coefy_2, float coefy_3, float coefy_4,
    float coefz_1, float coefz_2, float coefz_3, float coefz_4,
    const float * u, float * v, const float * vp,
    const float * phi, const float * eta)
{
    #pragma HLS INTERFACE s_axilite port=nx
    #pragma HLS INTERFACE s_axilite port=ny
    #pragma HLS INTERFACE s_axilite port=nz
    #pragma HLS INTERFACE s_axilite port=ldimx
    #pragma HLS INTERFACE s_axilite port=ldimy
    #pragma HLS INTERFACE s_axilite port=ldimz
    #pragma HLS INTERFACE s_axilite port=x3
    #pragma HLS INTERFACE s_axilite port=x4
    #pragma HLS INTERFACE s_axilite port=y3
    #pragma HLS INTERFACE s_axilite port=y4
    #pragma HLS INTERFACE s_axilite port=z3
    #pragma HLS INTERFACE s_axilite port=z4
    #pragma HLS INTERFACE s_axilite port=lx
    #pragma HLS INTERFACE s_axilite port=ly
    #pragma HLS INTERFACE s_axilite port=lz
    #pragma HLS INTERFACE s_axilite port=hdx_2
    #pragma HLS INTERFACE s_axilite port=hdy_2
    #pragma HLS INTERFACE s_axilite port=hdz_2
    #pragma HLS INTERFACE s_axilite port=coef0
    #pragma HLS INTERFACE s_axilite port=coefx_1
    #pragma HLS INTERFACE s_axilite port=coefx_2
    #pragma HLS INTERFACE s_axilite port=coefx_3
    #pragma HLS INTERFACE s_axilite port=coefx_4
    #pragma HLS INTERFACE s_axilite port=coefy_1
    #pragma HLS INTERFACE s_axilite port=coefy_2
    #pragma HLS INTERFACE s_axilite port=coefy_3
    #pragma HLS INTERFACE s_axilite port=coefy_4
    #pragma HLS INTERFACE s_axilite port=coefz_1
    #pragma HLS INTERFACE s_axilite port=coefz_2
    #pragma HLS INTERFACE s_axilite port=coefz_3
    #pragma HLS INTERFACE s_axilite port=coefz_4
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=vp offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=phi offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=eta offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_u complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_u complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            float s_u[NDIM+2*R][NDIM+2*R][NDIM+2*R];

                            const llint i0 = x3 + _bid_z * BLOCK_DIM_Z;
                            const llint j0 = y3 + _bid_y * BLOCK_DIM_Y;
                            const llint k0 = z3 + _bid_x * BLOCK_DIM_X;

                            const int ti = _tid_z;
                            const int tj = _tid_y;
                            const int tk = _tid_x;

                            const llint i = i0 + ti;
                            const llint j = j0 + tj;
                            const llint k = k0 + tk;

                            s_u[ti][tj][tk] = 0.f;

                            if (ti < 2*R && tj < 2*R && tk< 2*R)
                            s_u[NDIM+ti][NDIM+tj][NDIM+tk] = 0.f;

                            const llint sui = ti + R;
                            const llint suj = tj + R;
                            const llint suk = tk + R;

                            const int z_side = ti / R;
                            s_u[ti+z_side*NDIM][suj][suk] = u[IDX3(i+(z_side*2-1)*R,j,k)];
                            const int y_side = tj / R;
                            s_u[sui][tj+y_side*NDIM][suk] = u[IDX3(i,j+(y_side*2-1)*R,k)];
                            s_u[sui][suj][tk] = u[IDX3(i,j,k-R)];
                            s_u[sui][suj][tk+NDIM] = u[IDX3(i,j,k+R)];

                            if (i > x4-1 || j > y4-1 || k > z4-1) { return; }

                            float lap = coef0 * s_u[sui][suj][suk] +
                            coefx_1 * (s_u[sui+1][suj][suk] + s_u[sui-1][suj][suk]) +
                            coefy_1 * (s_u[sui][suj+1][suk] + s_u[sui][suj-1][suk]) +
                            coefz_1 * (s_u[sui][suj][suk+1] + s_u[sui][suj][suk-1]) +
                            coefx_2 * (s_u[sui+2][suj][suk] + s_u[sui-2][suj][suk]) +
                            coefy_2 * (s_u[sui][suj+2][suk] + s_u[sui][suj-2][suk]) +
                            coefz_2 * (s_u[sui][suj][suk+2] + s_u[sui][suj][suk-2]) +
                            coefx_3 * (s_u[sui+3][suj][suk] + s_u[sui-3][suj][suk]) +
                            coefy_3 * (s_u[sui][suj+3][suk] + s_u[sui][suj-3][suk]) +
                            coefz_3 * (s_u[sui][suj][suk+3] + s_u[sui][suj][suk-3]) +
                            coefx_4 * (s_u[sui+4][suj][suk] + s_u[sui-4][suj][suk]) +
                            coefy_4 * (s_u[sui][suj+4][suk] + s_u[sui][suj-4][suk]) +
                            coefz_4 * (s_u[sui][suj][suk+4] + s_u[sui][suj][suk-4]);
                            v[IDX3(i,j,k)] = 2.f * s_u[sui][suj][suk] + vp[IDX3(i,j,k)] * lap - v[IDX3(i,j,k)];

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void target_pml_3d_kernel(
    llint nx, llint ny, llint nz, int ldimx, int ldimy, int ldimz,
    llint x3, llint x4, llint y3, llint y4, llint z3, llint z4,
    llint lx, llint ly, llint lz,
    float hdx_2, float hdy_2, float hdz_2,
    float coef0,
    float coefx_1, float coefx_2, float coefx_3, float coefx_4,
    float coefy_1, float coefy_2, float coefy_3, float coefy_4,
    float coefz_1, float coefz_2, float coefz_3, float coefz_4,
    const float * u, float * v, const float * vp,
    float * phi, const float * eta)
{
    #pragma HLS INTERFACE s_axilite port=nx
    #pragma HLS INTERFACE s_axilite port=ny
    #pragma HLS INTERFACE s_axilite port=nz
    #pragma HLS INTERFACE s_axilite port=ldimx
    #pragma HLS INTERFACE s_axilite port=ldimy
    #pragma HLS INTERFACE s_axilite port=ldimz
    #pragma HLS INTERFACE s_axilite port=x3
    #pragma HLS INTERFACE s_axilite port=x4
    #pragma HLS INTERFACE s_axilite port=y3
    #pragma HLS INTERFACE s_axilite port=y4
    #pragma HLS INTERFACE s_axilite port=z3
    #pragma HLS INTERFACE s_axilite port=z4
    #pragma HLS INTERFACE s_axilite port=lx
    #pragma HLS INTERFACE s_axilite port=ly
    #pragma HLS INTERFACE s_axilite port=lz
    #pragma HLS INTERFACE s_axilite port=hdx_2
    #pragma HLS INTERFACE s_axilite port=hdy_2
    #pragma HLS INTERFACE s_axilite port=hdz_2
    #pragma HLS INTERFACE s_axilite port=coef0
    #pragma HLS INTERFACE s_axilite port=coefx_1
    #pragma HLS INTERFACE s_axilite port=coefx_2
    #pragma HLS INTERFACE s_axilite port=coefx_3
    #pragma HLS INTERFACE s_axilite port=coefx_4
    #pragma HLS INTERFACE s_axilite port=coefy_1
    #pragma HLS INTERFACE s_axilite port=coefy_2
    #pragma HLS INTERFACE s_axilite port=coefy_3
    #pragma HLS INTERFACE s_axilite port=coefy_4
    #pragma HLS INTERFACE s_axilite port=coefz_1
    #pragma HLS INTERFACE s_axilite port=coefz_2
    #pragma HLS INTERFACE s_axilite port=coefz_3
    #pragma HLS INTERFACE s_axilite port=coefz_4
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=vp offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=phi offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=eta offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_u complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_u complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            float s_u[NDIM+2*R][NDIM+2*R][NDIM+2*R];

                            const llint i0 = x3 + _bid_z * BLOCK_DIM_Z;
                            const llint j0 = y3 + _bid_y * BLOCK_DIM_Y;
                            const llint k0 = z3 + _bid_x * BLOCK_DIM_X;

                            const int ti = _tid_z;
                            const int tj = _tid_y;
                            const int tk = _tid_x;

                            const llint i = i0 + ti;
                            const llint j = j0 + tj;
                            const llint k = k0 + tk;

                            s_u[ti][tj][tk] = 0.f;

                            if (ti < 2*R && tj < 2*R && tk< 2*R)
                            s_u[NDIM+ti][NDIM+tj][NDIM+tk] = 0.f;

                            const llint sui = ti + R;
                            const llint suj = tj + R;
                            const llint suk = tk + R;

                            const int z_side = ti / R;
                            s_u[ti+z_side*NDIM][suj][suk] = u[IDX3(i+(z_side*2-1)*R,j,k)];
                            const int y_side = tj / R;
                            s_u[sui][tj+y_side*NDIM][suk] = u[IDX3(i,j+(y_side*2-1)*R,k)];
                            s_u[sui][suj][tk] = u[IDX3(i,j,k-R)];
                            s_u[sui][suj][tk+NDIM] = u[IDX3(i,j,k+R)];

                            if (i > x4-1 || j > y4-1 || k > z4-1) { return; }

                            float lap = coef0 * s_u[sui][suj][suk] +
                            coefx_1 * (s_u[sui+1][suj][suk] + s_u[sui-1][suj][suk]) +
                            coefy_1 * (s_u[sui][suj+1][suk] + s_u[sui][suj-1][suk]) +
                            coefz_1 * (s_u[sui][suj][suk+1] + s_u[sui][suj][suk-1]) +
                            coefx_2 * (s_u[sui+2][suj][suk] + s_u[sui-2][suj][suk]) +
                            coefy_2 * (s_u[sui][suj+2][suk] + s_u[sui][suj-2][suk]) +
                            coefz_2 * (s_u[sui][suj][suk+2] + s_u[sui][suj][suk-2]) +
                            coefx_3 * (s_u[sui+3][suj][suk] + s_u[sui-3][suj][suk]) +
                            coefy_3 * (s_u[sui][suj+3][suk] + s_u[sui][suj-3][suk]) +
                            coefz_3 * (s_u[sui][suj][suk+3] + s_u[sui][suj][suk-3]) +
                            coefx_4 * (s_u[sui+4][suj][suk] + s_u[sui-4][suj][suk]) +
                            coefy_4 * (s_u[sui][suj+4][suk] + s_u[sui][suj-4][suk]) +
                            coefz_4 * (s_u[sui][suj][suk+4] + s_u[sui][suj][suk-4]);

                            const float s_eta_c = eta[IDX3(i,j,k)];

                            v[IDX3(i,j,k)] = ((2.f*s_eta_c + 2.f - s_eta_c*s_eta_c)*s_u[sui][suj][suk] +
                            (vp[IDX3(i,j,k)] * (lap + phi[IDX3(i,j,k)]) - v[IDX3(i,j,k)])) /
                            (2.f*s_eta_c+1.f);

                            phi[IDX3(i,j,k)] =
                            (phi[IDX3(i,j,k)] -
                            ((eta[IDX3(i+1,j,k)]-eta[IDX3(i-1,j,k)]) *
                            (s_u[sui+1][suj][suk]-s_u[sui-1][suj][suk]) * hdx_2 +
                            (eta[IDX3(i,j+1,k)]-eta[IDX3(i,j-1,k)]) *
                            (s_u[sui][suj+1][suk]-s_u[sui][suj-1][suk]) * hdy_2 +
                            (eta[IDX3(i,j,k+1)]-eta[IDX3(i,j,k-1)]) *
                            (s_u[sui][suj][suk+1]-s_u[sui][suj][suk-1]) * hdz_2)) / (1.f + s_eta_c);

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void kernel_add_source_kernel(float *g_u, llint idx, float source) {
    #pragma HLS INTERFACE m_axi port=g_u offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=idx
    #pragma HLS INTERFACE s_axilite port=source
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            g_u[idx] += source;

                        }
                    }
                }
            }
        }
    }
}
