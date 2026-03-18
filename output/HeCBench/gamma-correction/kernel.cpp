#include "kernel.h"

// --- from main.cu ---
extern "C"
void gamma_correction(ImgPixel* pixel) {
    #pragma HLS INTERFACE m_axi port=pixel offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;

            // Lambda to process image with gamma = 2
            const float v = (0.3f * pixel[i].r + 0.59f * pixel[i].g + 0.11f * pixel[i].b) / 255.f;

            std::uint8_t gamma_pixel = static_cast<std::uint8_t>(255.f * v * v);
            if (gamma_pixel > 255) gamma_pixel = 255;
            pixel[i].set(gamma_pixel, gamma_pixel, gamma_pixel, gamma_pixel);

        }
    }
}
