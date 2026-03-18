#include "kernel.h"

// --- from main.cu ---
extern "C"
void firstColGPU(uint32_t *x, int s) {
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=s
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=cx complete dim=1
    #pragma HLS ARRAY_PARTITION variable=cx complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            uint32_t cx[2 * P4];

            uint32_t *px = &cx[P4];
            int myid = _tid_x;
            cx[myid] = x[myid];

            for (int k = 1; k < s / P4; k++) {

            for (int i = 0; i < P4; i += LWDR) {
            if (myid < LWDR) {
            px[i + myid] = px[i + myid - P1] + px[i + myid - P2]
            + px[i + myid - P3] + px[i + myid - P4];
            }
            }

            x[k * P4 + myid] = cx[myid] = px[myid];
            }

        }
    }
}
extern "C"

void colYGPU(uint32_t *y, int s) {
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=s
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=cy complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            uint32_t cy[3 * P4];

            uint32_t *ay = &cy[P4 * 2];
            int myid = _tid_x;
            ay[myid] = y[2 * P4 + myid];

            for (int k = 0; k < s / P4; k++) {

            cy[myid] = cy[myid + P4];
            cy[myid + P4] = ay[myid];

            for (int i = 0; i < P4; i += LWDR) {
            if (myid < LWDR) {
            ay[i + myid] = ay[i + myid - P1] + ay[i + myid - P2]
            + ay[i + myid - P3] + ay[i + myid - P4];
            }
            }
            }

            y[2 * P4 + myid] = cy[2 * P4 + myid];
            y[P4 + myid] = cy[P4 + myid];
            y[myid] = cy[myid];

        }
    }
}
extern "C"

void lastEntGPU(uint32_t * x, uint32_t * y, int s, int r) {
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=s
    #pragma HLS INTERFACE s_axilite port=r
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=a0 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=b0 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=c0 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=d0 complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            uint32_t a0[3 * P4];
            uint32_t b0[2 * P4];
            uint32_t c0[2 * P4];
            uint32_t d0[2 * P4];

            uint32_t *a = a0 + P4;
            uint32_t *b = b0 + P4;
            uint32_t *c = c0 + P4;
            uint32_t *d = d0 + P4;

            int myid = _tid_x;

            a0[myid] = y[myid];

            if (myid < P4)
            a0[myid + P4 * 2] = y[myid + P4 * 2];

            d0[myid] = c0[myid] = b0[myid] = a[myid];

            b[myid - P4] += a[-(P4 - P3) + myid];

            c[myid - P4] += (a[-(P3 - P2) + myid] + a[-(P4 - P2) + myid]);

            d[myid - P4] += (a[-(P2 - P1) + myid] + a[-(P3 - P1) + myid]
            + a[-(P4 - P1) + myid]);

            a += P4;

            for (int i = 1; i < r; i++) {

            uint32_t *xc = &x[i * s];
            uint32_t tmp = 0;

            if (myid < P4) {

            for (int k = 0; k < P4 - P3; k++)
            tmp += xc[-P4 + k] * a[myid - k];

            for (int k = 0; k < P3 - P2; k++)
            tmp += xc[-P3 + k] * b[myid - k];

            for (int k = 0; k < P2 - P1; k++)
            tmp += xc[-P2 + k] * c[myid - k];

            for (int k = 0; k < P1; k++)
            tmp += xc[-P1 + k] * d[myid - k];

            xc[s - P4 + myid] = tmp;

            }
            }

        }
    }
}
extern "C"

void colsGPU(uint32_t *x, int s, int r) {
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=s
    #pragma HLS INTERFACE s_axilite port=r
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=cx complete dim=1
    #pragma HLS ARRAY_PARTITION variable=cx complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int k0 = _bid_x * LKNB;     //
            int k1 = _tid_x / LWDR;    //
            int k2 = _tid_x % LWDR;    //

            uint32_t cx[LKNB][2 * P4];

            int fcol = (_bid_x == 0) ? 1 : 0;
            int ecol = (_bid_x == GRID_DIM_X - 1 && r % LKNB) ? r % LKNB : LKNB;

            for (int i = fcol; i < ecol; i++)
            cx[i][_tid_x] = x[(k0 + i) * s - P4 + _tid_x];

            uint32_t *pcx = &cx[k1][P4];

            for (int k = 0; k < s / P4 - 1; k++) {

            for (int i = 0; i < P4; i += LWDR)
            {
            if (!(_bid_x == 0 && _tid_x == 0)
            && !(_bid_x == GRID_DIM_X - 1 && k1 >= ecol))
            pcx[i + k2] = pcx[i + k2 - P1] + pcx[i + k2 - P2]
            + pcx[i + k2 - P3] + pcx[i + k2 - P4];

            }

            for (int i = fcol; i < ecol; i++)
            x[(k0 + i) * s + k * P4 + _tid_x] = cx[i][_tid_x] =
            cx[i][P4 + _tid_x];
            }

        }
    }
}
