
#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <cmath>
#include <stdio.h>
#include <set>
#include <math.h>
// This file is required for OpenCL C++ wrapper APIs
//#include "xcl2.hpp"

#include "BitBlender.h"
#include "SW_MurmurHash3.cpp"
#include "hostside_aurorahelpers.cpp"
#include "hostside_bitblender_dataprep.cpp"


int actual_population_inputs=0;    // Modified later

        
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


;

#ifndef NAIVE_MULTISTREAM
void crash_compilation(
    crash compilation
    You need to define NAIVE_MULTISTREAM when compiling!
}
#endif

DEFINE_string(bitstream, "", "Path to bitstream file. Run SW_EMU if empty.");
/**********************************************************/
/**********************************************************/
/**********************************************************/
/**********************************************************/

int main(int argc, char **argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);

    setbuf(stdout, NULL);
    bool match = 1;
    const int NUM_TESTS_TO_RUN = 10;
    int test_number = 0;
    INPUT_GEN_MODE_ENUM     gen_data_mode;
    actual_population_inputs = NUM_POPULATION_INPUTS;

    //gen_data_mode = INPUT_GEN_MODE_RANDOM;
    //gen_data_mode = INPUT_GEN_MODE_NO_CLASH;
    gen_data_mode = INPUT_GEN_MODE_ALL_CLASH;

    // I/O Data Vectors
    std::vector<KEY_DTYPE, tapa::aligned_allocator<KEY_DTYPE>>
        keys(TOTAL_NUM_KEYINPUT);
    std::vector<BIT_DTYPE, tapa::aligned_allocator<BIT_DTYPE>>
        krnl_merged_out(TOTAL_NUM_KEYINPUT);

    std::vector<BIT_DTYPE, tapa::aligned_allocator<BIT_DTYPE>>          source_bv(BV_LENGTH);
    std::vector<BV_LOAD_DTYPE, tapa::aligned_allocator<BV_LOAD_DTYPE>>  source_PACKED_bv(BV_NUM_LOADS);
    std::vector<BIT_DTYPE>                                              sw_results(TOTAL_NUM_KEYINPUT);

    std::vector<LOAD_DTYPE, tapa::aligned_allocator<LOAD_DTYPE>>        krnl_key_in_0(KEYPAIRS_PER_STM);
    std::vector<STORE_DTYPE, tapa::aligned_allocator<STORE_DTYPE>>      krnl_out_0(PACKED_OUTPUTS_PER_STM);
    #if NUM_AXI_PORTS != 1
    crash(compilation) //Need more krnl arguments.
    #endif

    #if ENABLE_PERF_CTRS
    const int TOTAL_NUM_PERFCTR_OUTPUTS = NUM_PERFCTR_MODULES*NUM_PERFCTR_OUTPUTS_PER_MODULE;
    std::vector<PERFCTR_DTYPE, tapa::aligned_allocator<PERFCTR_DTYPE>>    perfctrs(TOTAL_NUM_PERFCTR_OUTPUTS);
    #endif

    /**************************************/
    /*** Sanity Checks                  ***/
    /**************************************/
    #if (BV_LENGTH % BV_URAM_PACKED_BITWIDTH != 0)
    crash(;
    #endif
    #if ( (KEY_BITWIDTH/8) * MAX_KEYS_IN_ONE_AXI_PORT > (1024*1024*256) )
    crash(; // Over 256 MB, it cant fit in one hbm bank.
    #endif
    #if (BV_NUM_BRAM_PARTITIONS + BV_NUM_URAM_PARTITIONS != BV_NUM_PARTITIONS)
    crash(;
    #endif
    if (OUT_PACKED_BITWIDTH != 32      &&
        OUT_PACKED_BITWIDTH != 64      &&
        OUT_PACKED_BITWIDTH != 128     &&
        OUT_PACKED_BITWIDTH != 256     &&
        OUT_PACKED_BITWIDTH != 512     &&
        OUT_PACKED_BITWIDTH != 1024)
    {
        printf("ERROR: The OUT_PACKED_BITWIDTH must be a power of 2, between 32 and 1024.\n");
        printf("       Otherwise Vivado will have errors.\n");
        exit(-1);
    }
    /**************************************/
    /*** END OF Sanity Checks           ***/
    /**************************************/

    while (test_number < NUM_TESTS_TO_RUN)
    //while (match && test_number < NUM_TESTS_TO_RUN)
    {
        // Seed the random number generator so we have replicatable input vectors.
        srand(1+test_number++);

        // Only do verif if were using a random input sequence
        bool do_verif = 0;

        if (test_number == 1) {
            gen_data_mode = INPUT_GEN_MODE_NO_CLASH;
        }
        else if (test_number == 2) {
            gen_data_mode = INPUT_GEN_MODE_CYCLIC_CLASH;
        }
        else if (test_number == 3) {
            gen_data_mode = INPUT_GEN_MODE_ALL_CLASH;
        }
        else {
            gen_data_mode = INPUT_GEN_MODE_RANDOM;

            if (test_number == 9) {
                do_verif = 1;
                actual_population_inputs = NUM_POPULATION_INPUTS;
            }
            //// These take too long to run...
            //else if (test_number == 5) {
            //    do_verif = 1;
            //    actual_population_inputs = (BV_LENGTH/8);
            //}
            //else if (test_number == 6) {
            //    do_verif = 1;
            //    actual_population_inputs = (BV_LENGTH/32);
            //}
            //else if (test_number == 7) {
            //    do_verif = 1;
            //    actual_population_inputs = (BV_LENGTH/64);
            //}
        }

        ///////////////////////////////////
        // DEBUG PRINTS:
        printf("KDEBUG: TOTAL_NUM_KEYINPUT is %d\n", TOTAL_NUM_KEYINPUT);
        printf("KDEBUG: actual_population_inputs is %d\n", actual_population_inputs);
        printf("KDEBUG: BV_LENGTH is %d\n", BV_LENGTH);
        printf("KDEBUG: BV_SECTION_LENGTH is %d\n",
                    BV_SECTION_LENGTH);
        printf("KDEBUG: BV_PARTITION_LENGTH_IN_URAM_PACKED_ELEMS is %d\n",
                    BV_PARTITION_LENGTH_IN_URAM_PACKED_ELEMS);

        printf("\n");
        printf("KDEBUG: NUM_HASH is %d\n", NUM_HASH);
        printf("KDEBUG: BV_NUM_PARTITIONS is %d\n", BV_NUM_PARTITIONS);
        printf("KDEBUG: NUM_STM is %d\n", NUM_STM);
        printf("KDEBUG: ARB_RATELIM_DISTANCE is %d\n", ARB_RATELIM_DISTANCE);
        printf("\n");
        printf("KDEBUG: STM_DEPTH is %d\n\n", STM_DEPTH);

        #if ENABLE_PERF_CTRS
        printf("WARNING! PERFORMANCE COUNTERS ARE ENABLED! WARNING!\n");
        #endif
        if (NAIVE_MULTISTREAM) {
            printf("WARNING! using NAIVE multistream! WARNING!\n");
        }
        else {
            printf("Using our BV-sharing multistream design.\n");
        }


        if (gen_data_mode != INPUT_GEN_MODE_RANDOM) {
            printf("WARNING: ");
        }
        printf("gen_data_mode IS %s\n\n", INPUT_GEN_MODE_STRINGS[gen_data_mode]);
        ///////////////////////////////////


        #if ENABLE_PERF_CTRS
        perfctrs.clear();
        for (int i = 0; i < TOTAL_NUM_PERFCTR_OUTPUTS; ++i) {
            perfctrs.push_back(987654321);
        }
        #endif

        reset_krnl_inputs(
            gen_data_mode
            ,do_verif
            ,keys.data()
            ,sw_results.data()
            ,source_bv.data()
        );

        datapack_krnl_inputs(
            do_verif
            ,keys.data()

            ,krnl_key_in_0.data()
            #if NUM_AXI_PORTS != 1
            crash(compilation)
            #endif

            ,source_bv.data()
            ,source_PACKED_bv.data()
        );


        /****************************************/
        /***** KERNEL INVOCATION            *****/
        /****************************************/
        int64_t kernel_time_ns = tapa::invoke(
            workload,
            FLAGS_bitstream,
             tapa::read_only_mmap<BV_LOAD_DTYPE>(source_PACKED_bv)

            ,tapa::read_only_mmap<LOAD_DTYPE>(krnl_key_in_0)
            ,tapa::write_only_mmap<STORE_DTYPE>(krnl_out_0)

            #if NUM_AXI_PORTS != 1
            crash(compilation)
            #endif

            #if ENABLE_PERF_CTRS
            ,tapa::write_only_mmap<PERFCTR_DTYPE>(perfctrs)
            #endif

            , KEYPAIRS_PER_STM
        );
        /****************************************/
        /***** END KERNEL INVOCATION        *****/
        /****************************************/

        std::cout << "KERNEL time: " << kernel_time_ns * 1e-9 << " s" << std::endl; // DO NOT CHANGE: This line is used by the datacollection python script.


        if (do_verif){
            // Merge kernel outputs
            unpack_krnl_outputs(
                krnl_out_0.data(),
                #if NUM_AXI_PORTS != 1
                crash(compilation) //Need more krnl args.
                #endif

                krnl_merged_out.data()
            );

            match = verify(
                sw_results.data(),
                krnl_merged_out.data()
            );
        }

        #if ENABLE_PERF_CTRS
        print_perf_ctrs(perfctrs.data());
        #endif

        std::cout << "------------------------------------------" << std::endl;
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "------------------------------------------" << std::endl;
    }

    printf("\n\n\n\n\n\n\n\nHost is now exiting.\n\n\n\n\n\n\n\n");
    return (match ? EXIT_SUCCESS : EXIT_FAILURE);
}
