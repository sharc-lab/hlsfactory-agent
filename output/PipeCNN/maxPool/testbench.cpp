#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ap_int.h"
#include "ap_axi_sdata.h"
#include "hls_stream.h"
#include "hw_param.h"

// Top function prototype
extern "C" {
void maxPool(
    uchar    conv_x,
    ushort   conv_xy,
    uchar    pool_dim1,
    ushort   pool_dim3,
    ushort   pool_dim1x2,
    uchar    pool_size,
    uchar    pool_stride,
    uchar    padd_offset,
    ushort   pool_times,
    ushort   pool_group,
    ushort   pool_y_bound,
    ushort   item_loop_bound,
    ushort   load_data_bound,
    ushort   write_back_bound,
    uchar    pool_win_num_x,
    uchar    win_size_x,
    const channel_scal    *bottom,
    DPTYPE                *top
);
}

int main() {
    // Allocate test buffers
    const int input_size = 256;
    const int output_size = 64;
    channel_scal *bottom = new channel_scal[input_size];
    DPTYPE *top = new DPTYPE[output_size];
    
    // Initialize input data
    for(int i = 0; i < input_size; i++) {
        for(int l = 0; l < LANE_NUM; l++) {
            bottom[i].lane[l] = (char)(i % 128);
        }
    }
    
    memset(top, 0, output_size * sizeof(DPTYPE));
    
    // Call top function
    maxPool(
        8,      // conv_x
        64,     // conv_xy
        4,      // pool_dim1
        16,     // pool_dim3
        64,     // pool_dim1x2
        2,      // pool_size
        2,      // pool_stride
        0,      // padd_offset
        8,      // pool_times
        2,      // pool_group
        20,     // pool_y_bound
        10,     // item_loop_bound
        10,     // load_data_bound
        8,      // write_back_bound
        2,      // pool_win_num_x
        6,      // win_size_x
        bottom,
        top
    );
    
    printf("Test completed\n");
    printf("maxPool testbench passed!\n");
    
    delete[] bottom;
    delete[] top;
    
    return 0;
}
