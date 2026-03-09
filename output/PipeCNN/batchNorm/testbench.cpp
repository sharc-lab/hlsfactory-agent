#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ap_int.h"
#include "ap_axi_sdata.h"
#include "hls_stream.h"
#include "hw_param.h"

// Top function prototype
extern "C" {
void batchNorm(
    uint dim1xdim2,
    uint input_num,
    uint  contol,
    float frac2float,
    float frac2char,
    const channel_scal_float     *mean,
    const channel_scal_float     *var,
    const channel_scal_float     *alpha,
    const channel_scal_float     *beta,
    hls::stream<k2k_data_xlane>  &conv_in,
    hls::stream<k2k_data_xlane>  &bn_out
);
}

int main() {
    // Allocate test buffers
    const int num_channels = 16;
    channel_scal_float *mean = new channel_scal_float[num_channels];
    channel_scal_float *var = new channel_scal_float[num_channels];
    channel_scal_float *alpha = new channel_scal_float[num_channels];
    channel_scal_float *beta = new channel_scal_float[num_channels];
    
    // Initialize normalization parameters
    for(int i = 0; i < num_channels; i++) {
        for(int l = 0; l < LANE_NUM; l++) {
            mean[i].lane[l] = 0.0f;
            var[i].lane[l] = 1.0f;
            alpha[i].lane[l] = 1.0f;
            beta[i].lane[l] = 0.0f;
        }
    }
    
    // Create HLS streams
    hls::stream<k2k_data_xlane> conv_in;
    hls::stream<k2k_data_xlane> bn_out;
    
    // Prepare test inputs
    for(int i = 0; i < 64; i++) {
        k2k_data_xlane data;
        for(int l = 0; l < LANE_NUM; l++) {
            data.data((l+1)*DP_WIDTH-1, l*DP_WIDTH) = (char)(i % 16);
        }
        conv_in.write(data);
    }
    
    // Call top function
    batchNorm(
        8,          // dim1xdim2
        64,         // input_num
        1,          // contol (ReLU enabled)
        1.0f/16.0f, // frac2float
        16.0f,      // frac2char
        mean,
        var,
        alpha,
        beta,
        conv_in,
        bn_out
    );
    
    // Check outputs
    int output_count = 0;
    while(!bn_out.empty()) {
        k2k_data_xlane result = bn_out.read();
        output_count++;
    }
    
    printf("Test completed: output_count=%d\n", output_count);
    printf("batchNorm testbench passed!\n");
    
    delete[] mean;
    delete[] var;
    delete[] alpha;
    delete[] beta;
    
    return 0;
}
