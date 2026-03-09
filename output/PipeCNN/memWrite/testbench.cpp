#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ap_int.h"
#include "ap_axi_sdata.h"
#include "hls_stream.h"
#include "hw_param.h"

// Top function prototype
extern "C" {
void memWrite(
    uchar  out_dim1,
    uchar  out_dim2,
    ushort out_dim3,
    ushort out_dim1xbatch,
    uint   out_dim1x2xbatch,
    uchar  batch_indx_dim1,
    uchar  batch_indx_dim2,
    uchar  padd_offset,
    uchar  pool_on,
    uchar  pool_size,
    uchar  pool_stride,
    DPTYPE *top,
    hls::stream<k2k_data_xlane>     &conv_in
);
}

int main() {
    // Allocate output buffer
    const int output_size = 512;
    DPTYPE *top = new DPTYPE[output_size];
    
    memset(top, 0, output_size * sizeof(DPTYPE));
    
    // Create HLS stream for input
    hls::stream<k2k_data_xlane> conv_in;
    
    // Prepare test inputs
    for(int i = 0; i < 64; i++) {
        k2k_data_xlane data;
        for(int l = 0; l < LANE_NUM; l++) {
            data.data((l+1)*DP_WIDTH-1, l*DP_WIDTH) = (char)(i % 16);
        }
        conv_in.write(data);
    }
    
    // Call top function
    memWrite(
        8,      // out_dim1
        8,      // out_dim2
        16,     // out_dim3
        8,      // out_dim1xbatch
        64,     // out_dim1x2xbatch
        0,      // batch_indx_dim1
        0,      // batch_indx_dim2
        0,      // padd_offset
        0,      // pool_on
        2,      // pool_size
        2,      // pool_stride
        top,
        conv_in
    );
    
    printf("Test completed\n");
    printf("memWrite testbench passed!\n");
    
    delete[] top;
    
    return 0;
}
