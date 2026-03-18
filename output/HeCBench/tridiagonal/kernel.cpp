#include "kernel.h"

// --- from cyclic_kernels.cu ---
extern "C"
void cyclic_branch_free_kernel(
    const float* a_d, 
    const float* b_d, 
    const float* c_d, 
    const float* d_d, 
          float* x_d, 
    const int system_size, 
    const int num_systems, 
    const int iterations)
{
    #pragma HLS INTERFACE m_axi port=a_d offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b_d offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c_d offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_d offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=x_d offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=system_size
    #pragma HLS INTERFACE s_axilite port=num_systems
    #pragma HLS INTERFACE s_axilite port=iterations
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float shared[4096];

                    int thid = _tid_x;
                    int blid = _bid_x;

                    int stride = 1;
                    int half_size = system_size >> 1;
                    int thid_num = half_size;

                    float* a = shared;
                    float* b = &a[system_size];
                    float* c = &b[system_size];
                    float* d = &c[system_size];
                    float* x = &d[system_size];

                    a[thid] = a_d[thid + blid * system_size];
                    a[thid + thid_num] = a_d[thid + thid_num + blid * system_size];

                    b[thid] = b_d[thid + blid * system_size];
                    b[thid + thid_num] = b_d[thid + thid_num + blid * system_size];

                    c[thid] = c_d[thid + blid * system_size];
                    c[thid + thid_num] = c_d[thid + thid_num + blid * system_size];

                    d[thid] = d_d[thid + blid * system_size];
                    d[thid + thid_num] = d_d[thid + thid_num + blid * system_size];

                    // forward elimination
                    for (int j = 0; j < iterations; j++)
                    {

                    stride <<= 1;
                    int delta = stride >> 1;
                    if (thid < thid_num)
                    {
                    int i = stride * thid + stride - 1;
                    int iRight = i+delta;
                    iRight = iRight & (system_size-1);
                    #ifndef NATIVE_DIVIDE
                    float tmp1 = a[i] / b[i-delta];
                    float tmp2 = c[i] / b[iRight];
                    #else
                    float tmp1 = __fdiv_rn(a[i], b[i-delta]);
                    float tmp2 = __fdiv_rn(c[i], b[iRight]);
                    #endif
                    b[i] = b[i] - c[i-delta] * tmp1 - a[iRight] * tmp2;
                    d[i] = d[i] - d[i-delta] * tmp1 - d[iRight] * tmp2;
                    a[i] = -a[i-delta] * tmp1;
                    c[i] = -c[iRight]  * tmp2;
                    }

                    thid_num >>= 1;
                    }

                    if (thid < 2)
                    {
                    int addr1 = stride - 1;
                    int addr2 = (stride << 1) - 1;
                    float tmp3 = b[addr2] * b[addr1] - c[addr1] * a[addr2];
                    #ifndef NATIVE_DIVIDE
                    x[addr1] = (b[addr2] * d[addr1] - c[addr1] * d[addr2]) / tmp3;
                    x[addr2] = (d[addr2] * b[addr1] - d[addr1] * a[addr2]) / tmp3;
                    #else
                    x[addr1] = __fdiv_rn((b[addr2] * d[addr1] - c[addr1] * d[addr2]), tmp3);
                    x[addr2] = __fdiv_rn((d[addr2] * b[addr1] - d[addr1] * a[addr2]), tmp3);
                    #endif
                    }

                    // backward substitution
                    thid_num = 2;
                    for (int j = 0; j < iterations; j++)
                    {
                    int delta = stride >> 1;
                    if (thid < thid_num)
                    {
                    int i = stride * thid + (stride >> 1) - 1;
                    #ifndef NATIVE_DIVIDE
                    if (i == delta - 1)
                    x[i] = (d[i] - c[i] * x[i+delta]) / b[i];
                    else
                    x[i] = (d[i] - a[i] * x[i-delta] - c[i] * x[i+delta]) / b[i];
                    #else
                    if (i == delta - 1)
                    x[i] = __fdiv_rn((d[i] - c[i] * x[i+delta]), b[i]);
                    else
                    x[i] = __fdiv_rn((d[i] - a[i] * x[i-delta] - c[i] * x[i+delta]), b[i]);
                    #endif
                    }
                    stride >>= 1;
                    thid_num <<= 1;
                    }

                    x_d[thid + blid * system_size] = x[thid];
                    x_d[thid + half_size + blid * system_size] = x[thid + half_size];

                }
            }
        }
    }
}


// --- from pcr_kernels.cu ---
extern "C"
void pcr_branch_free_kernel(
    const float* a_d, 
    const float* b_d, 
    const float* c_d, 
    const float* d_d, 
          float* x_d, 
    const int system_size, 
    const int num_systems, 
    const int iterations)
{
    #pragma HLS INTERFACE m_axi port=a_d offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b_d offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c_d offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_d offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=x_d offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=system_size
    #pragma HLS INTERFACE s_axilite port=num_systems
    #pragma HLS INTERFACE s_axilite port=iterations
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float shared[4096];

                    int thid = _tid_x;
                    int blid = _bid_x;

                    int delta = 1;

                    float* a = shared;
                    float* b = &a[system_size+1];
                    float* c = &b[system_size+1];
                    float* d = &c[system_size+1];
                    float* x = &d[system_size+1];

                    a[thid] = a_d[thid + blid * system_size];
                    b[thid] = b_d[thid + blid * system_size];
                    c[thid] = c_d[thid + blid * system_size];
                    d[thid] = d_d[thid + blid * system_size];

                    float aNew, bNew, cNew, dNew;

                    // parallel cyclic reduction
                    for (int j = 0; j < iterations; j++)
                    {
                    int i = thid;

                    int iRight = i+delta;
                    iRight = iRight & (system_size-1);

                    int iLeft = i-delta;
                    iLeft = iLeft & (system_size-1);

                    #ifndef NATIVE_DIVIDE
                    float tmp1 = a[i] / b[iLeft];
                    float tmp2 = c[i] / b[iRight];
                    #else
                    float tmp1 = __fdiv_rn(a[i], b[iLeft]);
                    float tmp2 = __fdiv_rn(c[i], b[iRight]);
                    #endif

                    bNew = b[i] - c[iLeft] * tmp1 - a[iRight] * tmp2;
                    dNew = d[i] - d[iLeft] * tmp1 - d[iRight] * tmp2;
                    aNew = -a[iLeft] * tmp1;
                    cNew = -c[iRight] * tmp2;

                    b[i] = bNew;
                    d[i] = dNew;
                    a[i] = aNew;
                    c[i] = cNew;

                    delta *= 2;
                    }

                    if (thid < delta)
                    {
                    int addr1 = thid;
                    int addr2 = thid + delta;
                    float tmp3 = b[addr2] * b[addr1] - c[addr1] * a[addr2];
                    #ifndef NATIVE_DIVIDE
                    x[addr1] = (b[addr2] * d[addr1] - c[addr1] * d[addr2]) / tmp3;
                    x[addr2] = (d[addr2] * b[addr1] - d[addr1] * a[addr2]) / tmp3;
                    #else
                    x[addr1] = __fdiv_rn((b[addr2] * d[addr1] - c[addr1] * d[addr2]), tmp3);
                    x[addr2] = __fdiv_rn((d[addr2] * b[addr1] - d[addr1] * a[addr2]), tmp3);
                    #endif
                    }

                    x_d[thid + blid * system_size] = x[thid];

                }
            }
        }
    }
}


// --- from sweep_kernels.cu ---
inline int getLocalIdx(int i, int k, int num_systems)
{
  return i + num_systems * k;

  // uncomment for uncoalesced mem access
  // return k + system_size * i;
}
extern "C"

void sweep_small_systems_global_kernel(
    const float* a_d, 
    const float* b_d, 
    const float* c_d, 
    const float* d_d, 
          float* x_d, 
          float* w_d, 
    const int system_size, 
    const int num_systems,
    const bool reorder)
{
    #pragma HLS INTERFACE m_axi port=a_d offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b_d offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c_d offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_d offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=x_d offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=w_d offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=system_size
    #pragma HLS INTERFACE s_axilite port=num_systems
    #pragma HLS INTERFACE s_axilite port=reorder
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int i = _bid_x * BLOCK_DIM_X + _tid_x;

                    // need to check for in-bounds because of the thread block size
                    if (i >= num_systems) return;

                    int stride = reorder ? num_systems: 1;
                    int base_idx = reorder ? i : i * system_size;

                    float c1, c2, c3;
                    float f_i, x_prev, x_next;

                    // solving next system:
                    // c1 * u_i+1 + c2 * u_i + c3 * u_i-1 = f_i

                    c1 = c_d[base_idx];
                    c2 = b_d[base_idx];
                    f_i = d_d[base_idx];

                    #ifndef NATIVE_DIVIDE
                    w_d[getLocalIdx(i, 1, num_systems)] = - c1 / c2;
                    x_prev = f_i / c2;
                    #else
                    w_d[getLocalIdx(i, 1, num_systems)] = __fdiv_rn(-c1, c2);
                    x_prev = __fdiv_rn(f_i, c2);
                    #endif

                    // forward trace
                    int idx = base_idx;
                    x_d[base_idx] = x_prev;
                    for (int k = 1; k < system_size-1; k++)
                    {
                    idx += stride;

                    c1 = c_d[idx];
                    c2 = b_d[idx];
                    c3 = a_d[idx];
                    f_i = d_d[idx];

                    float q = (c3 * w_d[getLocalIdx(i, k, num_systems)] + c2);
                    #ifndef NATIVE_DIVIDE
                    float t = 1 / q;
                    #else
                    float t = __frcp_rn(q);
                    #endif
                    x_next = (f_i - c3 * x_prev) * t;
                    x_d[idx] = x_prev = x_next;

                    w_d[getLocalIdx(i, k+1, num_systems)] = - c1 * t;
                    }

                    idx += stride;

                    c2 = b_d[idx];
                    c3 = a_d[idx];
                    f_i = d_d[idx];

                    float q = (c3 * w_d[getLocalIdx(i, system_size-1, num_systems)] + c2);
                    #ifndef NATIVE_DIVIDE
                    float t = 1 / q;
                    #else
                    float t = __frcp_rn(q);
                    #endif
                    x_next = (f_i - c3 * x_prev) * t;
                    x_d[idx] = x_prev = x_next;

                    // backward trace
                    for (int k = system_size-2; k >= 0; k--)
                    {
                    idx -= stride;
                    x_next = x_d[idx];
                    x_next += x_prev * w_d[getLocalIdx(i, k+1, num_systems)];
                    x_d[idx] = x_prev = x_next;
                    }

                }
            }
        }
    }
}

inline float4 load(const float* a, int i)
{
  return float4(a[i], a[i+1], a[i+2], a[i+3]);
}

inline void store(float* a, int i, float4 v)
{
  a[i] = v.x;
  a[i+1] = v.y;
  a[i+2] = v.z;
  a[i+3] = v.w;
}
extern "C"

void sweep_small_systems_global_vec4_kernel(
    const float* a_d, 
    const float* b_d, 
    const float* c_d, 
    const float* d_d, 
          float* x_d, 
          float* w_d, 
    const int system_size, 
    const int num_systems,
    const bool reorder)
{
    #pragma HLS INTERFACE m_axi port=a_d offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b_d offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c_d offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_d offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=x_d offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=w_d offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=system_size
    #pragma HLS INTERFACE s_axilite port=num_systems
    #pragma HLS INTERFACE s_axilite port=reorder
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int j = _bid_x * BLOCK_DIM_X + _tid_x;
                    int i = j << 2;

                    // need to check for in-bounds because of the thread block size
                    if (i >= num_systems) return;

                    int stride = reorder ? num_systems: 4;
                    int base_idx = reorder ? i : i * system_size;

                    float4 c1, c2, c3;
                    float4 f_i, x_prev, x_next;

                    // solving next system:
                    // c1 * u_i+1 + c2 * u_i + c3 * u_i-1 = f_i

                    c1 = load(c_d, base_idx);
                    c2 = load(b_d, base_idx);
                    f_i = load(d_d, base_idx);

                    #ifndef NATIVE_DIVIDE
                    store(w_d, getLocalIdx(i, 1, num_systems), - c1 / c2);
                    x_prev = f_i / c2;
                    #else
                    store(w_d, getLocalIdx(i, 1, num_systems), native_divide(-c1, c2));
                    x_prev = native_divide(f_i, c2);
                    #endif

                    // forward trace
                    int idx = base_idx;
                    store(x_d, base_idx, x_prev);
                    for (int k = 1; k < system_size-1; k++)
                    {
                    idx += stride;

                    c1 = load(c_d, idx);
                    c2 = load(b_d, idx);
                    c3 = load(a_d, idx);
                    f_i = load(d_d, idx);

                    float4 q = (c3 * load(w_d, getLocalIdx(i, k, num_systems)) + c2);
                    #ifndef NATIVE_DIVIDE
                    float4 t = float4(1,1,1,1) / q;
                    #else
                    float4 t = native_recip(q);
                    #endif
                    x_next = (f_i - c3 * x_prev) * t;
                    x_prev = x_next;
                    store(x_d, idx, x_prev);

                    store(w_d, getLocalIdx(i, k+1, num_systems), - c1 * t);
                    }

                    idx += stride;

                    c2 = load(b_d, idx);
                    c3 = load(a_d, idx);
                    f_i = load(d_d, idx);

                    float4 q = (c3 * load(w_d, getLocalIdx(i, system_size-1, num_systems)) + c2);
                    #ifndef NATIVE_DIVIDE
                    float4 t = float4(1,1,1,1) / q;
                    #else
                    float4 t = native_recip(q);
                    #endif
                    x_next = (f_i - c3 * x_prev) * t;
                    x_prev = x_next;
                    store(x_d, idx, x_prev);

                    // backward trace
                    for (int k = system_size-2; k >= 0; k--)
                    {
                    idx -= stride;
                    x_next = load(x_d, idx);
                    x_next += x_prev * load(w_d, getLocalIdx(i, k+1, num_systems));
                    x_prev = x_next;
                    store(x_d, idx, x_prev);
                    }

                }
            }
        }
    }
}
extern "C"

void transpose(
          float* odata, 
    const float* idata, 
    const int width, 
    const int height) 
{
    #pragma HLS INTERFACE m_axi port=odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float block[TRANSPOSE_BLOCK_DIM * (TRANSPOSE_BLOCK_DIM+1)];

                    int blockIdxx = _bid_x;
                    int blockIdxy = _bid_y;

                    int threadIdxx = _tid_x;
                    int threadIdxy = _tid_y;

                    // evaluate coordinates and check bounds
                    int i0 = __mul24(blockIdxx, BLOCK_DIM) + threadIdxx;
                    int j0 = __mul24(blockIdxy, BLOCK_DIM) + threadIdxy;

                    if (i0 >= width || j0 >= height) return;

                    int i1 = __mul24(blockIdxy, BLOCK_DIM) + threadIdxx;
                    int j1 = __mul24(blockIdxx, BLOCK_DIM) + threadIdxy;

                    if (i1 >= height || j1 >= width) return;

                    int idx_a = i0 + __mul24(j0, width);
                    int idx_b = i1 + __mul24(j1, height);

                    // read the tile from global memory into shared memory
                    block[threadIdxy * (BLOCK_DIM+1) + threadIdxx] = idata[idx_a];

                    // write back to transposed array
                    odata[idx_b] = block[threadIdxx * (BLOCK_DIM+1) + threadIdxy];

                }
            }
        }
    }
}
