
/*
 * Copyright 2023-2024 Gerrit Pape (papeg@mail.upb.de)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Aurora.hpp"
#include <fstream>
#include <unistd.h>
#include <vector>
#include "BitBlender.h"
#include <getopt.h>

class Configuration
{
public:
    const char *optstring = "";

    // Defaults
    uint32_t device_id_offset = 0;

    std::string xclbin_file = "";

    uint32_t iterations = 1;
    uint32_t frame_size = 1;
    bool use_ack = false;
    uint32_t timeout_ms = 10000; // 10 seconds

    uint32_t BITBLD_key_loads_num_bytes = 0;
    uint32_t BITBLD_keys_num_loads      = 0;

    uint32_t BITBLD_bv_loads_num_bytes  = 0;
    uint32_t BITBLD_bv_num_loads        = 0;

    uint32_t BITBLD_stores_num_bytes    = 0;
    uint32_t BITBLD_num_stores          = 0;

    Configuration(int argc, char **argv)
    {
        int opt;

        struct option long_options[2] = {};
        long_options[0].name = "bitstream";
        long_options[0].has_arg = 1;
        long_options[0].flag = NULL;
        long_options[0].val = 0;

        while ((opt = getopt_long(argc, argv, optstring, long_options, NULL)) != -1) {
            if (opt == 0 && optarg) {
                xclbin_file = std::string(optarg);
            }
        }

        if (xclbin_file == "") {
            std::cerr << "Error: no bitstream file passed" << std::endl;
            exit(1);
        }

        BITBLD_keys_num_loads       = KEYPAIRS_PER_STM;
        BITBLD_num_stores           = PACKED_OUTPUTS_PER_STM;
        BITBLD_bv_num_loads         = BV_NUM_LOADS;

        BITBLD_bv_loads_num_bytes   = BITBLD_bv_num_loads * sizeof(BV_LOAD_DTYPE);
        BITBLD_key_loads_num_bytes  = BITBLD_keys_num_loads * sizeof(LOAD_DTYPE);
        BITBLD_stores_num_bytes     = BITBLD_num_stores * sizeof(STORE_DTYPE);
    }

    void print()
    {
        std::cout << std::endl;
        std::cout << "------------------------ aurora test ------------------------" << std::endl;
        std::cout << "Number of bytes: " << BITBLD_key_loads_num_bytes << std::endl;
        std::cout << "Selected bitstream: " << xclbin_file << std::endl;
        std::cout << "Frame size: " << frame_size << std::endl;
        //std::cout << "Random data" << std::endl;
        std::cout << "Using ack: " << use_ack << std::endl;
        std::cout << iterations << " iterations" << std::endl;
        std::cout << "Issue/Dump timeout: " << timeout_ms << " ms" << std::endl;
    }

    void write_results(double transmission_time)
    {
        char* hostname;
        hostname = new char[100];
        gethostname(hostname, 100);

        std::ofstream of;
        of.open("results.csv", std::ios_base::app);
        of << "hostname, frame_size, msg_size (B),"
            << " transmission time, used_acks" << std::endl;
        of << hostname
            << "," << frame_size
            << "," << BITBLD_key_loads_num_bytes
            << "," << transmission_time
            << "," << use_ack << std::endl;


        of.close();
    }
};










class IssueKernel
{
public:
    IssueKernel(uint32_t instance, xrt::device &device, xrt::uuid &xclbin_uuid, Configuration &config) : instance(instance), config(config)
    {
        char name[100];
        snprintf(name, 100, "issue:{issue_%u}", instance);
        kernel = xrt::kernel(device, xclbin_uuid, name);

        run = xrt::run(kernel);
        this->device = device;
   }

    void prepare_bitblender_inputs(
        std::vector<LOAD_DTYPE, tapa::aligned_allocator<LOAD_DTYPE>>    & key_input
    )
    {
        if (this->instance == 1) {
            uint64_t num_input_bytes = key_input.size() * sizeof(key_input[0]);
            assert(config.BITBLD_key_loads_num_bytes == num_input_bytes);

            int frame_size = 0;
            int iterations = 1;
            int use_ack = 0;

            data_bo = xrt::bo(device, config.BITBLD_key_loads_num_bytes, xrt::bo::flags::normal, kernel.group_id(1));
            data_bo.write(key_input.data());
            data_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);

            run.set_arg(1, data_bo);
            run.set_arg(2, config.BITBLD_key_loads_num_bytes);    // Message size
            run.set_arg(3, frame_size);
            run.set_arg(4, iterations);
            run.set_arg(5, use_ack);

            //printf("\nPREPARE BITBLD INPUTS:\n");
            //printf("sizeof key_input[0] = %lu\n", sizeof(key_input[0]));
            //printf("ARG key_input.size() = %lu\n", key_input.size());
            //printf("ARG num_input_bytes = %lu\n", num_input_bytes);
            //printf("CFG num_input_bytes = %lu\n", config.BITBLD_key_loads_num_bytes);
        }
        else {
            printf("ERROR: You're trying to prepare Bitblender inputs for the dump-connected issue?\n");
        }
    }

    void start()
    {
        run.start();
    }

    bool timeout()
    {
        return run.wait(std::chrono::milliseconds(config.timeout_ms)) == ERT_CMD_STATE_TIMEOUT;
    }

private:
    xrt::device device;
    xrt::bo data_bo;
    xrt::kernel kernel;
    xrt::run run;
    uint32_t instance;
    Configuration &config;
};






class DumpKernel
{
public:

    DumpKernel(uint32_t instance, xrt::device &device, xrt::uuid &xclbin_uuid, Configuration &config) : instance(instance), config(config)
    {
        char name[100];
        snprintf(name, 100, "dump:{dump_%u}", instance);
        kernel = xrt::kernel(device, xclbin_uuid, name);

        run = xrt::run(kernel);

        data_bo = xrt::bo(  device,
                            config.BITBLD_stores_num_bytes,
                            xrt::bo::flags::normal,
                            kernel.group_id(1)
        );

        output_packed_data.resize(config.BITBLD_num_stores);

        run.set_arg(1, data_bo);
        run.set_arg(2, config.BITBLD_stores_num_bytes);
        run.set_arg(3, config.iterations);
        run.set_arg(4, config.use_ack);

        //printf("\nDUMP KERNEL:\n");
        //printf("CFG store bytes = %lu\n",   config.BITBLD_stores_num_bytes);
        //printf("CFG num_stores = %lu\n",    config.BITBLD_num_stores);
        //printf("output_packed_data.size() = %lu\n", output_packed_data.size());
    }

    void start()
    {
        run.start();
    }

    bool timeout()
    {
        return run.wait(std::chrono::milliseconds(config.timeout_ms)) == ERT_CMD_STATE_TIMEOUT;
    }

    std::vector<STORE_DTYPE, tapa::aligned_allocator<STORE_DTYPE>>  get_output_data()
    {
        data_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
        data_bo.read(output_packed_data.data());
        return output_packed_data;
    }


    std::vector<STORE_DTYPE,    tapa::aligned_allocator<STORE_DTYPE>>       output_packed_data;

private:
    xrt::bo data_bo;
    xrt::kernel kernel;
    xrt::run run;
    uint32_t instance;
    Configuration &config;
};








class BitBlenderKernel
{
public:

    BitBlenderKernel(uint32_t instance
                    ,xrt::device &device
                    ,xrt::uuid &xclbin_uuid
                    ,Configuration &config
                    ,uint32_t packed_inputs_num_entries
                    ,uint32_t bv_num_entries
#if ENABLE_PERF_CTRS
                    ,int perfctr_entries
#endif
                    ,std::vector<BV_LOAD_DTYPE, tapa::aligned_allocator<BV_LOAD_DTYPE>>  packed_bv_data
    ) : instance(instance), config(config)
    {
        char name[100];
        snprintf(name, 100, "workload:{workload_%u}", instance);
        kernel = xrt::kernel(device, xclbin_uuid, name);

        run = xrt::run(kernel);

        bv_bo = xrt::bo(    device
                            ,bv_num_entries * sizeof(BV_LOAD_DTYPE)
                            ,xrt::bo::flags::normal
                            ,kernel.group_id(0)
        );
        bv_bo.write(packed_bv_data.data());
        bv_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
        /**********************************************/

#if ENABLE_PERF_CTRS
        perfctrs_data.resize(perfctr_entries);
        perfctrs_bo = xrt::bo(  device
                                ,perfctr_entries * sizeof(PERFCTR_DTYPE)
                                ,xrt::bo::flags::normal
                                ,kernel.group_id(3)
        );
#endif

        run.set_arg(0, bv_bo);
#if ENABLE_PERF_CTRS
        run.set_arg(3, perfctrs_bo);
        run.set_arg(4, packed_inputs_num_entries);
#else
        run.set_arg(3, packed_inputs_num_entries);
#endif

        //printf("\nBITBLENDER KERNEL:\n");
        //printf("CFG BV entries = %lu\n",    config.BITBLD_bv_num_loads);
        //printf("ARG BV entries = %lu\n",    bv_num_entries);
        //printf("sizeof(BV_LOAD_DTYPE) = %lu\n", sizeof(BV_LOAD_DTYPE));

        //printf("CFG packed keyinputs = %lu\n",  config.BITBLD_keys_num_loads);
        //printf("ARG packed keyinputs= %lu\n",   packed_inputs_num_entries);
        //printf("sizeof(LOAD_DTYPE) = %lu\n",        sizeof(LOAD_DTYPE));
    }

    void start()
    {
        run.start();
    }

    bool timeout()
    {
        return run.wait(std::chrono::milliseconds(config.timeout_ms)) == ERT_CMD_STATE_TIMEOUT;
    }


    std::vector<LOAD_DTYPE, tapa::aligned_allocator<LOAD_DTYPE>>  get_debug_in_data()
    {
        debug_in_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
        debug_in_bo.read(DEBUG_input_data.data());
        return DEBUG_input_data;
    }


    std::vector<STORE_DTYPE, tapa::aligned_allocator<STORE_DTYPE>>  get_debug_out_data()
    {
        debug_out_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
        debug_out_bo.read(DEBUG_output_packed_data.data());
        return DEBUG_output_packed_data;
    }


    std::vector<PERFCTR_DTYPE, tapa::aligned_allocator<PERFCTR_DTYPE>>  get_perfctrs_data()
    {
#if ENABLE_PERF_CTRS
        perfctrs_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
        perfctrs_bo.read(perfctrs_data.data());
#endif
        return perfctrs_data;
    }

    std::vector<LOAD_DTYPE,     tapa::aligned_allocator<LOAD_DTYPE>>        DEBUG_input_data;
    std::vector<STORE_DTYPE,    tapa::aligned_allocator<STORE_DTYPE>>       DEBUG_output_packed_data;
    std::vector<PERFCTR_DTYPE,  tapa::aligned_allocator<PERFCTR_DTYPE>>     perfctrs_data;

private:
    xrt::bo bv_bo;
    xrt::bo debug_out_bo;
    xrt::bo debug_in_bo;
    xrt::kernel kernel;
    xrt::run run;
    uint32_t instance;
    Configuration &config;

#if ENABLE_PERF_CTRS
    xrt::bo perfctrs_bo;
#endif
};






void check_core_status_global(Aurora &aurora, size_t timeout_ms)
{
    bool local_core_ok;
    local_core_ok = aurora.core_status_ok(3000);

    if (!local_core_ok) {
        std::cout << "problem with one of the aurora cores... #" << aurora.aurora_number << std::endl;
        //exit(-1);
    }
}
        