/*******************************************************************************
Vendor: Xilinx
Associated Filename: vadd.cpp
Purpose: VITIS vector addition

*******************************************************************************
Copyright (C) 2019 XILINX, Inc.

This file contains confidential and proprietary information of Xilinx, Inc. and
is protected under U.S. and international copyright and other intellectual
property laws.

DISCLAIMER
This disclaimer is not a license and does not grant any rights to the materials
distributed herewith. Except as otherwise provided in a valid license issued to
you by Xilinx, and to the maximum extent permitted by applicable law:
(1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS, AND XILINX
HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY,
INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR
FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether
in contract or tort, including negligence, or under any other theory of
liability) for any loss or damage of any kind or nature related to, arising under
or in connection with these materials, including for any direct, or any indirect,
special, incidental, or consequential loss or damage (including loss of data,
profits, goodwill, or any type of loss or damage suffered as a result of any
action brought by a third party) even if such damage or loss was reasonably
foreseeable or Xilinx had been advised of the possibility of the same.

CRITICAL APPLICATIONS
Xilinx products are not designed or intended to be fail-safe, or for use in any
application requiring fail-safe performance, such as life-support or safety
devices or systems, Class III medical devices, nuclear facilities, applications
related to the deployment of airbags, or any other applications that could lead
to death, personal injury, or severe property or environmental damage
(individually and collectively, "Critical Applications"). Customer assumes the
sole risk and liability of any use of Xilinx products in Critical Applications,
subject only to applicable laws and regulations governing limitations on product
liability.

THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT
ALL TIMES.

*******************************************************************************/

#define OCL_CHECK(error, call)                                                                   \
    call;                                                                                        \
    if (error != CL_SUCCESS) {                                                                   \
        printf("%s:%d Error calling " #call ", error code is: %d\n", __FILE__, __LINE__, error); \
        exit(EXIT_FAILURE);                                                                      \
    }

#include "host_opencl.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <ap_int.h>

static const int DATA_SIZE = 4096;

static const std::string error_message =
    "Error: Result mismatch:\n"
    "i = %d CPU result = %d Device result = %d\n";

int main(int argc, char* argv[]) {
    // TARGET_DEVICE macro needs to be passed from gcc command line
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <xclbin>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string xclbinFilename = argv[1];

    // Compute the size of array in bytes
    size_t size_in_bytes = DATA_SIZE * sizeof(int);
    int L = 64;
    if (argc == 3) {
        L = atoi(argv[2]);
    }
    const int D = 1024;
    const int NUM_DUM_SLR = 4;
    const int NUM_SLR = 4;
    const int D_head = 64;
    const int D_ffn = 4096;

    // Creates a vector of DATA_SIZE elements with an initial value of 10 and 32
    // using customized allocator for getting buffer alignment to 4k boundary

    std::vector<cl::Device> devices;
    cl_int err;
    cl::Context context;
    cl::CommandQueue q;
    cl::Kernel krnl_vector_add;
    cl::Program program;
    std::vector<cl::Platform> platforms;
    bool found_device = false;

    // traversing all Platforms To find Xilinx Platform and targeted
    // Device in Xilinx Platform
    cl::Platform::get(&platforms);
    for (size_t i = 0; (i < platforms.size()) & (found_device == false); i++) {
        cl::Platform platform = platforms[i];
        std::string platformName = platform.getInfo<CL_PLATFORM_NAME>();
        if (platformName == "Xilinx") {
            devices.clear();
            platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
            if (devices.size()) {
                found_device = true;
                break;
            }
        }
    }
    if (found_device == false) {
        std::cout << "Error: Unable to find Target Device " << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "INFO: Reading " << xclbinFilename << std::endl;
    FILE* fp;
    if ((fp = fopen(xclbinFilename.c_str(), "r")) == nullptr) {
        printf("ERROR: %s xclbin not available please build\n", xclbinFilename.c_str());
        exit(EXIT_FAILURE);
    }
    // Load xclbin
    std::cout << "Loading: '" << xclbinFilename << "'\n";
    std::ifstream bin_file(xclbinFilename, std::ifstream::binary);
    bin_file.seekg(0, bin_file.end);
    unsigned nb = bin_file.tellg();
    bin_file.seekg(0, bin_file.beg);
    char* buf = new char[nb];
    bin_file.read(buf, nb);

    // Creating Program from Binary File
    cl::Program::Binaries bins;
    bins.push_back({buf, nb});
    bool valid_device = false;
    for (unsigned int i = 0; i < devices.size(); i++) {
        auto device = devices[i];
        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err));
        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        cl::Program program(context, {device}, bins, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";
            OCL_CHECK(err, krnl_vector_add = cl::Kernel(program, "opt_kernel", &err));
            valid_device = true;
            break; // we break because we found a valid device
        }
    }
    if (!valid_device) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }

    // These commands will allocate memory on the Device. The cl::Buffer objects can
    // be used to reference the memory locations on the device.
    OCL_CHECK(err, cl::Buffer buffer_X_acc0(context, CL_MEM_READ_ONLY, (size_t)(L*D), NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_X_acc1(context, CL_MEM_READ_ONLY, (size_t)(L*D), NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_W_acc0(context, CL_MEM_READ_ONLY, (size_t)(D * D_head * NUM_DUM_SLR * 8 + D * D_ffn), NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_W_acc1(context, CL_MEM_READ_ONLY, (size_t)(D * D_head * NUM_DUM_SLR * 8 + D * D_ffn), NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_acc0_out(context, CL_MEM_WRITE_ONLY, (size_t)(NUM_SLR * L * D * 8), NULL, &err));
    // OCL_CHECK(err, cl::Buffer buffer_acc1_out(context, CL_MEM_WRITE_ONLY, (size_t)(NUM_SLR * L * D), NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_cycle(context, CL_MEM_WRITE_ONLY, sizeof(int), NULL, &err));

    std::cout << "Finish creating buffer\n";

    // set the kernel Arguments
    int narg = 0;
    OCL_CHECK(err, err = krnl_vector_add.setArg(narg++, L*D));
    OCL_CHECK(err, err = krnl_vector_add.setArg(narg++, L*D/16));
    OCL_CHECK(err, err = krnl_vector_add.setArg(narg++, L));
    OCL_CHECK(err, err = krnl_vector_add.setArg(narg++, buffer_X_acc0));
    OCL_CHECK(err, err = krnl_vector_add.setArg(narg++, buffer_X_acc1));
    OCL_CHECK(err, err = krnl_vector_add.setArg(narg++, buffer_W_acc0));
    OCL_CHECK(err, err = krnl_vector_add.setArg(narg++, buffer_W_acc1));
    OCL_CHECK(err, err = krnl_vector_add.setArg(narg++, buffer_acc0_out));
    // OCL_CHECK(err, err = krnl_vector_add.setArg(narg++, buffer_acc1_out));
    OCL_CHECK(err, err = krnl_vector_add.setArg(narg++, buffer_cycle));

    std::cout << "Finish setArgs\n";

    // We then need to map our OpenCL buffers to get the pointers
    ap_int<8>* X_acc0;
    ap_int<8>* X_acc1;
    ap_int<8>* W_acc0;
    ap_int<8>* W_acc1;
    ap_uint<128>* acc0_out;
    // ap_uint<64>* acc1_out;
    int* cycle;
    OCL_CHECK(err,
              X_acc0 = (ap_int<8>*)q.enqueueMapBuffer(buffer_X_acc0, CL_TRUE, CL_MAP_WRITE, 0, L*D, NULL, NULL, &err));
    OCL_CHECK(err,
              X_acc1 = (ap_int<8>*)q.enqueueMapBuffer(buffer_X_acc1, CL_TRUE, CL_MAP_WRITE, 0, L*D, NULL, NULL, &err));
    OCL_CHECK(err,
              W_acc0 = (ap_int<8>*)q.enqueueMapBuffer(buffer_W_acc0, CL_TRUE, CL_MAP_WRITE, 0, D * D_head * NUM_DUM_SLR * 8 + D * D_ffn, NULL, NULL, &err));
    OCL_CHECK(err,
              W_acc1 = (ap_int<8>*)q.enqueueMapBuffer(buffer_W_acc1, CL_TRUE, CL_MAP_WRITE, 0, D * D_head * NUM_DUM_SLR * 8 + D * D_ffn, NULL, NULL, &err));
    OCL_CHECK(err, acc0_out = (ap_uint<128>*)q.enqueueMapBuffer(buffer_acc0_out, CL_TRUE, CL_MAP_READ, 0, NUM_SLR * L * D * 2, NULL,
                                                         NULL, &err));
    // OCL_CHECK(err, acc1_out = (ap_uint<64>*)q.enqueueMapBuffer(buffer_acc1_out, CL_TRUE, CL_MAP_READ, 0, NUM_SLR * L * D, NULL,
    //                                                      NULL, &err));
    OCL_CHECK(err, cycle = (int*)q.enqueueMapBuffer(buffer_cycle, CL_TRUE, CL_MAP_READ, 0, sizeof(int), NULL,
                                                         NULL, &err));

    // Initialize the vectors used in the test
    for(int i = 0; i < L * D; i++){
        X_acc0[i] = 1;
        X_acc1[i] = 1;
    }

    for(int i = 0; i < D * D_head * NUM_DUM_SLR * 8 + D * D_ffn; i++){
        W_acc1[i] = 1;
    }

    for(int i = 0; i < D * D_head * NUM_DUM_SLR * 8 + D * D_ffn; i++){
        W_acc0[i] = 1;
    }

    std::cout << "Finish assigning values\n";

    cl::Event event;
    uint64_t nstimestart, nstimeend;
    uint64_t exe_time = 0;

    // Data will be migrated to kernel space
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_X_acc0, buffer_X_acc1, buffer_W_acc0, buffer_W_acc1}, 0 /* 0 means from host*/));

    std::cout << "Start kernel\n";

    // Launch the Kernel
    OCL_CHECK(err, err = q.enqueueTask(krnl_vector_add, nullptr, &event));

    std::cout << "Finish kernel\n";

    // The result of the previous kernel execution will need to be retrieved in
    // order to view the results. This call will transfer the data from FPGA to
    // source_results vector
    OCL_CHECK(err, q.enqueueMigrateMemObjects({buffer_acc0_out, buffer_cycle}, CL_MIGRATE_MEM_OBJECT_HOST));

    std::cout << "Receive data\n";

    OCL_CHECK(err, q.finish());
    OCL_CHECK(err, err = event.getProfilingInfo<uint64_t>(CL_PROFILING_COMMAND_START, &nstimestart));
    OCL_CHECK(err, err = event.getProfilingInfo<uint64_t>(CL_PROFILING_COMMAND_END, &nstimeend));
    exe_time += nstimeend - nstimestart;

    // Verify the result
    int match = 0;
    // for (int i = 0; i < DATA_SIZE; i++) {
    //     int host_result = ptr_a[i] + ptr_b[i];
    //     if (ptr_result[i] != host_result) {
    //         printf(error_message.c_str(), i, host_result, ptr_result[i]);
    //         match = 1;
    //         break;
    //     }
    // }
    std::cout << "Cycle count: " << cycle[0] << std::endl;
    std::cout << "Latency: " << exe_time << " ns" << std::endl;

    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_X_acc0, X_acc0));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_X_acc1, X_acc1));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_W_acc0, W_acc0));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_W_acc1, W_acc1));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_acc0_out, acc0_out));
    // OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_acc1_out, acc1_out));
    OCL_CHECK(err, err = q.enqueueUnmapMemObject(buffer_cycle, cycle));
    OCL_CHECK(err, err = q.finish());

    std::cout << "TEST " << (match ? "FAILED" : "PASSED") << std::endl;
    return (match ? EXIT_FAILURE : EXIT_SUCCESS);
}
