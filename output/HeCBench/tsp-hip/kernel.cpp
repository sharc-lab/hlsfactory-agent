#include "kernel.h"

// --- from main.cu ---
float LCG_random(unsigned int * seed) {
  const unsigned int m = 2147483648;
  const unsigned int a = 26757677;
  const unsigned int c = 1;
  *seed = (a * (*seed) + c) % m;
  return (float) (*seed) / (float) m;
}
extern "C"

void TwoOpt(int cities, 
    const float *__restrict posx_d,
    const float *__restrict posy_d,
    int *__restrict glob_d,
    int *__restrict climbs_d,
    int *__restrict best_d)
{
    #pragma HLS INTERFACE s_axilite port=cities
    #pragma HLS INTERFACE m_axi port=posx_d offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=posy_d offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=glob_d offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=climbs_d offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=best_d offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buf_s complete dim=1
    #pragma HLS ARRAY_PARTITION variable=px_s complete dim=1
    #pragma HLS ARRAY_PARTITION variable=py_s complete dim=1
    #pragma HLS ARRAY_PARTITION variable=bf_s complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int buf_s[4096];
            float px_s[tilesize];
            float py_s[tilesize];
            int bf_s[tilesize];

            int *buf = &glob_d[_bid_x * ((3 * cities + 2 + 31) / 32 * 32)];
            float *px = (float *)(&buf[cities]);
            float *py = &px[cities + 1];

            for (int i = _tid_x; i < cities; i += BLOCK_DIM_X) px[i] = posx_d[i];
            for (int i = _tid_x; i < cities; i += BLOCK_DIM_X) py[i] = posy_d[i];

            if (_tid_x == 0) {  // serial permutation
            unsigned int seed = _bid_x;
            for (unsigned int i = 1; i < cities; i++) {
            int j = (int)(LCG_random(&seed) * (cities - 1)) + 1;
            swap(px[i], px[j]);
            swap(py[i], py[j]);
            }
            px[cities] = px[0];
            py[cities] = py[0];
            }

            int minchange;
            do {
            for (int i = _tid_x; i < cities; i += BLOCK_DIM_X) buf[i] = -dist(i, i + 1);

            minchange = 0;
            int mini = 1;
            int minj = 0;
            for (int ii = 0; ii < cities - 2; ii += BLOCK_DIM_X) {
            int i = ii + _tid_x;
            float pxi0, pyi0, pxi1, pyi1, pxj1, pyj1;
            if (i < cities - 2) {
            minchange -= buf[i];
            pxi0 = px[i];
            pyi0 = py[i];
            pxi1 = px[i + 1];
            pyi1 = py[i + 1];
            pxj1 = px[cities];
            pyj1 = py[cities];
            }
            for (int jj = cities - 1; jj >= ii + 2; jj -= tilesize) {
            int bound = jj - tilesize + 1;
            for (int k = _tid_x; k < tilesize; k += BLOCK_DIM_X) {
            if (k + bound >= ii + 2) {
            px_s[k] = px[k + bound];
            py_s[k] = py[k + bound];
            bf_s[k] = buf[k + bound];
            }
            }

            int lower = bound;
            if (lower < i + 2) lower = i + 2;
            for (int j = jj; j >= lower; j--) {
            int jm = j - bound;
            float pxj0 = px_s[jm];
            float pyj0 = py_s[jm];
            int change = bf_s[jm]
            + int(sqrtf((pxi0 - pxj0) * (pxi0 - pxj0) + (pyi0 - pyj0) * (pyi0 - pyj0)))
            + int(sqrtf((pxi1 - pxj1) * (pxi1 - pxj1) + (pyi1 - pyj1) * (pyi1 - pyj1)));
            pxj1 = pxj0;
            pyj1 = pyj0;
            if (minchange > change) {
            minchange = change;
            mini = i;
            minj = j;
            }
            }
            }

            if (i < cities - 2) {
            minchange += buf[i];
            }
            }

            int change = buf_s[_tid_x] = minchange;
            if (_tid_x == 0) (*climbs_d += 1);  // stats only

            int j = BLOCK_DIM_X;
            do {
            int k = (j + 1) / 2;
            if ((_tid_x + k) < j) {
            int tmp = buf_s[_tid_x + k];
            if (change > tmp) change = tmp;
            buf_s[_tid_x] = change;
            }
            j = k;
            } while (j > 1);

            if (minchange == buf_s[0]) {
            buf_s[1] = _tid_x;  // non-deterministic winner
            }

            if (_tid_x == buf_s[1]) {
            buf_s[2] = mini + 1;
            buf_s[3] = minj;
            }

            minchange = buf_s[0];
            mini = buf_s[2];
            int sum = buf_s[3] + mini;
            for (int i = _tid_x; (i + i) < sum; i += BLOCK_DIM_X) {
            if (mini <= i) {
            int j = sum - i;
            swap(px[i], px[j]);
            swap(py[i], py[j]);
            }
            }
            } while (minchange < 0);

            int term = 0;
            for (int i = _tid_x; i < cities; i += BLOCK_DIM_X) {
            term += dist(i, i + 1);
            }
            buf_s[_tid_x] = term;

            int j = BLOCK_DIM_X;
            do {
            int k = (j + 1) / 2;
            if ((_tid_x + k) < j) {
            term += buf_s[_tid_x + k];
            }
            if ((_tid_x + k) < j) {
            buf_s[_tid_x] = term;
            }
            j = k;
            } while (j > 1);

            if (_tid_x == 0) {
            (*best_d = min(*best_d, term));
            }

        }
    }
}
