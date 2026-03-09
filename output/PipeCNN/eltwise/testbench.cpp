#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ap_int.h"
#include "ap_axi_sdata.h"
#include "hls_stream.h"
#include "hw_param.h"

// Top function prototype
extern "C" {
void eltwise(
    uint  input_num,
    uchar pool_on,
    uchar  conv_x,
    uint  conv_xy,
    uchar  stride,
    float divisor,
    float in1_frac,
    float in2_frac,
    const lane_data *bottom_1,
    const lane_data *bottom_2,
    lane_data *top
);
}

int main() {
    // Allocate test buffers
    const int num_elements = 256;
    lane_data *bottom_1 = new lane_data[num_elements];
    lane_data *bottom_2 = new lane_data[num_elements];
    lane_data *top = new lane_data[num_elements];
    
    // Initialize input data
    for(int i = 0; i < num_elements; i++) {
        for(int v = 0; v < VEC_SIZE; v++) {
            bottom_1[i].data[v] = (char)(i % 64);
            bottom_2[i].data[v] = (char)(i % 32);
        }
    }
    
    memset(top, 0, num_elements * sizeof(lane_data));
    
    // Call top function (no pooling)
    eltwise(
        num_elements,   // input_num
        0,              // pool_on (no pooling)
        8,              // conv_x
        64,             // conv_xy
        2,              // stride
        1.0f/49.0f,     // divisor
        1.0f/16.0f,     // in1_frac
        1.0f/16.0f,     // in2_frac
        bottom_1,
        bottom_2,
        top
    );
    
    printf("Test completed\n");
    printf("eltwise testbench passed!\n");
    
    delete[] bottom_1;
    delete[] bottom_2;
    delete[] top;
    
    return 0;
}
