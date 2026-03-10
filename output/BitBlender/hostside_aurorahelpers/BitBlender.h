#ifndef _BITBLENDER_H_
#define _BITBLENDER_H_
//#include <hls_math.h>
#include <iostream>
#include <stdint.h>
#include <inttypes.h>
#include <ap_int.h>
#include <tapa.h>


/*****************************/
// MACROS USED FOR CALCULATIONS
#define CEIL_DIVISION(X, Y)      ( (X-1)/Y + 1)
#define ROUND_DIVISION(X, Y)     ( ( (X) + ((Y) / 2)) / (Y) )
// Power of 2 rounding : https://stackoverflow.com/questions/466204/rounding-up-to-next-power-of-2
#define K_MSH_(X,Y)             ((X-1) >> Y)
#define K_R1_(X)                ( K_MSH_(X,0)   | K_MSH_(X, 1) )
#define K_R2_(X)                ( K_R1_(X)      | (K_R1_(X) >> 2) )
#define K_R3_(X)                ( K_R2_(X)      | (K_R2_(X) >> 4) )
#define K_R4_(X)                ( K_R3_(X)      | (K_R3_(X) >> 8) )
#define K_R5_(X)                ( K_R4_(X)      | (K_R4_(X) >> 16 ) )
#define ROUND_UP_TO_NEXT_POW_TWO(X)   (K_R5_(X))+1


#define __TWO_TO_POW_N__(n) \
    (1 << (n))

#define IS_POWER_OF_TWO(x) \
    (x == __TWO_TO_POW_N__(0) ||\
     x == __TWO_TO_POW_N__(1) ||\
     x == __TWO_TO_POW_N__(2) ||\
     x == __TWO_TO_POW_N__(3) ||\
     x == __TWO_TO_POW_N__(4) ||\
     x == __TWO_TO_POW_N__(5) ||\
     x == __TWO_TO_POW_N__(6) ||\
     x == __TWO_TO_POW_N__(7) ||\
     x == __TWO_TO_POW_N__(8) ||\
     x == __TWO_TO_POW_N__(9) ||\
     x == __TWO_TO_POW_N__(10) ||\
     x == __TWO_TO_POW_N__(11) ||\
     x == __TWO_TO_POW_N__(12) ||\
     x == __TWO_TO_POW_N__(13) ||\
     x == __TWO_TO_POW_N__(14) ||\
     x == __TWO_TO_POW_N__(15) ||\
     x == __TWO_TO_POW_N__(16) ||\
     x == __TWO_TO_POW_N__(17) ||\
     x == __TWO_TO_POW_N__(18) ||\
     x == __TWO_TO_POW_N__(19) ||\
     x == __TWO_TO_POW_N__(20) ||\
     x == __TWO_TO_POW_N__(21) ||\
     x == __TWO_TO_POW_N__(22) ||\
     x == __TWO_TO_POW_N__(23) ||\
     x == __TWO_TO_POW_N__(24) ||\
     x == __TWO_TO_POW_N__(25) ||\
     x == __TWO_TO_POW_N__(26) ||\
     x == __TWO_TO_POW_N__(27) ||\
     x == __TWO_TO_POW_N__(28) ||\
     x == __TWO_TO_POW_N__(29) ||\
     x == __TWO_TO_POW_N__(30) ||\
     x == __TWO_TO_POW_N__(31))


// https://stackoverflow.com/questions/6834868/macro-to-compute-number-of-bits-needed-to-store-a-number-n
#define __NBITS2__(n) ((n&2)?1:0)
#define __NBITS4__(n) ((n&(0xC))?(2+__NBITS2__(n>>2)):(__NBITS2__(n)))
#define __NBITS8__(n) ((n&0xF0)?(4+__NBITS4__(n>>4)):(__NBITS4__(n)))
#define __NBITS16__(n) ((n&0xFF00)?(8+__NBITS8__(n>>8)):(__NBITS8__(n)))
#define __NBITS32__(n) ((n&0xFFFF0000)?(16+__NBITS16__(n>>16)):(__NBITS16__(n)))
#define NBITS(n) (n==0?0:__NBITS32__(n)+1)

/*****************************/

#ifndef __SYNTHESIS__
    //#define __DO_DEBUG_PRINTS__
    #define __DO_THIS_DEBUG_PRINTS__
#endif
/*****************************/
#define MULTI_STREAMS_ENABLED (1)   // DO NOT MODIFY: This is defined by the python codegenerator scripts.
#define ENABLE_PERF_CTRS (1)       // DO NOT MODIFY: This is defined by the python codegenerator scripts.
#define NUM_HASH (5)               // DO NOT MODIFY: This is defined by the python codegenerator scripts.

#define NAIVE_MULTISTREAM (0)       // DO NOT MODIFY: This is defined by the python codegenerator scripts.

#define _KENNY_USING_AURORA_ ()                 // DO NOT MODIFY: Makefile automatically changes this line.
#define NUM_DESIRED_INPUT_KEYS ()               // DO NOT MODIFY: Makefile automatically changes this line.
#define BV_DESIRED_LENGTH ()                    // DO NOT MODIFY: Makefile automatically changes this line.
#define NUM_POPULATION_INPUTS ()                // DO NOT MODIFY: Makefile automatically changes this line.

/*****************************/

#define KEY_BITWIDTH                (32)
#define PARTIDX_BITWIDTH            (NBITS(BV_NUM_PARTITIONS))
#define LOOKUPIDX_BITWIDTH          (NBITS(BV_PARTITION_LENGTH))
#define BV_BRAM_PACKED_BITWIDTH     (32)    //On-chip memory for BV, packing this many bits into one cell  
#define BV_URAM_PACKED_BITWIDTH     (64)    //On-chip memory for BV, packing this many bits into one cell  
#define BV_LOAD_BITWIDTH            (BV_URAM_PACKED_BITWIDTH * NUM_HASH)
#define OUT_PACKED_BITWIDTH         (KEY_BITWIDTH)
const int BIT_BITWIDTH              = 1;
#define KEY_SIZE_IN_BYTES           (KEY_BITWIDTH / 8)
#define KEY_DTYPE                   ap_uint<KEY_BITWIDTH>
#define LOOKUPIDX_DTYPE             ap_uint<LOOKUPIDX_BITWIDTH>
#define HASHONLY_DTYPE              ap_uint<32>
#define PARTIDX_DTYPE               ap_uint<PARTIDX_BITWIDTH>
#define BV_BRAM_PACKED_DTYPE        ap_uint<BV_BRAM_PACKED_BITWIDTH>
#define BV_URAM_PACKED_DTYPE        ap_uint<BV_URAM_PACKED_BITWIDTH>
#define OUT_PACKED_DTYPE            ap_uint<OUT_PACKED_BITWIDTH>
#define BIT_DTYPE                   ap_uint<BIT_BITWIDTH>

typedef struct {
    KEY_DTYPE   k0;
    KEY_DTYPE   k1;
} TWOKEY_DTYPE;

typedef struct {
    BV_URAM_PACKED_DTYPE section0;

    BV_URAM_PACKED_DTYPE section1;

    BV_URAM_PACKED_DTYPE section2;

    BV_URAM_PACKED_DTYPE section3;

    BV_URAM_PACKED_DTYPE section4;

    BV_URAM_PACKED_DTYPE padding5;
    BV_URAM_PACKED_DTYPE padding6;
    BV_URAM_PACKED_DTYPE padding7;

    #if NUM_HASH != 5
    crash_compilation();
    #endif
} BV_LOAD_DTYPE;

#if ENABLE_PERF_CTRS
    #if NAIVE_MULTISTREAM
    #define NUM_PERFCTR_MODULES (NUM_STM*NUM_HASH)
    #else
    #define NUM_PERFCTR_MODULES (NUM_HASH)
    #endif

    #define PERFCTR_DTYPE uint64_t

    // Make "enough" space in the buffer for the performance counter to give info.
    #define NUM_PERFCTR_OUTPUTS_PER_MODULE (1)
#endif

#if MULTI_STREAMS_ENABLED
    #define BV_NUM_PARTITIONS (9)      // DO NOT MODIFY: This is defined by the python codegenerator scripts.
    #define NUM_STM (8)                // DO NOT MODIFY: This is defined by the python codegenerator scripts.
    #define NUM_AXI_PORTS (1)          // DO NOT MODIFY: This is defined by the python codegenerator scripts.
    // Number of buffered elements we can have for each hash/partition, in the shuffle buffer.
    #define ARB_RATELIM_DISTANCE (16)   // DO NOT MODIFY: This is defined by the python codegenerator scripts.

    const int ARB_RLDIST_NEXT_POW_TWO = ROUND_UP_TO_NEXT_POW_TWO(ARB_RATELIM_DISTANCE);
    #define DEBUG_ARBITER_SINK  (0)

    const int STRM_IDX_BITWIDTH = 8;
    const int INPUT_IDX_BITWIDTH = 24;
    const int MAX_INPUT_IDX = ( 1<<(INPUT_IDX_BITWIDTH) ) - 1;
    const int MAX_STRM_IDX = ( 1<<(STRM_IDX_BITWIDTH) ) - 1;
    const int BV_PLUS_IIDX_BITWIDTH     = BIT_BITWIDTH + INPUT_IDX_BITWIDTH;

    #define STRM_IDX_DTYPE      ap_uint<STRM_IDX_BITWIDTH>
    #define INPUT_IDX_DTYPE     ap_uint<INPUT_IDX_BITWIDTH>

    #define _BV_ROUNDING_FACTOR_                    (NUM_HASH * BV_URAM_PACKED_BITWIDTH * BV_NUM_PARTITIONS)
    #define BV_LENGTH                               (CEIL_DIVISION(BV_DESIRED_LENGTH, _BV_ROUNDING_FACTOR_) * _BV_ROUNDING_FACTOR_)

    //// The optimal BRAM/URAM split is tested in BRAM_URAM_Usage_testing.c
    #define BV_NUM_BRAM_PARTITIONS                      ROUND_DIVISION(BV_NUM_PARTITIONS, 5)
    #define BV_NUM_URAM_PARTITIONS                      (BV_NUM_PARTITIONS - BV_NUM_BRAM_PARTITIONS)

    #define BV_NUM_LOADS                                CEIL_DIVISION(BV_LENGTH, BV_LOAD_BITWIDTH)
    #define BV_SECTION_LENGTH                           CEIL_DIVISION(BV_LENGTH, NUM_HASH)
    #define BV_SECTION_LENGTH_IN_URAM_PACKED_ELEMS      CEIL_DIVISION(BV_LENGTH, (NUM_HASH*BV_URAM_PACKED_BITWIDTH))
    #define BV_PARTITION_LENGTH                         CEIL_DIVISION(BV_LENGTH, (NUM_HASH*BV_NUM_PARTITIONS))
    #define BV_PARTITION_LENGTH_IN_BRAM_PACKED_ELEMS    CEIL_DIVISION(BV_LENGTH, (NUM_HASH*BV_BRAM_PACKED_BITWIDTH*BV_NUM_PARTITIONS))
    #define BV_PARTITION_LENGTH_IN_URAM_PACKED_ELEMS    CEIL_DIVISION(BV_LENGTH, (NUM_HASH*BV_URAM_PACKED_BITWIDTH*BV_NUM_PARTITIONS))

    /************************/

    typedef struct {
        INPUT_IDX_DTYPE     iidx;
        STRM_IDX_DTYPE      sidx;
    } METADATA_DTYPE;

    typedef struct {
        BIT_DTYPE           bv_val;
        METADATA_DTYPE      md;
    } BV_PLUS_METADATA_PACKED_DTYPE;

    typedef struct {
        BIT_DTYPE           bv_val;
        INPUT_IDX_DTYPE     iidx;
    } BV_PLUS_IIDX_PACKED_DTYPE;

    typedef struct {
        LOOKUPIDX_DTYPE     hash;
        METADATA_DTYPE      md;
    } PACKED_HASH_DTYPE;

    typedef struct {
        // Which partition is this hash being sent to?
        PARTIDX_DTYPE       partition_idx;
        // Inside a partition, which bit-idx are we looking up?
        LOOKUPIDX_DTYPE     lookup_idx;
    } COMP2ARB_DTYPE;

#else // MULTI_STREAMS_ENABLED
    #define NUM_STM (1)
    #define BV_NUM_PARTITIONS (1)

#endif // MULTI_STREAMS_ENABLED


typedef struct {
    KEY_DTYPE    s0_k0;
    KEY_DTYPE    s0_k1;

    KEY_DTYPE    s1_k0;
    KEY_DTYPE    s1_k1;

    KEY_DTYPE    s2_k0;
    KEY_DTYPE    s2_k1;

    KEY_DTYPE    s3_k0;
    KEY_DTYPE    s3_k1;

    KEY_DTYPE    s4_k0;
    KEY_DTYPE    s4_k1;

    KEY_DTYPE    s5_k0;
    KEY_DTYPE    s5_k1;

    KEY_DTYPE    s6_k0;
    KEY_DTYPE    s6_k1;

    KEY_DTYPE    s7_k0;
    KEY_DTYPE    s7_k1;

} LOAD_DTYPE;

#if NUM_STM != 8
crash on purpose(,
#endif
#if KEY_BITWIDTH != 32
crash on purpose(,
We need to add support for different key-bitwidths in the LOAD_DTYPE!!
#endif

bool operator==(const LOAD_DTYPE& lhs, const LOAD_DTYPE& rhs);
bool operator!=(const LOAD_DTYPE& lhs, const LOAD_DTYPE& rhs);
std::ostream& operator<<(std::ostream& os, const LOAD_DTYPE& loadval);


// This naming is potentially confusing - we datapack TWICE.
// Within each key-stream we pack 32 elements together.
// Then, we coalesce among key-streams.
typedef struct {
    OUT_PACKED_DTYPE    s0_k0;
    OUT_PACKED_DTYPE    s0_k1;

    OUT_PACKED_DTYPE    s1_k0;
    OUT_PACKED_DTYPE    s1_k1;

    OUT_PACKED_DTYPE    s2_k0;
    OUT_PACKED_DTYPE    s2_k1;

    OUT_PACKED_DTYPE    s3_k0;
    OUT_PACKED_DTYPE    s3_k1;

    OUT_PACKED_DTYPE    s4_k0;
    OUT_PACKED_DTYPE    s4_k1;

    OUT_PACKED_DTYPE    s5_k0;
    OUT_PACKED_DTYPE    s5_k1;

    OUT_PACKED_DTYPE    s6_k0;
    OUT_PACKED_DTYPE    s6_k1;

    OUT_PACKED_DTYPE    s7_k0;
    OUT_PACKED_DTYPE    s7_k1;

} STORE_DTYPE;

#if NUM_STM != 8
crash on purpose(,
#endif
#if OUT_PACKED_BITWIDTH!= 32
crash on purpose(,
We need to add support for different out-packing-bitwidths in the STORE_DTYPE!!
#endif

bool operator==(const LOAD_DTYPE& lhs, const LOAD_DTYPE& rhs);
bool operator!=(const LOAD_DTYPE& lhs, const LOAD_DTYPE& rhs);
std::ostream& operator<<(std::ostream& os, const STORE_DTYPE& val);


/***************************************************/
/***************************************************/
/***************************************************/

/* STM_DEPTH: In the worst case, the arbiter can send all of the hashed values to the same FIFO.
 *  So that FIFO will get filled, and the shuffle unit wont be able to handle it.
 *  We need at least NUM_STM FIFO elements between arbiter and shuffle. */
#if (CEIL_DIVISION(NUM_STM, 2) < 2)
    #define STM_DEPTH (2)   // If less than 2, CSIM is very slow.
#else
    #define STM_DEPTH (CEIL_DIVISION(NUM_STM, 2))
#endif //(CEIL_DIVISION(NUM_STM, 2) < 2)

#define KEYS_PER_STM                (ROUND_UP_TO_NEXT_POW_TWO( CEIL_DIVISION(NUM_DESIRED_INPUT_KEYS, NUM_STM) ))
const int KEYPAIRS_PER_STM          = KEYS_PER_STM/2;   // 2 reads per cycle (2port bram)
#define TOTAL_NUM_KEYINPUT          (KEYS_PER_STM * NUM_STM)
#define MAX_KEYS_IN_ONE_AXI_PORT    (KEYS_PER_STM * 8)
const int PACKED_OUTPUTS_PER_STM    = CEIL_DIVISION(KEYPAIRS_PER_STM, OUT_PACKED_BITWIDTH);
const int PACKED_OUTPAIRS_PER_STM   = PACKED_OUTPUTS_PER_STM * 2;   // same number of outputs but half the stms if we pair them
const int NUM_PACKED_OUTPUTS        = PACKED_OUTPUTS_PER_STM * NUM_STM * 2;   // 2 reads per cycle (2port bram)

#define NUM_ARBITER_ATOMS       (NUM_STM - 1)
typedef struct {
    INPUT_IDX_DTYPE strm0_out_idx;
    INPUT_IDX_DTYPE strm1_out_idx;
    INPUT_IDX_DTYPE strm2_out_idx;
    INPUT_IDX_DTYPE strm3_out_idx;
    INPUT_IDX_DTYPE strm4_out_idx;
    INPUT_IDX_DTYPE strm5_out_idx;
    INPUT_IDX_DTYPE strm6_out_idx;
    INPUT_IDX_DTYPE strm7_out_idx;
    #if NUM_STM != 8
    crash!
    #endif
} RATEMON_FEEDBACK_DTYPE;


#endif  // _BITBLENDER_H_
