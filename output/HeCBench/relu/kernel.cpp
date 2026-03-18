#include "kernel.h"

// --- from main.cu ---
extern "C"
void ReluGrad_impl1(const half* gradient,
                    const half* feature,
                          half* backprop,
                    const int count)
{
    #pragma HLS INTERFACE m_axi port=gradient offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=feature offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=backprop offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=count
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int half2_count = count >> 1;
            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            const int total_device_threads = GRID_DIM_X * BLOCK_DIM_X;

            while (index < half2_count) {
            // The fast branch.
            // One half2, two fp16, is fetched and processed at a time.
            half2 gradient_h2 = reinterpret_cast<const half2*>(gradient)[index];
            half2 feature_h2 = reinterpret_cast<const half2*>(feature)[index];
            half2* p_backprop_h2 = reinterpret_cast<half2*>(backprop) + index;

            // Assume half2 primitives are available.
            const half2 kZero_h2 = __float2half2_rn(0.f);
            // mask = (feature > 0)
            half2 mask_h2 = __hgt2(feature_h2, kZero_h2);
            // backprop = mask * gradient
            half2 backprop_h2 = __hmul2(mask_h2, gradient_h2);

            // Write back the result.
            *p_backprop_h2 = backprop_h2;

            index += total_device_threads;
            }

            if ((count & 0x1) == 1 && index == half2_count) {
            // If the total number of the elements is odd, process the last element.
            half grad_h = gradient[count - 1];
            half feature_h = feature[count - 1];

            float grad_f = static_cast<float>(grad_h);
            float feature_f = static_cast<float>(feature_h);
            float backprop_f = (feature_f > 0) ? grad_f : 0;

            half backprop_h(backprop_f);
            backprop[count - 1] = backprop_h;
            }

        }
    }
}
extern "C"

void ReluGrad_impl2(const half*  gradient,
                    const half*  feature,
                          half*  backprop,
                    const int count)
{
    #pragma HLS INTERFACE m_axi port=gradient offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=feature offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=backprop offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=count
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int half8_count = count / VectorSize;
            int index = _bid_x * BLOCK_DIM_X + _tid_x;

            if (index < half8_count) {
            float4 gradient_h8 = reinterpret_cast<const float4*>(gradient)[index];
            float4 feature_h8 = reinterpret_cast<const float4*>(feature)[index];
            float4* p_backprop_h8 = reinterpret_cast<float4*>(backprop) + index;

            half2 *gradient_h2 = reinterpret_cast<half2*>(&gradient_h8);
            half2 *feature_h2 = reinterpret_cast<half2*>(&feature_h8);
            float4 backprop_h8;
            half2* p_backprop_h2 = reinterpret_cast<half2*>(&backprop_h8);

            // Assume half2 primitives are available.
            const half2 kZero_h2 = __float2half2_rn(0.f);

            for (int i = 0; i < VectorSize / 2; i++) {
            // mask = (feature > 0)
            half2 mask_h2 = __hgt2(feature_h2[i], kZero_h2);
            // backprop = mask * gradient
            half2 backprop_h2 = __hmul2(mask_h2, gradient_h2[i]);
            p_backprop_h2[i] = backprop_h2;
            }
            // Write back the result.
            *p_backprop_h8 = backprop_h8;
            }

            int remaining_count = (count % VectorSize);

            if (index < remaining_count) {
            // Use first threads to process the remaining elements.
            half grad_h = gradient[half8_count * VectorSize + index];
            half feature_h = feature[half8_count * VectorSize + index];

            float grad_f = static_cast<float>(grad_h);
            float feature_f = static_cast<float>(feature_h);
            float backprop_f = (feature_f > 0) ? grad_f : 0;

            half backprop_h(backprop_f);
            backprop[half8_count * VectorSize + index] = backprop_h;
            }

        }
    }
}
extern "C"

void Relu_impl1(int count, const int* input, int* output)
{
    #pragma HLS INTERFACE s_axilite port=count
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index < count) {
            uint4 v;
            char4 b;
            b.x = input[index] & 0xFF;
            b.y = (input[index] >> 8) & 0xFF;
            b.z = (input[index] >> 16) & 0xFF;
            b.w = (input[index] >> 24) & 0xFF;
            v.x = max(b.x, 0);
            v.y = max(b.y, 0);
            v.z = max(b.z, 0);
            v.w = max(b.w, 0);
            output[index] = v.w << 24 | v.z << 16 | v.y << 8 | v.x;
            }

        }
    }
}
extern "C"

void Relu_impl2(int count, const int* input, int* output)
{
    #pragma HLS INTERFACE s_axilite port=count
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index < count) {
            output[index] = __vmaxs4(input[index], 0);
            }

        }
    }
}
