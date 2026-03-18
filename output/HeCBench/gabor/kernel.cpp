#include "kernel.h"

// --- from main.cu ---
extern "C"
void gabor (
  double *gabor_spatial,
  const unsigned int height,
  const unsigned int width,
  const double center_y,
  const double center_x,
  const double ctheta,
  const double stheta,
  const double scale,
  const double sx_2,
  const double sy_2,
  const double fx)
{
    #pragma HLS INTERFACE m_axi port=gabor_spatial offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=center_y
    #pragma HLS INTERFACE s_axilite port=center_x
    #pragma HLS INTERFACE s_axilite port=ctheta
    #pragma HLS INTERFACE s_axilite port=stheta
    #pragma HLS INTERFACE s_axilite port=scale
    #pragma HLS INTERFACE s_axilite port=sx_2
    #pragma HLS INTERFACE s_axilite port=sy_2
    #pragma HLS INTERFACE s_axilite port=fx
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int x = _bid_x * BLOCK_DIM_X + _tid_x;
                    int y = _bid_y * BLOCK_DIM_Y + _tid_y;

                    double centered_x, centered_y, u, v;

                    if (x < width && y < height) {
                    centered_y = (double)y - center_y;
                    centered_x = (double)x - center_x;
                    u = ctheta * centered_x - stheta * centered_y;
                    v = ctheta * centered_y + stheta * centered_x;
                    gabor_spatial[y*width + x] = scale * exp(-0.5*(u*u/sx_2 + v*v/sy_2)) * cos(2.0*M_PI*fx*u);
                    }

                }
            }
        }
    }
}
