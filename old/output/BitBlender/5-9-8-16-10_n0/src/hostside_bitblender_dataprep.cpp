
#include "BitBlender.h"

extern int actual_population_inputs;

// https://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
//// WARNING: Probably do not change the INPUT_GEN_MODE_ prefix. That is used by the datacollection script.
////        Changes made here will require changes to the datacollection script.
#define __PREPROC__(FUNC) \
    FUNC(INPUT_GEN_MODE_RANDOM), \
    FUNC(INPUT_GEN_MODE_NO_CLASH), \
    FUNC(INPUT_GEN_MODE_CYCLIC_CLASH), \
    FUNC(INPUT_GEN_MODE_ALL_CLASH)

#define GENERATE_ENUM(ENUM) ENUM
#define GENERATE_STRING(STRING) #STRING

typedef enum {
    __PREPROC__(GENERATE_ENUM)
} INPUT_GEN_MODE_ENUM;
static const char *INPUT_GEN_MODE_STRINGS[] = {
    __PREPROC__(GENERATE_STRING)
};



bool operator==(const LOAD_DTYPE& lhs, const LOAD_DTYPE& rhs)
{
    bool match = true;

    if (lhs.s0_k0 != rhs.s0_k0) {
        match = false;
    } else if (lhs.s0_k1 != rhs.s0_k1) {
        match = false;
    }
    else if (lhs.s1_k0 != rhs.s1_k0) {
        match = false;
    } else if (lhs.s1_k1 != rhs.s1_k1) {
        match = false;
    }
    else if (lhs.s2_k0 != rhs.s2_k0) {
        match = false;
    } else if (lhs.s2_k1 != rhs.s2_k1) {
        match = false;
    }
    else if (lhs.s3_k0 != rhs.s3_k0) {
        match = false;
    } else if (lhs.s3_k1 != rhs.s3_k1) {
        match = false;
    }
    else if (lhs.s4_k0 != rhs.s4_k0) {
        match = false;
    } else if (lhs.s4_k1 != rhs.s4_k1) {
        match = false;
    }
    else if (lhs.s5_k0 != rhs.s5_k0) {
        match = false;
    } else if (lhs.s5_k1 != rhs.s5_k1) {
        match = false;
    }
    else if (lhs.s6_k0 != rhs.s6_k0) {
        match = false;
    } else if (lhs.s6_k1 != rhs.s6_k1) {
        match = false;
    }
    else if (lhs.s7_k0 != rhs.s7_k0) {
        match = false;
    } else if (lhs.s7_k1 != rhs.s7_k1) {
        match = false;
    }

    return match;
}

bool operator!=(const LOAD_DTYPE& lhs, const LOAD_DTYPE& rhs)
{
    return (!(lhs == rhs));
}


std::ostream& operator<<(std::ostream& os, const LOAD_DTYPE& loadval) {
    os  << loadval.s0_k0.to_int() << " " << loadval.s0_k1.to_int() << "\n"
        << loadval.s1_k0.to_int() << " " << loadval.s1_k1.to_int() << "\n"
        << loadval.s2_k0.to_int() << " " << loadval.s2_k1.to_int() << "\n"
        << loadval.s3_k0.to_int() << " " << loadval.s3_k1.to_int() << "\n"
        << loadval.s4_k0.to_int() << " " << loadval.s4_k1.to_int() << "\n"
        << loadval.s5_k0.to_int() << " " << loadval.s5_k1.to_int() << "\n"
        << loadval.s6_k0.to_int() << " " << loadval.s6_k1.to_int() << "\n"
        << loadval.s7_k0.to_int() << " " << loadval.s7_k1.to_int();

    return os;
}








bool operator==(const STORE_DTYPE& lhs, const STORE_DTYPE& rhs)
{
    bool match = true;

    if (lhs.s0_k0 != rhs.s0_k0) {
        match = false;
    } else if (lhs.s0_k1 != rhs.s0_k1) {
        match = false;
    }
    else if (lhs.s1_k0 != rhs.s1_k0) {
        match = false;
    } else if (lhs.s1_k1 != rhs.s1_k1) {
        match = false;
    }
    else if (lhs.s2_k0 != rhs.s2_k0) {
        match = false;
    } else if (lhs.s2_k1 != rhs.s2_k1) {
        match = false;
    }
    else if (lhs.s3_k0 != rhs.s3_k0) {
        match = false;
    } else if (lhs.s3_k1 != rhs.s3_k1) {
        match = false;
    }
    else if (lhs.s4_k0 != rhs.s4_k0) {
        match = false;
    } else if (lhs.s4_k1 != rhs.s4_k1) {
        match = false;
    }
    else if (lhs.s5_k0 != rhs.s5_k0) {
        match = false;
    } else if (lhs.s5_k1 != rhs.s5_k1) {
        match = false;
    }
    else if (lhs.s6_k0 != rhs.s6_k0) {
        match = false;
    } else if (lhs.s6_k1 != rhs.s6_k1) {
        match = false;
    }
    else if (lhs.s7_k0 != rhs.s7_k0) {
        match = false;
    } else if (lhs.s7_k1 != rhs.s7_k1) {
        match = false;
    }

    return match;
}

bool operator!=(const STORE_DTYPE& lhs, const STORE_DTYPE& rhs)
{
    return (!(lhs == rhs));
}


std::ostream& print_bits(std::ostream& os, const OUT_PACKED_DTYPE& val) {
    os << val[31] << val[30] << val[29] << val[28]
       << val[27] << val[26] << val[25] << val[24]
       << val[23] << val[22] << val[21] << val[20]
       << val[19] << val[18] << val[17] << val[16]
       << val[15] << val[14] << val[13] << val[12]
       << val[11] << val[10] << val[9] << val[8]
       << val[7] << val[6] << val[5] << val[4]
       << val[3] << val[2] << val[1] << val[0] << std::endl;
    return os;
}


std::ostream& operator<<(std::ostream& os, const STORE_DTYPE& val) {
    print_bits(os, val.s0_k0);
    print_bits(os, val.s0_k1);
    print_bits(os, val.s1_k0);
    print_bits(os, val.s1_k1);
    print_bits(os, val.s2_k0);
    print_bits(os, val.s2_k1);
    print_bits(os, val.s3_k0);
    print_bits(os, val.s3_k1);
    print_bits(os, val.s4_k0);
    print_bits(os, val.s4_k1);
    print_bits(os, val.s5_k0);
    print_bits(os, val.s5_k1);
    print_bits(os, val.s6_k0);
    print_bits(os, val.s6_k1);
    print_bits(os, val.s7_k0);
    print_bits(os, val.s7_k1);
    return os;
}






int min(int in1, int in2) {
    return (in1 < in2 ? in1 : in2);
}

uint32_t bv_size(float fp, int insert_num){
    //fp: false positive rate, number of insertion to bit vector, the bit vector size m should be: m = -(insert_num) * ln(fp) / ( (ln2)^2 )
    float tmp1 = -(insert_num) * (log(fp)) / (log(2) * log(2) );
    int tmp2 = (int)ceil(tmp1);

    return tmp2;
}


void generate_keys(
    INPUT_GEN_MODE_ENUM     input_generation_mode
    ,KEY_DTYPE              *keys
    ,std::vector<uint32_t>  seed
) {
    const int NOT_TAKEN = -1;

    static bool     computed_unclashing_keys = 0;
    static KEY_DTYPE unclashing_keys[NUM_STM];

    int taken_by_stmidx[NUM_HASH][BV_NUM_PARTITIONS] = {};

    for (int i = 0; i < NUM_HASH; ++i) {
        for (int j = 0; j < BV_NUM_PARTITIONS; ++j) {
            taken_by_stmidx[i][j] = NOT_TAKEN;
        }
    }

    if (!computed_unclashing_keys) {
        /* For each hash function, ensure that different streams will output
         *  to different partitions.
         */
        if (input_generation_mode != INPUT_GEN_MODE_RANDOM)
        {
            if (NUM_STM > BV_NUM_PARTITIONS){
                printf("minor warning: NUM_STM > BV_NUM_PARTITIONS. Generating invalid unclashing keys.\n");
                for (int stm_idx = 0; stm_idx < NUM_STM; ++stm_idx) {
                    unclashing_keys[stm_idx] = 5;
                }
            }
            else {
                for (int stm_idx = 0; stm_idx < NUM_STM; ++stm_idx)
                {
                    int found = 0;
                    KEY_DTYPE proposed_key = 0;
                    int taken_partition_idces[NUM_HASH] = {};

                    while (!found) {
                        found = 1;
                        proposed_key++;
                        for (int i = 0; i < NUM_HASH; ++i) {
                            taken_partition_idces[i] = 0;
                        }

                        for (int hash_idx = 0; hash_idx < NUM_HASH; ++hash_idx) {
                            uint32_t bv_index = SW_MurmurHash3_x86_32(
                                proposed_key,
                                KEY_SIZE_IN_BYTES,
                                seed[hash_idx]
                            );
                            bv_index %= BV_SECTION_LENGTH;
                            uint32_t bv_partition_idx = bv_index / BV_PARTITION_LENGTH;

                            if (taken_by_stmidx[hash_idx][bv_partition_idx] == NOT_TAKEN) {
                                taken_partition_idces[hash_idx] = bv_partition_idx;
                            }
                            else {
                                // One stream will clash.
                                found = 0;
                            }
                        }
                    }

                    /////////
                    // FOUND.
                    unclashing_keys[stm_idx] = proposed_key;
                    for (int hash_idx = 0; hash_idx < NUM_HASH; ++hash_idx) {
                        taken_by_stmidx[hash_idx][ taken_partition_idces[hash_idx] ] = stm_idx;
                    }

                    #ifdef __DO_THIS_DEBUG_PRINTS__
                    printf("FOUND an unclashing query (%d) for stm %d\n", proposed_key.to_int(), stm_idx);
                    for (int hash_idx = 0; hash_idx < NUM_HASH; ++hash_idx) {
                        for (int pidx = 0; pidx < BV_NUM_PARTITIONS; ++pidx) {

                            if (taken_by_stmidx[hash_idx][pidx] == NOT_TAKEN) {
                                printf("%5c", '-');
                            }
                            else {
                                printf("%5d", taken_by_stmidx[hash_idx][pidx]);
                            }

                        }
                        printf("\n");
                    }
                    #endif
                }
            }
        }
        computed_unclashing_keys = 1;
    }


    if (input_generation_mode == INPUT_GEN_MODE_RANDOM) {
        for (uint32_t input_idx = 0; input_idx < TOTAL_NUM_KEYINPUT; ++input_idx) {
            KEY_DTYPE in = static_cast<KEY_DTYPE>(rand());
            keys[input_idx] = in;
        }
    }
    else if (input_generation_mode == INPUT_GEN_MODE_NO_CLASH) {
        /* Each stream sends the exact same data over and over,
         * and each stream basically has it's own dedicated partition.
         */
        for (int i=0; i < KEYPAIRS_PER_STM; ++i) {
            for (int j = 0; j < NUM_STM; ++j) {
                for (int kpidx = 0; kpidx < 2; kpidx++) {
                    keys[i*2*NUM_STM + j*2 + kpidx] = unclashing_keys[j];
                }
            }
        }
    }
    else if (input_generation_mode == INPUT_GEN_MODE_CYCLIC_CLASH) {
        /* Each input index is an N-clash, but the partitions on which
         * they clash is always cycling around.
         */
        for (int i = 0; i < KEYPAIRS_PER_STM; ++i) {
            for (int j = 0; j < NUM_STM; ++j) {
                for (int kpidx = 0; kpidx < 2; kpidx++) {
                    keys[i*2*NUM_STM + j*2 + kpidx] = unclashing_keys[i%NUM_STM];
                }
            }
        }
    }
    else if (input_generation_mode == INPUT_GEN_MODE_ALL_CLASH) {
        /* For each hash function, ensure that each stream will hash to the
         * SAME partition, so it always clashes every single time.
         */
        for (int i = 0; i < KEYPAIRS_PER_STM; ++i) {
            for (int j = 0; j < NUM_STM; ++j) {
                for (int kpidx = 0; kpidx < 2; kpidx++) {
                    keys[i*2*NUM_STM + j*2 + kpidx] = 5;
                }
            }
        }
    }
    else {
        // Crash - unsupported case.
        assert(1 == 2);
    }
}



void populate_bv(
    BIT_DTYPE *bv
    ,std::vector<uint32_t>   seed
    ,KEY_DTYPE *keys
    ,std::set<KEY_DTYPE>     &unique_inserted_keys
) {
    int input_idces_to_add[actual_population_inputs];

    /* Knuth's algorithm for unique random number generation in an integer array.
     *  https://stackoverflow.com/questions/1608181/unique-random-number-generation-in-an-integer-array
     */

    const int N_ = TOTAL_NUM_KEYINPUT;
    for (int in = 0, im = 0;
            in < N_ && im < actual_population_inputs;
            ++in) {
        int rn = N_ - in;
        int rm = actual_population_inputs - im;
        if (rand() % rn < rm){
            input_idces_to_add[im++] = in;
        }
    }

    // Use those input indices to populate the BV.
    for (int i = 0; i < actual_population_inputs; ++i) {
        int         add_idx = input_idces_to_add[i];
        uint32_t    bv_index = 0;

        unique_inserted_keys.insert(keys[add_idx]);

        for(int hash_idx = 0; hash_idx < NUM_HASH; hash_idx ++){
            bv_index = SW_MurmurHash3_x86_32(
                keys[add_idx],
                KEY_SIZE_IN_BYTES,  // length of each key in bytes
                seed[hash_idx]
            );
            bv_index %= BV_SECTION_LENGTH;
            bv_index += (hash_idx*BV_SECTION_LENGTH);

            bv[bv_index] = 1;


            #ifdef __DO_DEBUG_PRINTS__
            printf("HOST: Populating bit vector to have input_idx=%d be a hit. So we set bv[%d]=1.\n",
                        add_idx, bv_index
            );
            #endif
        }
    }
}


void compute_expected_outputs(
    BIT_DTYPE               *bv
    ,std::vector<uint32_t>  seed
    ,KEY_DTYPE              *keys
    ,std::set<KEY_DTYPE>    &unique_hit_keys
    ,BIT_DTYPE              *sw_results
) {
    for (uint32_t input_idx = 0; input_idx < TOTAL_NUM_KEYINPUT; ++input_idx)
    {
        uint32_t    bv_index = 0;
        BIT_DTYPE   cur_result = 1;

        for(int hash_idx = 0; hash_idx < NUM_HASH; hash_idx ++){
            bv_index = SW_MurmurHash3_x86_32(
                keys[input_idx],
                4,      // length of each key in bytes
                seed[hash_idx]
            );
            bv_index %= BV_SECTION_LENGTH;
            bv_index += (hash_idx*BV_SECTION_LENGTH);
            cur_result &= bv[bv_index];

            #ifdef __DO_DEBUG_PRINTS__
            printf("HOST: For input %d, hash #%d, this looked up BV[%d]=%d\n",
                    input_idx,
                    hash_idx,
                    bv_index,
                    bv[bv_index].to_int()
            );
            #endif
        }
        if (cur_result) {
            unique_hit_keys.insert(keys[input_idx]);
        }

        sw_results[input_idx] = cur_result;
    }

}






bool verify(
    BIT_DTYPE   *sw_results,
    BIT_DTYPE   *hw_results
) {
    const int MAX_REPORTED_FAILS = 100;
    bool match = true;
    int errcount = 0;
    int failed_indices[MAX_REPORTED_FAILS] = {0};

    for (int i = 0; i < TOTAL_NUM_KEYINPUT; ++i) {
        if (hw_results[i] != sw_results[i]) {
            if (errcount < MAX_REPORTED_FAILS-1) {
                failed_indices[errcount] = i;
            }
            errcount ++;

            match = false;
        }
    }

    std::cout << "TEST " << (match ? "PASSED" : "FAILED") << std::endl;
    if (!match)
    {
        std::cout << "Num failures: " << (errcount) << std::endl;

        std::cout << "(THIS ERR REPORTING IS STILL BUGGY)"<<
                " Failed on the following indices (reporting up to the first " <<
                MAX_REPORTED_FAILS << "):" <<  std::endl;
        for (int i = 0; i < (MAX_REPORTED_FAILS); ++i) {
            std::cout << failed_indices[i] << ", ";
        }
    }

    std::cout << std::endl;

    return match;
}



#if ENABLE_PERF_CTRS
void print_perf_ctrs(PERFCTR_DTYPE* perfctrs) {

    printf("NUM_PERFCTR_MODULES = %d\n", NUM_PERFCTR_MODULES); // DO NOT CHANGE: This line is used by the datacollection python script.

    for (int i = 0; i < NUM_PERFCTR_MODULES; ++i)
    {
        printf("PERFORMANCE_COUNTER_VALUE[%d] = %25lu\n",
                i, perfctrs[i]
        ); // DO NOT CHANGE: This line is used by the datacollection python script.
    }

}
#endif




double get_theoretical_fp_rate() {
    double n = (double)actual_population_inputs;
    double m = (double)BV_LENGTH;
    double k = (double)NUM_HASH;

    double result = pow(( 1 - exp(-k * n / m)), k);
    return result;
}



void reset_krnl_inputs(
    INPUT_GEN_MODE_ENUM input_generation_mode
    ,bool       do_verif
    ,KEY_DTYPE *keys
    ,BIT_DTYPE *sw_results
    ,BIT_DTYPE *bv
) {
    /*
        These 3 sets are used to compute the false-pos rate.
        Think about it this way: if our input keys only contain 5 unique
        values but they look like this:
            [1, 2, 3, 4, 5, 5, 5, 5, 5...]
        and we insert (1,2,3). And suppose 4 is NOT a false-positive, but 5 IS.
            - if we don't consider unique entries and only ask "how many indices hit?"
              our false-positive rate would be 99%.
            - If we consider unique entries, then our false-positive rate will be
              50%, which is correct.
     */
    std::set<KEY_DTYPE>     unique_inserted_keys;
    std::set<KEY_DTYPE>     unique_uninserted_keys;
    std::set<KEY_DTYPE>     unique_hit_keys;

    std::vector<uint32_t> seed(NUM_HASH);

    // Populate the seeds, for each hash function.
    for(int hash_idx = 0; hash_idx < NUM_HASH; hash_idx ++){
        seed[hash_idx] = hash_idx;
    }

    // Initialize the bitvector to all zeroes.
    for (uint32_t bv_idx = 0; bv_idx < BV_LENGTH; bv_idx++) {
        bv[bv_idx] = 0;
    }

    // Initialize the input keys
    generate_keys(input_generation_mode, keys, seed);

    if (do_verif)
    {
        // Populate the bitvector
        populate_bv(bv, seed, keys, unique_inserted_keys);

        // Use the populated bitvector to calculate the ACTUAL expected outputs.
        compute_expected_outputs(bv, seed, keys, unique_hit_keys, sw_results);

        for (uint32_t input_idx = 0; input_idx < TOTAL_NUM_KEYINPUT; ++input_idx) {
            KEY_DTYPE k = keys[input_idx];

            if (unique_inserted_keys.count(k) == 0) {
                unique_uninserted_keys.insert(k);
            }
        }

        int unique_hits     = unique_hit_keys.size();
        int unique_inserts  = unique_inserted_keys.size();
        int unique_uninserted = unique_uninserted_keys.size();
        int unique_falsepos = unique_hits - unique_inserts;
        double actual_fp_rate           = (double)unique_falsepos / (double)unique_uninserted;
        double theoretical_fp_rate      = get_theoretical_fp_rate();
        double fp_difference_percent = 0;

        if (actual_fp_rate > 0){
            fp_difference_percent = fabs(theoretical_fp_rate - actual_fp_rate) / actual_fp_rate;
        }

        printf("The number of inserted elements is %d, but the number of HIT elements is %d.\n",
                unique_inserts,
                unique_hits
        );
        printf("Therefore, we have %d false positives out of %d uninserted.\n",
                unique_falsepos,
                unique_uninserted
        );

        /*****************************************************************/
        /*** THESE LINES ARE USED BY THE COLLECT_ALL_RESULTS.py SCRIPT ***/

        printf("number of inserted elements = %d \n",
                actual_population_inputs
        );
        printf("ACTUAL fp rate = %lf \n",
                actual_fp_rate
        );
        printf("THEORETICAL fp rate = %lf \n",
                theoretical_fp_rate
        );

        /*** END of lines are used by the collect_all_results.py script ***/
        /*****************************************************************/

        if (fp_difference_percent > 0.05){
            printf("\n\n\nWarning: FP RATES ARE DIFFERENT BY %lf\n",
                    fp_difference_percent
            );
        }

        printf("\n\n\n");

        if (unique_inserts > unique_hits) {
            printf(" SOMETHING IS WRONG WITH THE FP RATE ANALYSIS.\n");
            assert(1==2);
        }
    }


    /*********************/
    // DEBUG: print the bv.
    //#ifdef __DO_DEBUG_PRINTS__
    //for (int i = 0; i < BV_LENGTH; ++i) {
    //    printf("HOST: BV[%d] = %d\n",
    //            i, bv[i].to_int()
    //    );
    //}
    //#endif
    // #ifdef __DO_DEBUG_PRINTS__
    // const int START_DEBUG_PRINTS = 0;
    // const int END_DEBUG_PRINTS = 3;
    // for (int i = START_DEBUG_PRINTS; i < END_DEBUG_PRINTS; ++i) {
    //     printf("HOST: packed_BV[%d] = %x\n", i, packed_bv_0[i].to_int());
    //     for (int j = 0; j < BV_URAM_PACKED_BITWIDTH; ++j) {
    //         int total_idx = i*BV_URAM_PACKED_BITWIDTH + j;
    //         printf("HOST: BV[%d] = %d\n", total_idx, bv[total_idx].to_int());
    //     }
    //     printf("\n");
    // }
    // #endif
}

        //////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////


void datapack_bv(
    BIT_DTYPE *bv
    ,BV_LOAD_DTYPE *packed_bv
) {
    std::vector<BV_URAM_PACKED_DTYPE> bv_section0(BV_SECTION_LENGTH_IN_URAM_PACKED_ELEMS);
    std::vector<BV_URAM_PACKED_DTYPE> bv_section1(BV_SECTION_LENGTH_IN_URAM_PACKED_ELEMS);
    std::vector<BV_URAM_PACKED_DTYPE> bv_section2(BV_SECTION_LENGTH_IN_URAM_PACKED_ELEMS);
    std::vector<BV_URAM_PACKED_DTYPE> bv_section3(BV_SECTION_LENGTH_IN_URAM_PACKED_ELEMS);
    std::vector<BV_URAM_PACKED_DTYPE> bv_section4(BV_SECTION_LENGTH_IN_URAM_PACKED_ELEMS);
    #if NUM_HASH != 5
    crash(compilation)
    #endif
    // Datapack the bitvector
    for (int i = 0; i < BV_LENGTH; ++i) {
        int section_idx     = (i/BV_URAM_PACKED_BITWIDTH) / BV_SECTION_LENGTH_IN_URAM_PACKED_ELEMS;
        int array_idx       = (i/BV_URAM_PACKED_BITWIDTH) % BV_SECTION_LENGTH_IN_URAM_PACKED_ELEMS;
        int bit_idx         = (i%BV_URAM_PACKED_BITWIDTH);

        if (section_idx == 0) {
            bv_section0[array_idx].range(bit_idx, bit_idx) = bv[i].range(0, 0);
        }
        else if (section_idx == 1) {
            bv_section1[array_idx].range(bit_idx, bit_idx) = bv[i].range(0, 0);
        }
        else if (section_idx == 2) {
            bv_section2[array_idx].range(bit_idx, bit_idx) = bv[i].range(0, 0);
        }
        else if (section_idx == 3) {
            bv_section3[array_idx].range(bit_idx, bit_idx) = bv[i].range(0, 0);
        }
        else if (section_idx == 4) {
            bv_section4[array_idx].range(bit_idx, bit_idx) = bv[i].range(0, 0);
        }
        else {
            printf("Something went wrong with the BV datapacking computation...\n");
            exit(-1);
        }
        #if NUM_HASH != 5
        crash();
        #endif
    }

    // Pack the sections into the final BV.
    for (int i = 0; i < BV_SECTION_LENGTH_IN_URAM_PACKED_ELEMS; ++i) {
        packed_bv[i].section0 = bv_section0[i];
        packed_bv[i].section1 = bv_section1[i];
        packed_bv[i].section2 = bv_section2[i];
        packed_bv[i].section3 = bv_section3[i];
        packed_bv[i].section4 = bv_section4[i];
        #if NUM_HASH != 5
        crash();
        #endif
    }
}




void datapack_keys(
    KEY_DTYPE *keys
    ,LOAD_DTYPE *krnl_key_in_0
    #if NUM_AXI_PORTS != 1
    crash(compilation)
    #endif
) {
    // Pack the key inputs for the kernel.
    for (int i = 0; i < KEYPAIRS_PER_STM; ++i)
    {
        LOAD_DTYPE cur_packed_input_axi0;

        cur_packed_input_axi0.s0_k0 = keys[i*2*NUM_STM + 2*0 + 0];
        cur_packed_input_axi0.s0_k1 = keys[i*2*NUM_STM + 2*0 + 1];

        cur_packed_input_axi0.s1_k0 = keys[i*2*NUM_STM + 2*1 + 0];
        cur_packed_input_axi0.s1_k1 = keys[i*2*NUM_STM + 2*1 + 1];

        cur_packed_input_axi0.s2_k0 = keys[i*2*NUM_STM + 2*2 + 0];
        cur_packed_input_axi0.s2_k1 = keys[i*2*NUM_STM + 2*2 + 1];

        cur_packed_input_axi0.s3_k0 = keys[i*2*NUM_STM + 2*3 + 0];
        cur_packed_input_axi0.s3_k1 = keys[i*2*NUM_STM + 2*3 + 1];

        cur_packed_input_axi0.s4_k0 = keys[i*2*NUM_STM + 2*4 + 0];
        cur_packed_input_axi0.s4_k1 = keys[i*2*NUM_STM + 2*4 + 1];

        cur_packed_input_axi0.s5_k0 = keys[i*2*NUM_STM + 2*5 + 0];
        cur_packed_input_axi0.s5_k1 = keys[i*2*NUM_STM + 2*5 + 1];

        cur_packed_input_axi0.s6_k0 = keys[i*2*NUM_STM + 2*6 + 0];
        cur_packed_input_axi0.s6_k1 = keys[i*2*NUM_STM + 2*6 + 1];

        cur_packed_input_axi0.s7_k0 = keys[i*2*NUM_STM + 2*7 + 0];
        cur_packed_input_axi0.s7_k1 = keys[i*2*NUM_STM + 2*7 + 1];

        #if NUM_STM != 8
        crash on purpose(,
        #endif

        krnl_key_in_0[i] = cur_packed_input_axi0;

        #if NUM_AXI_PORTS != 1
        crash(compilation)
        #endif

    }
}



void datapack_krnl_inputs(
    bool            do_verif
    ,KEY_DTYPE      *keys
    ,LOAD_DTYPE     *krnl_key_in_0

    #if NUM_AXI_PORTS != 1
    crash(compilation)
    #endif

    ,BIT_DTYPE      *bv
    ,BV_LOAD_DTYPE  *packed_bv
) {
    datapack_keys(
        keys
        ,krnl_key_in_0
        #if NUM_AXI_PORTS != 1
        crash(compilation)
        #endif
    );

    /* Only datapack the BV if were doing verification. Because otherwise we only want to test
     * the runtime, and datapacking costs some server CPU time.
     */
    if (do_verif) {
        datapack_bv(bv, packed_bv);
    }
}




//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

void unpack_krnl_outputs(
    STORE_DTYPE     *krnl_out_0,
    #if NUM_AXI_PORTS != 1
    crash(compilation)
    #endif

    BIT_DTYPE      *krnl_merged_out
) {
    int out_bit_idx = 0;
    for (int i = 0; i < PACKED_OUTPUTS_PER_STM; ++i)
    {
        for (int j = 0; j < OUT_PACKED_BITWIDTH; ++j)
        {
            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s0_k0.range(j, j);
            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s0_k1.range(j, j);

            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s1_k0.range(j, j);
            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s1_k1.range(j, j);

            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s2_k0.range(j, j);
            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s2_k1.range(j, j);

            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s3_k0.range(j, j);
            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s3_k1.range(j, j);

            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s4_k0.range(j, j);
            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s4_k1.range(j, j);

            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s5_k0.range(j, j);
            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s5_k1.range(j, j);

            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s6_k0.range(j, j);
            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s6_k1.range(j, j);

            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s7_k0.range(j, j);
            krnl_merged_out[out_bit_idx++].range(0, 0) =
                krnl_out_0[i].s7_k1.range(j, j);

        }
    }
}
