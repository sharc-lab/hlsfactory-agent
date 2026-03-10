
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

        

/**********************************************************/
/**********************************************************/
/**********************************************************/
/**********************************************************/


int main(int argc, char **argv)
{
    Configuration config(argc, argv);
    setbuf(stdout, NULL);

    printf("\n\n\n\n STARTING HOST CODE NOW!!! \n\n\n");


    bool match = 1;
    const int NUM_TESTS_TO_RUN = 10;
    int test_number = 0;
    INPUT_GEN_MODE_ENUM     gen_data_mode;
    actual_population_inputs = NUM_POPULATION_INPUTS;

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



    bool emulation = (std::getenv("XCL_EMULATION_MODE") != nullptr);

    uint32_t device_id = emulation ? 0 : (0) %3;

    // NOTE: THESE values depend on the connectivity cfg/ini file.
    uint32_t issue_instance_id = 1;
    uint32_t dump_instance_id = 1;
    uint32_t bloom_bitblender_instance_id = 0;

    xrt::device device = xrt::device(device_id);
    xrt::uuid xclbin_uuid = device.load_xclbin(config.xclbin_file);

    Aurora aurora_0, aurora_1;
    if (!emulation) {
        aurora_0 = Aurora(0, device, xclbin_uuid);
        aurora_1 = Aurora(1, device, xclbin_uuid);

        check_core_status_global(aurora_0, config.timeout_ms);
        check_core_status_global(aurora_1, config.timeout_ms);

        if (aurora_0.has_framing() != aurora_1.has_framing()) {
            printf("ERROR: FRAMING DIFFERENCES.\n");
            exit(-1);
        }

        if (!aurora_0.has_framing()) {
            config.frame_size = 0;
        }
    }

    double start_time, finish_time;





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

        printf("KDEBUG: KEYPAIRS_PER_STM is %d\n", KEYPAIRS_PER_STM);
        printf("KDEBUG: NUM_PACKED_OUTPUTS is %d\n", NUM_PACKED_OUTPUTS);

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
            ,source_bv.data()
            ,source_PACKED_bv.data()
        );

        /**********************************************************************/
        /**********************************************************************/
        /***        RUN THE KERNEL                                          ***/
        /**********************************************************************/
        /**********************************************************************/

        IssueKernel bloom_issue(issue_instance_id, device, xclbin_uuid, config);
        DumpKernel bloom_dump(dump_instance_id, device, xclbin_uuid, config);
        BitBlenderKernel bloom_bitblender(
                bloom_bitblender_instance_id
                ,device
                ,xclbin_uuid
                ,config
                ,KEYPAIRS_PER_STM           // packed keys num entries
                ,source_PACKED_bv.size()    // bv num entries
    #if ENABLE_PERF_CTRS
                ,TOTAL_NUM_PERFCTR_OUTPUTS    // perfctrs num entries
    #endif
                ,source_PACKED_bv
        );

        bloom_bitblender.start();
        bloom_dump.start();


        bloom_issue.prepare_bitblender_inputs(krnl_key_in_0);
        start_time = aurora_get_wtime();
        bloom_issue.start();


        if (bloom_issue.timeout()) {
            printf("ERROR: Issue timeout. Something is terribly wrong!\n");
        }

        if (bloom_bitblender.timeout()) {
            printf("ERROR: BITBLENDER timeout...\n");

            aurora_0.print_core_status();
            aurora_1.print_core_status();
            exit(-1);
        }
        else {
            printf("BITBLENDER KERNEL FINISHED!!!!\n");
            finish_time = aurora_get_wtime();
        }

        double runtime = finish_time - start_time;
        std::cout << std::endl << std::endl;
        std::cout << "KERNEL time: " << runtime << " s" << std::endl; // DO NOT CHANGE: This line is used by the datacollection python script.


        /**********************************************************************/
        /**********************************************************************/
        /***        DO VERIFICATION                                         ***/
        /**********************************************************************/
        /**********************************************************************/


        if (do_verif){
            krnl_out_0 = bloom_dump.get_output_data();

            unpack_krnl_outputs(
                krnl_out_0.data(),
                krnl_merged_out.data()
            );

            match = verify(
                sw_results.data(),
                krnl_merged_out.data()
            );
        }

        #if ENABLE_PERF_CTRS
        perfctrs = bloom_bitblender.get_perfctrs_data();
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
        