#include "kernel.h"

// --- from main.cu ---
extern "C"
void surfel_render(
  const T * s,
  int N,
  T f,
  int w,
  int h,
  T * d)
{
    #pragma HLS INTERFACE m_axi port=s offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=f
    #pragma HLS INTERFACE s_axilite port=w
    #pragma HLS INTERFACE s_axilite port=h
    #pragma HLS INTERFACE m_axi port=d offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const int x = _tid_x + _bid_x*BLOCK_DIM_X;
                    const int y = _tid_y + _bid_y*BLOCK_DIM_Y;

                    if(x < w && y < h)
                    {
                    T ray[3];
                    ray[0] = T(x)-(w-1)*(T)0.5;
                    ray[1] = T(y)-(h-1)*(T)0.5;
                    ray[2] = f;
                    T pt[3];
                    T n[3];
                    T p[3];
                    T dMin = 1e20;

                    for (int i=0; i<N; ++i) {
                    p[0] = s[i*COL_DIM+COL_P_X];
                    p[1] = s[i*COL_DIM+COL_P_Y];
                    p[2] = s[i*COL_DIM+COL_P_Z];
                    n[0] = s[i*COL_DIM+COL_N_X];
                    n[1] = s[i*COL_DIM+COL_N_Y];
                    n[2] = s[i*COL_DIM+COL_N_Z];
                    T rSqMax = s[i*COL_DIM+COL_RSq];
                    T pDotn = p[0]*n[0]+p[1]*n[1]+p[2]*n[2];
                    T dsDotRay = ray[0]*n[0] + ray[1]*n[1] + ray[2]*n[2];
                    T alpha = pDotn / dsDotRay;
                    pt[0] = ray[0]*alpha - p[0];
                    pt[1] = ray[1]*alpha - p[1];
                    pt[2] = ray[2]*alpha - p[2];
                    T t = ray[2]*alpha;
                    T rSq = pt[0] * pt[0] + pt[1] * pt[1] + pt[2] * pt[2];
                    if (rSq < rSqMax && dMin > t) {
                    dMin = t; // ray hit the surfel
                    }
                    }
                    d[y*w+x] = dMin > (T)100 ? (T)0 : dMin;
                    }

                }
            }
        }
    }
}
extern "C"

void surfel_render_tile(
   const T * s,
   int N,
   T f,
   int w,
   int h,
   T * d)
{
    #pragma HLS INTERFACE m_axi port=s offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=f
    #pragma HLS INTERFACE s_axilite port=w
    #pragma HLS INTERFACE s_axilite port=h
    #pragma HLS INTERFACE m_axi port=d offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sh complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const int x = _bid_x * BLOCK_DIM_X + _tid_x;
                    const int y = _bid_y * BLOCK_DIM_Y + _tid_y;

                    if (x >= w || y >= h) return;

                    // Camera ray
                    T rayx = T(x) - (w - 1) * T(0.5);
                    T rayy = T(y) - (h - 1) * T(0.5);
                    T rayz = f;

                    T dMin = 1e20;

                    // Shared memory for surfels
                    T sh[TILE * COL_DIM];

                    for (int base = 0; base < N; base += TILE) {

                    int tid = _tid_y * BLOCK_DIM_X + _tid_x;
                    if (tid < TILE && base + tid < N) {
                    #pragma unroll
                    for (int k = 0; k < COL_DIM; ++k) {
                    sh[tid * COL_DIM + k] = s[(base + tid) * COL_DIM + k];
                    }
                    }

                    int tileCount = min(TILE, N - base);

                    #pragma unroll
                    for (int i = 0; i < tileCount; ++i) {

                    T px = sh[i * COL_DIM + COL_P_X];
                    T py = sh[i * COL_DIM + COL_P_Y];
                    T pz = sh[i * COL_DIM + COL_P_Z];

                    T nx = sh[i * COL_DIM + COL_N_X];
                    T ny = sh[i * COL_DIM + COL_N_Y];
                    T nz = sh[i * COL_DIM + COL_N_Z];

                    T rSqMax = sh[i * COL_DIM + COL_RSq];

                    T dsDotRay = rayx * nx + rayy * ny + rayz * nz;
                    T pDotn = px * nx + py * ny + pz * nz;
                    T alpha = pDotn / dsDotRay;
                    T t = rayz * alpha;

                    T dx = rayx * alpha - px;
                    T dy = rayy * alpha - py;
                    T dz = rayz * alpha - pz;

                    T rSq = dx*dx + dy*dy + dz*dz;
                    if (rSq < rSqMax && t < dMin) {
                    dMin = t;
                    }
                    }
                    }

                    d[y * w + x] = (dMin > T(100)) ? T(0) : dMin;

                }
            }
        }
    }
}
