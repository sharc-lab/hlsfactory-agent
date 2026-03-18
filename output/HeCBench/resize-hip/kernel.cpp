#include "kernel.h"

// --- from main.cu ---
extern "C"
void resize (
    T * output,
    size_t output_size, int out_height, int out_width,
    const T * input, int in_height, int in_width,
    float o2i_fy, float o2i_fx, bool round, bool half_pixel_centers)
{
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=output_size
    #pragma HLS INTERFACE s_axilite port=out_height
    #pragma HLS INTERFACE s_axilite port=out_width
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=in_height
    #pragma HLS INTERFACE s_axilite port=in_width
    #pragma HLS INTERFACE s_axilite port=o2i_fy
    #pragma HLS INTERFACE s_axilite port=o2i_fx
    #pragma HLS INTERFACE s_axilite port=round
    #pragma HLS INTERFACE s_axilite port=half_pixel_centers
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            auto in_image_size = in_height * in_width;
            auto out_image_size = out_height * out_width;

            /* think of the output and input as a collection of 2d images with the last axis
            * representing the width and the last but one axis representing the height
            *
            * the remaining axis together form a collection of these images/channels
            */
            auto num_effective_channels = output_size / out_image_size;

            /* we process multiple channels every iteration to reuse the identical computation
            * involved with the spatial dimensions
            *
            * if we are processing `CHANNELS_PER_ITER` channels per iteration, we will need
            * (num_effective_channels / CHANNELS_PER_ITER) iterations per (x, y) location
            */
            auto num_channel_iters_per_xy = (num_effective_channels / CHANNELS_PER_ITER);

            /* we need `num_channel_iters_per_xy` iterations per (x, y) and there are `out_image_size`
            * combinations of (x, y); hence, we'll need `num_channel_iters_per_xy * out_image_size`
            * iterations in total to finish the resize operation
            */
            auto iters_required = num_channel_iters_per_xy * out_image_size;

            for (int iter = _bid_x * BLOCK_DIM_X + _tid_x;
            iter < iters_required; iter += BLOCK_DIM_X * GRID_DIM_X) {

            const int c_start = (iter / out_image_size) * CHANNELS_PER_ITER;

            /* note here that consecutive `iter` values will often have consecutive `x` values
            * => stores into output will be coalesced across threads
            */
            const int y = (iter % out_image_size) / out_width;
            const int x = iter % out_width;

            auto in_yf = half_pixel_centers ? (y + 0.5f) * o2i_fy : y * o2i_fy;
            int in_y = round ? lroundf(in_yf) : static_cast<int>(in_yf);

            auto in_xf = half_pixel_centers ? (x + 0.5f) * o2i_fx : x * o2i_fx;
            int in_x = round ? lroundf(in_xf) : static_cast<int>(in_xf);

            in_x = min(in_x, in_width - 1);
            in_y = min(in_y, in_height - 1);

            int in_idx = c_start * in_image_size + in_y * in_width + in_x;
            int out_idx = c_start * out_image_size + y * out_width + x;

            for (int i = 0; i < CHANNELS_PER_ITER; i++) {
            output[out_idx] = input[in_idx];
            in_idx += in_image_size;
            out_idx += out_image_size;
            }
            }

        }
    }
}
extern "C"

void resize_bilinear(
    T * output,
    size_t output_size, int out_height, int out_width,
    const T * input, int in_height, int in_width,
    float o2i_fy, float o2i_fx, bool half_pixel_centers)
{
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=output_size
    #pragma HLS INTERFACE s_axilite port=out_height
    #pragma HLS INTERFACE s_axilite port=out_width
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=in_height
    #pragma HLS INTERFACE s_axilite port=in_width
    #pragma HLS INTERFACE s_axilite port=o2i_fy
    #pragma HLS INTERFACE s_axilite port=o2i_fx
    #pragma HLS INTERFACE s_axilite port=half_pixel_centers
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            auto in_image_size = in_height * in_width;
            auto out_image_size = out_height * out_width;

            /* think of the output and input as a collection of 2d images with the last axis
            * representing the width and the last but one axis representing the height
            *
            * the remaining axis together form a collection of these images/channels
            */
            auto num_effective_channels = output_size / out_image_size;

            /* we process multiple channels every iteration to reuse the identical computation
            * involved with the spatial dimensions
            *
            * if we are processing `CHANNELS_PER_ITER` channels per iteration, we will need
            * (num_effective_channels / CHANNELS_PER_ITER) iterations per (x, y) location
            */
            auto num_channel_iters_per_xy = (num_effective_channels / CHANNELS_PER_ITER);

            /* we need `num_channel_iters_per_xy` iterations per (x, y) and there are `out_image_size`
            * combinations of (x, y); hence, we'll need `num_channel_iters_per_xy * out_image_size`
            * iterations in total to finish the resize operation
            */
            auto iters_required = num_channel_iters_per_xy * out_image_size;

            for (int iter = _bid_x * BLOCK_DIM_X + _tid_x;
            iter < iters_required; iter += BLOCK_DIM_X * GRID_DIM_X) {

            const int c_start = (iter / out_image_size) * CHANNELS_PER_ITER;
            const int c_end = c_start + CHANNELS_PER_ITER;

            /* note here that consecutive `iter` values will often have consecutive `x` values
            * => stores into output will be coalesced across threads
            */
            const int y = (iter % out_image_size) / out_width;
            const int x = iter % out_width;

            auto in_x = half_pixel_centers ? fmaxf((x + 0.5f) * o2i_fx - 0.5f, 0.0f) : x * o2i_fx;
            auto in_y = half_pixel_centers ? fmaxf((y + 0.5f) * o2i_fy - 0.5f, 0.0f) : y * o2i_fy;

            auto in_x0 = static_cast<int>(in_x);
            auto in_x1 = min(in_x0 + 1, in_width - 1);

            auto in_y0 = static_cast<int>(in_y);

            auto in_y1 = min(in_y0, in_height - 1);
            auto in_y2 = min(in_y0 + 1, in_height - 1);

            int in_offset_r0 = c_start * in_image_size + in_y1 * in_width;
            int in_offset_r1 = c_start * in_image_size + in_y2 * in_width;
            int out_idx = c_start * out_image_size + y * out_width + x;

            #pragma unroll 1 /* disable unrolling to reduce register pressure; not sure how but it works */
            for (auto c = c_start; c < c_end; c++) {
            auto v_00 = input[in_offset_r0 + in_x0],
            v_01 = input[in_offset_r0 + in_x1],
            v_10 = input[in_offset_r1 + in_x0],
            v_11 = input[in_offset_r1 + in_x1];

            output[out_idx] =
            v_00 +
            T(in_y - in_y0) * T(v_10 - v_00) +
            T(in_x - in_x0) * T(v_01 - v_00) +
            T(in_y - in_y0) * T(in_x - in_x0) * T(v_11 - v_01 - v_10 + v_00);

            in_offset_r0 += in_image_size;
            in_offset_r1 += in_image_size;
            out_idx += out_image_size;
            }
            }

        }
    }
}
