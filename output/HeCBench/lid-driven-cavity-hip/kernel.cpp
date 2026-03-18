#include "kernel.h"

// --- from main.cu ---
extern "C"
void set_BCs (Real* u, Real* v) 
{
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int ind = (_bid_x * BLOCK_DIM_X) + _tid_x + 1;

                    // left boundary
                    u(0, ind) = ZERO;
                    v(0, ind) = -v(1, ind);

                    // right boundary
                    u(NUM, ind) = ZERO;
                    v(NUM + 1, ind) = -v(NUM, ind);

                    // bottom boundary
                    u(ind, 0) = -u(ind, 1);
                    v(ind, 0) = ZERO;

                    // top boundary
                    u(ind, NUM + 1) = TWO - u(ind, NUM);
                    v(ind, NUM) = ZERO;

                    if (ind == NUM) {
                    // left boundary
                    u(0, 0) = ZERO;
                    v(0, 0) = -v(1, 0);
                    u(0, NUM + 1) = ZERO;
                    v(0, NUM + 1) = -v(1, NUM + 1);

                    // right boundary
                    u(NUM, 0) = ZERO;
                    v(NUM + 1, 0) = -v(NUM, 0);
                    u(NUM, NUM + 1) = ZERO;
                    v(NUM + 1, NUM + 1) = -v(NUM, NUM + 1);

                    // bottom boundary
                    u(0, 0) = -u(0, 1);
                    v(0, 0) = ZERO;
                    u(NUM + 1, 0) = -u(NUM + 1, 1);
                    v(NUM + 1, 0) = ZERO;

                    // top boundary
                    u(0, NUM + 1) = TWO - u(0, NUM);
                    v(0, NUM) = ZERO;
                    u(NUM + 1, NUM + 1) = TWO - u(NUM + 1, NUM);
                    v(ind, NUM + 1) = ZERO;
                    } // end if


                }
            }
        }
    }
} // end set_BCs
extern "C"

void calculate_F (const Real dt,
                  const Real* u,
                  const Real* v,
                        Real* F) 
{
    #pragma HLS INTERFACE s_axilite port=dt
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=F offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int row = (_bid_x * BLOCK_DIM_X) + _tid_x + 1;
                    int col = (_bid_y * BLOCK_DIM_Y) + _tid_y + 1;

                    if (col == NUM) {
                    // right boundary, F_ij = u_ij
                    // also do left boundary
                    F(0, row) = u(0, row);
                    F(NUM, row) = u(NUM, row);
                    } else {

                    // u velocities
                    Real u_ij = u(col, row);
                    Real u_ip1j = u(col + 1, row);
                    Real u_ijp1 = u(col, row + 1);
                    Real u_im1j = u(col - 1, row);
                    Real u_ijm1 = u(col, row - 1);

                    // v velocities
                    Real v_ij = v(col, row);
                    Real v_ip1j = v(col + 1, row);
                    Real v_ijm1 = v(col, row - 1);
                    Real v_ip1jm1 = v(col + 1, row - 1);

                    // finite differences
                    Real du2dx, duvdy, d2udx2, d2udy2;

                    du2dx = (((u_ij + u_ip1j) * (u_ij + u_ip1j) - (u_im1j + u_ij) * (u_im1j + u_ij))
                    + mix_param * (fabs(u_ij + u_ip1j) * (u_ij - u_ip1j)
                    - fabs(u_im1j + u_ij) * (u_im1j - u_ij))) / (FOUR * dx);
                    duvdy = ((v_ij + v_ip1j) * (u_ij + u_ijp1) - (v_ijm1 + v_ip1jm1) * (u_ijm1 + u_ij)
                    + mix_param * (fabs(v_ij + v_ip1j) * (u_ij - u_ijp1)
                    - fabs(v_ijm1 + v_ip1jm1) * (u_ijm1 - u_ij))) / (FOUR * dy);
                    d2udx2 = (u_ip1j - (TWO * u_ij) + u_im1j) / (dx * dx);
                    d2udy2 = (u_ijp1 - (TWO * u_ij) + u_ijm1) / (dy * dy);

                    F(col, row) = u_ij + dt * (((d2udx2 + d2udy2) / Re_num) - du2dx - duvdy + gx);

                    } // end if


                }
            }
        }
    }
} // end calculate_F
extern "C"

void calculate_G (const Real dt,
                  const Real* u,
                  const Real* v,
                        Real* G) 
{
    #pragma HLS INTERFACE s_axilite port=dt
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=G offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int row = (_bid_x * BLOCK_DIM_X) + _tid_x + 1;
                    int col = (_bid_y * BLOCK_DIM_Y) + _tid_y + 1;

                    if (row == NUM) {
                    // top and bottom boundaries
                    G(col, 0) = v(col, 0);
                    G(col, NUM) = v(col, NUM);

                    } else {

                    // u velocities
                    Real u_ij = u(col, row);
                    Real u_ijp1 = u(col, row + 1);
                    Real u_im1j = u(col - 1, row);
                    Real u_im1jp1 = u(col - 1, row + 1);

                    // v velocities
                    Real v_ij = v(col, row);
                    Real v_ijp1 = v(col, row + 1);
                    Real v_ip1j = v(col + 1, row);
                    Real v_ijm1 = v(col, row - 1);
                    Real v_im1j = v(col - 1, row);

                    // finite differences
                    Real dv2dy, duvdx, d2vdx2, d2vdy2;

                    dv2dy = ((v_ij + v_ijp1) * (v_ij + v_ijp1) - (v_ijm1 + v_ij) * (v_ijm1 + v_ij)
                    + mix_param * (fabs(v_ij + v_ijp1) * (v_ij - v_ijp1)
                    - fabs(v_ijm1 + v_ij) * (v_ijm1 - v_ij))) / (FOUR * dy);
                    duvdx = ((u_ij + u_ijp1) * (v_ij + v_ip1j) - (u_im1j + u_im1jp1) * (v_im1j + v_ij)
                    + mix_param * (fabs(u_ij + u_ijp1) * (v_ij - v_ip1j)
                    - fabs(u_im1j + u_im1jp1) * (v_im1j - v_ij))) / (FOUR * dx);
                    d2vdx2 = (v_ip1j - (TWO * v_ij) + v_im1j) / (dx * dx);
                    d2vdy2 = (v_ijp1 - (TWO * v_ij) + v_ijm1) / (dy * dy);

                    G(col, row) = v_ij + dt * (((d2vdx2 + d2vdy2) / Re_num) - dv2dy - duvdx + gy);

                    } // end if


                }
            }
        }
    }
} // end calculate_G
extern "C"

void sum_pressure (const Real* pres_red,
                   const Real* pres_black, 
                         Real* pres_sum) 
{
    #pragma HLS INTERFACE m_axi port=pres_red offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pres_black offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=pres_sum offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sum_cache complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sum_cache complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int row = (_bid_x * BLOCK_DIM_X) + _tid_x + 1;
                    int col = (_bid_y * BLOCK_DIM_Y) + _tid_y + 1;

                    // shared memory for block's sum
                    Real sum_cache[BLOCK_SIZE];

                    int NUM_2 = NUM >> 1;

                    Real pres_r = pres_red(col, row);
                    Real pres_b = pres_black(col, row);

                    // add squared pressure
                    sum_cache[_tid_x] = (pres_r * pres_r) + (pres_b * pres_b);

                    // synchronize threads in block to ensure all thread values stored

                    // add up values for block
                    int i = BLOCK_SIZE >> 1;
                    while (i != 0) {
                    if (_tid_x < i) {
                    sum_cache[_tid_x] += sum_cache[_tid_x + i];
                    }
                    i >>= 1;
                    }

                    // store block's summed values
                    if (_tid_x == 0) {
                    pres_sum[_bid_y + (GRID_DIM_Y * _bid_x)] = sum_cache[0];
                    }


                }
            }
        }
    }
} // end sum_pressure
extern "C"

void set_horz_pres_BCs (Real* pres_red, Real* pres_black) 
{
    #pragma HLS INTERFACE m_axi port=pres_red offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pres_black offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int col = (_bid_x * BLOCK_DIM_X) + _tid_x + 1;
                    col = (col * 2) - 1;

                    int NUM_2 = NUM >> 1;

                    // p_i,0 = p_i,1
                    pres_black(col, 0) = pres_red(col, 1);
                    pres_red(col + 1, 0) = pres_black(col + 1, 1);

                    // p_i,jmax+1 = p_i,jmax
                    pres_red(col, NUM_2 + 1) = pres_black(col, NUM_2);
                    pres_black(col + 1, NUM_2 + 1) = pres_red(col + 1, NUM_2);


                }
            }
        }
    }
} // end set_horz_pres_BCs
extern "C"

void set_vert_pres_BCs (Real* pres_red, Real* pres_black) 
{
    #pragma HLS INTERFACE m_axi port=pres_red offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pres_black offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int row = (_bid_x * BLOCK_DIM_X) + _tid_x + 1;

                    int NUM_2 = NUM >> 1;

                    // p_0,j = p_1,j
                    pres_black(0, row) = pres_red(1, row);
                    pres_red(0, row) = pres_black(1, row);

                    // p_imax+1,j = p_imax,j
                    pres_black(NUM + 1, row) = pres_red(NUM, row);
                    pres_red(NUM + 1, row) = pres_black(NUM, row);


                }
            }
        }
    }
} // end set_pressure_BCs
extern "C"

void red_kernel (const Real dt,
                 const Real* F, 
                 const Real* G,
                 const Real* pres_black,
                       Real* pres_red) 
{
    #pragma HLS INTERFACE s_axilite port=dt
    #pragma HLS INTERFACE m_axi port=F offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=G offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=pres_black offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=pres_red offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int row = (_bid_x * BLOCK_DIM_X) + _tid_x + 1;
                    int col = (_bid_y * BLOCK_DIM_Y) + _tid_y + 1;

                    int NUM_2 = NUM >> 1;

                    Real p_ij = pres_red(col, row);

                    Real p_im1j = pres_black(col - 1, row);
                    Real p_ip1j = pres_black(col + 1, row);
                    Real p_ijm1 = pres_black(col, row - (col & 1));
                    Real p_ijp1 = pres_black(col, row + ((col + 1) & 1));

                    // right-hand side
                    Real rhs = (((F(col, (2 * row) - (col & 1))
                    - F(col - 1, (2 * row) - (col & 1))) / dx)
                    + ((G(col, (2 * row) - (col & 1))
                    - G(col, (2 * row) - (col & 1) - 1)) / dy)) / dt;

                    pres_red(col, row) = p_ij * (ONE - omega) + omega *
                    (((p_ip1j + p_im1j) / (dx * dx)) + ((p_ijp1 + p_ijm1) / (dy * dy)) -
                    rhs) / ((TWO / (dx * dx)) + (TWO / (dy * dy)));


                }
            }
        }
    }
} // end red_kernel
extern "C"

void black_kernel (const Real dt,
                   const Real* F, 
                   const Real* G,
                   const Real* pres_red, 
                         Real* pres_black) 
{
    #pragma HLS INTERFACE s_axilite port=dt
    #pragma HLS INTERFACE m_axi port=F offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=G offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=pres_red offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=pres_black offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int row = (_bid_x * BLOCK_DIM_X) + _tid_x + 1;
                    int col = (_bid_y * BLOCK_DIM_Y) + _tid_y + 1;

                    int NUM_2 = NUM >> 1;

                    Real p_ij = pres_black(col, row);

                    Real p_im1j = pres_red(col - 1, row);
                    Real p_ip1j = pres_red(col + 1, row);
                    Real p_ijm1 = pres_red(col, row - ((col + 1) & 1));
                    Real p_ijp1 = pres_red(col, row + (col & 1));

                    // right-hand side
                    Real rhs = (((F(col, (2 * row) - ((col + 1) & 1))
                    - F(col - 1, (2 * row) - ((col + 1) & 1))) / dx)
                    + ((G(col, (2 * row) - ((col + 1) & 1))
                    - G(col, (2 * row) - ((col + 1) & 1) - 1)) / dy)) / dt;

                    pres_black(col, row) = p_ij * (ONE - omega) + omega *
                    (((p_ip1j + p_im1j) / (dx * dx)) + ((p_ijp1 + p_ijm1) / (dy * dy)) -
                    rhs) / ((TWO / (dx * dx)) + (TWO / (dy * dy)));


                }
            }
        }
    }
} // end black_kernel
extern "C"

void calc_residual (const Real dt,
                    const Real* F,
                    const Real* G, 
                    const Real* pres_red,
                    const Real* pres_black,
                          Real* res_array)
{
    #pragma HLS INTERFACE s_axilite port=dt
    #pragma HLS INTERFACE m_axi port=F offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=G offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=pres_red offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=pres_black offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=res_array offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sum_cache complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sum_cache complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int row = (_bid_x * BLOCK_DIM_X) + _tid_x + 1;
                    int col = (_bid_y * BLOCK_DIM_Y) + _tid_y + 1;

                    int NUM_2 = NUM >> 1;

                    Real p_ij, p_im1j, p_ip1j, p_ijm1, p_ijp1, rhs, res, res2;

                    // red point
                    p_ij = pres_red(col, row);

                    p_im1j = pres_black(col - 1, row);
                    p_ip1j = pres_black(col + 1, row);
                    p_ijm1 = pres_black(col, row - (col & 1));
                    p_ijp1 = pres_black(col, row + ((col + 1) & 1));

                    rhs = (((F(col, (2 * row) - (col & 1)) - F(col - 1, (2 * row) - (col & 1))) / dx)
                    +  ((G(col, (2 * row) - (col & 1)) - G(col, (2 * row) - (col & 1) - 1)) / dy)) / dt;

                    // calculate residual
                    res = ((p_ip1j - (TWO * p_ij) + p_im1j) / (dx * dx))
                    + ((p_ijp1 - (TWO * p_ij) + p_ijm1) / (dy * dy)) - rhs;

                    // black point
                    p_ij = pres_black(col, row);

                    p_im1j = pres_red(col - 1, row);
                    p_ip1j = pres_red(col + 1, row);
                    p_ijm1 = pres_red(col, row - ((col + 1) & 1));
                    p_ijp1 = pres_red(col, row + (col & 1));

                    // right-hand side
                    rhs = (((F(col, (2 * row) - ((col + 1) & 1)) - F(col - 1, (2 * row) - ((col + 1) & 1))) / dx)
                    +  ((G(col, (2 * row) - ((col + 1) & 1)) - G(col, (2 * row) - ((col + 1) & 1) - 1)) / dy)) / dt;

                    // calculate residual
                    res2 = ((p_ip1j - (TWO * p_ij) + p_im1j) / (dx * dx))
                    + ((p_ijp1 - (TWO * p_ij) + p_ijm1) / (dy * dy)) - rhs;

                    // shared memory for block's sum
                    Real sum_cache[BLOCK_SIZE];

                    sum_cache[_tid_x] = (res * res) + (res2 * res2);

                    // synchronize threads in block to ensure all residuals stored

                    // add up squared residuals for block
                    int i = BLOCK_SIZE >> 1;
                    while (i != 0) {
                    if (_tid_x < i) {
                    sum_cache[_tid_x] += sum_cache[_tid_x + i];
                    }
                    i >>= 1;
                    }

                    // store block's summed residuals
                    if (_tid_x == 0) {
                    res_array[_bid_y + (GRID_DIM_Y * _bid_x)] = sum_cache[0];
                    }

                }
            }
        }
    }
} 
extern "C"

void calculate_u (const Real dt,
                  const Real* F, 
                  const Real* pres_red,
                  const Real* pres_black, 
                        Real* u,
                        Real* max_u)
{
    #pragma HLS INTERFACE s_axilite port=dt
    #pragma HLS INTERFACE m_axi port=F offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pres_red offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=pres_black offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=max_u offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=max_cache complete dim=1
    #pragma HLS ARRAY_PARTITION variable=max_cache complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int row = (_bid_x * BLOCK_DIM_X) + _tid_x + 1;
                    int col = (_bid_y * BLOCK_DIM_Y) + _tid_y + 1;

                    // allocate shared memory to store max velocities
                    Real max_cache[BLOCK_SIZE];
                    max_cache[_tid_x] = ZERO;

                    int NUM_2 = NUM >> 1;
                    Real new_u = ZERO;

                    if (col != NUM) {

                    Real p_ij, p_ip1j, new_u2;

                    // red point
                    p_ij = pres_red(col, row);
                    p_ip1j = pres_black(col + 1, row);

                    new_u = F(col, (2 * row) - (col & 1)) - (dt * (p_ip1j - p_ij) / dx);
                    u(col, (2 * row) - (col & 1)) = new_u;

                    // black point
                    p_ij = pres_black(col, row);
                    p_ip1j = pres_red(col + 1, row);

                    new_u2 = F(col, (2 * row) - ((col + 1) & 1)) - (dt * (p_ip1j - p_ij) / dx);
                    u(col, (2 * row) - ((col + 1) & 1)) = new_u2;

                    // check for max of these two
                    new_u = fmax(fabs(new_u), fabs(new_u2));

                    if ((2 * row) == NUM) {
                    // also test for max velocity at vertical boundary
                    new_u = fmax(new_u, fabs( u(col, NUM + 1) ));
                    }
                    } else {
                    // check for maximum velocity in boundary cells also
                    new_u = fmax(fabs( u(NUM, (2 * row)) ), fabs( u(0, (2 * row)) ));
                    new_u = fmax(fabs( u(NUM, (2 * row) - 1) ), new_u);
                    new_u = fmax(fabs( u(0, (2 * row) - 1) ), new_u);

                    new_u = fmax(fabs( u(NUM + 1, (2 * row)) ), new_u);
                    new_u = fmax(fabs( u(NUM + 1, (2 * row) - 1) ), new_u);

                    } // end if

                    // store maximum u for block from each thread
                    max_cache[_tid_x] = new_u;

                    // synchronize threads in block to ensure all velocities stored

                    // calculate maximum for block
                    int i = BLOCK_SIZE >> 1;
                    while (i != 0) {
                    if (_tid_x < i) {
                    max_cache[_tid_x] = fmax(max_cache[_tid_x], max_cache[_tid_x + i]);
                    }
                    i >>= 1;
                    }

                    // store block's maximum
                    if (_tid_x == 0) {
                    max_u[_bid_y + (GRID_DIM_Y * _bid_x)] = max_cache[0];
                    }

                }
            }
        }
    }
} // end calculate_u
extern "C"

void calculate_v (const Real dt,
                  const Real* G, 
                  const Real* pres_red,
                  const Real* pres_black, 
                        Real* v,
                        Real* max_v)
{
    #pragma HLS INTERFACE s_axilite port=dt
    #pragma HLS INTERFACE m_axi port=G offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pres_red offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=pres_black offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=max_v offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=max_cache complete dim=1
    #pragma HLS ARRAY_PARTITION variable=max_cache complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int row = (_bid_x * BLOCK_DIM_X) + _tid_x + 1;
                    int col = (_bid_y * BLOCK_DIM_Y) + _tid_y + 1;

                    // allocate shared memory to store maximum velocities
                    Real max_cache[BLOCK_SIZE];
                    max_cache[_tid_x] = ZERO;

                    int NUM_2 = NUM >> 1;
                    Real new_v = ZERO;

                    if (row != NUM_2) {
                    Real p_ij, p_ijp1, new_v2;

                    // red pressure point
                    p_ij = pres_red(col, row);
                    p_ijp1 = pres_black(col, row + ((col + 1) & 1));

                    new_v = G(col, (2 * row) - (col & 1)) - (dt * (p_ijp1 - p_ij) / dy);
                    v(col, (2 * row) - (col & 1)) = new_v;

                    // black pressure point
                    p_ij = pres_black(col, row);
                    p_ijp1 = pres_red(col, row + (col & 1));

                    new_v2 = G(col, (2 * row) - ((col + 1) & 1)) - (dt * (p_ijp1 - p_ij) / dy);
                    v(col, (2 * row) - ((col + 1) & 1)) = new_v2;

                    // check for max of these two
                    new_v = fmax(fabs(new_v), fabs(new_v2));

                    if (col == NUM) {
                    // also test for max velocity at vertical boundary
                    new_v = fmax(new_v, fabs( v(NUM + 1, (2 * row)) ));
                    }

                    } else {

                    if ((col & 1) == 1) {
                    // black point is on boundary, only calculate red point below it
                    Real p_ij = pres_red(col, row);
                    Real p_ijp1 = pres_black(col, row + ((col + 1) & 1));

                    new_v = G(col, (2 * row) - (col & 1)) - (dt * (p_ijp1 - p_ij) / dy);
                    v(col, (2 * row) - (col & 1)) = new_v;
                    } else {
                    // red point is on boundary, only calculate black point below it
                    Real p_ij = pres_black(col, row);
                    Real p_ijp1 = pres_red(col, row + (col & 1));

                    new_v = G(col, (2 * row) - ((col + 1) & 1)) - (dt * (p_ijp1 - p_ij) / dy);
                    v(col, (2 * row) - ((col + 1) & 1)) = new_v;
                    }

                    // get maximum v velocity
                    new_v = fabs(new_v);

                    // check for maximum velocity in boundary cells also
                    new_v = fmax(fabs( v(col, NUM) ), new_v);
                    new_v = fmax(fabs( v(col, 0) ), new_v);

                    new_v = fmax(fabs( v(col, NUM + 1) ), new_v);
                    } // end if

                    // store absolute value of velocity
                    max_cache[_tid_x] = new_v;

                    // synchronize threads in block to ensure all velocities stored

                    // calculate maximum for block
                    int i = BLOCK_SIZE >> 1;
                    while (i != 0) {
                    if (_tid_x < i) {
                    max_cache[_tid_x] = fmax(max_cache[_tid_x], max_cache[_tid_x + i]);
                    }
                    i >>= 1;
                    }

                    // store block's summed residuals
                    if (_tid_x == 0) {
                    max_v[_bid_y + (GRID_DIM_Y * _bid_x)] = max_cache[0];
                    }

                }
            }
        }
    }
} // end calculate_v
