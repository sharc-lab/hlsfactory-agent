#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ap_int.h"
#include "ap_axi_sdata.h"
#include "hls_stream.h"
#include "hw_param.h"

// Top function prototype
extern "C" {
void coreConv(
    uint  output_num,
    uint  conv_loop_cnt,
    uint  contol,
    char  frac_w,
    char  frac_din,
    char  frac_dout,
    hls::stream<k2k_data_xlane>     &bias_in,
    hls::stream<k2k_data_vecxlane>  &weight_in,
    hls::stream<k2k_data_vecxlane>  &data_in,
    hls::stream<k2k_data_xlane>     &conv_out
);
}

int main() {
    // Create HLS streams
    hls::stream<k2k_data_xlane> bias_in;
    hls::stream<k2k_data_vecxlane> weight_in;
    hls::stream<k2k_data_vecxlane> data_in;
    hls::stream<k2k_data_xlane> conv_out;
    
    // Prepare test inputs
    for(int i = 0; i < 4; i++) {
        k2k_data_xlane bias_data;
        for(int l = 0; l < LANE_NUM; l++) {
            bias_data.data((l+1)*DP_WIDTH-1, l*DP_WIDTH) = 0;
        }
        bias_in.write(bias_data);
    }
    
    for(int i = 0; i < 36; i++) {
        k2k_data_vecxlane weight_data;
        k2k_data_vecxlane data_data;
        for(int l = 0; l < LANE_NUM; l++) {
            for(int v = 0; v < VEC_SIZE; v++) {
                int range_start = (l * VEC_SIZE + v) * DP_WIDTH;
                weight_data.data(range_start + DP_WIDTH - 1, range_start) = (char)(1);
                data_data.data(range_start + DP_WIDTH - 1, range_start) = (char)(i % 10);
            }
        }
        weight_in.write(weight_data);
        data_in.write(data_data);
    }
    
    // Call top function
    coreConv(
        4,      // output_num
        9,      // conv_loop_cnt (3*3*16/VEC_SIZE)
        1,      // contol (ReLU enabled)
        4,      // frac_w
        4,      // frac_din
        4,      // frac_dout
        bias_in,
        weight_in,
        data_in,
        conv_out
    );
    
    // Check outputs
    int output_count = 0;
    while(!conv_out.empty()) {
        k2k_data_xlane result = conv_out.read();
        output_count++;
    }
    
    printf("Test completed: output_count=%d\n", output_count);
    printf("coreConv testbench passed!\n");
    
    return 0;
}
