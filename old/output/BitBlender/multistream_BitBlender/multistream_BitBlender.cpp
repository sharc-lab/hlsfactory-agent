
//#ifndef __SYNTHESIS__
//    #include <time.h>
//#endif

#include "BitBlender.h"

#if NAIVE_MULTISTREAM != 0
void crash_compilation(
crash compilation.
}
#endif


void loadKey_MAXSTM(
#if _KENNY_USING_AURORA_
        tapa::istream<LOAD_DTYPE>   & key_in_stm
#else   //_KENNY_USING_AURORA_
        tapa::async_mmap<LOAD_DTYPE>   & key_in_mmap
#endif  //_KENNY_USING_AURORA_
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S0_kp0
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S0_kp1
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S1_kp0
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S1_kp1
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S2_kp0
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S2_kp1
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S3_kp0
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S3_kp1
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S4_kp0
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S4_kp1
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S5_kp0
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S5_kp1
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S6_kp0
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S6_kp1
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S7_kp0
        ,tapa::ostream<KEY_DTYPE>           & key_stream_S7_kp1
        ,int NUM_LOADS_PER_STM
){

    LOAD_DTYPE cur_load;

    for (int i_req = 0, i_resp = 0;
            i_resp < NUM_LOADS_PER_STM; )
    {
        #pragma HLS PIPELINE II=1

#if _KENNY_USING_AURORA_
        if (!key_in_stm.empty()){
            cur_load = key_in_stm.read(nullptr);
#else
        if (i_req < NUM_LOADS_PER_STM && key_in_mmap.read_addr.try_write(i_req)) {
            ++i_req;
        }
        if (!key_in_mmap.read_data.empty()) {
            cur_load = key_in_mmap.read_data.read(nullptr);
#endif

            #define WRITE_SIDX(SIDX)    \
                key_stream_S##SIDX##_kp0.write(cur_load.s##SIDX##_k0); \
                key_stream_S##SIDX##_kp1.write(cur_load.s##SIDX##_k1);

            WRITE_SIDX(0)
            WRITE_SIDX(1)
            WRITE_SIDX(2)
            WRITE_SIDX(3)
            WRITE_SIDX(4)
            WRITE_SIDX(5)
            WRITE_SIDX(6)
            WRITE_SIDX(7)

            #if NUM_STM != 8
            crash on purpose(,
            #endif


            #ifdef __DO_DEBUG_PRINTS__
            printf("KDEBUG: LOADKEY_MAXSTM - Loaded the %d'th keypair. In stm0, this is = %d, %d\n",
                    i_resp,
                    cur_load.s0_k0.to_int(),
                    cur_load.s0_k1.to_int()
            );
            #endif

            ++i_resp;
        }
    }

    #ifdef __DO_DEBUG_PRINTS__
    printf("\n\nLOADKEY - IS DONE NOW.\n\n");
    #endif
    return;
}



void loadKey_REMAINDERSTM(
#if _KENNY_USING_AURORA_
        tapa::istream<LOAD_DTYPE>   & key_in_stm
#else   //_KENNY_USING_AURORA_
        tapa::async_mmap<LOAD_DTYPE>   & key_in_mmap
#endif  //_KENNY_USING_AURORA_
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S0_kp0
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S0_kp1
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S1_kp0
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S1_kp1
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S2_kp0
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S2_kp1
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S3_kp0
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S3_kp1
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S4_kp0
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S4_kp1
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S5_kp0
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S5_kp1
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S6_kp0
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S6_kp1
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S7_kp0
        ,tapa::ostream<KEY_DTYPE>       & key_stream_S7_kp1
        ,int NUM_LOADS_PER_STM
){

    LOAD_DTYPE cur_load;

    for (int i_req = 0, i_resp = 0;
            i_resp < NUM_LOADS_PER_STM; )
    {
        #pragma HLS PIPELINE II=1

#if _KENNY_USING_AURORA_
        if (!key_in_stm.empty()){
            cur_load = key_in_stm.read(nullptr);
#else
        if (i_req < NUM_LOADS_PER_STM && key_in_mmap.read_addr.try_write(i_req)) {
            ++i_req;
        }
        if (!key_in_mmap.read_data.empty()) {
            cur_load = key_in_mmap.read_data.read(nullptr);
#endif

            #define WRITE_SIDX(SIDX)    \
                key_stream_S##SIDX##_kp0.write(cur_load.s##SIDX##_k0); \
                key_stream_S##SIDX##_kp1.write(cur_load.s##SIDX##_k1);

            WRITE_SIDX(0)
            WRITE_SIDX(1)
            WRITE_SIDX(2)
            WRITE_SIDX(3)
            WRITE_SIDX(4)
            WRITE_SIDX(5)
            WRITE_SIDX(6)
            WRITE_SIDX(7)

            #if NUM_STM != 8
            crash on purpose(,
            #endif


            #ifdef __DO_DEBUG_PRINTS__
            printf("KDEBUG: LOADKEY_REMAINDERSTM - Loaded the %d'th keypair. In stm0, this is = %d, %d\n",
                    i_resp,
                    cur_load.s0_k0.to_int(),
                    cur_load.s0_k1.to_int()
            );
            #endif

            ++i_resp;
        }
    }

    #ifdef __DO_DEBUG_PRINTS__
    printf("\n\nLOADKEY - IS DONE NOW.\n\n");
    #endif
    return;
}



#define LOAD_INVOKES    \
    .invoke(loadKey_REMAINDERSTM    \
            ,key_in_0    \
            ,key_stream_kp0[0]    \
            ,key_stream_kp1[0]    \
            ,key_stream_kp0[1]    \
            ,key_stream_kp1[1]    \
            ,key_stream_kp0[2]    \
            ,key_stream_kp1[2]    \
            ,key_stream_kp0[3]    \
            ,key_stream_kp1[3]    \
            ,key_stream_kp0[4]    \
            ,key_stream_kp1[4]    \
            ,key_stream_kp0[5]    \
            ,key_stream_kp1[5]    \
            ,key_stream_kp0[6]    \
            ,key_stream_kp1[6]    \
            ,key_stream_kp0[7]    \
            ,key_stream_kp1[7]    \
            ,NUM_LOADS_PER_STM\
    )    \

    #if NUM_AXI_PORTS != 1
    crash(compilation)
    #endif




/*************************************************************************************/

void loadBV(
    tapa::async_mmap<BV_LOAD_DTYPE>    & input_bv
    ,tapa::ostream<BV_URAM_PACKED_DTYPE>    & bv_load_stream_0
    ,tapa::ostream<BV_URAM_PACKED_DTYPE>    & bv_load_stream_1
    ,tapa::ostream<BV_URAM_PACKED_DTYPE>    & bv_load_stream_2
    ,tapa::ostream<BV_URAM_PACKED_DTYPE>    & bv_load_stream_3
    ,tapa::ostream<BV_URAM_PACKED_DTYPE>    & bv_load_stream_4
    #if NUM_HASH != 5
    ,crash,
    #endif
){

    #if BV_LOAD_BITWIDTH > 1024
    crash(); on purpose();
    // We need more sophisticated loading logic. AXI bitwidth shouldnt be higher than 1024.
    #endif

    BV_LOAD_DTYPE  cur_bv_val;

    for (int i_req = 0, i_resp = 0;
            i_resp < BV_NUM_LOADS; )
    {
        #pragma HLS PIPELINE II=1

        if (i_req < BV_NUM_LOADS && input_bv.read_addr.try_write(i_req)) {
            ++i_req;
        }
        if (!input_bv.read_data.empty()) {
            cur_bv_val = input_bv.read_data.read(nullptr);

            bv_load_stream_0.write(cur_bv_val.section0);
            bv_load_stream_1.write(cur_bv_val.section1);
            bv_load_stream_2.write(cur_bv_val.section2);
            bv_load_stream_3.write(cur_bv_val.section3);
            bv_load_stream_4.write(cur_bv_val.section4);
            #if NUM_HASH != 5
            crash!!
            #endif

            #ifdef __DO_DEBUG_PRINTS__
            for (int i = 0; i < BV_URAM_PACKED_BITWIDTH; ++i)
            {
                int total_idx = i_resp*BV_URAM_PACKED_BITWIDTH + i;
                BIT_DTYPE cur_bit;
                cur_bit.range(0,0) = cur_bv_val.section0.range(i, i);
                printf("KDEBUG: LOADBV - The %dth packed BV value of section 0 is %d\n",
                        total_idx, cur_bit.to_int());
            }
            #endif

            ++i_resp;
        }
    }

    #ifdef __DO_DEBUG_PRINTS__
    printf("\n\nLOADBV IS DONE NOW.\n\n");
    #endif
    return;
}


/*************************************************************************************/


uint32_t MurmurHash3_x86_32 (
    KEY_DTYPE key,
    uint32_t seed
){
#pragma HLS inline
    const int nblocks = KEY_SIZE_IN_BYTES / 4;
    uint32_t h1 = seed;
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    //length is limited as this:  KEY_SIZE_IN_BYTES / 4  <= KEY_SIZE_IN_BYTES
    BLOCK_DIVIDING:
    for( int i = 0; i < nblocks; i++){
    #pragma HLS UNROLL
        KEY_DTYPE tmp;
        tmp.range(31,0) = key.range(32*i+31, 32*i);

        uint32_t k1 = tmp;
        k1 *= c1;
        // copy-paste the body of the rotl() function, otherwise it doesn't work.
        //k1 = ROTL32(k1,15);
        k1 = ( (k1 << 15) | (k1 >> (17)) );
        k1 = k1*c2;

        h1 ^= k1;
        // copy-paste the body of the rotl() function, otherwise it doesn't work.
        //h1 = ROTL32(h1,13);
        h1 = ( (h1 << 13) | (h1 >> 19) );
        h1 = h1*5 + 0xe6546b64;

        #if PRINT_HASH_VALUES
        printf("KERNEL DEBUG: key = %d, k = %d, h = %d, i = %d\n",
                key.to_int(), k1, h1, i
        );
        #endif
        
    }
    //Remainder from block division
    uint32_t tail = key[nblocks];

    //Finalization
    h1 ^= KEY_SIZE_IN_BYTES;
    //h1 = fmix32(h1);
    h1 ^= h1>>16;
    h1 *= 0x85ebca6b;
    h1 ^= h1>>13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1>>16;

    uint32_t retval;
    //retval = (uint32_t)key * (seed+3);
    retval = h1;
    return retval;
}



void computeHash_Feeder(
        int                                     strm_idx
        ,int                                    keypair_idx
        ,tapa::istream<KEY_DTYPE>               & key_in_stream
        ,tapa::ostreams<KEY_DTYPE, NUM_HASH>    & key_out_stream
        ,int                                    NUM_LOADS_PER_STM
){
    int total_num_reads = 0;


    KEY_DTYPE       key;
    bool            key_written[NUM_HASH];
    #pragma HLS ARRAY_PARTITION variable=key_written dim=0 complete

    INIT_KEY_WRITTEN:
    for (int i = 0; i < NUM_HASH; ++i) {
        key_written[i] = 1;
    }

    while (1) {
    #pragma HLS PIPELINE II=1

        // Only read if there is data to read, AND we already wrote the previous data.
        bool do_read = !key_in_stream.empty();
        HASH_RD_LOOP:
        for(int hash_idx = 0; hash_idx < NUM_HASH; ++hash_idx){
            if (key_written[hash_idx] == 0) {
                do_read = 0;
            }
        }

        if (do_read) {
            ///////////////////////////////////
            // READ LOGIC:

            // NOTE: This blocking read is ok because we only have one input stream
            key = key_in_stream.read();

            #ifdef __DO_DEBUG_PRINTS__
            printf("COMPUTEHASH_FEEDER #%d kp%d - Read input #%d, with value %d.\n",
                strm_idx, keypair_idx,
                total_num_reads, key.to_int()
            );
            total_num_reads++;
            #endif

            for (int j = 0; j < NUM_HASH; ++j) {
                key_written[j] = 0;
            }
        }


        ///////////////////////////////////
        // WRITE LOGIC:

        for (int hash_idx = 0; hash_idx < NUM_HASH; ++hash_idx) {
        #pragma HLS UNROLL
            if (key_written[hash_idx] == 0) {
                if (key_out_stream[hash_idx].try_write(key)) {
                    key_written[hash_idx] = 1;
                }
            }
        }
    }

    #ifdef __DO_DEBUG_PRINTS__
    printf("\n\nCOMPUTEHASH_FEEDER #%d kp%d - DONE NOW.\n\n",
            strm_idx, keypair_idx
    );
    #endif
    return;
}



void computeHash_Computer(
        int                             stm_idx
        ,int                            hash_idx
        ,int                            keypair_idx
        ,tapa::istream<KEY_DTYPE>       & key_stream
        ,tapa::ostream<COMP2ARB_DTYPE>  & comp2arb_stream
        ,int                            NUM_LOADS_PER_STM
){
    const int WRITE_STOP_COUNT =    NUM_LOADS_PER_STM;
    int total_num_writes = 0;
    COMP2ARB_DTYPE  wr;

    #ifdef __DO_THIS_DEBUG_PRINTS__
    int input_idx = 0;
    int module_idx = stm_idx*NUM_HASH + hash_idx;
    #endif

    MAIN_LOOP:
    while ( total_num_writes < WRITE_STOP_COUNT){
    #pragma HLS PIPELINE II=1
        KEY_DTYPE key = key_stream.read();
        uint32_t hash = MurmurHash3_x86_32(key, hash_idx);

        hash %= BV_SECTION_LENGTH;

        //comp2arb_stream.write(hash);
        wr.partition_idx = (hash / BV_PARTITION_LENGTH);
        wr.lookup_idx = (hash % BV_PARTITION_LENGTH);
        comp2arb_stream.write(wr);

        total_num_writes++;

        #ifdef __DO_DEBUG_PRINTS__
        printf("COMPUTEHASH_COMPUTER #%d kp%d - (STM %d, HASH %d): read input #%d, key %d, computed hash = %d\n",
                module_idx,
                keypair_idx,
                stm_idx,
                hash_idx,
                input_idx,
                key.to_int(),
                hash
        );
        input_idx++;
        #endif

    }
}

#define COMPUTEHASH_STREAM_DECLS_KP(KP_IDX)   \
    tapa::streams<KEY_DTYPE, NUM_HASH>      key_tmp_stream_0_kp##KP_IDX;\
    tapa::streams<KEY_DTYPE, NUM_HASH>      key_tmp_stream_1_kp##KP_IDX;\
    tapa::streams<KEY_DTYPE, NUM_HASH>      key_tmp_stream_2_kp##KP_IDX;\
    tapa::streams<KEY_DTYPE, NUM_HASH>      key_tmp_stream_3_kp##KP_IDX;\
    tapa::streams<KEY_DTYPE, NUM_HASH>      key_tmp_stream_4_kp##KP_IDX;\
    tapa::streams<KEY_DTYPE, NUM_HASH>      key_tmp_stream_5_kp##KP_IDX;\
    tapa::streams<KEY_DTYPE, NUM_HASH>      key_tmp_stream_6_kp##KP_IDX;\
    tapa::streams<KEY_DTYPE, NUM_HASH>      key_tmp_stream_7_kp##KP_IDX;\


#define INVOKE_COMPUTERS_FOR_HASH(HASH_IDX, STM_IDX, KP_IDX)\
    .invoke(computeHash_Computer\
            ,STM_IDX\
            ,HASH_IDX\
            ,KP_IDX\
            ,key_tmp_stream_##STM_IDX##_kp##KP_IDX[HASH_IDX]\
            ,comp2arb_stream_h##HASH_IDX##_kp##KP_IDX[STM_IDX]\
            ,NUM_LOADS_PER_STM\
    )


// CONFIG: need NUM_HASH calls to INVOKE_COMPUTERS_FOR_HASH
#define INVOKE_COMPUTERS_FOR_STM(STM_IDX, KP_IDX)\
    INVOKE_COMPUTERS_FOR_HASH(0, STM_IDX, KP_IDX)\
    INVOKE_COMPUTERS_FOR_HASH(1, STM_IDX, KP_IDX)\
    INVOKE_COMPUTERS_FOR_HASH(2, STM_IDX, KP_IDX)\
    INVOKE_COMPUTERS_FOR_HASH(3, STM_IDX, KP_IDX)\
    INVOKE_COMPUTERS_FOR_HASH(4, STM_IDX, KP_IDX)\


#define COMPUTEHASH_INVOKES_FOR_KP(KP_IDX)  \
        .invoke<tapa::detach>(computeHash_Feeder \
                    ,0  \
                    ,KP_IDX    \
                    ,key_stream_kp##KP_IDX[0]   \
                    ,key_tmp_stream_0_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(computeHash_Feeder \
                    ,1  \
                    ,KP_IDX    \
                    ,key_stream_kp##KP_IDX[1]   \
                    ,key_tmp_stream_1_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(computeHash_Feeder \
                    ,2  \
                    ,KP_IDX    \
                    ,key_stream_kp##KP_IDX[2]   \
                    ,key_tmp_stream_2_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(computeHash_Feeder \
                    ,3  \
                    ,KP_IDX    \
                    ,key_stream_kp##KP_IDX[3]   \
                    ,key_tmp_stream_3_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(computeHash_Feeder \
                    ,4  \
                    ,KP_IDX    \
                    ,key_stream_kp##KP_IDX[4]   \
                    ,key_tmp_stream_4_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(computeHash_Feeder \
                    ,5  \
                    ,KP_IDX    \
                    ,key_stream_kp##KP_IDX[5]   \
                    ,key_tmp_stream_5_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(computeHash_Feeder \
                    ,6  \
                    ,KP_IDX    \
                    ,key_stream_kp##KP_IDX[6]   \
                    ,key_tmp_stream_6_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(computeHash_Feeder \
                    ,7  \
                    ,KP_IDX    \
                    ,key_stream_kp##KP_IDX[7]   \
                    ,key_tmp_stream_7_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
    /* Need NUM_STM of these^ invokes */ \
    INVOKE_COMPUTERS_FOR_STM(0, KP_IDX) \
    INVOKE_COMPUTERS_FOR_STM(1, KP_IDX) \
    INVOKE_COMPUTERS_FOR_STM(2, KP_IDX) \
    INVOKE_COMPUTERS_FOR_STM(3, KP_IDX) \
    INVOKE_COMPUTERS_FOR_STM(4, KP_IDX) \
    INVOKE_COMPUTERS_FOR_STM(5, KP_IDX) \
    INVOKE_COMPUTERS_FOR_STM(6, KP_IDX) \
    INVOKE_COMPUTERS_FOR_STM(7, KP_IDX) \
    /* Need NUM_STM of these^ invokes */

#if NUM_STM != 8
void crash(){ crash compilation
#endif



/*************************************************************************************/





//////////////////////////////////////////////////
//////////////////////////////////////////////////
///////// Arbiter                           //////
//////////////////////////////////////////////////
//////////////////////////////////////////////////


void bloom_arb_forwarder(
        int arb_idx
        ,int kp_idx
        ,tapa::istreams<COMP2ARB_DTYPE, NUM_STM>                         & comp2arb_stream
        ,tapa::ostreams<PACKED_HASH_DTYPE, NUM_STM*BV_NUM_PARTITIONS>    & arb_stream
        ,int NUM_LOADS_PER_STM
){
    typedef struct {
        ap_uint<1>          valid;
        PACKED_HASH_DTYPE   value;
        PARTIDX_DTYPE       target_partition_idx;
    } XBAR_DTYPE;

    int finishcheck_num_streams_done = 0;
    int finishcheck_written_xbar_entries = 0;
    bool finished_computing = 0;

    #ifdef __SYNTHESIS__
    /* TAPA Known-issue: Static keyword fails CSIM because this
       isnt thread-safe. But when running the HW build, it will 
       instantiate several copies of this function. So this is OK.
    */
    static
    #endif
    INPUT_IDX_DTYPE     reads_per_input[NUM_STM];
    #pragma HLS ARRAY_PARTITION variable=reads_per_input dim=0 complete
    #ifdef __SYNTHESIS__
    /* TAPA Known-issue: Static keyword fails CSIM because this
       isnt thread-safe. But when running the HW build, it will 
       instantiate several copies of this function. So this is OK.
    */
    static
    #endif
    XBAR_DTYPE          xbar[NUM_STM];
    #pragma HLS ARRAY_PARTITION variable=xbar dim=0 complete

    #ifndef __SYNTHESIS__
    printf("NOTE: USING SEPARATED_HIERARB_PER_HASH ARBITER!!!\n");
    #endif

    INIT_LOOP:
    for (int i = 0; i < NUM_STM; ++i)
    {
        reads_per_input[i] = 0;
        xbar[i].valid = 0;
    }

    MAIN_LOOP:
    while (finished_computing == 0) {
    #pragma HLS PIPELINE II=1
        ////////////////////////
        //// FINISHING LOGIC
        ////////////////////////

        FINISH_COMPUTATION_LOGIC:
        // Determine whether or not this module can stop.
        // In this implementation, every cycle it will check one stream to see if it is done.
        // I think this will achieve higher frequency than checking all of them each cycle.
        // And it doesnt matter if we "waste" a few cycles waiting for us to finish.
        if (finishcheck_num_streams_done < NUM_STM) {
            if ( reads_per_input[finishcheck_num_streams_done] == NUM_LOADS_PER_STM ) {
                #ifdef __DO_DEBUG_PRINTS__
                printf("ARBITER FORWARDER #%d kp%d - stream #%d has finished.\n",
                        arb_idx, kp_idx,
                        finishcheck_num_streams_done
                );
                #endif
                finishcheck_num_streams_done++;
            }
        }
        else if (finishcheck_num_streams_done == NUM_STM &&
                finishcheck_written_xbar_entries < NUM_STM
        ) {
            if (xbar[finishcheck_written_xbar_entries].valid == 0) {
                finishcheck_written_xbar_entries++;
            }
        }
        else if (finishcheck_num_streams_done == NUM_STM &&
                finishcheck_written_xbar_entries == NUM_STM
        ) {
            finished_computing = 1;
        }


        RD_LOGIC:
        for (int strm_idx = 0; strm_idx < NUM_STM; ++strm_idx) {
        #pragma HLS UNROLL
            // Metadata:
            INPUT_IDX_DTYPE cur_input_idx;
            STRM_IDX_DTYPE cur_strm_idx;
            METADATA_DTYPE cur_metadata;
            PACKED_HASH_DTYPE packed_hashval;

            if (xbar[strm_idx].valid == 1)
            {
                // Dont replace this value.
            }
            else if (!comp2arb_stream[strm_idx].empty())
            {
                // Hash and partition data:
                COMP2ARB_DTYPE  rd_val = comp2arb_stream[strm_idx].read();

                reads_per_input[strm_idx]++;

                // Pack metadata
                cur_metadata.sidx = strm_idx;
                cur_metadata.iidx = reads_per_input[strm_idx];

                // Pack final payload
                packed_hashval.md = cur_metadata;
                packed_hashval.hash = rd_val.lookup_idx;

                xbar[strm_idx].valid = 1;
                xbar[strm_idx].value = packed_hashval;
                xbar[strm_idx].target_partition_idx = rd_val.partition_idx;
            }
        }

        #ifdef __DO_DEBUG_PRINTS__
        printf("ARBITER FORWARDER #%d kp%d - xbar[target_partition][strm_idx]\n",
            arb_idx, kp_idx
        );
        for (int strm = 0; strm < NUM_STM; ++strm)
        {
            printf("ARBITER FORWARDER #%d kp%d - xbar[%d][%d]: valid=%d, input_idx=%d\n",
                    arb_idx, kp_idx,
                    xbar[strm].target_partition_idx.to_int(),
                    strm,
                    xbar[strm].valid.to_int(),
                    xbar[strm].value.md.iidx.to_int()
            );
        }
        #endif


        WR_LOGIC:
        for (int partition_idx = 0; partition_idx < BV_NUM_PARTITIONS; ++partition_idx) 
        {
        #pragma HLS UNROLL

            for (int strm_idx = 0; strm_idx < NUM_STM; ++strm_idx)
            {
            #pragma HLS UNROLL
                int out_fifo_idx = partition_idx*NUM_STM + strm_idx;

                if (xbar[strm_idx].valid == 1 &&
                    xbar[strm_idx].target_partition_idx == partition_idx)
                {
                    if (arb_stream[out_fifo_idx].try_write( xbar[strm_idx].value ))
                    {
                        xbar[strm_idx].valid = 0;

                        #ifdef __DO_DEBUG_PRINTS__
                        printf("ARBITER FORWARDER #%d kp%d - Wrote to outfifo %d\n",
                                arb_idx, kp_idx, out_fifo_idx
                        );
                        #endif
                    }
                    #ifdef __DO_DEBUG_PRINTS__
                    else{
                        printf("ARBITER FORWARDER #%d kp%d - Failed to write to outfifo %d\n",
                                arb_idx, kp_idx, out_fifo_idx
                        );
                    }
                    #endif
                }
            }
        }

    }

    #ifdef __DO_DEBUG_PRINTS__
    printf("\n\nARBITER FORWARDER #%d kp%d - DONE NOW.\n\n",
            arb_idx, kp_idx
    );
    #endif
}



void bloom_hier_arbiter_atom(
        int arb_idx,
        int partition_idx,
        int kp_idx,
        int atom_ID,
        tapa::istream<RATEMON_FEEDBACK_DTYPE>   & ratemon_stream,
        tapa::istream<PACKED_HASH_DTYPE>        & in_stream0,
        tapa::istream<PACKED_HASH_DTYPE>        & in_stream1,
        tapa::ostream<PACKED_HASH_DTYPE>        & out_stream
){
    typedef struct {
        ap_uint<1>          valid;
        PACKED_HASH_DTYPE   value;
    } XBAR_DTYPE;

    XBAR_DTYPE xbar[2];
    #pragma HLS ARRAY_PARTITION variable=xbar dim=0 complete

    RATEMON_FEEDBACK_DTYPE  feedback;
    INPUT_IDX_DTYPE         min_output_idx_s0 = 0;
    INPUT_IDX_DTYPE         min_output_idx_s1 = 0;
    INPUT_IDX_DTYPE         min_output_idx_s2 = 0;
    INPUT_IDX_DTYPE         min_output_idx_s3 = 0;
    INPUT_IDX_DTYPE         min_output_idx_s4 = 0;
    INPUT_IDX_DTYPE         min_output_idx_s5 = 0;
    INPUT_IDX_DTYPE         min_output_idx_s6 = 0;
    INPUT_IDX_DTYPE         min_output_idx_s7 = 0;
    #ifdef __DO_DEBUG_PRINTS__
    INPUT_IDX_DTYPE         min_output_idx = 0;
    bool    print_xbar = 0;
    #endif
    
    /* Initialize for SW_EMU... but will this guaranteed work for HW builds?
     * It might not be needed for HW builds because each xbar entry should just
     * be invalidated anyways, after writing.
     */
    INIT_LOOP:
    for (int i = 0; i < 2; ++i) {
        xbar[i].valid = 0;
    }

    MAIN_LOOP:
    while (1) {
    #pragma HLS PIPELINE II=1

        RATEMON_LOGIC:
        if (!ratemon_stream.empty()) {
            feedback = ratemon_stream.read();

            #ifdef __DO_DEBUG_PRINTS__
            //if (partition_idx == 0 && atom_ID == 'a')
            //{
            //    printf("ARBITER ATOM [%d][%d][%c] - feedback came in. %d, %d, %d, %d.\n",
            //            arb_idx, partition_idx, atom_ID,
            //            feedback.strm0_out_idx.to_int(),
            //            feedback.strm1_out_idx.to_int(),
            //            feedback.strm2_out_idx.to_int(),
            //            feedback.strm3_out_idx.to_int()
            //    );
            //}
            #endif

            // Manually unroll the min_output_idx logic, to reduce latency within the atoms.
            // With only one variable this takes one more cycle.
            min_output_idx_s0 = feedback.strm0_out_idx;
            min_output_idx_s1 = feedback.strm1_out_idx;
            min_output_idx_s2 = feedback.strm2_out_idx;
            min_output_idx_s3 = feedback.strm3_out_idx;
            min_output_idx_s4 = feedback.strm4_out_idx;
            min_output_idx_s5 = feedback.strm5_out_idx;
            min_output_idx_s6 = feedback.strm6_out_idx;
            min_output_idx_s7 = feedback.strm7_out_idx;
        }

        RD_LOGIC:
        if (xbar[0].valid == 1) {
            // Dont overwrite it
        }
        else if (!in_stream0.empty()) {
            PACKED_HASH_DTYPE   packed_val = in_stream0.read();

            xbar[0].value = packed_val;
            xbar[0].valid = 1;

            #ifdef __DO_DEBUG_PRINTS__
            printf("ARBITER ATOM [%d][%d][%c] kp%d - read from hash/part/strm (%d,%d,0)\n",
                    arb_idx, partition_idx, atom_ID,
                    kp_idx,
                    arb_idx, partition_idx
            );
            print_xbar = 1;
            #endif
        }
        if (xbar[1].valid == 1) {
            // Dont overwrite it
        }
        else if (!in_stream1.empty()) {
            PACKED_HASH_DTYPE   packed_val = in_stream1.read();

            xbar[1].value = packed_val;
            xbar[1].valid = 1;

            #ifdef __DO_DEBUG_PRINTS__
            printf("ARBITER ATOM [%d][%d][%c] kp%d - read from hash/part/strm (%d,%d,1)\n",
                    arb_idx, partition_idx, atom_ID,
                    kp_idx,
                    arb_idx, partition_idx
            );
            print_xbar = 1;
            #endif
        }

        #ifdef __DO_DEBUG_PRINTS__
        min_output_idx = min_output_idx_s0;
        min_output_idx = min_output_idx_s1 < min_output_idx ? min_output_idx_s1 : min_output_idx;
        min_output_idx = min_output_idx_s2 < min_output_idx ? min_output_idx_s2 : min_output_idx;
        min_output_idx = min_output_idx_s3 < min_output_idx ? min_output_idx_s3 : min_output_idx;
        min_output_idx = min_output_idx_s4 < min_output_idx ? min_output_idx_s4 : min_output_idx;
        min_output_idx = min_output_idx_s5 < min_output_idx ? min_output_idx_s5 : min_output_idx;
        min_output_idx = min_output_idx_s6 < min_output_idx ? min_output_idx_s6 : min_output_idx;
        min_output_idx = min_output_idx_s7 < min_output_idx ? min_output_idx_s7 : min_output_idx;
        #if NUM_STM != 8
        crash(compilation);
        #endif

        if (print_xbar){
            for (int i = 0; i < 2; ++i)
            {
                printf("ARBITER ATOM [%d][%d][%c] kp%d - xbar[%d]: valid=%d, input_idx=%d, strm_idx=%d, bv_idx=%d\n",
                        arb_idx, partition_idx, atom_ID,
                        kp_idx,
                        i,
                        xbar[i].valid.to_int(),
                        xbar[i].value.md.iidx.to_int(),
                        xbar[i].value.md.sidx.to_int(),
                        xbar[i].value.hash.to_int()
                );
            }
            printf("ARBITER ATOM [%d][%d][%c] kp%d - min_output_idx = %d\n",
                    arb_idx, partition_idx, atom_ID,
                    kp_idx,
                    min_output_idx.to_int()
            );
        }
        #endif

        WR_LOGIC:
        #ifdef __DO_DEBUG_PRINTS__
        int     print_xbar_idx = -1;
        #endif
        int     valid_idxes = 0;
        int allowed_idx_s0 = min_output_idx_s0 + (ARB_RATELIM_DISTANCE);
        int allowed_idx_s1 = min_output_idx_s1 + (ARB_RATELIM_DISTANCE);
        int allowed_idx_s2 = min_output_idx_s2 + (ARB_RATELIM_DISTANCE);
        int allowed_idx_s3 = min_output_idx_s3 + (ARB_RATELIM_DISTANCE);
        int allowed_idx_s4 = min_output_idx_s4 + (ARB_RATELIM_DISTANCE);
        int allowed_idx_s5 = min_output_idx_s5 + (ARB_RATELIM_DISTANCE);
        int allowed_idx_s6 = min_output_idx_s6 + (ARB_RATELIM_DISTANCE);
        int allowed_idx_s7 = min_output_idx_s7 + (ARB_RATELIM_DISTANCE);
        if (xbar[0].valid &&
            xbar[0].value.md.iidx <= allowed_idx_s0 &&
            xbar[0].value.md.iidx <= allowed_idx_s1 &&
            xbar[0].value.md.iidx <= allowed_idx_s2 &&
            xbar[0].value.md.iidx <= allowed_idx_s3 &&
            xbar[0].value.md.iidx <= allowed_idx_s4 &&
            xbar[0].value.md.iidx <= allowed_idx_s5 &&
            xbar[0].value.md.iidx <= allowed_idx_s6 &&
            xbar[0].value.md.iidx <= allowed_idx_s7 
        ) { valid_idxes += 1; }
        if (xbar[1].valid &&
            xbar[1].value.md.iidx <= allowed_idx_s0 &&
            xbar[1].value.md.iidx <= allowed_idx_s1 &&
            xbar[1].value.md.iidx <= allowed_idx_s2 &&
            xbar[1].value.md.iidx <= allowed_idx_s3 &&
            xbar[1].value.md.iidx <= allowed_idx_s4 &&
            xbar[1].value.md.iidx <= allowed_idx_s5 &&
            xbar[1].value.md.iidx <= allowed_idx_s6 &&
            xbar[1].value.md.iidx <= allowed_idx_s7 
        ) { valid_idxes += 2; }
        #if NUM_STM != 8
        crash!
        #endif

        if (valid_idxes == 3) {
            if (xbar[1].value.md.iidx <= xbar[0].value.md.iidx) {
                if (out_stream.try_write(xbar[1].value)) {
                    xbar[1].valid = 0;
                    #ifdef __DO_DEBUG_PRINTS__
                    print_xbar_idx = 1;
                    #endif
                }
            }
            else {
                if (out_stream.try_write(xbar[0].value)) {
                    xbar[0].valid = 0;
                    #ifdef __DO_DEBUG_PRINTS__
                    print_xbar_idx = 0;
                    #endif
                }
            }
        }
        else if (valid_idxes == 2) {
            if (out_stream.try_write(xbar[1].value)) {
                xbar[1].valid = 0;
                #ifdef __DO_DEBUG_PRINTS__
                print_xbar_idx = 1;
                #endif
            }
        }
        else if (valid_idxes == 1) {
            if (out_stream.try_write(xbar[0].value)) {
                xbar[0].valid = 0;
                #ifdef __DO_DEBUG_PRINTS__
                print_xbar_idx = 0;
                #endif
            }
        }

        #ifdef __DO_DEBUG_PRINTS__
        if (print_xbar_idx != -1) {
            int allowed_idx = min_output_idx + (ARB_RATELIM_DISTANCE);
            printf("ARBITER ATOM [%d][%d][%c] kp%d - WROTE from xbar %d. hash/part/strm = (%d,%d,%d), input_idx=%d, allowed_idx=%d\n",
                    arb_idx, partition_idx, atom_ID,
                    kp_idx,
                    print_xbar_idx,
                    arb_idx,
                    partition_idx,
                    xbar[print_xbar_idx].value.md.sidx.to_int(),
                    xbar[print_xbar_idx].value.md.iidx.to_int(),
                    allowed_idx
            );
        }
        #endif

    }
}



void bloom_arbiter_ratemonitor(
    int arb_idx
    ,int kp_idx
    ,char ratemon_ID
    ,tapa::istreams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS>       &arb_stream_in
    ,tapa::ostreams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS>       &arb_stream_out

    ,tapa::ostreams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>  &fdbk_stream_0
    ,tapa::ostreams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>  &fdbk_stream_1
    ,tapa::ostreams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>  &fdbk_stream_2
    ,tapa::ostreams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>  &fdbk_stream_3
    ,tapa::ostreams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>  &fdbk_stream_4
    ,tapa::ostreams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>  &fdbk_stream_5
    ,tapa::ostreams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>  &fdbk_stream_6
    ,tapa::ostreams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>  &fdbk_stream_7
    ,tapa::ostreams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>  &fdbk_stream_8
    #if BV_NUM_PARTITIONS != 9
    crash!
    #endif

    ,int NUM_LOADS_PER_STM
){

    int WRITE_STOP_COUNT = NUM_STM * NUM_LOADS_PER_STM;
    int writes_per_partition[BV_NUM_PARTITIONS] = {};


    typedef struct {
        BIT_DTYPE           valid;
        PACKED_HASH_DTYPE   value;
    } XBAR_DTYPE;

    typedef enum {
        WR_FEEDBACK,
        WR_OUTPUT
    } RATEMON_MODE;

    XBAR_DTYPE              xbar[BV_NUM_PARTITIONS];
    #pragma HLS ARRAY_PARTITION variable=xbar dim=0 complete
    INPUT_IDX_DTYPE         min_output_idx[NUM_STM];
    #pragma HLS ARRAY_PARTITION variable=min_output_idx dim=0 complete
    BIT_DTYPE               idx_tracker[NUM_STM][ARB_RLDIST_NEXT_POW_TWO];
    #pragma HLS ARRAY_PARTITION variable=idx_tracker dim=0 complete

    INIT_LOOP:
    for (int i = 0; i < BV_NUM_PARTITIONS; ++i) {
        xbar[i].valid = 0;
        writes_per_partition[i] = 0;
    }

    INIT_LOOP_2:
    for (int i = 0; i < NUM_STM; ++i) {
        min_output_idx[i] = 0;

        for (int j = 0; j < ARB_RLDIST_NEXT_POW_TWO; ++j) {
            idx_tracker[i][j] = 0;
        }
    }

    bool finished_computing = 0;
    int finishcheck_partition_tracker = 0;
    int finishcheck_num_writes_counter = 0;

    MAIN_LOOP:
    while (finished_computing == 0) {
    #pragma HLS PIPELINE II=1
        RATEMON_FEEDBACK_DTYPE  feedback;

        ////////////////////////
        //// FINISHING LOGIC
        ////////////////////////

        FINISH_COMPUTATION_LOGIC:
        if (finishcheck_partition_tracker < BV_NUM_PARTITIONS)
        {
            finishcheck_num_writes_counter += writes_per_partition[finishcheck_partition_tracker];
            finishcheck_partition_tracker++;
        }

        else if (finishcheck_partition_tracker == BV_NUM_PARTITIONS &&
            finishcheck_num_writes_counter == WRITE_STOP_COUNT
        ) {
            finished_computing = 1;
        }
        else if (finishcheck_partition_tracker == BV_NUM_PARTITIONS)
        {
            finishcheck_partition_tracker = 0;
            finishcheck_num_writes_counter = 0;
        }

        ////////////////////////
        //// READ LOGIC
        ////////////////////////

        RD_INPUTS:
        for (int partition_idx = 0; partition_idx < BV_NUM_PARTITIONS; ++partition_idx) {
            INPUT_IDX_DTYPE     cur_input_idx;
            STRM_IDX_DTYPE      cur_strm_idx;
            METADATA_DTYPE      cur_metadata;

            if (xbar[partition_idx].valid == 0 &&
                !arb_stream_in[partition_idx].empty()
            ){
                xbar[partition_idx].valid = 1;
                xbar[partition_idx].value = arb_stream_in[partition_idx].read();

                #ifdef __DO_DEBUG_PRINTS__
                printf("ARBITER RATEMON %d %c kp%d - Read from     h/p/s=(%d,%d,%d), input_idx=%d\n",
                        arb_idx,
                        ratemon_ID,
                        kp_idx,
                        arb_idx,
                        partition_idx,
                        xbar[partition_idx].value.md.sidx.to_int(),
                        xbar[partition_idx].value.md.iidx.to_int()
                );
                #endif
            }
        }

        ///////////////////////
        // WR_OUTPUTS:
        ///////////////////////

        #define RATEMON_WR_OUTPUT_FOR_PART_STM(PART, STM)   \
            if (xbar[PART].valid &&     \
                xbar[PART].value.md.sidx == STM &&  \
                !arb_stream_out[PART].full()    \
            )   \
            {   \
                int offset = (xbar[PART].value.md.iidx) % ARB_RLDIST_NEXT_POW_TWO;    \
                xbar[PART].valid = 0;   \
                arb_stream_out[PART].write(xbar[PART].value);   \
                idx_tracker[ STM ][offset] = 1;    \
                writes_per_partition[PART]++;   \
            }

        #define RATEMON_WR_OUTPUT_FOR_PART(PART)    \
            RATEMON_WR_OUTPUT_FOR_PART_STM(PART, 0) \
            RATEMON_WR_OUTPUT_FOR_PART_STM(PART, 1) \
            RATEMON_WR_OUTPUT_FOR_PART_STM(PART, 2) \
            RATEMON_WR_OUTPUT_FOR_PART_STM(PART, 3) \
            RATEMON_WR_OUTPUT_FOR_PART_STM(PART, 4) \
            RATEMON_WR_OUTPUT_FOR_PART_STM(PART, 5) \
            RATEMON_WR_OUTPUT_FOR_PART_STM(PART, 6) \
            RATEMON_WR_OUTPUT_FOR_PART_STM(PART, 7) \
        
        RATEMON_WR_OUTPUT_FOR_PART(0)
        RATEMON_WR_OUTPUT_FOR_PART(1)
        RATEMON_WR_OUTPUT_FOR_PART(2)
        RATEMON_WR_OUTPUT_FOR_PART(3)
        RATEMON_WR_OUTPUT_FOR_PART(4)
        RATEMON_WR_OUTPUT_FOR_PART(5)
        RATEMON_WR_OUTPUT_FOR_PART(6)
        RATEMON_WR_OUTPUT_FOR_PART(7)
        RATEMON_WR_OUTPUT_FOR_PART(8)
            #if BV_NUM_PARTITIONS != 9
            crash!
            #endif


        ///////////////////////
        // UPDATE_IDCES:
        ///////////////////////
        #define RATEMON_UPDATE_IDX_FOR_STM(STM)     \
            int shuf_idx##STM = (min_output_idx[STM] + 1) % ARB_RLDIST_NEXT_POW_TWO;   \
            if (idx_tracker[STM][shuf_idx##STM] == 1) {  \
                min_output_idx[STM] += 1;   \
                idx_tracker[STM][shuf_idx##STM] = 0;     \
            }   \
                //#ifdef __DO_DEBUG_PRINTS__    \
                //printf("ARBITER RATEMON %d %c kp%d - Updating min_output_idx[%d]=%d\n",   \
                //        arb_idx,  \
                //        ratemon_ID,   \
                //        kp_idx,   \
                //        STM,  \
                //        min_output_idx[STM].to_int()  \
                //);    \
                //#endif    \

        RATEMON_UPDATE_IDX_FOR_STM(0)
        RATEMON_UPDATE_IDX_FOR_STM(1)
        RATEMON_UPDATE_IDX_FOR_STM(2)
        RATEMON_UPDATE_IDX_FOR_STM(3)
        RATEMON_UPDATE_IDX_FOR_STM(4)
        RATEMON_UPDATE_IDX_FOR_STM(5)
        RATEMON_UPDATE_IDX_FOR_STM(6)
        RATEMON_UPDATE_IDX_FOR_STM(7)

        WRITE_FEEDBACK:
        feedback.strm0_out_idx = min_output_idx[0];
        feedback.strm1_out_idx = min_output_idx[1];
        feedback.strm2_out_idx = min_output_idx[2];
        feedback.strm3_out_idx = min_output_idx[3];
        feedback.strm4_out_idx = min_output_idx[4];
        feedback.strm5_out_idx = min_output_idx[5];
        feedback.strm6_out_idx = min_output_idx[6];
        feedback.strm7_out_idx = min_output_idx[7];
        #if NUM_STM != 8
        crash!
        #endif

        for (int i = 0; i < NUM_ARBITER_ATOMS; ++i) {
            fdbk_stream_0[i].try_write(feedback);
            fdbk_stream_1[i].try_write(feedback);
            fdbk_stream_2[i].try_write(feedback);
            fdbk_stream_3[i].try_write(feedback);
            fdbk_stream_4[i].try_write(feedback);
            fdbk_stream_5[i].try_write(feedback);
            fdbk_stream_6[i].try_write(feedback);
            fdbk_stream_7[i].try_write(feedback);
            fdbk_stream_8[i].try_write(feedback);
            #if BV_NUM_PARTITIONS != 9
            crash!
            #endif
        }
    }

    EXITING_LOGIC:
    /* For the next kernel call, we need to reset the ratemonitoring 
        information. Otherwise, initially the atoms will not ratelimit.
        (You can alternatively think of this as resetting the atoms)
    */
    RATEMON_FEEDBACK_DTYPE  feedback;
    feedback.strm0_out_idx = 0;
    feedback.strm1_out_idx = 0;
    feedback.strm2_out_idx = 0;
    feedback.strm3_out_idx = 0;
    feedback.strm4_out_idx = 0;
    feedback.strm5_out_idx = 0;
    feedback.strm6_out_idx = 0;
    feedback.strm7_out_idx = 0;
    #if NUM_STM != 8
    crash!
    #endif

    for (int i = 0; i < NUM_ARBITER_ATOMS; ++i) {
    #pragma HLS UNROLL
        fdbk_stream_0[i].write(feedback);
        fdbk_stream_1[i].write(feedback);
        fdbk_stream_2[i].write(feedback);
        fdbk_stream_3[i].write(feedback);
        fdbk_stream_4[i].write(feedback);
        fdbk_stream_5[i].write(feedback);
        fdbk_stream_6[i].write(feedback);
        fdbk_stream_7[i].write(feedback);
        fdbk_stream_8[i].write(feedback);
        #if BV_NUM_PARTITIONS != 9
        crash!
        #endif
    }
    #ifdef __DO_DEBUG_PRINTS__
    printf("ARBITER RATEMON %d %c - DONE NOW!\n", arb_idx, ratemon_ID);
    #endif
}



void bloom_arbiter_tree_singlepartition(
    int arb_idx
    ,int partition_idx
    ,int kp_idx

    ,tapa::istream<PACKED_HASH_DTYPE>           &arb_stm0
    ,tapa::istream<PACKED_HASH_DTYPE>           &arb_stm1
    ,tapa::istream<PACKED_HASH_DTYPE>           &arb_stm2
    ,tapa::istream<PACKED_HASH_DTYPE>           &arb_stm3
    ,tapa::istream<PACKED_HASH_DTYPE>           &arb_stm4
    ,tapa::istream<PACKED_HASH_DTYPE>           &arb_stm5
    ,tapa::istream<PACKED_HASH_DTYPE>           &arb_stm6
    ,tapa::istream<PACKED_HASH_DTYPE>           &arb_stm7
    #if NUM_STM != 8
    crash!
    #endif

    ,tapa::istreams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>  &ratemon_feedback
    ,tapa::ostream<PACKED_HASH_DTYPE>                           &arbtree_out
) {
    tapa::streams<PACKED_HASH_DTYPE, 4>     arb_stage1_outputs;
    tapa::streams<PACKED_HASH_DTYPE, 2>     arb_stage2_outputs;
    #if NUM_ARBITER_ATOMS != 7
    crash!
    #endif

    tapa::task()
        .invoke<tapa::detach>(
                bloom_hier_arbiter_atom
                ,arb_idx
                ,partition_idx
                ,kp_idx
                ,'a'
                ,ratemon_feedback[0]
                ,arb_stm0
                ,arb_stm1
                ,arb_stage1_outputs[0]
        )
        .invoke<tapa::detach>(
                bloom_hier_arbiter_atom
                ,arb_idx
                ,partition_idx
                ,kp_idx
                ,'b'
                ,ratemon_feedback[1]
                ,arb_stm2
                ,arb_stm3
                ,arb_stage1_outputs[1]
        )
        .invoke<tapa::detach>(
                bloom_hier_arbiter_atom
                ,arb_idx
                ,partition_idx
                ,kp_idx
                ,'c'
                ,ratemon_feedback[2]
                ,arb_stm4
                ,arb_stm5
                ,arb_stage1_outputs[2]
        )
        .invoke<tapa::detach>(
                bloom_hier_arbiter_atom
                ,arb_idx
                ,partition_idx
                ,kp_idx
                ,'d'
                ,ratemon_feedback[3]
                ,arb_stm6
                ,arb_stm7
                ,arb_stage1_outputs[3]
        )
        .invoke<tapa::detach>(
                bloom_hier_arbiter_atom
                ,arb_idx
                ,partition_idx
                ,kp_idx
                ,'e'
                ,ratemon_feedback[4]
                ,arb_stage1_outputs[0]
                ,arb_stage1_outputs[1]
                ,arb_stage2_outputs[0]
        )
        .invoke<tapa::detach>(
                bloom_hier_arbiter_atom
                ,arb_idx
                ,partition_idx
                ,kp_idx
                ,'f'
                ,ratemon_feedback[5]
                ,arb_stage1_outputs[2]
                ,arb_stage1_outputs[3]
                ,arb_stage2_outputs[1]
        )
        .invoke<tapa::detach>(
                bloom_hier_arbiter_atom
                ,arb_idx
                ,partition_idx
                ,kp_idx
                ,'g'
                ,ratemon_feedback[6]
                ,arb_stage2_outputs[0]
                ,arb_stage2_outputs[1]
                ,arbtree_out
        )
        #if NUM_ARBITER_ATOMS != 7
        crash!
        #endif
    ;
}



void bloom_single_arbiter(
        int arb_idx
        , int kp_idx
        , tapa::istreams<PACKED_HASH_DTYPE, NUM_STM*BV_NUM_PARTITIONS>  &in_arb_streams
        , tapa::ostreams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS>          &bv_lookup_stream
        , int NUM_LOADS_PER_STM
) {
    tapa::streams<PACKED_HASH_DTYPE,    BV_NUM_PARTITIONS>      arbtree_outputs;

    tapa::streams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>    ratemon_fdbk_streams_p0;
    tapa::streams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>    ratemon_fdbk_streams_p1;
    tapa::streams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>    ratemon_fdbk_streams_p2;
    tapa::streams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>    ratemon_fdbk_streams_p3;
    tapa::streams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>    ratemon_fdbk_streams_p4;
    tapa::streams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>    ratemon_fdbk_streams_p5;
    tapa::streams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>    ratemon_fdbk_streams_p6;
    tapa::streams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>    ratemon_fdbk_streams_p7;
    tapa::streams<RATEMON_FEEDBACK_DTYPE, NUM_ARBITER_ATOMS>    ratemon_fdbk_streams_p8;
    #if BV_NUM_PARTITIONS != 9
    crash!
    #endif


    tapa::task()
        .invoke<tapa::detach>(
                bloom_arbiter_tree_singlepartition
                ,arb_idx
                ,0
                ,kp_idx
                ,in_arb_streams[NUM_STM*0 + 0]
                ,in_arb_streams[NUM_STM*0 + 1]
                ,in_arb_streams[NUM_STM*0 + 2]
                ,in_arb_streams[NUM_STM*0 + 3]
                ,in_arb_streams[NUM_STM*0 + 4]
                ,in_arb_streams[NUM_STM*0 + 5]
                ,in_arb_streams[NUM_STM*0 + 6]
                ,in_arb_streams[NUM_STM*0 + 7]
                ,ratemon_fdbk_streams_p0
                ,arbtree_outputs[0]
        )
        .invoke<tapa::detach>(
                bloom_arbiter_tree_singlepartition
                ,arb_idx
                ,1
                ,kp_idx
                ,in_arb_streams[NUM_STM*1 + 0]
                ,in_arb_streams[NUM_STM*1 + 1]
                ,in_arb_streams[NUM_STM*1 + 2]
                ,in_arb_streams[NUM_STM*1 + 3]
                ,in_arb_streams[NUM_STM*1 + 4]
                ,in_arb_streams[NUM_STM*1 + 5]
                ,in_arb_streams[NUM_STM*1 + 6]
                ,in_arb_streams[NUM_STM*1 + 7]
                ,ratemon_fdbk_streams_p1
                ,arbtree_outputs[1]
        )
        .invoke<tapa::detach>(
                bloom_arbiter_tree_singlepartition
                ,arb_idx
                ,2
                ,kp_idx
                ,in_arb_streams[NUM_STM*2 + 0]
                ,in_arb_streams[NUM_STM*2 + 1]
                ,in_arb_streams[NUM_STM*2 + 2]
                ,in_arb_streams[NUM_STM*2 + 3]
                ,in_arb_streams[NUM_STM*2 + 4]
                ,in_arb_streams[NUM_STM*2 + 5]
                ,in_arb_streams[NUM_STM*2 + 6]
                ,in_arb_streams[NUM_STM*2 + 7]
                ,ratemon_fdbk_streams_p2
                ,arbtree_outputs[2]
        )
        .invoke<tapa::detach>(
                bloom_arbiter_tree_singlepartition
                ,arb_idx
                ,3
                ,kp_idx
                ,in_arb_streams[NUM_STM*3 + 0]
                ,in_arb_streams[NUM_STM*3 + 1]
                ,in_arb_streams[NUM_STM*3 + 2]
                ,in_arb_streams[NUM_STM*3 + 3]
                ,in_arb_streams[NUM_STM*3 + 4]
                ,in_arb_streams[NUM_STM*3 + 5]
                ,in_arb_streams[NUM_STM*3 + 6]
                ,in_arb_streams[NUM_STM*3 + 7]
                ,ratemon_fdbk_streams_p3
                ,arbtree_outputs[3]
        )
        .invoke<tapa::detach>(
                bloom_arbiter_tree_singlepartition
                ,arb_idx
                ,4
                ,kp_idx
                ,in_arb_streams[NUM_STM*4 + 0]
                ,in_arb_streams[NUM_STM*4 + 1]
                ,in_arb_streams[NUM_STM*4 + 2]
                ,in_arb_streams[NUM_STM*4 + 3]
                ,in_arb_streams[NUM_STM*4 + 4]
                ,in_arb_streams[NUM_STM*4 + 5]
                ,in_arb_streams[NUM_STM*4 + 6]
                ,in_arb_streams[NUM_STM*4 + 7]
                ,ratemon_fdbk_streams_p4
                ,arbtree_outputs[4]
        )
        .invoke<tapa::detach>(
                bloom_arbiter_tree_singlepartition
                ,arb_idx
                ,5
                ,kp_idx
                ,in_arb_streams[NUM_STM*5 + 0]
                ,in_arb_streams[NUM_STM*5 + 1]
                ,in_arb_streams[NUM_STM*5 + 2]
                ,in_arb_streams[NUM_STM*5 + 3]
                ,in_arb_streams[NUM_STM*5 + 4]
                ,in_arb_streams[NUM_STM*5 + 5]
                ,in_arb_streams[NUM_STM*5 + 6]
                ,in_arb_streams[NUM_STM*5 + 7]
                ,ratemon_fdbk_streams_p5
                ,arbtree_outputs[5]
        )
        .invoke<tapa::detach>(
                bloom_arbiter_tree_singlepartition
                ,arb_idx
                ,6
                ,kp_idx
                ,in_arb_streams[NUM_STM*6 + 0]
                ,in_arb_streams[NUM_STM*6 + 1]
                ,in_arb_streams[NUM_STM*6 + 2]
                ,in_arb_streams[NUM_STM*6 + 3]
                ,in_arb_streams[NUM_STM*6 + 4]
                ,in_arb_streams[NUM_STM*6 + 5]
                ,in_arb_streams[NUM_STM*6 + 6]
                ,in_arb_streams[NUM_STM*6 + 7]
                ,ratemon_fdbk_streams_p6
                ,arbtree_outputs[6]
        )
        .invoke<tapa::detach>(
                bloom_arbiter_tree_singlepartition
                ,arb_idx
                ,7
                ,kp_idx
                ,in_arb_streams[NUM_STM*7 + 0]
                ,in_arb_streams[NUM_STM*7 + 1]
                ,in_arb_streams[NUM_STM*7 + 2]
                ,in_arb_streams[NUM_STM*7 + 3]
                ,in_arb_streams[NUM_STM*7 + 4]
                ,in_arb_streams[NUM_STM*7 + 5]
                ,in_arb_streams[NUM_STM*7 + 6]
                ,in_arb_streams[NUM_STM*7 + 7]
                ,ratemon_fdbk_streams_p7
                ,arbtree_outputs[7]
        )
        .invoke<tapa::detach>(
                bloom_arbiter_tree_singlepartition
                ,arb_idx
                ,8
                ,kp_idx
                ,in_arb_streams[NUM_STM*8 + 0]
                ,in_arb_streams[NUM_STM*8 + 1]
                ,in_arb_streams[NUM_STM*8 + 2]
                ,in_arb_streams[NUM_STM*8 + 3]
                ,in_arb_streams[NUM_STM*8 + 4]
                ,in_arb_streams[NUM_STM*8 + 5]
                ,in_arb_streams[NUM_STM*8 + 6]
                ,in_arb_streams[NUM_STM*8 + 7]
                ,ratemon_fdbk_streams_p8
                ,arbtree_outputs[8]
        )
    #if BV_NUM_PARTITIONS != 9
    crash!
    #endif
        .invoke(
                bloom_arbiter_ratemonitor
                ,arb_idx
                ,kp_idx
                ,'a'
                ,arbtree_outputs
                ,bv_lookup_stream
                ,ratemon_fdbk_streams_p0
                ,ratemon_fdbk_streams_p1
                ,ratemon_fdbk_streams_p2
                ,ratemon_fdbk_streams_p3
                ,ratemon_fdbk_streams_p4
                ,ratemon_fdbk_streams_p5
                ,ratemon_fdbk_streams_p6
                ,ratemon_fdbk_streams_p7
                ,ratemon_fdbk_streams_p8
                ,NUM_LOADS_PER_STM
        )
    ;
}



#define ARBITER_STREAM_DECLS_KP(KP_IDX)    \
    tapa::streams<PACKED_HASH_DTYPE, NUM_STM*BV_NUM_PARTITIONS> arb0_streams_kp##KP_IDX;   \
    tapa::streams<PACKED_HASH_DTYPE, NUM_STM*BV_NUM_PARTITIONS> arb1_streams_kp##KP_IDX;   \
    tapa::streams<PACKED_HASH_DTYPE, NUM_STM*BV_NUM_PARTITIONS> arb2_streams_kp##KP_IDX;   \
    tapa::streams<PACKED_HASH_DTYPE, NUM_STM*BV_NUM_PARTITIONS> arb3_streams_kp##KP_IDX;   \
    tapa::streams<PACKED_HASH_DTYPE, NUM_STM*BV_NUM_PARTITIONS> arb4_streams_kp##KP_IDX;   \

#if NUM_HASH != (5)
crash!,
#endif


#define ARBITER_INVOKES_FOR_KP(KP_IDX)  \
        .invoke( bloom_arb_forwarder,   \
                    0,  \
                    KP_IDX, \
                    comp2arb_stream_h0_kp##KP_IDX,  \
                    arb0_streams_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke( bloom_arb_forwarder,   \
                    1,  \
                    KP_IDX, \
                    comp2arb_stream_h1_kp##KP_IDX,  \
                    arb1_streams_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke( bloom_arb_forwarder,   \
                    2,  \
                    KP_IDX, \
                    comp2arb_stream_h2_kp##KP_IDX,  \
                    arb2_streams_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke( bloom_arb_forwarder,   \
                    3,  \
                    KP_IDX, \
                    comp2arb_stream_h3_kp##KP_IDX,  \
                    arb3_streams_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke( bloom_arb_forwarder,   \
                    4,  \
                    KP_IDX, \
                    comp2arb_stream_h4_kp##KP_IDX,  \
                    arb4_streams_kp##KP_IDX    \
                    ,NUM_LOADS_PER_STM  \
        )   \
\
        .invoke( bloom_single_arbiter,  \
                    0,  \
                    KP_IDX, \
                    arb0_streams_kp##KP_IDX,   \
                    bv_lookup_stream_h0_kp##KP_IDX  \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke( bloom_single_arbiter,  \
                    1,  \
                    KP_IDX, \
                    arb1_streams_kp##KP_IDX,   \
                    bv_lookup_stream_h1_kp##KP_IDX  \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke( bloom_single_arbiter,  \
                    2,  \
                    KP_IDX, \
                    arb2_streams_kp##KP_IDX,   \
                    bv_lookup_stream_h2_kp##KP_IDX  \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke( bloom_single_arbiter,  \
                    3,  \
                    KP_IDX, \
                    arb3_streams_kp##KP_IDX,   \
                    bv_lookup_stream_h3_kp##KP_IDX  \
                    ,NUM_LOADS_PER_STM  \
        )   \
        .invoke( bloom_single_arbiter,  \
                    4,  \
                    KP_IDX, \
                    arb4_streams_kp##KP_IDX,   \
                    bv_lookup_stream_h4_kp##KP_IDX  \
                    ,NUM_LOADS_PER_STM  \
        )   \


#if NUM_HASH != (5)
crash!,
#endif




//////////////////////////////////////////////////
//////////////////////////////////////////////////
///////// END OF Arbiter                    //////
//////////////////////////////////////////////////
//////////////////////////////////////////////////





/*************************************************************************************/


void queryResult_per_hash(
        int hash_idx
        ,tapa::istream<BV_URAM_PACKED_DTYPE>                                & bv_load_stream
        ,tapa::istreams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS>               & bv_lookup_stream_kp0
        ,tapa::istreams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS>               & bv_lookup_stream_kp1
        ,tapa::ostreams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS>   & query_bv_packed_stream_kp0
        ,tapa::ostreams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS>   & query_bv_packed_stream_kp1

        #if ENABLE_PERF_CTRS
        ,tapa::ostream<PERFCTR_DTYPE>       & cyclectr_out
        #endif

        ,int NUM_LOADS_PER_STM
) {
    // Each stream loads N keyPAIRS. Query consumes on a KEY-by-KEY basis.

    const int MAX_NUM_READS = NUM_STM * (NUM_LOADS_PER_STM*2);
    //int num_writes = 0;

    bool finished_computing = 0;
    bool finished_reading = 0;
    int finish_read_check_pidx = 0;
    int finish_write_check_BRAM_pidx = 0;
    int finish_write_check_URAM_pidx = 0;
    int total_num_reads = 0;

    int num_reads_per_partition[BV_NUM_PARTITIONS];
    #pragma HLS ARRAY_PARTITION variable=num_reads_per_partition dim=0 complete

    #if ENABLE_PERF_CTRS
    bool cyclecount_enable = 0;
    PERFCTR_DTYPE cyclectr = 0;
    #endif

    /* This is pretty confusing. We LOAD in chunks of size URAM_PACKED_BITWIDTH
     *  but we need to put it into the BRAMS.
     *  So the BRAMs will take 64 bits, which gives it 2 packed-value.
     *  The URAMs also take 64 bits, but this only gives it 1 packed-value.
     */
    BV_BRAM_PACKED_DTYPE     bv_buf_BRAMS[BV_NUM_BRAM_PARTITIONS][BV_PARTITION_LENGTH_IN_BRAM_PACKED_ELEMS];
    #pragma HLS BIND_STORAGE variable=bv_buf_BRAMS type=RAM_T2P impl=bram
    #pragma HLS ARRAY_PARTITION variable=bv_buf_BRAMS dim=1 complete

    BV_URAM_PACKED_DTYPE     bv_buf_URAMS[BV_NUM_URAM_PARTITIONS][BV_PARTITION_LENGTH_IN_URAM_PACKED_ELEMS];
    #pragma HLS BIND_STORAGE variable=bv_buf_URAMS type=RAM_T2P impl=uram
    #pragma HLS ARRAY_PARTITION variable=bv_buf_URAMS dim=1 complete


    BV_URAM_PACKED_DTYPE cur_bv_val_uram;
    BV_BRAM_PACKED_DTYPE cur_bv_val_bram0;
    BV_BRAM_PACKED_DTYPE cur_bv_val_bram1;

    typedef struct {
        ap_uint<1>      valid;
        BV_PLUS_METADATA_PACKED_DTYPE data;
    } TO_WRITE_DTYPE;

    TO_WRITE_DTYPE bram_queried_vals_buf[BV_NUM_BRAM_PARTITIONS][2];
    #pragma HLS ARRAY_PARTITION variable=bram_queried_vals_buf dim=0 complete

    TO_WRITE_DTYPE uram_queried_vals_buf[BV_NUM_URAM_PARTITIONS][2];
    #pragma HLS ARRAY_PARTITION variable=uram_queried_vals_buf dim=0 complete

    #ifndef __SYNTHESIS__
    printf("INFO: We are using the SPLIT QUERY unit!\n");
    #endif


    INIT_NUM_READS:
    for (int i = 0; i < BV_NUM_PARTITIONS; ++i) {
        num_reads_per_partition[i] = 0;
    }


    INIT_BRAM_QUERIED_VALS_BUF:
    for (int j = 0; j < BV_NUM_BRAM_PARTITIONS; ++j) {
    #pragma HLS UNROLL
        bram_queried_vals_buf[j][0].valid=0;
        bram_queried_vals_buf[j][1].valid=0;
    }

    INIT_URAM_QUERIED_VALS_BUF:
    for (int j = 0; j < BV_NUM_URAM_PARTITIONS; ++j) {
    #pragma HLS UNROLL
        uram_queried_vals_buf[j][0].valid=0;
        uram_queried_vals_buf[j][1].valid=0;
    }


    LOAD_BV_VALUES:
    for (int i = 0; i < BV_SECTION_LENGTH_IN_URAM_PACKED_ELEMS; ++i) {
        #pragma HLS PIPELINE II=1
        cur_bv_val_uram = bv_load_stream.read();

        int partition_idx = i/BV_PARTITION_LENGTH_IN_URAM_PACKED_ELEMS;
        int uram_element_idx = i%BV_PARTITION_LENGTH_IN_URAM_PACKED_ELEMS;
        int bram_element_idx = (uram_element_idx*2);

        if (partition_idx < BV_NUM_BRAM_PARTITIONS) {
            cur_bv_val_bram0.range(BV_BRAM_PACKED_BITWIDTH-1,0) =
                cur_bv_val_uram.range(BV_BRAM_PACKED_BITWIDTH-1,0);
            cur_bv_val_bram1.range(BV_BRAM_PACKED_BITWIDTH-1,0) =
                cur_bv_val_uram.range(BV_URAM_PACKED_BITWIDTH-1,BV_BRAM_PACKED_BITWIDTH);

            #if (BV_URAM_PACKED_BITWIDTH != 2*BV_BRAM_PACKED_BITWIDTH)
            crash(; // We need to have URAM_PACKED_BITWIDTH=64, BRAM_PACKED_BITWIDTH=32.
            #endif

            bv_buf_BRAMS[partition_idx][bram_element_idx+0] = cur_bv_val_bram0;
            bv_buf_BRAMS[partition_idx][bram_element_idx+1] = cur_bv_val_bram1;

            #ifdef __DO_DEBUG_PRINTS__
            printf("QUERY UNIT %d - %d, bram%d\n",
                    hash_idx, partition_idx, bram_element_idx);
            for (int j = 0; j < BV_BRAM_PACKED_BITWIDTH; ++j) {
                BIT_DTYPE tmp;
                tmp.range(0,0) = bv_buf_BRAMS[partition_idx][bram_element_idx+0].range(j,j);
                printf("QUERY UNIT %d - bv[%d] = %d\n",
                        hash_idx,
                        partition_idx*BV_PARTITION_LENGTH +
                            (bram_element_idx+0)*BV_BRAM_PACKED_BITWIDTH+j,
                        tmp.to_int()
                );
            }
            for (int j = 0; j < BV_BRAM_PACKED_BITWIDTH; ++j) {
                BIT_DTYPE tmp;
                tmp.range(0,0) = bv_buf_BRAMS[partition_idx][bram_element_idx+1].range(j,j);
                printf("QUERY UNIT %d - bv[%d] = %d\n",
                        hash_idx,
                        partition_idx*BV_PARTITION_LENGTH +
                            (bram_element_idx+1)*BV_BRAM_PACKED_BITWIDTH+j,
                        tmp.to_int()
                );
            }
            #endif

        }
        else {
            bv_buf_URAMS[partition_idx-BV_NUM_BRAM_PARTITIONS][uram_element_idx] = cur_bv_val_uram;

            #ifdef __DO_DEBUG_PRINTS__
            printf("QUERY UNIT %d - %d, uram%d\n",
                    hash_idx, partition_idx, uram_element_idx);
            for (int j = 0; j < BV_URAM_PACKED_BITWIDTH; ++j) {
                BIT_DTYPE tmp;
                tmp.range(0,0) = bv_buf_URAMS[partition_idx-BV_NUM_BRAM_PARTITIONS][uram_element_idx].range(j,j);
                printf("QUERY UNIT %d - bv[%d] = %d\n",
                        hash_idx,
                        i*BV_URAM_PACKED_BITWIDTH+j,
                        tmp.to_int()
                );
            }
            #endif

        }
    }


    PROCESS_QUERIES:
    while (finished_computing == 0){
    #pragma HLS PIPELINE II=1
        /////////////////////
        // FINISH COMPUTATION LOGIC
        /////////////////////
        FINISH_COMPUTATION_LOGIC:
        if (finished_reading == 0) {
            /* This implementation slowly adds all the values up
             * so we dont have too big of a logic-chain.
             */
            if (finish_read_check_pidx == BV_NUM_PARTITIONS) {
                finish_read_check_pidx = 0;
                total_num_reads = 0;
            } else {
                total_num_reads += num_reads_per_partition[finish_read_check_pidx];
                finish_read_check_pidx++;
            }
        }
        else {
            // After we know were finished reading, we need to make sure were finished writing.
            if (finish_write_check_BRAM_pidx < BV_NUM_BRAM_PARTITIONS &&
                bram_queried_vals_buf[finish_write_check_BRAM_pidx][0].valid == 0 &&
                bram_queried_vals_buf[finish_write_check_BRAM_pidx][1].valid == 0
            ) {
                // Both keys in the pair have finished writing.
                finish_write_check_BRAM_pidx++;
            }
            if (finish_write_check_URAM_pidx < BV_NUM_URAM_PARTITIONS &&
                uram_queried_vals_buf[finish_write_check_URAM_pidx][0].valid == 0 &&
                uram_queried_vals_buf[finish_write_check_URAM_pidx][1].valid == 0
            ) {
                finish_write_check_URAM_pidx++;
            }
        }

        if (total_num_reads == MAX_NUM_READS) {
            #ifdef __DO_DEBUG_PRINTS__
            if (finished_reading == 0) {
                printf("QUERY UNIT %d kp0 and kp1 - h/p/s=(%d,all,all). Finished reading.\n",
                        hash_idx,
                        hash_idx
                );
            }
            #endif
            finished_reading = 1;
        }
        if (finish_write_check_BRAM_pidx == BV_NUM_BRAM_PARTITIONS &&
            finish_write_check_URAM_pidx == BV_NUM_URAM_PARTITIONS
        ) {
            #ifdef __DO_DEBUG_PRINTS__
            printf("QUERY UNIT %d kp0 and kp1 - h/p/s=(%d,all,all). Finished writing.\n",
                    hash_idx,
                    hash_idx
            );
            #endif
            finished_computing = 1;
        }

        /////////////////////
        // END OF FINISH COMPUTATION LOGIC
        /////////////////////


        #if ENABLE_PERF_CTRS
        if (cyclecount_enable) {
            cyclectr += 1;
        }
        #endif

        BV_BRAM_PARTITION_LOOP:
        for (int bram_partition_idx = 0; bram_partition_idx < BV_NUM_BRAM_PARTITIONS; ++bram_partition_idx) {
        #pragma HLS UNROLL

            //////////////////////////////////////////////////////////
            // READ LOGIC

            // READ PORT 0
            if (!bv_lookup_stream_kp0[bram_partition_idx].empty() &&
                !bram_queried_vals_buf[bram_partition_idx][0].valid)
            {
                PACKED_HASH_DTYPE   packed_hash;
                METADATA_DTYPE      cur_metadata;
                LOOKUPIDX_DTYPE      bv_lookup_idx;
                BIT_DTYPE           cur_bv_val;
                BV_PLUS_METADATA_PACKED_DTYPE     data_to_write;

                LOOKUPIDX_DTYPE      bv_outer_idx;
                LOOKUPIDX_DTYPE      bv_inner_idx;

                #if ENABLE_PERF_CTRS
                cyclecount_enable = 1;
                #endif

                packed_hash = bv_lookup_stream_kp0[bram_partition_idx].read();
                num_reads_per_partition[bram_partition_idx]++;

                // Unpack the values
                cur_metadata = packed_hash.md;
                bv_lookup_idx = packed_hash.hash;

                // Read the bitvector
                bv_outer_idx = bv_lookup_idx/BV_BRAM_PACKED_BITWIDTH;
                bv_inner_idx = bv_lookup_idx%BV_BRAM_PACKED_BITWIDTH;
                cur_bv_val.range(0, 0) =
                    bv_buf_BRAMS[bram_partition_idx][bv_outer_idx].range(bv_inner_idx, bv_inner_idx);

                #ifdef __DO_DEBUG_PRINTS__
                printf("QUERY UNIT %d kp%d - h/p/s=(%d,%d,%d). input idx %d: This gave a bit value of %d.\n",
                        hash_idx,
                        0,
                        hash_idx,
                        bram_partition_idx,
                        cur_metadata.sidx.to_int(),
                        cur_metadata.iidx.to_int(),
                        cur_bv_val.to_int()
                );
                #endif

                // Pack final payload
                data_to_write.md = cur_metadata;
                data_to_write.bv_val = cur_bv_val;

                bram_queried_vals_buf[bram_partition_idx][0].valid = 1;
                bram_queried_vals_buf[bram_partition_idx][0].data = data_to_write;
            }

            // READ PORT 1
            if (!bv_lookup_stream_kp1[bram_partition_idx].empty() &&
                !bram_queried_vals_buf[bram_partition_idx][1].valid)
            {
                PACKED_HASH_DTYPE   packed_hash;
                METADATA_DTYPE      cur_metadata;
                LOOKUPIDX_DTYPE      bv_lookup_idx;
                BIT_DTYPE           cur_bv_val;
                BV_PLUS_METADATA_PACKED_DTYPE     data_to_write;

                LOOKUPIDX_DTYPE      bv_outer_idx;
                LOOKUPIDX_DTYPE      bv_inner_idx;

                #if ENABLE_PERF_CTRS
                cyclecount_enable = 1;
                #endif

                packed_hash = bv_lookup_stream_kp1[bram_partition_idx].read();

                num_reads_per_partition[bram_partition_idx]++;

                // Unpack the values
                cur_metadata = packed_hash.md;
                bv_lookup_idx = packed_hash.hash;

                // Read the bitvector
                bv_outer_idx = bv_lookup_idx/BV_BRAM_PACKED_BITWIDTH;
                bv_inner_idx = bv_lookup_idx%BV_BRAM_PACKED_BITWIDTH;
                cur_bv_val.range(0, 0) =
                    bv_buf_BRAMS[bram_partition_idx][bv_outer_idx].range(bv_inner_idx, bv_inner_idx);

                #ifdef __DO_DEBUG_PRINTS__
                printf("QUERY UNIT %d kp%d - h/p/s=(%d,%d,%d). input idx %d: This gave a bit value of %d.\n",
                        hash_idx,
                        1,
                        hash_idx,
                        bram_partition_idx,
                        cur_metadata.sidx.to_int(),
                        cur_metadata.iidx.to_int(),
                        cur_bv_val.to_int()
                );
                #endif

                // Pack final payload
                data_to_write.md = cur_metadata;
                data_to_write.bv_val = cur_bv_val;

                bram_queried_vals_buf[bram_partition_idx][1].valid = 1;
                bram_queried_vals_buf[bram_partition_idx][1].data = data_to_write;
            }

            //////////////////////////////////////////////////////////
            // WRITE LOGIC
            // WRITE PORT 0
            if (bram_queried_vals_buf[bram_partition_idx][0].valid &&
                query_bv_packed_stream_kp0[bram_partition_idx].try_write(
                    bram_queried_vals_buf[bram_partition_idx][0].data
                )
            ) {
                bram_queried_vals_buf[bram_partition_idx][0].valid = 0;
            }

            // WRITE PORT 1
            if (bram_queried_vals_buf[bram_partition_idx][1].valid &&
                query_bv_packed_stream_kp1[bram_partition_idx].try_write(
                    bram_queried_vals_buf[bram_partition_idx][1].data
                )
            ) {
                bram_queried_vals_buf[bram_partition_idx][1].valid = 0;
            }
        }






























        BV_URAM_PARTITION_LOOP:
        for (int uram_partition_idx = 0; uram_partition_idx < BV_NUM_URAM_PARTITIONS; ++uram_partition_idx) {
        #pragma HLS UNROLL

            //////////////////////////////////////////////////////////
            // READ LOGIC

            // READ PORT 0
            if (!bv_lookup_stream_kp0[uram_partition_idx + BV_NUM_BRAM_PARTITIONS].empty() &&
                !uram_queried_vals_buf[uram_partition_idx][0].valid)
            {
                PACKED_HASH_DTYPE   packed_hash;
                METADATA_DTYPE      cur_metadata;
                LOOKUPIDX_DTYPE      bv_lookup_idx;
                BIT_DTYPE           cur_bv_val;
                BV_PLUS_METADATA_PACKED_DTYPE     data_to_write;

                LOOKUPIDX_DTYPE      bv_outer_idx;
                LOOKUPIDX_DTYPE      bv_inner_idx;

                #if ENABLE_PERF_CTRS
                cyclecount_enable = 1;
                #endif

                packed_hash = bv_lookup_stream_kp0[uram_partition_idx + BV_NUM_BRAM_PARTITIONS].read();
                num_reads_per_partition[uram_partition_idx + BV_NUM_BRAM_PARTITIONS]++;

                // Unpack the values
                cur_metadata = packed_hash.md;
                bv_lookup_idx = packed_hash.hash;

                // Read the bitvector
                bv_outer_idx = bv_lookup_idx/BV_URAM_PACKED_BITWIDTH;
                bv_inner_idx = bv_lookup_idx%BV_URAM_PACKED_BITWIDTH;
                cur_bv_val.range(0, 0) =
                    bv_buf_URAMS[uram_partition_idx][bv_outer_idx].range(bv_inner_idx, bv_inner_idx);

                #ifdef __DO_DEBUG_PRINTS__
                printf("QUERY UNIT %d kp%d - h/p/s=(%d,%d,%d). input idx %d: This gave a bit value of %d.\n",
                        hash_idx,
                        0,
                        hash_idx,
                        uram_partition_idx + BV_NUM_BRAM_PARTITIONS,
                        cur_metadata.sidx.to_int(),
                        cur_metadata.iidx.to_int(),
                        cur_bv_val.to_int()
                );
                #endif

                // Pack final payload
                data_to_write.md = cur_metadata;
                data_to_write.bv_val = cur_bv_val;

                uram_queried_vals_buf[uram_partition_idx][0].valid = 1;
                uram_queried_vals_buf[uram_partition_idx][0].data = data_to_write;
            }

            // READ PORT 1
            if (!bv_lookup_stream_kp1[uram_partition_idx + BV_NUM_BRAM_PARTITIONS].empty() &&
                !uram_queried_vals_buf[uram_partition_idx][1].valid)
            {
                PACKED_HASH_DTYPE   packed_hash;
                METADATA_DTYPE      cur_metadata;
                LOOKUPIDX_DTYPE      bv_lookup_idx;
                BIT_DTYPE           cur_bv_val;
                BV_PLUS_METADATA_PACKED_DTYPE     data_to_write;

                LOOKUPIDX_DTYPE      bv_outer_idx;
                LOOKUPIDX_DTYPE      bv_inner_idx;

                #if ENABLE_PERF_CTRS
                cyclecount_enable = 1;
                #endif

                packed_hash = bv_lookup_stream_kp1[uram_partition_idx + BV_NUM_BRAM_PARTITIONS].read();
                num_reads_per_partition[uram_partition_idx + BV_NUM_BRAM_PARTITIONS]++;

                // Unpack the values
                cur_metadata = packed_hash.md;
                bv_lookup_idx = packed_hash.hash;

                // Read the bitvector
                bv_outer_idx = bv_lookup_idx/BV_URAM_PACKED_BITWIDTH;
                bv_inner_idx = bv_lookup_idx%BV_URAM_PACKED_BITWIDTH;
                cur_bv_val.range(0, 0) =
                    bv_buf_URAMS[uram_partition_idx][bv_outer_idx].range(bv_inner_idx, bv_inner_idx);

                #ifdef __DO_DEBUG_PRINTS__
                printf("QUERY UNIT %d kp%d - h/p/s=(%d,%d,%d). input idx %d: This gave a bit value of %d.\n",
                        hash_idx,
                        1,
                        hash_idx,
                        uram_partition_idx + BV_NUM_BRAM_PARTITIONS,
                        cur_metadata.sidx.to_int(),
                        cur_metadata.iidx.to_int(),
                        cur_bv_val.to_int()
                );
                #endif

                // Pack final payload
                data_to_write.md = cur_metadata;
                data_to_write.bv_val = cur_bv_val;

                uram_queried_vals_buf[uram_partition_idx][1].valid = 1;
                uram_queried_vals_buf[uram_partition_idx][1].data = data_to_write;
            }

            //////////////////////////////////////////////////////////
            // WRITE LOGIC
            // WRITE PORT 0
            if (uram_queried_vals_buf[uram_partition_idx][0].valid &&
                query_bv_packed_stream_kp0[uram_partition_idx + BV_NUM_BRAM_PARTITIONS].try_write(
                    uram_queried_vals_buf[uram_partition_idx][0].data
                )
            ) {
                uram_queried_vals_buf[uram_partition_idx][0].valid = 0;
            }

            // WRITE PORT 1
            if (uram_queried_vals_buf[uram_partition_idx][1].valid &&
                query_bv_packed_stream_kp1[uram_partition_idx + BV_NUM_BRAM_PARTITIONS].try_write(
                    uram_queried_vals_buf[uram_partition_idx][1].data
                )
            ) {
                uram_queried_vals_buf[uram_partition_idx][1].valid = 0;
            }
        }

    }

    #if ENABLE_PERF_CTRS
    cyclectr_out.write(cyclectr);
    #endif

    #ifdef __DO_DEBUG_PRINTS__
    printf("\n\nQUERY UNIT %d - DONE NOW.\n\n",
            hash_idx
    );
    #endif

    return;


}



#define QUERY_INVOKES   \
        .invoke(\
                queryResult_per_hash\
                , 0\
                , bv_load_stream_0\
                , bv_lookup_stream_h0_kp0\
                , bv_lookup_stream_h0_kp1\
                , query_bv_packed_stream_hash0_kp0\
                , query_bv_packed_stream_hash0_kp1\
                \
                , perfctr_stms[0]\
                , NUM_LOADS_PER_STM \
        )\
        .invoke(\
                queryResult_per_hash\
                , 1\
                , bv_load_stream_1\
                , bv_lookup_stream_h1_kp0\
                , bv_lookup_stream_h1_kp1\
                , query_bv_packed_stream_hash1_kp0\
                , query_bv_packed_stream_hash1_kp1\
                \
                , perfctr_stms[1]\
                , NUM_LOADS_PER_STM \
        )\
        .invoke(\
                queryResult_per_hash\
                , 2\
                , bv_load_stream_2\
                , bv_lookup_stream_h2_kp0\
                , bv_lookup_stream_h2_kp1\
                , query_bv_packed_stream_hash2_kp0\
                , query_bv_packed_stream_hash2_kp1\
                \
                , perfctr_stms[2]\
                , NUM_LOADS_PER_STM \
        )\
        .invoke(\
                queryResult_per_hash\
                , 3\
                , bv_load_stream_3\
                , bv_lookup_stream_h3_kp0\
                , bv_lookup_stream_h3_kp1\
                , query_bv_packed_stream_hash3_kp0\
                , query_bv_packed_stream_hash3_kp1\
                \
                , perfctr_stms[3]\
                , NUM_LOADS_PER_STM \
        )\
        .invoke(\
                queryResult_per_hash\
                , 4\
                , bv_load_stream_4\
                , bv_lookup_stream_h4_kp0\
                , bv_lookup_stream_h4_kp1\
                , query_bv_packed_stream_hash4_kp0\
                , query_bv_packed_stream_hash4_kp1\
                \
                , perfctr_stms[4]\
                , NUM_LOADS_PER_STM \
        )\

#if NUM_HASH != 5
crash compilation()
#endif



/*************************************************************************************/





//////////////////////////////////////////////////
//////////////////////////////////////////////////
///////// UnShuffle                         //////
//////////////////////////////////////////////////
//////////////////////////////////////////////////



void shuffle_TtoS_per_hash(
        int shuffle_idx
        ,int kp_idx

        ,tapa::istreams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS> & query_bv_packed_stream
        ,tapa::ostreams<BV_PLUS_IIDX_PACKED_DTYPE, NUM_STM*BV_NUM_PARTITIONS> & inter_shuffle_stream
){
    #ifndef __SYNTHESIS__
    printf("INFO: Using separated shuffle.\n");
    #endif

    typedef struct {
        BIT_DTYPE       BV;
        INPUT_IDX_DTYPE     input_idx;
        bool                valid;
    } PEEKED_DTYPE;

    // This is a buffer for data from each partition.
    // We also introduce a NUM_STM dimension, otherwise we hang.
    PEEKED_DTYPE shuffle_peek_emulator[BV_NUM_PARTITIONS][NUM_STM];
    #pragma HLS ARRAY_PARTITION variable=shuffle_peek_emulator dim=0 complete


    PEEK_EMULATOR_INITIALIZATION:
    for (int j=0; j<BV_NUM_PARTITIONS; ++j){
    #pragma HLS UNROLL
        for (int k=0; k<NUM_STM; ++k) {
        #pragma HLS UNROLL
            shuffle_peek_emulator[j][k].BV = 0;
            shuffle_peek_emulator[j][k].input_idx = 0;
            shuffle_peek_emulator[j][k].valid = 0;
        }
    }


    while(1)
    {
    #pragma HLS PIPELINE II=1

        ////////////////////////////////////////////
        // READ LOGIC. Read from each partition stream
        ////////////////////////////////////////////
        RD_BV_PARTITION_LOOP:
        for (int partition_idx = 0;
                partition_idx < BV_NUM_PARTITIONS;
                ++partition_idx)
        {
        #pragma HLS UNROLL

            // DATAPACKED TRANSFER:
            INPUT_IDX_DTYPE     cur_input_idx;
            STRM_IDX_DTYPE      cur_strm_idx;
            BIT_DTYPE           cur_bv_val;
            METADATA_DTYPE      cur_metadata;
            BV_PLUS_METADATA_PACKED_DTYPE     cur_packed_data;
            bool                peek_success;

            peek_success = query_bv_packed_stream[partition_idx].try_peek(
                cur_packed_data
            );

            cur_metadata = cur_packed_data.md;
            cur_bv_val = cur_packed_data.bv_val;

            // Unpack metadata
            cur_strm_idx = cur_metadata.sidx;
            cur_input_idx = cur_metadata.iidx;

                // If the current "peeked" value is not valid, overwrite it
                // with valid data.
                if (peek_success &&
                    shuffle_peek_emulator[partition_idx][cur_strm_idx].valid == 0
                )
                {
                    query_bv_packed_stream[partition_idx].read();

                    // Write it into the buffer
                    shuffle_peek_emulator[partition_idx]
                        [cur_strm_idx].valid = 1;
                    shuffle_peek_emulator[partition_idx]
                        [cur_strm_idx].BV = cur_bv_val;
                    shuffle_peek_emulator[partition_idx]
                        [cur_strm_idx].input_idx = cur_input_idx;

                    #ifdef __DO_DEBUG_PRINTS__
                    printf("SHUFFLE TtoS #%d kp%d - (Strm #%d): Read from (hash, partition, stream) [%d][%d][%d], input_idx=%d\n",
                        shuffle_idx,
                        kp_idx,
                        cur_strm_idx.to_int(),
                        shuffle_idx,
                        partition_idx,
                        cur_strm_idx.to_int(),
                        cur_input_idx.to_int()
                    );
                    #endif
                }
                #ifdef __DO_DEBUG_PRINTS__
                else if (!peek_success)
                {
                    // do nothing
                } else {
                    printf("SHUFFLE TtoS #%d kp%d - peeked value is valid for hash %d, partition %d, strm %d\n",
                            shuffle_idx,
                            kp_idx,
                            shuffle_idx,
                            partition_idx,
                            cur_strm_idx.to_int()
                    );
                }
                #endif
        }

        ////////////////////////////////////////////
        // WRITE OUTPUTS from the shuffle-buffer
        ////////////////////////////////////////////
        WR_STM_LOOP:
        for (int strm_idx = 0; strm_idx < NUM_STM; ++strm_idx)
        {
            WR_BV_PARTITION_LOOP:
            for (int partition_idx = 0;
                    partition_idx < BV_NUM_PARTITIONS;
                    ++partition_idx)
            {
            #pragma HLS UNROLL
                BV_PLUS_IIDX_PACKED_DTYPE   outval;
                outval.bv_val = shuffle_peek_emulator[partition_idx][strm_idx].BV;
                outval.iidx = shuffle_peek_emulator[partition_idx][strm_idx].input_idx;

                if (shuffle_peek_emulator[partition_idx][strm_idx].valid == 1 &&
                    inter_shuffle_stream[strm_idx*BV_NUM_PARTITIONS + partition_idx].try_write(outval)
                ) {
                    shuffle_peek_emulator[partition_idx][strm_idx].valid = 0;

                    #ifdef __DO_DEBUG_PRINTS__
                    INPUT_IDX_DTYPE     cur_input_idx;
                    cur_input_idx = shuffle_peek_emulator[partition_idx][strm_idx].input_idx;
                    printf("SHUFFLE TtoS #%d kp%d - (Strm #%d): Outputting BV_val=%d to partition=%d. From hash=%d, input_idx=%d\n",
                            shuffle_idx,
                            kp_idx,
                            strm_idx,
                            outval.bv_val.to_int(),
                            partition_idx,
                            shuffle_idx,
                            cur_input_idx.to_int()
                    );
                    #endif
                }
            }
        }
    }


    #ifdef __DO_DEBUG_PRINTS__
    printf("\n\nSHUFFLE TtoS #%d kp%d - DONE NOW.\n\n",
            shuffle_idx, kp_idx
    );
    #endif

    return;
}


void shuffle_reordering_per_hash(
        int shuffle_idx
        ,int stm_idx
        ,int kp_idx

        ,tapa::istream<BV_PLUS_IIDX_PACKED_DTYPE> & inter_shuffle_stream_p0
        ,tapa::istream<BV_PLUS_IIDX_PACKED_DTYPE> & inter_shuffle_stream_p1
        ,tapa::istream<BV_PLUS_IIDX_PACKED_DTYPE> & inter_shuffle_stream_p2
        ,tapa::istream<BV_PLUS_IIDX_PACKED_DTYPE> & inter_shuffle_stream_p3
        ,tapa::istream<BV_PLUS_IIDX_PACKED_DTYPE> & inter_shuffle_stream_p4
        ,tapa::istream<BV_PLUS_IIDX_PACKED_DTYPE> & inter_shuffle_stream_p5
        ,tapa::istream<BV_PLUS_IIDX_PACKED_DTYPE> & inter_shuffle_stream_p6
        ,tapa::istream<BV_PLUS_IIDX_PACKED_DTYPE> & inter_shuffle_stream_p7
        ,tapa::istream<BV_PLUS_IIDX_PACKED_DTYPE> & inter_shuffle_stream_p8
        
        #if BV_NUM_PARTITIONS != 9
        crash!,,,
        #endif

        ,tapa::ostream<BIT_DTYPE> & reconstruct_stream
        ,int NUM_LOADS_PER_STM
)
{
    typedef struct {
        BIT_DTYPE           bv;
        INPUT_IDX_DTYPE     iidx;
        bool                valid;
    } PEEKED_DTYPE;

    int next_output_idx = 1;

    PEEKED_DTYPE                shufbuf[BV_NUM_PARTITIONS];
    #pragma HLS ARRAY_PARTITION variable=shufbuf dim=0 complete

    #ifdef __DO_DEBUG_PRINTS__
    printf("HELLO from shuffle-ordering, hash %d, stm %d, kp %d!\n",
            shuffle_idx, stm_idx, kp_idx);
    #endif

    PEEK_EMULATOR_INIT:
    for (int p = 0; p < BV_NUM_PARTITIONS; ++p) {
        shufbuf[p].bv = 0;
        shufbuf[p].iidx = 0;
        shufbuf[p].valid = 0;
    }

    MAIN_LOOP:
    while(1)
    {
    #pragma HLS PIPELINE II=1

        /////////////////////////////
        // READ LOGIC
        /////////////////////////////

        #define READ_PARTITION(PART)    \
            BV_PLUS_IIDX_PACKED_DTYPE   read_val_p##PART;   \
            \
            if (!inter_shuffle_stream_p##PART.empty() && \
                shufbuf[PART].valid == 0  \
            ) { \
                read_val_p##PART = inter_shuffle_stream_p##PART.read();  \
            \
                shufbuf[PART].bv = read_val_p##PART.bv_val;   \
                shufbuf[PART].iidx = read_val_p##PART.iidx;   \
                shufbuf[PART].valid = 1;  \
            }   \

                //#ifdef __DO_DEBUG_PRINTS__
                //printf("SHUFFLE ORDERING stm%d kp%d hash%d - read BV %d from partition %d, iidx %d, into buf_idx %d\n",
                //        stm_idx, kp_idx, shuffle_idx,
                //        read_val.bv_val.to_int(),
                //        partition_idx,
                //        read_val.iidx.to_int(),
                //        rd_buf_idx
                //);
                //#endif

        READ_PARTITION(0)
        READ_PARTITION(1)
        READ_PARTITION(2)
        READ_PARTITION(3)
        READ_PARTITION(4)
        READ_PARTITION(5)
        READ_PARTITION(6)
        READ_PARTITION(7)
        READ_PARTITION(8)

        #if BV_NUM_PARTITIONS != 9
        crash();
        #endif



        /////////////////////////////
        // WRITE LOGIC
        /////////////////////////////
        bool    write_ready = 0;
        int     ready_partition_idx = 0;

         /*** NOTE:
          * The check for input_idx == next_output_idx
          * is necessary to make sure were looking at the right PARTITION. Any partition could have some valid data.
          * But only ONE partition has ***the*** valid data with next_output_idx.
          */ 
        for (int partition_idx = 0; partition_idx < BV_NUM_PARTITIONS; ++partition_idx)
        {
            if (shufbuf[partition_idx].valid    == 1 &&
                shufbuf[partition_idx].iidx     == next_output_idx
            )
            {
                write_ready = 1;
                ready_partition_idx = partition_idx;
                break;
            }
        }

        if (write_ready){
            BIT_DTYPE write_success;
            BIT_DTYPE v = shufbuf[ready_partition_idx].bv;

            write_success = reconstruct_stream.try_write(v);

            if (write_success) {
                shufbuf[ready_partition_idx].valid = 0;

                // MANOJ: Try ternary operator, it should be better.
                if (next_output_idx == NUM_LOADS_PER_STM) {
                    next_output_idx = 1;
                }
                else {
                    next_output_idx++;
                }

                #ifdef __DO_DEBUG_PRINTS__
                printf("SHUFFLE ORDERING stm%d kp%d hash%d - write BV %d\n",
                        stm_idx, kp_idx, shuffle_idx,
                        v.to_int()
                );
                #endif
            }
        }
    }
}



#define SHUFFLE_STREAM_DECLS_KP(KP_IDX)    \
    tapa::streams<BV_PLUS_IIDX_PACKED_DTYPE, NUM_STM*BV_NUM_PARTITIONS, ARB_RATELIM_DISTANCE> inter_shuf0_stm_kp##KP_IDX;   \
    tapa::streams<BV_PLUS_IIDX_PACKED_DTYPE, NUM_STM*BV_NUM_PARTITIONS, ARB_RATELIM_DISTANCE> inter_shuf1_stm_kp##KP_IDX;   \
    tapa::streams<BV_PLUS_IIDX_PACKED_DTYPE, NUM_STM*BV_NUM_PARTITIONS, ARB_RATELIM_DISTANCE> inter_shuf2_stm_kp##KP_IDX;   \
    tapa::streams<BV_PLUS_IIDX_PACKED_DTYPE, NUM_STM*BV_NUM_PARTITIONS, ARB_RATELIM_DISTANCE> inter_shuf3_stm_kp##KP_IDX;   \
    tapa::streams<BV_PLUS_IIDX_PACKED_DTYPE, NUM_STM*BV_NUM_PARTITIONS, ARB_RATELIM_DISTANCE> inter_shuf4_stm_kp##KP_IDX;   \

#if NUM_HASH != 5
crash!,
#endif




#define SHUFFLE_REORDER_INVOKES_FOR_SHUF_KP(SHUF_IDX, KP_IDX)      \
        .invoke<tapa::detach>(  \
            shuffle_reordering_per_hash \
            ,SHUF_IDX   \
            ,0  \
            ,KP_IDX \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[0+BV_NUM_PARTITIONS*0]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[1+BV_NUM_PARTITIONS*0]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[2+BV_NUM_PARTITIONS*0]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[3+BV_NUM_PARTITIONS*0]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[4+BV_NUM_PARTITIONS*0]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[5+BV_NUM_PARTITIONS*0]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[6+BV_NUM_PARTITIONS*0]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[7+BV_NUM_PARTITIONS*0]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[8+BV_NUM_PARTITIONS*0]    \
            ,reconstruct_stream_stm0_kp##KP_IDX[SHUF_IDX] \
            ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(  \
            shuffle_reordering_per_hash \
            ,SHUF_IDX   \
            ,1  \
            ,KP_IDX \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[0+BV_NUM_PARTITIONS*1]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[1+BV_NUM_PARTITIONS*1]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[2+BV_NUM_PARTITIONS*1]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[3+BV_NUM_PARTITIONS*1]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[4+BV_NUM_PARTITIONS*1]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[5+BV_NUM_PARTITIONS*1]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[6+BV_NUM_PARTITIONS*1]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[7+BV_NUM_PARTITIONS*1]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[8+BV_NUM_PARTITIONS*1]    \
            ,reconstruct_stream_stm1_kp##KP_IDX[SHUF_IDX] \
            ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(  \
            shuffle_reordering_per_hash \
            ,SHUF_IDX   \
            ,2  \
            ,KP_IDX \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[0+BV_NUM_PARTITIONS*2]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[1+BV_NUM_PARTITIONS*2]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[2+BV_NUM_PARTITIONS*2]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[3+BV_NUM_PARTITIONS*2]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[4+BV_NUM_PARTITIONS*2]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[5+BV_NUM_PARTITIONS*2]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[6+BV_NUM_PARTITIONS*2]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[7+BV_NUM_PARTITIONS*2]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[8+BV_NUM_PARTITIONS*2]    \
            ,reconstruct_stream_stm2_kp##KP_IDX[SHUF_IDX] \
            ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(  \
            shuffle_reordering_per_hash \
            ,SHUF_IDX   \
            ,3  \
            ,KP_IDX \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[0+BV_NUM_PARTITIONS*3]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[1+BV_NUM_PARTITIONS*3]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[2+BV_NUM_PARTITIONS*3]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[3+BV_NUM_PARTITIONS*3]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[4+BV_NUM_PARTITIONS*3]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[5+BV_NUM_PARTITIONS*3]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[6+BV_NUM_PARTITIONS*3]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[7+BV_NUM_PARTITIONS*3]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[8+BV_NUM_PARTITIONS*3]    \
            ,reconstruct_stream_stm3_kp##KP_IDX[SHUF_IDX] \
            ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(  \
            shuffle_reordering_per_hash \
            ,SHUF_IDX   \
            ,4  \
            ,KP_IDX \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[0+BV_NUM_PARTITIONS*4]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[1+BV_NUM_PARTITIONS*4]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[2+BV_NUM_PARTITIONS*4]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[3+BV_NUM_PARTITIONS*4]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[4+BV_NUM_PARTITIONS*4]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[5+BV_NUM_PARTITIONS*4]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[6+BV_NUM_PARTITIONS*4]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[7+BV_NUM_PARTITIONS*4]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[8+BV_NUM_PARTITIONS*4]    \
            ,reconstruct_stream_stm4_kp##KP_IDX[SHUF_IDX] \
            ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(  \
            shuffle_reordering_per_hash \
            ,SHUF_IDX   \
            ,5  \
            ,KP_IDX \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[0+BV_NUM_PARTITIONS*5]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[1+BV_NUM_PARTITIONS*5]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[2+BV_NUM_PARTITIONS*5]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[3+BV_NUM_PARTITIONS*5]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[4+BV_NUM_PARTITIONS*5]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[5+BV_NUM_PARTITIONS*5]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[6+BV_NUM_PARTITIONS*5]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[7+BV_NUM_PARTITIONS*5]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[8+BV_NUM_PARTITIONS*5]    \
            ,reconstruct_stream_stm5_kp##KP_IDX[SHUF_IDX] \
            ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(  \
            shuffle_reordering_per_hash \
            ,SHUF_IDX   \
            ,6  \
            ,KP_IDX \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[0+BV_NUM_PARTITIONS*6]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[1+BV_NUM_PARTITIONS*6]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[2+BV_NUM_PARTITIONS*6]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[3+BV_NUM_PARTITIONS*6]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[4+BV_NUM_PARTITIONS*6]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[5+BV_NUM_PARTITIONS*6]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[6+BV_NUM_PARTITIONS*6]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[7+BV_NUM_PARTITIONS*6]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[8+BV_NUM_PARTITIONS*6]    \
            ,reconstruct_stream_stm6_kp##KP_IDX[SHUF_IDX] \
            ,NUM_LOADS_PER_STM  \
        )   \
        .invoke<tapa::detach>(  \
            shuffle_reordering_per_hash \
            ,SHUF_IDX   \
            ,7  \
            ,KP_IDX \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[0+BV_NUM_PARTITIONS*7]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[1+BV_NUM_PARTITIONS*7]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[2+BV_NUM_PARTITIONS*7]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[3+BV_NUM_PARTITIONS*7]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[4+BV_NUM_PARTITIONS*7]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[5+BV_NUM_PARTITIONS*7]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[6+BV_NUM_PARTITIONS*7]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[7+BV_NUM_PARTITIONS*7]    \
            ,inter_shuf##SHUF_IDX##_stm_kp##KP_IDX[8+BV_NUM_PARTITIONS*7]    \
            ,reconstruct_stream_stm7_kp##KP_IDX[SHUF_IDX] \
            ,NUM_LOADS_PER_STM  \
        )   \

#if BV_NUM_PARTITIONS != 9
crash!,
#endif
#if NUM_STM != 8
crash!,
#endif


#define SHUFFLE_INVOKES_FOR_KP(KP_IDX)     \
        .invoke<tapa::detach>(    \
            shuffle_TtoS_per_hash    \
            , 0 \
            , KP_IDX    \
            , query_bv_packed_stream_hash0_kp##KP_IDX  \
            , inter_shuf0_stm_kp##KP_IDX \
            \
        )   \
        .invoke<tapa::detach>(    \
            shuffle_TtoS_per_hash    \
            , 1 \
            , KP_IDX    \
            , query_bv_packed_stream_hash1_kp##KP_IDX  \
            , inter_shuf1_stm_kp##KP_IDX \
            \
        )   \
        .invoke<tapa::detach>(    \
            shuffle_TtoS_per_hash    \
            , 2 \
            , KP_IDX    \
            , query_bv_packed_stream_hash2_kp##KP_IDX  \
            , inter_shuf2_stm_kp##KP_IDX \
            \
        )   \
        .invoke<tapa::detach>(    \
            shuffle_TtoS_per_hash    \
            , 3 \
            , KP_IDX    \
            , query_bv_packed_stream_hash3_kp##KP_IDX  \
            , inter_shuf3_stm_kp##KP_IDX \
            \
        )   \
        .invoke<tapa::detach>(    \
            shuffle_TtoS_per_hash    \
            , 4 \
            , KP_IDX    \
            , query_bv_packed_stream_hash4_kp##KP_IDX  \
            , inter_shuf4_stm_kp##KP_IDX \
            \
        )   \
        SHUFFLE_REORDER_INVOKES_FOR_SHUF_KP(0, KP_IDX)  \
        SHUFFLE_REORDER_INVOKES_FOR_SHUF_KP(1, KP_IDX)  \
        SHUFFLE_REORDER_INVOKES_FOR_SHUF_KP(2, KP_IDX)  \
        SHUFFLE_REORDER_INVOKES_FOR_SHUF_KP(3, KP_IDX)  \
        SHUFFLE_REORDER_INVOKES_FOR_SHUF_KP(4, KP_IDX)  \

#if NUM_HASH != 5
crash!
#endif







//////////////////////////////////////////////////
//////////////////////////////////////////////////
///////// END OF UnShuffle                  //////
//////////////////////////////////////////////////
//////////////////////////////////////////////////




void bloom_aggregate_SPLIT(
        int     agg_idx
        ,int     kp_idx
        ,tapa::istreams<BIT_DTYPE, NUM_HASH>   & reconstruct_stream
        ,tapa::ostream<BIT_DTYPE>   & aggregate_stream
        ,int NUM_LOADS_PER_STM
){
    #ifndef __SYNTHESIS__
    //printf("NOTE: Using SPLIT AGGREGATE!!\n");
    #endif

    int num_writes_TOTAL = 0;
    int num_reads = 0;
    int all_hashes_available = 0;
    uint32_t result = 1;

    while (num_writes_TOTAL < NUM_LOADS_PER_STM)
    {
    #pragma HLS PIPELINE=1
        // Check if all of our hash values are available:
        all_hashes_available = 1;

        for (int i = 0; i < NUM_HASH; ++i) {
            if (reconstruct_stream[i].empty()) {
                all_hashes_available = 0;
            } 
        }

        if (all_hashes_available)
        {
            result = 1;
            for (int i = 0; i < NUM_HASH; ++i) {
                result &= reconstruct_stream[i].read();
            }

            num_reads++;
            #ifdef __DO_DEBUG_PRINTS__
            printf("AGGREGATE #%d kp%d - input query %d got a value of %d\n",
                    agg_idx, kp_idx, num_reads, result
            );
            #endif

            aggregate_stream.write(result);
            num_writes_TOTAL++;
        }
    }

    #ifdef __DO_DEBUG_PRINTS__
    printf("\n\nAGGREGATE #%d kp%d - DONE NOW.\n\n",
            agg_idx, kp_idx
    );
    #endif
    return;
}



#define AGGREGATE_INVOKES_FOR_KP(KP_IDX)    \
        .invoke(bloom_aggregate_SPLIT  \
                    ,0  \
                    ,KP_IDX \
                    ,reconstruct_stream_stm0_kp##KP_IDX \
                    ,aggregate_stream_kp##KP_IDX[0]  \
                    ,NUM_LOADS_PER_STM   \
        )   \
        .invoke(bloom_aggregate_SPLIT  \
                    ,1  \
                    ,KP_IDX \
                    ,reconstruct_stream_stm1_kp##KP_IDX \
                    ,aggregate_stream_kp##KP_IDX[1]  \
                    ,NUM_LOADS_PER_STM   \
        )   \
        .invoke(bloom_aggregate_SPLIT  \
                    ,2  \
                    ,KP_IDX \
                    ,reconstruct_stream_stm2_kp##KP_IDX \
                    ,aggregate_stream_kp##KP_IDX[2]  \
                    ,NUM_LOADS_PER_STM   \
        )   \
        .invoke(bloom_aggregate_SPLIT  \
                    ,3  \
                    ,KP_IDX \
                    ,reconstruct_stream_stm3_kp##KP_IDX \
                    ,aggregate_stream_kp##KP_IDX[3]  \
                    ,NUM_LOADS_PER_STM   \
        )   \
        .invoke(bloom_aggregate_SPLIT  \
                    ,4  \
                    ,KP_IDX \
                    ,reconstruct_stream_stm4_kp##KP_IDX \
                    ,aggregate_stream_kp##KP_IDX[4]  \
                    ,NUM_LOADS_PER_STM   \
        )   \
        .invoke(bloom_aggregate_SPLIT  \
                    ,5  \
                    ,KP_IDX \
                    ,reconstruct_stream_stm5_kp##KP_IDX \
                    ,aggregate_stream_kp##KP_IDX[5]  \
                    ,NUM_LOADS_PER_STM   \
        )   \
        .invoke(bloom_aggregate_SPLIT  \
                    ,6  \
                    ,KP_IDX \
                    ,reconstruct_stream_stm6_kp##KP_IDX \
                    ,aggregate_stream_kp##KP_IDX[6]  \
                    ,NUM_LOADS_PER_STM   \
        )   \
        .invoke(bloom_aggregate_SPLIT  \
                    ,7  \
                    ,KP_IDX \
                    ,reconstruct_stream_stm7_kp##KP_IDX \
                    ,aggregate_stream_kp##KP_IDX[7]  \
                    ,NUM_LOADS_PER_STM   \
        )   \

#if NUM_STM != 8
crash!!!
#endif


/*************************************************************************************/

/*************************************************************************************/

#if ENABLE_PERF_CTRS

void write_perfctrs(
    tapa::istreams<PERFCTR_DTYPE, NUM_PERFCTR_MODULES>  & querycycle_in
    ,tapa::mmap<PERFCTR_DTYPE>                          perfctr_mmap
) {
    for (int i = 0; i < NUM_PERFCTR_MODULES; ++i) {
        perfctr_mmap[i] = querycycle_in[i].read();
    }
}

#endif

/*************************************************************************************/

void packOutput(
        int strm_idx
        ,int kp_idx
        ,tapa::istream<BIT_DTYPE>           & aggregate_stream
        ,tapa::ostream<OUT_PACKED_DTYPE>    & packed_outputs_stream
        ,int NUM_LOADS_PER_STM
) {
    int                 pk_idx;
    OUT_PACKED_DTYPE    packed;
    BIT_DTYPE           val;

    #ifdef __DO_DEBUG_PRINTS__
    int num_writes = 0;
    #endif

    for (int i = 0; i < NUM_LOADS_PER_STM; ++i) {
        pk_idx = i % OUT_PACKED_BITWIDTH;

        val = aggregate_stream.read();
        packed.range(pk_idx, pk_idx) = val.range(0, 0);

        if (pk_idx == OUT_PACKED_BITWIDTH - 1){
            packed_outputs_stream.write(packed);
            packed = 0;

            #ifdef __DO_DEBUG_PRINTS__
            num_writes += 1;
            #endif
        }
    }

    #ifdef __DO_DEBUG_PRINTS__
    printf("PACKOUTPUT #%d kp%d - finishing after writing %d times.\n",
            strm_idx, kp_idx, num_writes
    );
    #endif
}






void writeOutput_MAXSTM(
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s0_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s0_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s1_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s1_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s2_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s2_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s3_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s3_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s4_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s4_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s5_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s5_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s6_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s6_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s7_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s7_kp1,
#if _KENNY_USING_AURORA_
        tapa::ostream<STORE_DTYPE>     & out_bits_stm,
#else
        tapa::mmap<STORE_DTYPE>        out_bits_mmap,
#endif
        int NUM_LOADS_PER_STM
){
    STORE_DTYPE     to_store;
    int NUM_PACKED_OUTPUTS_PER_STM = CEIL_DIVISION(NUM_LOADS_PER_STM, OUT_PACKED_BITWIDTH);

    for (int i = 0; i < NUM_PACKED_OUTPUTS_PER_STM; ++i) {
        to_store.s0_k0 = packed_outputs_stream_s0_kp0.read();
        to_store.s0_k1 = packed_outputs_stream_s0_kp1.read();

        to_store.s1_k0 = packed_outputs_stream_s1_kp0.read();
        to_store.s1_k1 = packed_outputs_stream_s1_kp1.read();

        to_store.s2_k0 = packed_outputs_stream_s2_kp0.read();
        to_store.s2_k1 = packed_outputs_stream_s2_kp1.read();

        to_store.s3_k0 = packed_outputs_stream_s3_kp0.read();
        to_store.s3_k1 = packed_outputs_stream_s3_kp1.read();

        to_store.s4_k0 = packed_outputs_stream_s4_kp0.read();
        to_store.s4_k1 = packed_outputs_stream_s4_kp1.read();

        to_store.s5_k0 = packed_outputs_stream_s5_kp0.read();
        to_store.s5_k1 = packed_outputs_stream_s5_kp1.read();

        to_store.s6_k0 = packed_outputs_stream_s6_kp0.read();
        to_store.s6_k1 = packed_outputs_stream_s6_kp1.read();

        to_store.s7_k0 = packed_outputs_stream_s7_kp0.read();
        to_store.s7_k1 = packed_outputs_stream_s7_kp1.read();

#if _KENNY_USING_AURORA_
        out_bits_stm.write(to_store);
#else
        out_bits_mmap[i] = to_store;
#endif
    }

    #ifdef __DO_DEBUG_PRINTS__
    printf("WRITEOUTPUT - (synchronous version) exiting now\n");
    #endif
}






void writeOutput_REMAINDERSTM(
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s0_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s0_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s1_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s1_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s2_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s2_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s3_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s3_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s4_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s4_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s5_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s5_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s6_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s6_kp1,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s7_kp0,
        tapa::istream<OUT_PACKED_DTYPE>&  packed_outputs_stream_s7_kp1,
#if _KENNY_USING_AURORA_
        tapa::ostream<STORE_DTYPE>     & out_bits_stm,
#else
        tapa::mmap<STORE_DTYPE>        out_bits_mmap,
#endif
        int NUM_LOADS_PER_STM
){
    STORE_DTYPE     to_store;
    int NUM_PACKED_OUTPUTS_PER_STM = CEIL_DIVISION(NUM_LOADS_PER_STM, OUT_PACKED_BITWIDTH);

    for (int i = 0; i < NUM_PACKED_OUTPUTS_PER_STM; ++i) {
        to_store.s0_k0 = packed_outputs_stream_s0_kp0.read();
        to_store.s0_k1 = packed_outputs_stream_s0_kp1.read();

        to_store.s1_k0 = packed_outputs_stream_s1_kp0.read();
        to_store.s1_k1 = packed_outputs_stream_s1_kp1.read();

        to_store.s2_k0 = packed_outputs_stream_s2_kp0.read();
        to_store.s2_k1 = packed_outputs_stream_s2_kp1.read();

        to_store.s3_k0 = packed_outputs_stream_s3_kp0.read();
        to_store.s3_k1 = packed_outputs_stream_s3_kp1.read();

        to_store.s4_k0 = packed_outputs_stream_s4_kp0.read();
        to_store.s4_k1 = packed_outputs_stream_s4_kp1.read();

        to_store.s5_k0 = packed_outputs_stream_s5_kp0.read();
        to_store.s5_k1 = packed_outputs_stream_s5_kp1.read();

        to_store.s6_k0 = packed_outputs_stream_s6_kp0.read();
        to_store.s6_k1 = packed_outputs_stream_s6_kp1.read();

        to_store.s7_k0 = packed_outputs_stream_s7_kp0.read();
        to_store.s7_k1 = packed_outputs_stream_s7_kp1.read();

#if _KENNY_USING_AURORA_
        out_bits_stm.write(to_store);
#else
        out_bits_mmap[i] = to_store;
#endif
    }

    #ifdef __DO_DEBUG_PRINTS__
    printf("WRITEOUTPUT - (synchronous version) exiting now\n");
    #endif
}






#define WRITEOUT_INVOKES    \
    .invoke(writeOutput_REMAINDERSTM   \
                ,packed_output_stm_kp0[0] \
                ,packed_output_stm_kp1[0] \
                ,packed_output_stm_kp0[1] \
                ,packed_output_stm_kp1[1] \
                ,packed_output_stm_kp0[2] \
                ,packed_output_stm_kp1[2] \
                ,packed_output_stm_kp0[3] \
                ,packed_output_stm_kp1[3] \
                ,packed_output_stm_kp0[4] \
                ,packed_output_stm_kp1[4] \
                ,packed_output_stm_kp0[5] \
                ,packed_output_stm_kp1[5] \
                ,packed_output_stm_kp0[6] \
                ,packed_output_stm_kp1[6] \
                ,packed_output_stm_kp0[7] \
                ,packed_output_stm_kp1[7] \
                ,out_bits_0  \
                ,NUM_LOADS_PER_STM  \
    )    \





/*************************************************************************************/

void workload(
    tapa::mmap<BV_LOAD_DTYPE>       input_bv
#if _KENNY_USING_AURORA_
    ,tapa::istream<LOAD_DTYPE>      & key_in_0
    ,tapa::ostream<STORE_DTYPE>     & out_bits_0
#else   //_KENNY_USING_AURORA_
    ,tapa::mmap<LOAD_DTYPE>         key_in_0
    ,tapa::mmap<STORE_DTYPE>        out_bits_0
#endif  //_KENNY_USING_AURORA_

    #if NUM_AXI_PORTS != 1
    crash(compilation)
    #endif

    #if NUM_STM != 8
    , crash! //crash on purpose; we may need more streams.
    #endif

    #if ENABLE_PERF_CTRS
    ,tapa::mmap<PERFCTR_DTYPE>      perfctr_mmap
    #endif

    ,int                            NUM_LOADS_PER_STM
#if _KENNY_USING_AURORA_
    ,tapa::ostream<bool>            DUMMY_ack_out
    ,tapa::istream<bool>            DUMMY_ack_in
#endif  //_KENNY_USING_AURORA_
)


{
    //////////////////////////////////////////////////////////
    // Connections BETWEEN modules:
    tapa::streams<KEY_DTYPE, NUM_STM, STM_DEPTH> key_stream_kp0;
    tapa::streams<KEY_DTYPE, NUM_STM, STM_DEPTH> key_stream_kp1;

    // loadBV outputs
    tapa::stream<BV_URAM_PACKED_DTYPE, STM_DEPTH> bv_load_stream_0;
    tapa::stream<BV_URAM_PACKED_DTYPE, STM_DEPTH> bv_load_stream_1;
    tapa::stream<BV_URAM_PACKED_DTYPE, STM_DEPTH> bv_load_stream_2;
    tapa::stream<BV_URAM_PACKED_DTYPE, STM_DEPTH> bv_load_stream_3;
    tapa::stream<BV_URAM_PACKED_DTYPE, STM_DEPTH> bv_load_stream_4;
    #if NUM_HASH != 5
    crash!
    #endif

    // Computehash outputs (kp stands for key-pair)
    tapa::streams<COMP2ARB_DTYPE, NUM_STM, STM_DEPTH> comp2arb_stream_h0_kp0;
    tapa::streams<COMP2ARB_DTYPE, NUM_STM, STM_DEPTH> comp2arb_stream_h0_kp1;
    tapa::streams<COMP2ARB_DTYPE, NUM_STM, STM_DEPTH> comp2arb_stream_h1_kp0;
    tapa::streams<COMP2ARB_DTYPE, NUM_STM, STM_DEPTH> comp2arb_stream_h1_kp1;
    tapa::streams<COMP2ARB_DTYPE, NUM_STM, STM_DEPTH> comp2arb_stream_h2_kp0;
    tapa::streams<COMP2ARB_DTYPE, NUM_STM, STM_DEPTH> comp2arb_stream_h2_kp1;
    tapa::streams<COMP2ARB_DTYPE, NUM_STM, STM_DEPTH> comp2arb_stream_h3_kp0;
    tapa::streams<COMP2ARB_DTYPE, NUM_STM, STM_DEPTH> comp2arb_stream_h3_kp1;
    tapa::streams<COMP2ARB_DTYPE, NUM_STM, STM_DEPTH> comp2arb_stream_h4_kp0;
    tapa::streams<COMP2ARB_DTYPE, NUM_STM, STM_DEPTH> comp2arb_stream_h4_kp1;
    #if NUM_HASH != 5
    crash!
    #endif

    // Arbiter outputs
    tapa::streams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> bv_lookup_stream_h0_kp0;
    tapa::streams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> bv_lookup_stream_h0_kp1;
    tapa::streams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> bv_lookup_stream_h1_kp0;
    tapa::streams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> bv_lookup_stream_h1_kp1;
    tapa::streams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> bv_lookup_stream_h2_kp0;
    tapa::streams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> bv_lookup_stream_h2_kp1;
    tapa::streams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> bv_lookup_stream_h3_kp0;
    tapa::streams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> bv_lookup_stream_h3_kp1;
    tapa::streams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> bv_lookup_stream_h4_kp0;
    tapa::streams<PACKED_HASH_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> bv_lookup_stream_h4_kp1;
    #if NUM_HASH != 5
    crash!
    #endif

    // Query unit outputs
    tapa::streams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> query_bv_packed_stream_hash0_kp0;
    tapa::streams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> query_bv_packed_stream_hash0_kp1;
    tapa::streams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> query_bv_packed_stream_hash1_kp0;
    tapa::streams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> query_bv_packed_stream_hash1_kp1;
    tapa::streams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> query_bv_packed_stream_hash2_kp0;
    tapa::streams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> query_bv_packed_stream_hash2_kp1;
    tapa::streams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> query_bv_packed_stream_hash3_kp0;
    tapa::streams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> query_bv_packed_stream_hash3_kp1;
    tapa::streams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> query_bv_packed_stream_hash4_kp0;
    tapa::streams<BV_PLUS_METADATA_PACKED_DTYPE, BV_NUM_PARTITIONS, STM_DEPTH> query_bv_packed_stream_hash4_kp1;
    #if NUM_HASH != 5
    crash!!!
    #endif

    // Shuffle unit output
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm0_kp0;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm0_kp1;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm1_kp0;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm1_kp1;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm2_kp0;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm2_kp1;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm3_kp0;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm3_kp1;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm4_kp0;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm4_kp1;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm5_kp0;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm5_kp1;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm6_kp0;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm6_kp1;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm7_kp0;
    tapa::streams<BIT_DTYPE, NUM_HASH, STM_DEPTH> reconstruct_stream_stm7_kp1;
    #if NUM_STM != 8
    crash!!!!!!!
    #endif

    // Aggregate output
    tapa::streams<BIT_DTYPE,        NUM_STM, STM_DEPTH>     aggregate_stream_kp0;
    tapa::streams<BIT_DTYPE,        NUM_STM, STM_DEPTH>     aggregate_stream_kp1;

    // Datapacked outputs
    tapa::streams<OUT_PACKED_DTYPE,   NUM_STM, STM_DEPTH>     packed_output_stm_kp0;
    tapa::streams<OUT_PACKED_DTYPE,   NUM_STM, STM_DEPTH>     packed_output_stm_kp1;
    
    #if ENABLE_PERF_CTRS
    // Perfctr info
    tapa::streams<PERFCTR_DTYPE, NUM_PERFCTR_MODULES, STM_DEPTH>   perfctr_stms;
    #endif


    //////////////////////////////////////////////////////////
    // Connections WITHIN modules:

    // FIFOS within compute:
    COMPUTEHASH_STREAM_DECLS_KP(0)
    COMPUTEHASH_STREAM_DECLS_KP(1)

    ARBITER_STREAM_DECLS_KP(0)
    ARBITER_STREAM_DECLS_KP(1)

    SHUFFLE_STREAM_DECLS_KP(0)
    SHUFFLE_STREAM_DECLS_KP(1)

    //////////////////////////////////////////////////////////
    // MODULE INVOCATIONS.
    tapa::task()
        .invoke(loadBV
                ,input_bv
                ,bv_load_stream_0
                ,bv_load_stream_1
                ,bv_load_stream_2
                ,bv_load_stream_3
                ,bv_load_stream_4
                #if NUM_HASH != 5
                crash!
                #endif
        )

        LOAD_INVOKES



        COMPUTEHASH_INVOKES_FOR_KP(0)
        COMPUTEHASH_INVOKES_FOR_KP(1)


        ARBITER_INVOKES_FOR_KP(0)
        ARBITER_INVOKES_FOR_KP(1)


        QUERY_INVOKES


        SHUFFLE_INVOKES_FOR_KP(0)
        SHUFFLE_INVOKES_FOR_KP(1)


        AGGREGATE_INVOKES_FOR_KP(0)
        AGGREGATE_INVOKES_FOR_KP(1)


        .invoke(packOutput, 0, 0, aggregate_stream_kp0[0], packed_output_stm_kp0[0], NUM_LOADS_PER_STM)
        .invoke(packOutput, 1, 0, aggregate_stream_kp0[1], packed_output_stm_kp0[1], NUM_LOADS_PER_STM)
        .invoke(packOutput, 2, 0, aggregate_stream_kp0[2], packed_output_stm_kp0[2], NUM_LOADS_PER_STM)
        .invoke(packOutput, 3, 0, aggregate_stream_kp0[3], packed_output_stm_kp0[3], NUM_LOADS_PER_STM)
        .invoke(packOutput, 4, 0, aggregate_stream_kp0[4], packed_output_stm_kp0[4], NUM_LOADS_PER_STM)
        .invoke(packOutput, 5, 0, aggregate_stream_kp0[5], packed_output_stm_kp0[5], NUM_LOADS_PER_STM)
        .invoke(packOutput, 6, 0, aggregate_stream_kp0[6], packed_output_stm_kp0[6], NUM_LOADS_PER_STM)
        .invoke(packOutput, 7, 0, aggregate_stream_kp0[7], packed_output_stm_kp0[7], NUM_LOADS_PER_STM)

        .invoke(packOutput, 0, 1, aggregate_stream_kp1[0], packed_output_stm_kp1[0], NUM_LOADS_PER_STM)
        .invoke(packOutput, 1, 1, aggregate_stream_kp1[1], packed_output_stm_kp1[1], NUM_LOADS_PER_STM)
        .invoke(packOutput, 2, 1, aggregate_stream_kp1[2], packed_output_stm_kp1[2], NUM_LOADS_PER_STM)
        .invoke(packOutput, 3, 1, aggregate_stream_kp1[3], packed_output_stm_kp1[3], NUM_LOADS_PER_STM)
        .invoke(packOutput, 4, 1, aggregate_stream_kp1[4], packed_output_stm_kp1[4], NUM_LOADS_PER_STM)
        .invoke(packOutput, 5, 1, aggregate_stream_kp1[5], packed_output_stm_kp1[5], NUM_LOADS_PER_STM)
        .invoke(packOutput, 6, 1, aggregate_stream_kp1[6], packed_output_stm_kp1[6], NUM_LOADS_PER_STM)
        .invoke(packOutput, 7, 1, aggregate_stream_kp1[7], packed_output_stm_kp1[7], NUM_LOADS_PER_STM)

        WRITEOUT_INVOKES



        #if ENABLE_PERF_CTRS
        .invoke(write_perfctrs, perfctr_stms, perfctr_mmap)
        #endif


    ;
    return;
}
