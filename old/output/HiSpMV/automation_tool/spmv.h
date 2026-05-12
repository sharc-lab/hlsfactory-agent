#ifndef SPMV_H
#define SPMV_H
// #include "bmem_tasks.h"
#include <cstdint>
#include <tapa.h>
#include <ap_int.h>
#include "hw_defs.h"


#ifdef BUILD_ROW_DIST_NETWORK
#define FIFO_DEPTH 2
#else
#define FIFO_DEPTH 8
#endif

#ifdef HIGH_FREQ_DESIGN
#define FP_ACC_LATENCY 10
#else
#ifdef BUILD_PRE_ACCUMULATOR
#define FP_ACC_LATENCY 4
#else
#define FP_ACC_LATENCY 5
#endif
#endif

#define PES_PER_CH (CH_WIDTH/64)
#define NUM_PES (NUM_A_CH*PES_PER_CH)
#define NUM_PES_HALF (NUM_PES/2)

#define B_PART (NUM_B_CH*(CH_WIDTH/32/2))
#define B_WINDOW (B_PART*1024)
#define W_WINDOW 512

#define II_DIST (FP_ACC_LATENCY + 1) 
#define MAX_ROWS_PER_PE (URAMS_PER_PE * 4096)
#define D (NUM_PES * MAX_ROWS_PER_PE)

#define FP32_PER_CH (CH_WIDTH / 32)

using buffersB_t = tapa::buffers<
        float[B_WINDOW], //buffer type
        NUM_PES_HALF, //num buffer channels 
        1, //n-sections 
        tapa::array_partition<tapa::cyclic<B_PART>>, // partition info
        tapa::memcore<tapa::bram>>; //memory core type

using ibufferB_t = tapa::ibuffer<
        float[B_WINDOW], //buffer type
        1, //n-sections 
        tapa::array_partition<tapa::cyclic<B_PART>>, // partition info
        tapa::memcore<tapa::bram>>; //memory core typ

using obuffersB_t = tapa::obuffers<
        float[B_WINDOW], //buffer type
        LOAD_GROUP_SIZE, //num buffer channels 
        1, //n-sections 
        tapa::array_partition<tapa::cyclic<B_PART>>, // partition info
        tapa::memcore<tapa::bram>>; //memory core type;

using channelB_t = tapa::vec_t<float, FP32_PER_CH>;
using channelA_t = tapa::vec_t<uint64_t, PES_PER_CH>;
using channelC_t = tapa::vec_t<float, FP32_PER_CH>;


struct flags_pkt {
    bool sharedRow;
    bool tileEnd;
    bool last;
};

struct Cnoc_pkt {
    bool dummy;
    bool last;
    bool tileEnd;
    bool sharedRow;
    uint16_t row16;
    uint8_t bank;
    float val;
};


void SpMV(tapa::mmaps<channelA_t, NUM_A_CH> A,
          tapa::mmaps<channelB_t, NUM_B_CH> b,
          tapa::mmaps<channelC_t, NUM_C_CH> c_in,
          tapa::mmaps<channelC_t, NUM_C_CH> c_out,
          const float alpha, const float beta,
          const uint32_t A_off, const uint32_t A_len,
          const uint32_t num_rows_per_pe, const uint32_t B_len,
          const uint16_t num_tiles_r, const uint16_t num_tiles_c,
          const uint32_t num_tiles_rp_time, const uint16_t rp_time,
          const bool DENSE_MODE);
#endif