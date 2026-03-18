#include "kernel.h"

// --- from main.cu ---
extern "C"
void conv_depthwise2d_forward_kernel(
    const PackedTensorAccessor32<scalar_t, 4, RestrictPtrTraits> input,
          PackedTensorAccessor32<scalar_t, 4, RestrictPtrTraits> output,
    const PackedTensorAccessor32<scalar_t, 4, RestrictPtrTraits> weight,
    const PackedTensorAccessor32<scalar_t, 1, RestrictPtrTraits> bias,
    bool biasEnabled,
    index_t totalElements,
    const int outputChannels,
    const int depthwiseMultiplier,
    const int inputWidth, const int inputHeight,
    const int outputWidth, const int outputHeight,
    const int kernelWidth, const int kernelHeight,
    const int strideWidth, const int strideHeight,
    const int padWidth, const int padHeight,
    const int dilationWidth, const int dilationHeight)
{
    #pragma HLS INTERFACE s_axilite port=PackedTensorAccessor32<scalar_t
    #pragma HLS INTERFACE s_axilite port=4
    #pragma HLS INTERFACE s_axilite port=input
    #pragma HLS INTERFACE s_axilite port=PackedTensorAccessor32<scalar_t
    #pragma HLS INTERFACE s_axilite port=4
    #pragma HLS INTERFACE s_axilite port=output
    #pragma HLS INTERFACE s_axilite port=PackedTensorAccessor32<scalar_t
    #pragma HLS INTERFACE s_axilite port=4
    #pragma HLS INTERFACE s_axilite port=weight
    #pragma HLS INTERFACE s_axilite port=PackedTensorAccessor32<scalar_t
    #pragma HLS INTERFACE s_axilite port=1
    #pragma HLS INTERFACE s_axilite port=bias
    #pragma HLS INTERFACE s_axilite port=biasEnabled
    #pragma HLS INTERFACE s_axilite port=totalElements
    #pragma HLS INTERFACE s_axilite port=outputChannels
    #pragma HLS INTERFACE s_axilite port=depthwiseMultiplier
    #pragma HLS INTERFACE s_axilite port=inputWidth
    #pragma HLS INTERFACE s_axilite port=inputHeight
    #pragma HLS INTERFACE s_axilite port=outputWidth
    #pragma HLS INTERFACE s_axilite port=outputHeight
    #pragma HLS INTERFACE s_axilite port=kernelWidth
    #pragma HLS INTERFACE s_axilite port=kernelHeight
    #pragma HLS INTERFACE s_axilite port=strideWidth
    #pragma HLS INTERFACE s_axilite port=strideHeight
    #pragma HLS INTERFACE s_axilite port=padWidth
    #pragma HLS INTERFACE s_axilite port=padHeight
    #pragma HLS INTERFACE s_axilite port=dilationWidth
    #pragma HLS INTERFACE s_axilite port=dilationHeight
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int KW_LIMIT = (kSize != 0) ? kSize : kernelWidth;
            const int KH_LIMIT = (kSize != 0) ? kSize : kernelHeight;

            index_t linearIndex = _bid_x * BLOCK_DIM_X + _tid_x;
            if (linearIndex < totalElements) {
            //calculate n,c,h,w indices, replacing modulos by divide and multiply add,
            //result is same as would be in the code below
            //const int n = linearIndex / batchStride; //batchStride = outputChannels * outputHeight * outputWidth
            //const int c = (linearIndex / channelStride) % outputChannels; //channelStride = outputHeight * outputWidth
            //const int h = (linearIndex / outputWidth) % outputHeight;
            //const int w = linearIndex % outputWidth;

            int indtmp1 = linearIndex/outputWidth;
            const int w = linearIndex - indtmp1 * outputWidth;
            int indtmp2 = indtmp1/outputHeight;
            const int h = indtmp1 - indtmp2 * outputHeight;
            indtmp1 = indtmp2;
            indtmp2 = indtmp1/outputChannels;
            const int c = indtmp1 - indtmp2 * outputChannels;
            const int n = indtmp2;

            int inputChannel = c;
            int inputChannels = outputChannels;
            if (depthwiseMultiplier !=1) {
            inputChannel /= depthwiseMultiplier;
            inputChannels /= depthwiseMultiplier;
            }

            int weightOffset = c * kernelHeight * kernelWidth;

            acc_t value = biasEnabled ? static_cast<acc_t>(bias.data()[c]) : acc_t(0);
            const index_t offset0 = (n * inputChannels + inputChannel) * inputHeight * inputWidth;
            #pragma unroll
            for (int kH = 0; kH < KH_LIMIT; ++kH) {
            #pragma unroll
            for (int kW = 0; kW < KW_LIMIT; ++kW) {
            const int h_in = -padHeight + h * strideHeight + kH * dilationHeight;
            const int w_in = -padWidth + w * strideWidth + kW * dilationWidth;

            if ((h_in >= 0) && (h_in < inputHeight) && (w_in >= 0) && (w_in < inputWidth)) {
            const index_t offset = offset0 + h_in * inputWidth + w_in;
            value += (static_cast<acc_t>(weight.data()[weightOffset]) *
            static_cast<acc_t>(input.data()[offset]));
            }
            ++weightOffset;
            }
            }
            output.data()[linearIndex] = static_cast<scalar_t>(value);
            }

        }
    }
}
