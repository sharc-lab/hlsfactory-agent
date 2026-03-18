#include "kernel.h"

// --- from main.cu ---
void setcols(int cw[MAXCOLS][3], int r, int g, int b, int k)
{
  cw[k][0] = r;
  cw[k][1] = g;
  cw[k][2] = b;
}

void computeColor(float fx, float fy, uchar *pix)
{
  int cw[MAXCOLS][3];  // color wheel

  // relative lengths of color transitions:
  // these are chosen based on perceptual similarity
  // (e.g. one can distinguish more shades between red and yellow 
  //  than between yellow and green)
  int i;
  int k = 0;
  for (i = 0; i < RY; i++) setcols(cw, 255,     255*i/RY,   0,       k++);
  for (i = 0; i < YG; i++) setcols(cw, 255-255*i/YG, 255,     0,     k++);
  for (i = 0; i < GC; i++) setcols(cw, 0,       255,     255*i/GC,   k++);
  for (i = 0; i < CB; i++) setcols(cw, 0,       255-255*i/CB, 255,   k++);
  for (i = 0; i < BM; i++) setcols(cw, 255*i/BM,     0,     255,     k++);
  for (i = 0; i < MR; i++) setcols(cw, 255,     0,     255-255*i/MR, k++);

  float rad = sqrtf(fx * fx + fy * fy);
  float a = atan2f(-fy, -fx) / (float)M_PI;
  float fk = (a + 1.f) / 2.f * (MAXCOLS-1);
  int k0 = (int)fk;
  int k1 = (k0 + 1) % MAXCOLS;
  float f = fk - k0;
  for (int b = 0; b < 3; b++) {
    float col0 = cw[k0][b] / 255.f;
    float col1 = cw[k1][b] / 255.f;
    float col = (1.f - f) * col0 + f * col1;
    if (rad <= 1)
      col = 1.f - rad * (1.f - col); // increase saturation with radius
    else
      col *= .75f; // out of range
    pix[2 - b] = (int)(255.f * col);
  }
}
extern "C"

void color (uchar* pix, int size, int half_size, float range, float truerange)
{
    #pragma HLS INTERFACE m_axi port=pix offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=half_size
    #pragma HLS INTERFACE s_axilite port=range
    #pragma HLS INTERFACE s_axilite port=truerange
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int y = BLOCK_DIM_Y * _bid_y + _tid_y;
                    int x = BLOCK_DIM_X * _bid_x + _tid_x;

                    if (y < size && x < size) {
                    float fx = (float)x / (float)half_size * range - range;
                    float fy = (float)y / (float)half_size * range - range;
                    if (x == half_size || y == half_size) return; // make black coordinate axes
                    size_t idx = ((size_t)y * size + x) * 3;
                    computeColor(fx/truerange, fy/truerange, pix+idx);
                    }

                }
            }
        }
    }
}
