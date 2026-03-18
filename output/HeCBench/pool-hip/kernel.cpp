#include "kernel.h"

// --- from main.cu ---
extern "C"
void KernelPool2DGrad(
    const int nthreads,
    const T* input_data,
    const T* output_data,
    const T* output_grad,
    const int channels,
    const int input_height,
    const int input_width,
    const int output_height,
    const int output_width,
    PoolProcess pool_process,
    T* input_grad,
    bool channel_last = false)
{
    #pragma HLS INTERFACE s_axilite port=nthreads
    #pragma HLS INTERFACE m_axi port=input_data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=output_data offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=output_grad offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=channels
    #pragma HLS INTERFACE s_axilite port=input_height
    #pragma HLS INTERFACE s_axilite port=input_width
    #pragma HLS INTERFACE s_axilite port=output_height
    #pragma HLS INTERFACE s_axilite port=output_width
    #pragma HLS INTERFACE s_axilite port=pool_process
    #pragma HLS INTERFACE m_axi port=input_grad offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=false
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            for (int index = _bid_x * BLOCK_DIM_X + _tid_x; index < nthreads;
            index += BLOCK_DIM_X * GRID_DIM_X) {
            int w_offset, h_offset, offsetC, batch_idx;
            int tmp;
            if (!channel_last) { /* NCHW */
            w_offset = index % input_width + padding_width;
            tmp = index / input_width;
            h_offset = tmp % input_height + padding_height;
            tmp = tmp / input_height;
            offsetC = tmp % channels;
            batch_idx = tmp / channels;
            } else { /* NHWC */
            offsetC = index % channels;
            tmp = index / channels;
            w_offset = tmp % input_width + padding_width;
            tmp = tmp / input_width;
            h_offset = tmp % input_height + padding_height;
            batch_idx = tmp / input_height;
            }

            int phstart, phend;
            int pwstart, pwend;
            phstart = (h_offset < ksize_height) ? 0 : (h_offset - ksize_height) / stride_height + 1;
            pwstart = (w_offset < ksize_width) ? 0 : (w_offset - ksize_width) / stride_width + 1;
            phend = min(h_offset / stride_height + 1, output_height);
            pwend = min(w_offset / stride_width + 1, output_width);

            // initial gradient value
            T gradient = static_cast<T>(0.0);
            T input = input_data[index];

            int output_stride = batch_idx * output_height * output_width * channels;
            if (!channel_last)
            output_stride += offsetC * output_height * output_width;

            const T * output_data_t = output_data + output_stride;
            const T * output_grad_t = output_grad + output_stride;

            for (int ph = phstart; ph < phend; ++ph) {
            for (int pw = pwstart; pw < pwend; ++pw) {
            int pool_size;
            int hstart = ph * stride_height - padding_height;
            int wstart = pw * stride_width - padding_width;
            int hend = min(hstart + ksize_height, input_height);
            int wend = min(wstart + ksize_width, input_width);
            hstart = max(hstart, 0);
            wstart = max(wstart, 0);
            pool_size = exclusive ? (hend - hstart) * (wend - wstart)
            : ksize_height * ksize_width;

            int output_sub_idx = channel_last
            ? (ph * output_width + pw) * channels + offsetC
            : ph * output_width + pw;
            pool_process.compute(input, output_data_t[output_sub_idx],
            output_grad_t[output_sub_idx],
            static_cast<T>(1.f / pool_size), &gradient);
            }
            }
            input_grad[index] = gradient;
            }

        }
    }
}
