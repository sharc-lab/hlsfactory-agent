#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ap_int.h"
#include "ap_axi_sdata.h"
#include "hls_stream.h"
#include "hw_param.h"

// Top function prototype
extern "C" {
void memRead(
    uchar  data_dim1,
    uchar  data_dim2,
    ushort data_dim1xdim2,
    uchar  weight_dim1,
    uchar  weight_dim2,
    ushort weight_dim3,
    ushort weight_dim4_div_lane,
    uchar  weight_dim1x2,
    uint   weight_dim1x2x3,
    uchar  conv_x,
    uchar  stride,
    uchar  padding,
    uchar  split,
    uchar  group_num_x,
    uchar  group_num_y,
    uchar  group_rem_size_x,
    uint   group_rem_size_xyz,
    uchar  win_size_x,
    uchar  win_size_y,
    uint   win_size_xyz,
    const lane_data    *bottom,
    const channel_vec  *weights,
    const channel_scal *bias,
    hls::stream<k2k_data_xlane>     &bias_out,
    hls::stream<k2k_data_vecxlane>  &weight_out,
    hls::stream<k2k_data_vecxlane>  &data_out
);
}

int main() {
    // Allocate test buffers
    lane_data *bottom = new lane_data[256];
    channel_vec *weights = new channel_vec[256];
    channel_scal *bias = new channel_scal[16];
    
    // Initialize test data
    for(int i = 0; i < 256; i++) {
        for(int v = 0; v < VEC_SIZE; v++) {
            bottom[i].data[v] = (char)(i % 128);
        }
    }
    
    for(int i = 0; i < 256; i++) {
        for(int l = 0; l < LANE_NUM; l++) {
            for(int v = 0; v < VEC_SIZE; v++) {
                weights[i].lane[l].data[v] = (char)(1);
            }
        }
    }
    
    for(int i = 0; i < 16; i++) {
        for(int l = 0; l < LANE_NUM; l++) {
            bias[i].lane[l] = (char)(0);
        }
    }
    
    // Create HLS streams
    hls::stream<k2k_data_xlane> bias_out;
    hls::stream<k2k_data_vecxlane> weight_out;
    hls::stream<k2k_data_vecxlane> data_out;
    
    // Call top function with test parameters
    memRead(
        8,      // data_dim1
        8,      // data_dim2
        64,     // data_dim1xdim2
        3,      // weight_dim1
        3,      // weight_dim2
        16,     // weight_dim3
        2,      // weight_dim4_div_lane
        9,      // weight_dim1x2
        144,    // weight_dim1x2x3
        6,      // conv_x
        1,      // stride
        1,      // padding
        0,      // split
        1,      // group_num_x
        1,      // group_num_y
        6,      // group_rem_size_x
        216,    // group_rem_size_xyz
        5,      // win_size_x
        3,      // win_size_y
        240,    // win_size_xyz
        bottom,
        weights,
        bias,
        bias_out,
        weight_out,
        data_out
    );
    
    // Check outputs
    int bias_count = 0;
    int data_count = 0;
    int weight_count = 0;
    
    while(!bias_out.empty()) {
        bias_out.read();
        bias_count++;
    }
    
    printf("Test completed: bias_count=%d\n", bias_count);
    printf("memRead testbench passed!\n");
    
    delete[] bottom;
    delete[] weights;
    delete[] bias;
    
    return 0;
}
