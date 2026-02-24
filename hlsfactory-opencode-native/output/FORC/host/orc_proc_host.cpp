#include "orc_proc_host.h"

void orc_proc(tapa::mmap<_512b> input_port, 
                    tapa::mmap<_512b> FilterConf_port,
                    tapa::mmap<_512b> output_port0_32b_8b, 
                    tapa::mmap<_512b> output_port1_16b_8b,
                    tapa::mmap<_512b> output_port2_16b_8b,
                    tapa::mmap<_512b> output_port3_8b,
                    tapa::mmap<_512b> data_Idx,
                    tapa::mmap<_512b> output_port4_Track,
                    uint32_t data_count
                    );

DEFINE_string(bitstream, "", "path to bitstream file, run csim if empty");
DEFINE_string(comp, "", "Compressed zlib file path");
DEFINE_string(orig, "", "Original file path");
DEFINE_bool(is_orc, false, "Specify whether the input file is an ORC file");
DEFINE_int32(RR, 0, "Right Range (RR) value");
DEFINE_bool(VERIF, false, "Specify whether to verify Output");

void async_readnorm(struct aiocb* aio_rf, void* data_in, int Fd, int vector_size_bytes, int offset)
{
    aio_rf->aio_buf = data_in;
    aio_rf->aio_fildes = Fd;
    aio_rf->aio_nbytes = vector_size_bytes;
    aio_rf->aio_offset = offset;
    int result = aio_read(aio_rf);
    if (result < 0)
    {
        printf("Read Failed: %d \n", result);
    }
}
void copy_data(unsigned char* src, unsigned char* dest, size_t size, size_t offset) {
    unsigned char* src_ptr = src;
    unsigned char* dest_ptr = dest + offset;
    for (size_t i = 0; i < size; ++i) {
        dest_ptr[i] = src_ptr[i];
    }
}

// void writeZeros(void* ptr, uint32_t offset, uint32_t size) {
//     // Calculate the address with the offset
//     void* target = static_cast<char*>(ptr) + offset;

//     // Write zeros to the memory region
//     memset(target, 0, size);
// }

void print_data(uint8_t* data0, uint8_t* data1, uint8_t* data2, uint8_t* data3, uint32_t stripe_rows, uint32_t offset) {
    // Read the total number of rows
    int tRows = stripe_rows;
    
    int numbers_to_print = 16; // Number of 32-bit numbers to print from each pointer

    // Array of pointers for easier access in the loop, with offset added
    uint8_t* data_out_HBM[4] = {
        data0 + offset,
        data1 + offset,
        data2 + offset,
        data3 + offset
    };

    // Start printing numbers from the pointers in sequence until tRows
    for (int row = 0, cRow = 0; cRow < tRows; cRow += (64)) {
        for (int i = 0; i < 4; ++i) {  // Loop through the four data pointers
            for (int j = 0; j < numbers_to_print; ++j) {  // Print 16 numbers from each pointer
                int index = row + j;
                if (index < tRows) {
                    // Read 32-bit (4 bytes) numbers
                    uint32_t number = *reinterpret_cast<uint32_t*>(data_out_HBM[i] + index * 4);
                    std::cout << "Data[" << i << "][" << index << "] = " << number << std::endl;
                }
            }
        }
        row += numbers_to_print;
    }
}

void print_Fdata(uint8_t* data0, uint8_t* data1, uint8_t* data2, uint8_t* data3, uint32_t *stripe_rows, uint32_t stripeCount) {
    std::ofstream outFile("output.txt");  // Create and open the output file
    
    if (!outFile.is_open()) {
        std::cerr << "Failed to open the file!" << std::endl;
        return;
    }
    
    uint32_t stCount = 0;
    uint32_t offset = 0;
    uint32_t Doffset = 0;
    // Array of pointers for easier access in the loop, with offset added
    uint8_t* data_out_HBM[4];

    while (stCount < stripeCount) {
        // Read the total number of rows
        uint32_t tRows = stripe_rows[stCount];
        offset = (stCount == 0) ? 0 : stripe_rows[stCount - 1];
        Doffset += offset;

        std::cout << "Doffset: " << Doffset << std::endl;

        // Assign each pointer individually
        data_out_HBM[0] = data0 + Doffset;
        data_out_HBM[1] = data1 + Doffset;
        data_out_HBM[2] = data2 + Doffset;
        data_out_HBM[3] = data3 + Doffset;

        // Start writing numbers from the pointers in sequence until tRows
        for (int row = 0, cRow = 0; cRow < tRows; cRow += (64)) {
            for (int i = 0; i < 4; ++i) {  // Loop through the four data pointers
                for (int j = 0; j < 16; ++j) {  // Write 16 numbers from each pointer
                    int index = row + j;
                    if (index < tRows) {
                        // Read 32-bit (4 bytes) numbers
                        uint32_t number = *reinterpret_cast<uint32_t*>(data_out_HBM[i] + index * 4);
                        outFile << number << std::endl;
                    }
                }
            }
            row += 16;
        }
        stCount++;
    }
    
    outFile.close();  // Close the file when done
}

void processFlags(const std::vector<_512b, tapa::aligned_allocator<_512b>>& data);

void verif_sorted(std::vector<int32_t> combinedData, uint32_t* FilRows);

void update_patch_data(int32_t *datain0, int32_t *datain1, int32_t *datain2, int32_t *datain3, int32_t *track);


int main(int argc, char* argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);


    uint8_t* dataOut[4]; 
    uint8_t* trackOut;
    uint8_t* idxOut;
    uint32_t* filterRowCount;

    uint32_t ORIG_file_size_bytes = 0;
    uint32_t file_size_rem = 0;
    uint32_t KRNL_file_size_bytes = 0;
    uint32_t Aligned_KRNL_file_size_bytes = 0;
    uint32_t KRNL_file_size_count = 0;
    uint32_t file_size_bytes = 0;
    uint32_t Data_offset = 0;
    uint32_t Data_length = 0;
    uint32_t KRNL_Data_Write = 0;
    std::string nvme_file;

    if (FLAGS_comp.empty()) {
        std::cerr << "Error: Encoded file path (--enc) is required." << std::endl;
        return EXIT_FAILURE;
    }

    // Open the compressed file
    std::ifstream comp_file(FLAGS_comp, std::ios::binary);
    if (!comp_file.is_open()) {
        std::cerr << "Error: Could not open encoded file " << FLAGS_comp << std::endl;
        return EXIT_FAILURE;
    }

    nvme_file = FLAGS_comp;
    uint64_t stripeCount = 0;
    if (FLAGS_is_orc)
    {
        ////ORC READER////
        orc::ReaderOptions readerOpts;
        std::unique_ptr<orc::Reader> reader =
            orc::createReader(orc::readFile(FLAGS_comp, readerOpts.getReaderMetrics()), readerOpts);

        // nvme_file = orc_file;
        std::cout << "{ \"name\": \"" << FLAGS_comp << "\",\n";
        uint64_t numberColumns = reader->getType().getMaximumColumnId() + 1;

        std::cout << "\n  \"file length\": " << reader->getFileLength() << ",\n";
        // std::cout << "  \"type\": \"" << reader->getType().toString() << "\",\n";
        
        // nrows = reader->getNumberOfRows();
        std::cout << "  \"rows\": " << reader->getNumberOfRows() << ",\n";
        stripeCount = reader->getNumberOfStripes();
        std::cout << "  \"stripe count\": " << stripeCount << ",\n";

        for (uint64_t col = 0; col < numberColumns; ++col) {
        orc::ColumnEncodingKind encoding = reader->getStripe(0)->getColumnEncoding(col);
        std::cout << "         { \"column\": " << col << ", \"encoding\": \""
            << columnEncodingKindToString(encoding) << "\"";
        if (encoding == orc::ColumnEncodingKind_DICTIONARY ||
            encoding == orc::ColumnEncodingKind_DICTIONARY_V2) {
            std::cout << ", \"count\": " << reader->getStripe(0)->getDictionarySize(col);
        }
        std::cout << " }";
        std::cout << std::endl;
        }
        
        std::unique_ptr<orc::StripeInformation> stripeInfo = reader->getStripe(0);
        nrows = stripeInfo->getNumberOfRows();
        std::cout << "  \"rows in stripe 0\": " << nrows << ",\n";
        for (uint64_t str = 0; str < reader->getStripe(0)->getNumberOfStreams(); ++str) {
        if (str != 0) {
            std::cout << ",\n";
        }
        std::unique_ptr<orc::StreamInformation> stream = reader->getStripe(0)->getStreamInformation(str);
        std::cout << "        { \"id\": " << str << ", \"column\": " << stream->getColumnId()
            << ", \"kind\": \"" << streamKindToString(stream->getKind())
            << "\", \"offset\": " << stream->getOffset() << ", \"length\": " << stream->getLength()
            << " }";

            if(stream->getKind() == 1) 
            {
                std::cout << "\n \nData stream found" << std::endl;
                Data_offset = stream->getOffset();
                Data_length = stream->getLength();
            }
        }
        reader.reset(); //       
    }
    else
    {
        // Set total rows in header file incase not orc file
        nrows = Myrows;
        stripeCount = 1;
        comp_file.seekg(0, std::ios::end);
        Data_length = static_cast<uint64_t>(comp_file.tellg());  // Cast tellg() to uint64_t
        comp_file.seekg(0, std::ios::beg);
        
        Data_offset = 0;        
    }
    std::cout << "Data_offset: " << Data_offset << std::endl;
    std::cout << "Data_length: " << Data_length << std::endl;
    std::cout << "Number of rows are: " << nrows << std::endl;    


    ///////DECLARE READ WRITE HOST PTRs////////
    uint8_t* data_in_HBM[BUFFERS_IN];           //input port
    uint8_t* FilterConf_HBM[BUFFERS_IN];           //Filter Conf 
    uint8_t* data_out_HBM[BUFFERS_OUT];         //4 output, 1 meta and 1 idx    = 5*2 = 12

    uint32_t max_input_size = Data_length;
    uint32_t max_output_size = nrows/64;
    uint32_t remR =  nrows%64;
    if(remR!=0)
    {
        max_output_size+=1;
    }
    max_output_size = max_output_size*64;
    uint32_t max_track_size = max_output_size*1.2; //max it can be 2x of the one data port size
    remR = max_track_size%16;  //128bit is 16bytes
    if(remR != 0)
    {
        max_track_size += (16 - remR);
    }

    uint32_t filterConfSize = 192;

    for(int i = 0; i < BUFFERS_IN; i++)
    {
        data_in_HBM[i] = static_cast<uint8_t*>(aligned_alloc(ALIGNED_BYTES, max_input_size));
    }
    for(int i = 0; i < BUFFERS_IN; i++)
    {
        FilterConf_HBM[i] = static_cast<uint8_t*>(aligned_alloc(ALIGNED_BYTES, 192));
    }
    //out ports and idx port
    for (int i = 0; i < BUFFERS_OUT-2; ++i) {
        data_out_HBM[i] = static_cast<uint8_t*>(aligned_alloc(ALIGNED_BYTES, max_output_size));
    }
    //track ports
    for (int i = BUFFERS_OUT-2; i < BUFFERS_OUT; ++i) {
        data_out_HBM[i] = static_cast<uint8_t*>(aligned_alloc(ALIGNED_BYTES, max_track_size));
    }

    for (int i = 0; i < 4; ++i) {
        dataOut[i] = static_cast<uint8_t*>(aligned_alloc(ALIGNED_BYTES, (max_output_size*DATA_MUL)));
    }
    //track port
    trackOut = static_cast<uint8_t*>(aligned_alloc(ALIGNED_BYTES, (max_track_size*DATA_MUL)));
    idxOut = static_cast<uint8_t*>(aligned_alloc(ALIGNED_BYTES, (max_output_size*DATA_MUL)));
    filterRowCount = static_cast<uint32_t*>(aligned_alloc(ALIGNED_BYTES, (stripeCount*DATA_MUL)*192));

    //Write Filter Config
    uint8_t idx_flag = 0;           //use filter condition(0),  use stored idx(1)
    uint8_t range_flag = 0;         //either use both ranges(1), or only right range(0) , Currently unused (using both in ranges in kernel)
    uint8_t RROP = 0;
    uint8_t LROP = 0;
    int32_t RR = 0;
    int32_t LR = 0;

    RROP = 2;   // less than(1), less than equal to(2)
    LROP = 6;   //  5 = greater than, 6 = greater than equal
    RR = FLAGS_RR;   //SR->300, DD->100, max 4294967295, ...100000000<#<330000000 ... LR<#<RR ... 4294967295 ... 2147483647
    LR = 0;    //SR->10, DD->30

    _512b filconf = (uint32_t)(LR);
    filconf = filconf << 32;    //4
    filconf = filconf | (uint32_t)(RR);
    filconf = filconf << 8;     //1
    filconf = filconf | LROP;
    filconf = filconf << 8;     //1
    filconf = filconf | RROP;
    filconf = filconf << 8;     //1
    filconf = filconf | range_flag;
    filconf = filconf << 8;     //1
    filconf = filconf | idx_flag;

    uint32_t MyremDiv = nrows%64;
    uint32_t readCnt = nrows/64;
    if(MyremDiv!=0)
    {
        readCnt += 1;
    }

    // Cast the FilterConf_HBM pointer to an ap_uint<512>* to store filconf
    _512b* ptr = reinterpret_cast<ap_uint<512>*>(FilterConf_HBM[0]);
    _512b* ptr1 = reinterpret_cast<ap_uint<512>*>(FilterConf_HBM[1]);

    // Assign the first 512 bits (64 bytes) to 'filconf'
    ptr[0] = filconf;
    ptr1[0] = filconf;

    // The second 512 bits will store the 'readCnt'. We'll place 'readCnt' in the first 32 bits of this block.
    _512b readCntBlock = 0;
    readCntBlock.range(31, 0) = readCnt;

    // Assign the second 512 bits (64 bytes) to 'readCnt'
    ptr[1] = readCntBlock;
    ptr1[1] = readCntBlock;
    // std::cout << "readCnt:  " << readCnt << std::endl;

        
    ///////////////////////////

    ///////Getting nvme ssd file desc////////
        auto FileTimeS = std::chrono::steady_clock::now();
        
        nvmeFd = open(nvme_file.c_str(), O_RDONLY); // | O_DIRECT | O_SYNC  O_RDONLY  O_RDWR
        if (nvmeFd < 0) {
            std::cerr << "ERROR: open " << nvme_file << "failed: " << std::endl;
            return EXIT_FAILURE;
        }
        auto FileTimeE = std::chrono::steady_clock::now();
        auto FileTime = std::chrono::duration_cast<std::chrono::microseconds>(FileTimeE - FileTimeS);
        std::cout << "File Opening Time (us):  " << FileTime.count() << std::endl;
        std::cout << "INFO: Successfully opened NVME SSD1 " << nvme_file << std::endl;
    ///////////////////////////
    if(FLAGS_bitstream != "") // FLAGS_bitstream != "" : Dont use for hw_emu/csim/sw_emu
    {
        /////////MY HOST///////////
            cl::Device device_;
            cl::Context context_;
            cl::CommandQueue cmd_;
            cl::Program program_;
            std::string My_device_name;
            std::map<int, cl::Kernel> kernels_;

            std::string target_device_name;
            std::vector<std::string> kernel_names;
            std::vector<int> kernel_arg_counts;
            int arg_count = 0;

            LOG(INFO) << "Loading Binaries From: " << FLAGS_bitstream << std::endl;
            cl::Program::Binaries binaries;
            {
                std::ifstream stream(FLAGS_bitstream, std::ios::binary);
                binaries = {{std::istreambuf_iterator<char>(stream),
                            std::istreambuf_iterator<char>()}};
            }
            const auto axlf_top = reinterpret_cast<const axlf*>(binaries.begin()->data());
            switch (axlf_top->m_header.m_mode) {
                case XCLBIN_FLAT:
                case XCLBIN_PR:
                case XCLBIN_TANDEM_STAGE2:
                case XCLBIN_TANDEM_STAGE2_WITH_PR:
                break;
                case XCLBIN_HW_EMU:
                setenv("XCL_EMULATION_MODE", "hw_emu", 0);
                break;
                case XCLBIN_SW_EMU:
                setenv("XCL_EMULATION_MODE", "sw_emu", 0);
                break;
                default:
                LOG(FATAL) << "Unknown xclbin mode";
            }
            target_device_name =
                reinterpret_cast<const char*>(axlf_top->m_header.m_platformVBNV);
            std::cout << "target_device_name: " << target_device_name << std::endl;
            if (auto metadata = xclbin::get_axlf_section(axlf_top, EMBEDDED_METADATA)) {
                TiXmlDocument doc;
                doc.Parse(
                    reinterpret_cast<const char*>(axlf_top) + metadata->m_sectionOffset,
                    nullptr, TIXML_ENCODING_UTF8);
                auto xml_core = doc.FirstChildElement("project")
                                    ->FirstChildElement("platform")
                                    ->FirstChildElement("device")
                                    ->FirstChildElement("core");
                std::string target_meta = xml_core->Attribute("target");
                for (auto xml_kernel = xml_core->FirstChildElement("kernel");
                    xml_kernel != nullptr;
                    xml_kernel = xml_kernel->NextSiblingElement("kernel")) 
                {
                    kernel_names.push_back(xml_kernel->Attribute("name"));
                    kernel_arg_counts.push_back(arg_count);
                    ++arg_count;
                    size_t i = kernel_names.size() - 1;
                    std::cout << "Kernel Name: " << kernel_names[i] << ", Argument Count: " << kernel_arg_counts[i] << std::endl;
                }
                if (target_meta == "hw_em") {
                setenv("XCL_EMULATION_MODE", "hw_emu", 0);
                } else if (target_meta == "csim") {
                setenv("XCL_EMULATION_MODE", "sw_emu", 0);
                }
            }
            else {
                LOG(FATAL) << "Cannot determine kernel name from binary";
            }
            if (const char* xcl_emulation_mode = getenv("XCL_EMULATION_MODE")) {
                LOG(FATAL) << "Cannot RUN EMU MODE"; 
            }
            else {
                LOG(INFO) << "Running on-board execution with Xilinx OpenCL";
            }

            std::vector<cl::Platform> platforms;
            CL_CHECK(cl::Platform::get(&platforms));
            cl_int err;
            for (const auto& platform : platforms) {
                std::string platformName = platform.getInfo<CL_PLATFORM_NAME>(&err);
                CL_CHECK(err);
                LOG(INFO) << "Found platform: " << platformName.c_str();
                if (platformName == "Xilinx") {
                    std::vector<cl::Device> devices;
                    CL_CHECK(platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices));
                    for (const auto& device : devices) {
                        const std::string device_name = device.getInfo<CL_DEVICE_NAME>();
                        char bdf[32];
                        size_t bdf_size = 0;
                        CL_CHECK(clGetDeviceInfo(device.get(), CL_DEVICE_PCIE_BDF, sizeof(bdf), bdf,
                                                &bdf_size));
                        LOG(INFO) << "Found device: " << device_name;
                        if(device_name == target_device_name)
                        {
                            My_device_name = device_name;
                            device_ = device;
                            break;
                        }
                    }

                    LOG(INFO) << "Using " << My_device_name;
                    context_ = cl::Context(device_, nullptr, nullptr, nullptr, &err);
                    if (err == CL_DEVICE_NOT_AVAILABLE) {
                        LOG(WARNING) << "Device '" << My_device_name << "' not available";
                        continue;
                    }
                    CL_CHECK(err);
                    cmd_ = cl::CommandQueue(context_, device_, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE, &err);
                    CL_CHECK(err);
                    
                    std::vector<int> binary_status;
                    program_ =
                        cl::Program(context_, {device_}, binaries, &binary_status, &err);
                    for (auto status : binary_status) {
                        CL_CHECK(status);
                    }
                    CL_CHECK(err);
                    CL_CHECK(program_.build());
                    for (int i = 0; i < kernel_names.size(); ++i) {
                        // std::cout << "Kernels Count: " << kernel_arg_counts[i] << std::endl;
                        kernels_[kernel_arg_counts[i]] =
                            cl::Kernel(program_, kernel_names[i].c_str(), &err);
                        CL_CHECK(err);
                    }
                }
                else
                {
                    LOG(FATAL) << "Target platform 'Xilinx' not found";
                }
            }
            size_t map_size = kernels_.size();
            std::cout << "Kernels Size: " << map_size << std::endl;
            std::cout << "Kernel Programmed " << std::endl;
        ///////////////////////////

        ///////DECLARE BUFFERS FOR KERNEL////////
            cl::Buffer buffer_in_HBM[BUFFERS_IN];
            cl_mem_ext_ptr_t mIN_HBM[BUFFERS_IN];

            cl::Buffer filConf_in_HBM[BUFFERS_IN];
            cl_mem_ext_ptr_t mFCIN_HBM[BUFFERS_IN];

            cl::Buffer buffer_out_HBM[BUFFERS_OUT];
            cl_mem_ext_ptr_t mOUT_HBM[BUFFERS_OUT];

            //HBM Bank for input is 16:17
            for(uint32_t i = 0; i < BUFFERS_IN; i ++)
            {
                mIN_HBM[i] = {XCL_MEM_TOPOLOGY | (unsigned int)(i+16), data_in_HBM[i], 0};
                buffer_in_HBM[i] = cl::Buffer(context_, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                                (size_t)(max_input_size), &mIN_HBM[i], &err);     // CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE
                CL_CHECK(err);
            }
            //Filter Conf
            for(uint32_t i = 0; i < BUFFERS_IN; i ++)
            {
                mFCIN_HBM[i] = {XCL_MEM_TOPOLOGY | (unsigned int)(i), FilterConf_HBM[i], 0};
                filConf_in_HBM[i] = cl::Buffer(context_, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                (size_t)(192), &mFCIN_HBM[i], &err);     // CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE
                CL_CHECK(err);
            }

            //output ports
            for(int i = 0; i < BUFFERS_OUT-4; i++)
            {
                // (XCL_MEM_TOPOLOGY | memory bank)
                mOUT_HBM[i] = {XCL_MEM_TOPOLOGY | (unsigned int)(i+2), data_out_HBM[i], 0};
                buffer_out_HBM[i] = cl::Buffer(context_, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                                (size_t)(max_output_size), &mOUT_HBM[i], &err);     // CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE
                CL_CHECK(err);
            }

            //idx port
            for(int i = BUFFERS_OUT-4; i < BUFFERS_OUT-2; i++)
            {
                // (XCL_MEM_TOPOLOGY | memory bank)
                mOUT_HBM[i] = {XCL_MEM_TOPOLOGY | (unsigned int)(i+2), data_out_HBM[i], 0};
                buffer_out_HBM[i] = cl::Buffer(context_, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                (size_t)(max_output_size), &mOUT_HBM[i], &err);     // CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE
                CL_CHECK(err);
            }
            //meta port
            for(int i = BUFFERS_OUT-2; i < BUFFERS_OUT; i++)
            {
                // (XCL_MEM_TOPOLOGY | memory bank)
                mOUT_HBM[i] = {XCL_MEM_TOPOLOGY | (unsigned int)(i+2), data_out_HBM[i], 0};
                buffer_out_HBM[i] = cl::Buffer(context_, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                                (size_t)(max_track_size), &mOUT_HBM[i], &err);     // CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY, CL_MEM_READ_WRITE
                CL_CHECK(err);
            }

            std::cout << "Data in buffer size(MB): " << (max_input_size / (1000.0 * 1000.0)) << std::endl;
            std::cout << "Data out buffer size(MB): " << (max_output_size / (1000.0 * 1000.0)) << std::endl;

            cl::Kernel kernelDD;

            uint32_t dCount = Data_length / 64;
            if ((Data_length % 64) != 0) {
                dCount += 1;
            }

            for (const auto& kvp : kernels_) {
                int index = kvp.first; // Get the index (key) of the kernel
                std::cout << "Setting Kernel["<<index<<"] Arg" << std::endl;
                kernelDD = kvp.second; // Get the kernel associated with the index (key)
                kernelDD.setArg(0, buffer_in_HBM[0]);
                kernelDD.setArg(1, filConf_in_HBM[0]);
                kernelDD.setArg(2, buffer_out_HBM[0]);          //data0
                kernelDD.setArg(3, buffer_out_HBM[2]);          //data1
                kernelDD.setArg(4, buffer_out_HBM[4]);          //data2
                kernelDD.setArg(5, buffer_out_HBM[6]);          //data3
                kernelDD.setArg(6, buffer_out_HBM[8]);          //idx
                kernelDD.setArg(7, buffer_out_HBM[10]);         //data4(meta)
                kernelDD.setArg(8, sizeof(dCount), &dCount);
            }
            std::cout << "Kernels Argument Set." << std::endl;
            
        int ret_aio = 0;
        struct aiocb aio_rf;
        struct aiocb aio_rf1;
        ///////Launching KERNEL SINGLE SHOT////////
            std::vector<cl::Event> kernel_events(3);
            std::vector<cl::Event> kernel_wait_events;

            memset(data_in_HBM[0], 0, max_input_size);
            memset(data_in_HBM[1], 0, max_input_size);

            async_readnorm(&aio_rf, (void *)(data_in_HBM[0]), nvmeFd, Data_length, Data_offset); 
            while( aio_error(&aio_rf) == EINPROGRESS ) {;}
            ret_aio = aio_return (&aio_rf);
            printf("Bytes Read. %d \n", ret_aio);

            CL_CHECK(cmd_.enqueueWriteBuffer(buffer_in_HBM[0], CL_FALSE, 0, max_input_size, data_in_HBM[0], nullptr, &kernel_events[0]));
            // CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_in_HBM[0])} , 0 , nullptr, &kernel_events[0]));  //DRAM FPGA
            kernel_wait_events.resize(0);
            CL_CHECK(cmd_.flush());
            CL_CHECK(cmd_.finish());
            std::cout << "C2F Done" << std::endl;

            kernelDD.setArg(0, buffer_in_HBM[0]);
            kernelDD.setArg(1, filConf_in_HBM[0]);
            kernelDD.setArg(2, buffer_out_HBM[0]);          //data0
            kernelDD.setArg(3, buffer_out_HBM[2]);          //data1
            kernelDD.setArg(4, buffer_out_HBM[4]);          //data2
            kernelDD.setArg(5, buffer_out_HBM[6]);          //data3
            kernelDD.setArg(6, buffer_out_HBM[8]);          //idx
            kernelDD.setArg(7, buffer_out_HBM[10]);         //data4(meta)
            kernelDD.setArg(8, sizeof(dCount), &dCount);

            CL_CHECK(cmd_.flush());
            CL_CHECK(cmd_.finish());
            std::cout << "Kernel Arg Set" << std::endl;

            kernel_wait_events.push_back(kernel_events[0]);
            CL_CHECK(cmd_.enqueueTask(kernelDD, &kernel_wait_events, &kernel_events[1]));
            kernel_wait_events.resize(0);
            kernel_wait_events.push_back(kernel_events[1]);
            CL_CHECK(cmd_.flush());
            CL_CHECK(cmd_.finish());
            std::cout << "Kernel Done" << std::endl;

            //CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED,  CL_MIGRATE_MEM_OBJECT_HOST     
            CL_CHECK(cmd_.enqueueMigrateMemObjects({(filConf_in_HBM[0])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                        &kernel_wait_events, &kernel_events[2]));
            CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[0])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                        &kernel_wait_events, &kernel_events[2]));
            CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[2])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                        &kernel_wait_events, &kernel_events[2]));
            CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[4])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                        &kernel_wait_events, &kernel_events[2]));
            CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[6])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                        &kernel_wait_events, &kernel_events[2]));
            CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[8])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                        &kernel_wait_events, &kernel_events[2]));
            CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[10])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                        &kernel_wait_events, &kernel_events[2]));

            CL_CHECK(cmd_.flush());
            CL_CHECK(cmd_.finish());
            std::cout << "F2C Done" << std::endl;
            std::cout << "Initial Kernels Finished" << std::endl;

            int64_t load_time_ns = 0;
            int64_t compute_time_ns = 0; 
            int64_t store_time_ns = 0;
            double load_gbps = 0;
            double store_gbps = 0;

            ///////KERNEL Profiling////////
                cl_ulong start, end;
                kernel_events[0].getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
                kernel_events[0].getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
                load_time_ns = (end - start); //-- actual time is reported in nanoseconds

                kernel_events[1].getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
                kernel_events[1].getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
                compute_time_ns = (end - start); //-- actual time is reported in nanoseconds

                kernel_events[2].getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
                kernel_events[2].getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
                store_time_ns = (end - start); //-- actual time is reported in nanoseconds
            ///////////////////////////
        #ifdef PRINT_DEBUG
            // std::cout << "Kernel Exec time(ms): " << (compute_time_ns) * 1e-6 << std::endl;
            // std::cout << "Data Input Size (MB): " << (float)(Data_lengths[0]/(1024.0*1024.0)) << std::endl;
            // std::cout << "Data Output Size (MB): " << (float)((stripe_rows[0]*4)/(1024.0*1024.0)) << std::endl;
            // std::cout << "Kernel throughput(GB/s): " << (float)(Data_lengths[0])/(float)(compute_time_ns) << std::endl;

            // std::cout << "CPU-2-FPGA Transfer time(ms): " << (load_time_ns) * 1e-6 << std::endl;
            // std::cout << "FPGA-2-CPU Transfer time(ms): " << (store_time_ns) * 1e-6 << std::endl;
        #endif
        ///////Launching KERNEL DATAFLOW////////
            //non multiple RL adjustment
            // stripeCount -= 833;    //remove stripes

            uint32_t T_ITER = stripeCount*DATA_MUL;
            uint32_t NITERS = T_ITER + 4;  //IO, C2F, FCOMP, F2C, dCopy
            

            cl_uint one = 1;
            std::vector<cl::Event> C2F_events(NITERS);
            std::vector<cl::Event> FilC_events(4);
            std::vector<cl::Event> Comp_events(NITERS);
            std::vector<cl::Event> F2C_events(NITERS*7);
            std::vector<cl::Event> kernel_wait_events0;
            std::vector<cl::Event> kernel_wait_events1;

            auto asyncTimeS = std::chrono::steady_clock::now();
            auto asyncTimeE = std::chrono::steady_clock::now();
            auto asyncTime = std::chrono::duration_cast<std::chrono::microseconds>(asyncTimeE - asyncTimeS);

            auto readTimeS = std::chrono::steady_clock::now();
            auto readTimeE = std::chrono::steady_clock::now();
            auto readTime = std::chrono::duration_cast<std::chrono::microseconds>(asyncTimeE - asyncTimeS);

            auto FPGATimeS = std::chrono::steady_clock::now();
            auto FPGATimeE = std::chrono::steady_clock::now();
            auto FPGATime = std::chrono::duration_cast<std::chrono::microseconds>(asyncTimeE - asyncTimeS);

            auto dCopyTimeS = std::chrono::steady_clock::now();
            auto dCopyTimeE = std::chrono::steady_clock::now();
            auto dCopyTime = std::chrono::duration_cast<std::chrono::microseconds>(asyncTimeE - asyncTimeS);

            auto wrTimeS = std::chrono::steady_clock::now();
            auto wrTimeE = std::chrono::steady_clock::now();
            auto wrTime = std::chrono::duration_cast<std::chrono::microseconds>(asyncTimeE - asyncTimeS);

            auto tempTimeA = std::chrono::steady_clock::now();
            auto tempTimeB = std::chrono::steady_clock::now();
            auto tempTime = std::chrono::duration_cast<std::chrono::microseconds>(tempTimeA - tempTimeB);

            auto C2FTimeS = std::chrono::steady_clock::now();
            auto C2FTimeE = std::chrono::steady_clock::now();
            auto C2FTime = std::chrono::duration_cast<std::chrono::microseconds>(asyncTimeE - asyncTimeS);

            auto COMPTimeS = std::chrono::steady_clock::now();
            auto COMPTimeE = std::chrono::steady_clock::now();
            auto COMPTime = std::chrono::duration_cast<std::chrono::microseconds>(asyncTimeE - asyncTimeS);

            auto F2CTimeS = std::chrono::steady_clock::now();
            auto F2CTimeE = std::chrono::steady_clock::now();
            auto F2CTime = std::chrono::duration_cast<std::chrono::microseconds>(asyncTimeE - asyncTimeS);

            double time_dCopy[NITERS] = {0.0F};
            double time_fpga[NITERS] = {0.0F};
            double time_C2F[NITERS] = {0.0F};
            double time_COMP[NITERS] = {0.0F};
            double time_F2C[NITERS] = {0.0F};
            double time_read[NITERS] = {0.0F};
            double time_wr[NITERS] = {0.0F};
            double time_async[NITERS] = {0.0F};
            // async_readnorm(void* data_in, int nvmeFd, int vector_size_bytes, int offset)
            // std::cout << "Starting DF, Total Iters: " << NITERS << std::endl;
            // std::cout << "stripeCount: " << stripeCount << std::endl;
            uint32_t offsetD = 0;
            uint32_t offsetT = 0;
            uint32_t dSize_prev = 0;
            uint32_t tTrackSize_prev = 0;
            std::thread t1, t2, t3, t4, t5; // Declare threads outside the if block

            auto dfstart = std::chrono::steady_clock::now();
            if(dataflow)
            {
                std::cout << "***Dataflow Implementation***" << std::endl;
                dfstart = std::chrono::steady_clock::now();
                for (int i = 0; i < NITERS; i++)
                {
                    // std::cout << "ITER COUNT: " << i << std::endl;
                    asyncTimeS = std::chrono::steady_clock::now();
                    //IO READ
                    if(i < T_ITER)
                    {
                        if((i%2) == 0)
                        {
                            async_readnorm(&aio_rf, (void *)(data_in_HBM[0]), nvmeFd, Data_length, Data_offset); 
                            // std::cout << "IO_E" << std::endl;
                        }
                        else
                        {
                            async_readnorm(&aio_rf1, (void *)(data_in_HBM[1]), nvmeFd, Data_length, Data_offset); 
                            // std::cout << "IO_O" << std::endl;
                        }
                    }   

                    // tempTimeA = std::chrono::steady_clock::now();
                    //CPU_2_FPGA
                    if((i >= 1) && (i < (T_ITER+1)))
                    {
                        // int Ssize = Data_lengths[i-1];
                        if(((i-1)%2) == 0)
                        {
                            CL_CHECK(cmd_.enqueueWriteBuffer(buffer_in_HBM[0], CL_FALSE, 0, Data_length, data_in_HBM[0], nullptr, &C2F_events[i-1]));
                            if(i==1)
                            {
                                CL_CHECK(cmd_.enqueueWriteBuffer(filConf_in_HBM[0], CL_FALSE, 0, 192, FilterConf_HBM[0], nullptr, &FilC_events[i-1]));
                            }
                            
                            // cmd_.enqueueMigrateMemObjects({(buffer_in_HBM[0])} , 0 , nullptr, &C2F_events[i-1]);
                            // std::cout << "C2F_E" << std::endl;
                        }
                        else
                        {
                            CL_CHECK(cmd_.enqueueWriteBuffer(buffer_in_HBM[1], CL_FALSE, 0, Data_length, data_in_HBM[1], nullptr, &C2F_events[i-1]));
                            if(i==2)
                            {
                                CL_CHECK(cmd_.enqueueWriteBuffer(filConf_in_HBM[1], CL_FALSE, 0, 192, FilterConf_HBM[1], nullptr, &FilC_events[i-1]));
                            }
                            // cmd_.enqueueMigrateMemObjects({(buffer_in_HBM[1])} , 0 , nullptr, &C2F_events[i-1]);
                            // std::cout << "C2F_O" << std::endl;
                        }
                        // std::cout << "C2F" << ":" << i-1 << std::endl;
                    }
                    // tempTimeB = std::chrono::steady_clock::now();
                    // tempTime = std::chrono::duration_cast<std::chrono::microseconds>(tempTimeB - tempTimeA);
                    // std::cout << "time_async C2F: " << static_cast<double>(tempTime.count()) <<std::endl;

                    //KERNEL CALL
                    if((i >= 2) && (i < (T_ITER+2)))
                    {
                        uint32_t dCount = Data_length / 64;
                        if ((Data_length % 64) != 0) {
                            dCount += 1;
                        }

                        if(((i-2)%2) == 0)
                        {
                            //Set Arg
                                kernelDD.setArg(0, buffer_in_HBM[0]);
                                kernelDD.setArg(1, filConf_in_HBM[0]);
                                kernelDD.setArg(2, buffer_out_HBM[0]);          //data0
                                kernelDD.setArg(3, buffer_out_HBM[2]);          //data1
                                kernelDD.setArg(4, buffer_out_HBM[4]);          //data2
                                kernelDD.setArg(5, buffer_out_HBM[6]);          //data3
                                kernelDD.setArg(6, buffer_out_HBM[8]);          //idx
                                kernelDD.setArg(7, buffer_out_HBM[10]);         //data4(meta)
                                kernelDD.setArg(8, sizeof(dCount), &dCount);
                            //Kernel Call
                                // std::cout << "COMP_E" << std::endl;
                        }
                        else
                        {
                            //Set Arg
                                kernelDD.setArg(0, buffer_in_HBM[1]);
                                kernelDD.setArg(1, filConf_in_HBM[1]);
                                kernelDD.setArg(2, buffer_out_HBM[1]);          //data0
                                kernelDD.setArg(3, buffer_out_HBM[3]);          //data1
                                kernelDD.setArg(4, buffer_out_HBM[5]);          //data2
                                kernelDD.setArg(5, buffer_out_HBM[7]);          //data3
                                kernelDD.setArg(6, buffer_out_HBM[9]);          //idx
                                kernelDD.setArg(7, buffer_out_HBM[11]);         //data4(meta)
                                kernelDD.setArg(8, sizeof(dCount), &dCount);
                            //Kernel Call
                                // std::cout << "COMP_O" << std::endl;
                        }
                        CL_CHECK(cmd_.enqueueTask(kernelDD, nullptr, &Comp_events[i-2]));
                        // std::cout << "COMP" << ":" << i-2 << std::endl;
                    }

                    //FPGA_2_CPU
                    // if((i >= 3) && (i < (T_ITER+3)))
                    // {
                    //     if(((i-3)%2) == 0)
                    //     {
                    //         CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[0])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //                                         nullptr, &F2C_events[((i-3)*7)+0]));
                    //         CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[2])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //                                         nullptr, &F2C_events[((i-3)*7)+1]));
                    //         CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[4])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //                                         nullptr, &F2C_events[((i-3)*7)+2]));
                    //         CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[6])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //                                         nullptr, &F2C_events[((i-3)*7)+3]));
                    //         // CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[8])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //         //                                 nullptr, &F2C_events[((i-3)*7)+4]));
                    //         // CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[10])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //         //                                 nullptr, &F2C_events[((i-3)*7)+5]));
                    //         //read filtered Rows Val
                    //         CL_CHECK(cmd_.enqueueMigrateMemObjects({(filConf_in_HBM[0])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //                                         nullptr, &F2C_events[((i-3)*7)+6]));
                    //         // std::cout << "F2C_E" << std::endl;
                    //     }
                    //     else
                    //     {
                    //         CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[1])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //                                         nullptr, &F2C_events[((i-3)*7)+0]));
                    //         CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[3])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //                                         nullptr, &F2C_events[((i-3)*7)+1]));
                    //         CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[5])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //                                         nullptr, &F2C_events[((i-3)*7)+2]));
                    //         CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[7])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //                                         nullptr, &F2C_events[((i-3)*7)+3]));
                    //         // CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[9])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //         //                                 nullptr, &F2C_events[((i-3)*7)+4]));
                    //         // CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[11])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //         //                                 nullptr, &F2C_events[((i-3)*7)+5]));
                    //         //read filtered Rows Val
                    //         CL_CHECK(cmd_.enqueueMigrateMemObjects({(filConf_in_HBM[1])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                    //                                         nullptr, &F2C_events[((i-3)*7)+6]));
                    //         // std::cout << "F2C_O" << std::endl;
                    //     }

                    //     // std::cout << "F2C" << ":" << i-3 << std::endl;
                    // }

                    if((i >= 3) && (i < (T_ITER+3)))
                    {
                        uint32_t dSizeBytes = 0;
                        uint32_t tTrack = 0;
                        // offsetD += dSize_prev;
                        // offsetT += tTrackSize_prev;

                        if(((i-3)%2) == 0)
                        {
                            //read filtered Rows Val
                            CL_CHECK(cmd_.enqueueMigrateMemObjects({(filConf_in_HBM[0])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                            nullptr, &F2C_events[((i-3)*7)+6]));
                            CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+6])));

                            _512b* ptr = reinterpret_cast<ap_uint<512>*>(FilterConf_HBM[0]);
                            _512b tNum = ptr[2];
                            int filrows = tNum.range(31,0);
                            filterRowCount[i-3] = filrows;
                            uint32_t tRem = filrows%64;    //number not multiple of 64 numbers (16*4).
                            dSizeBytes = filrows/64;  //count of how many 512b(64bytes) to copy. 
                            if(tRem != 0)
                            {
                                dSizeBytes += 1;     //add one count.
                            }
                            dSizeBytes = dSizeBytes*64;       //each 512bit is 64bytes


                            CL_CHECK(cmd_.enqueueReadBuffer({(buffer_out_HBM[0])}, CL_FALSE , 0, dSizeBytes, (data_out_HBM[0]),
                                                            nullptr, &F2C_events[((i-3)*7)+0]));
                            CL_CHECK(cmd_.enqueueReadBuffer({(buffer_out_HBM[2])}, CL_FALSE , 0, dSizeBytes, (data_out_HBM[2]),
                                                            nullptr, &F2C_events[((i-3)*7)+1]));
                            CL_CHECK(cmd_.enqueueReadBuffer({(buffer_out_HBM[4])}, CL_FALSE , 0, dSizeBytes, (data_out_HBM[4]),
                                                            nullptr, &F2C_events[((i-3)*7)+2]));
                            CL_CHECK(cmd_.enqueueReadBuffer({(buffer_out_HBM[6])}, CL_FALSE , 0, dSizeBytes, (data_out_HBM[6]),
                                                            nullptr, &F2C_events[((i-3)*7)+3]));
                            // CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[8])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                            //                                 nullptr, &F2C_events[((i-3)*7)+4]));
                            // CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[10])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                            //                                 nullptr, &F2C_events[((i-3)*7)+5]));
                            //read filtered Rows Val
                            // CL_CHECK(cmd_.enqueueMigrateMemObjects({(filConf_in_HBM[0])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                            //                                 nullptr, &F2C_events[((i-3)*7)+6]));
                            // std::cout << "F2C_E" << std::endl;
                        }
                        else
                        {
                            //read filtered Rows Val
                            CL_CHECK(cmd_.enqueueMigrateMemObjects({(filConf_in_HBM[1])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                            nullptr, &F2C_events[((i-3)*7)+6]));
                            CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+6])));


                            _512b* ptr = reinterpret_cast<ap_uint<512>*>(FilterConf_HBM[1]);
                            _512b tNum = ptr[2];
                            int filrows = tNum.range(31,0);
                            filterRowCount[i-3] = filrows;
                            uint32_t tRem = filrows%64;    //number not multiple of 64 numbers (16*4).
                            dSizeBytes = filrows/64;  //count of how many 512b(64bytes) to copy. 
                            if(tRem != 0)
                            {
                                dSizeBytes += 1;     //add one count.
                            }
                            dSizeBytes = dSizeBytes*64;       //each 512bit is 64bytes


                            CL_CHECK(cmd_.enqueueReadBuffer({(buffer_out_HBM[1])}, CL_FALSE , 0, dSizeBytes, (data_out_HBM[1]),
                                                            nullptr, &F2C_events[((i-3)*7)+0]));
                            CL_CHECK(cmd_.enqueueReadBuffer({(buffer_out_HBM[3])}, CL_FALSE , 0, dSizeBytes, (data_out_HBM[3]),
                                                            nullptr, &F2C_events[((i-3)*7)+1]));
                            CL_CHECK(cmd_.enqueueReadBuffer({(buffer_out_HBM[5])}, CL_FALSE , 0, dSizeBytes, (data_out_HBM[5]),
                                                            nullptr, &F2C_events[((i-3)*7)+2]));
                            CL_CHECK(cmd_.enqueueReadBuffer({(buffer_out_HBM[7])}, CL_FALSE , 0, dSizeBytes, (data_out_HBM[7]),
                                                            nullptr, &F2C_events[((i-3)*7)+3]));
                            // CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[9])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                            //                                 nullptr, &F2C_events[((i-3)*7)+4]));
                            // CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[11])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                            //                                 nullptr, &F2C_events[((i-3)*7)+5]));
                            
                            
                            // std::cout << "F2C_O" << std::endl;
                        }

                        // dSize_prev = dSizeBytes;
                        // tTrackSize_prev = tTrack;

                        // std::cout << "F2C" << ":" << i-3 << std::endl;
                    }

                    //Data Copy Calls
                    if((i >= 4) && (i < (T_ITER+4)))
                    {
                        uint32_t dSizeBytes = 0;
                        uint32_t tTrack = 0;
                        offsetD += dSize_prev;
                        offsetT += tTrackSize_prev;
                        int filrows = filterRowCount[i-4];
                        uint32_t tRem = filrows%64;    //number not multiple of 64 numbers (16*4).
                        dSizeBytes = filrows/64;  //count of how many 512b(64bytes) to copy. 
                        if(tRem != 0)
                        {
                            dSizeBytes += 1;     //add one count.
                        }
                        dSizeBytes = dSizeBytes*64;       //each 512bit is 64bytes
                        // tTrack = dSizeBytes*1.2;
                        // tRem = tTrack%16;  //128bit is 16bytes
                        // if(tRem != 0)
                        // {
                        //     tTrack += (16 - tRem);
                        // }
                        if (((i - 4) % 2) == 0) {                            
                            // Launch threads with offset handling
                            t1 = std::thread(copy_data, data_out_HBM[0], dataOut[0], dSizeBytes, offsetD);
                            t2 = std::thread(copy_data, data_out_HBM[2], dataOut[1], dSizeBytes, offsetD);
                            t3 = std::thread(copy_data, data_out_HBM[4], dataOut[2], dSizeBytes, offsetD);
                            t4 = std::thread(copy_data, data_out_HBM[6], dataOut[3], dSizeBytes, offsetD);
                            // t5 = std::thread(copy_data, data_out_HBM[10], trackOut, tTrack, offsetT);
                        } else {
                            // Launch threads with offset handling
                            t1 = std::thread(copy_data, data_out_HBM[1], dataOut[0], dSizeBytes, offsetD);
                            t2 = std::thread(copy_data, data_out_HBM[3], dataOut[1], dSizeBytes, offsetD);
                            t3 = std::thread(copy_data, data_out_HBM[5], dataOut[2], dSizeBytes, offsetD);
                            t4 = std::thread(copy_data, data_out_HBM[7], dataOut[3], dSizeBytes, offsetD);
                            // t5 = std::thread(copy_data, data_out_HBM[11], trackOut, tTrack, offsetT);
                        }

                        dSize_prev = dSizeBytes;
                        tTrackSize_prev = tTrack;

                        // std::cout << "D_COPY" << ":" << i-4 << std::endl;
                    }


                    asyncTimeE = std::chrono::steady_clock::now();
                    // asyncTime = std::chrono::duration_cast<std::chrono::microseconds>(asyncTimeE - asyncTimeS);
                    // time_async[i] = static_cast<double>(asyncTime.count());

                    ///WAITS///
                    
                    //IO READ
                    readTimeS = std::chrono::steady_clock::now();
                    if(i < T_ITER)
                    {
                        int ret = 0;
                        if(i%2 == 0)
                        {
                            while( aio_error(&aio_rf) == EINPROGRESS ) {;}
                            ret = aio_return (&aio_rf);
                            if(ret <= 0)
                            {
                                std::cerr << "Read Error. Bytes Read: " << ret << std::endl;
                            }
                            // readTimeE = std::chrono::steady_clock::now();
                            // wrTimeS = std::chrono::steady_clock::now();
                            // memset(data_in_HBM[0], 0, max_input_size);
                            // writeZeros(data_in_HBM[0], Data_lengths[i], PIPELINE_DEPTH);
                        }
                        else
                        {
                            while( aio_error(&aio_rf1) == EINPROGRESS ) {;}
                            ret = aio_return (&aio_rf1);
                            if(ret <= 0)
                            {
                                std::cerr << "Read Error. Bytes Read: " << ret << std::endl;
                            }
                            // readTimeE = std::chrono::steady_clock::now();
                            // wrTimeS = std::chrono::steady_clock::now();
                            // memset(data_in_HBM[1], 0, max_input_size);
                            // writeZeros(data_in_HBM[1], Data_lengths[i], PIPELINE_DEPTH);
                        }
                        // std::cout << "Read Bytes: " << ret << std::endl;
                    }   
                    readTimeE = std::chrono::steady_clock::now();
                    // wrTimeE = std::chrono::steady_clock::now();
                    // wrTime = std::chrono::duration_cast<std::chrono::microseconds>(wrTimeE - wrTimeS);
                    // readTime = std::chrono::duration_cast<std::chrono::microseconds>(readTimeE - readTimeS);
                    // time_read[i] = static_cast<double>(readTime.count());
                    // time_wr[i] = static_cast<double>(wrTime.count());

                    // FPGATimeS = std::chrono::steady_clock::now();
                    C2FTimeS = std::chrono::steady_clock::now();
                    //CPU_2_FPGA
                    if((i >= 1) && (i < (T_ITER+1)))
                    {
                        CL_CHECK(clWaitForEvents(one,(cl_event*)(&C2F_events[i-1])));
                        if(i < 3)
                        {
                            CL_CHECK(clWaitForEvents(one,(cl_event*)(&FilC_events[i-1])));
                        }
                        // std::cout << "C2F Wait" << ":" << i-1 << std::endl;
                    }                    
                    C2FTimeE = std::chrono::steady_clock::now();
                    // C2FTime = std::chrono::duration_cast<std::chrono::microseconds>(C2FTimeE - C2FTimeS);
                    // time_C2F[i] = static_cast<double>(C2FTime.count());
                    
                    COMPTimeS = std::chrono::steady_clock::now();
                    //KERNEL CALL
                    if((i >= 2) && (i < (T_ITER+2)))
                    {
                        CL_CHECK(clWaitForEvents(one,(cl_event*)(&Comp_events[i-2])));
                        // std::cout << "COMP Wait" <<":" << i-2 << std::endl;
                    }
                    COMPTimeE = std::chrono::steady_clock::now();
                    // COMPTime = std::chrono::duration_cast<std::chrono::microseconds>(COMPTimeE - COMPTimeS);
                    // time_COMP[i] = static_cast<double>(COMPTime.count());

                    F2CTimeS = std::chrono::steady_clock::now();
                    //FPGA_2_CPU
                    if((i >= 3) && (i < (T_ITER+3)))
                    {
                        if(((i-3)%2) == 0)
                        {
                            CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+0])));
                            CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+1])));
                            CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+2])));
                            CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+3])));
                            // CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+4])));        //data idx
                            // CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+5])));        //meta
                        }
                        else
                        {
                            CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+0])));
                            CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+1])));
                            CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+2])));
                            CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+3])));
                            // CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+4])));        //data idx
                            // CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[((i-3)*7)+5])));        //meta
                        }
                        // std::cout << "F2C Wait" << ":" << i-3 << std::endl;
                    }
                    // FPGATimeE = std::chrono::steady_clock::now();
                    // FPGATime = std::chrono::duration_cast<std::chrono::microseconds>(FPGATimeE - FPGATimeS);
                    // time_fpga[i] = static_cast<double>(FPGATime.count());
                    F2CTimeE = std::chrono::steady_clock::now();
                    // F2CTime = std::chrono::duration_cast<std::chrono::microseconds>(F2CTimeE - F2CTimeS);
                    // time_F2C[i] = static_cast<double>(F2CTime.count());


                    //dataCopy
                    dCopyTimeS = std::chrono::steady_clock::now();
                    if((i >= 4) && (i < (T_ITER+4)))
                    {
                        // Wait for all threads to finish
                        t1.join();
                        t2.join();
                        t3.join();
                        t4.join();
                        // t5.join();
                        // std::cout << "D_COPY wait" << ":" << i-4 << std::endl;
                    }
                    dCopyTimeE = std::chrono::steady_clock::now();
                    // dCopyTime = std::chrono::duration_cast<std::chrono::microseconds>(dCopyTimeE - dCopyTimeS);
                    // time_dCopy[i] = static_cast<double>(dCopyTime.count());
                }
            }
            else
            {
                std::cout << "***Sequential Implementation***" << std::endl;
                dfstart = std::chrono::steady_clock::now();
                for (int i = 0; i < T_ITER; i++)
                {
                    std::cout << "ITER COUNT: " << i << std::endl;
                    //IO READ
                    memset(data_in_HBM[0], 0, max_input_size);
                    async_readnorm(&aio_rf, (void *)(data_in_HBM[0]), nvmeFd, Data_length, Data_offset);  

                    std::cout << "Data_length: " << Data_length << std::endl;
                    std::cout << "Data_offset: " << Data_offset << std::endl;

                    while( aio_error(&aio_rf) == EINPROGRESS ) {;}
                    int ret = aio_return (&aio_rf);
                    if(ret <= 0)
                    {
                        std::cerr << "Read Error. Bytes Read: " << ret << std::endl;
                    }
                    else
                    {
                        std::cout << "Bytes Read: " << ret << std::endl;
                    }

                    //CPU_2_FPGA
                    int Ssize = Data_length;
                    CL_CHECK(cmd_.enqueueWriteBuffer(buffer_in_HBM[0], CL_FALSE, 0, Ssize, data_in_HBM[0], nullptr, &C2F_events[i]));
                    CL_CHECK(cmd_.enqueueWriteBuffer(filConf_in_HBM[0], CL_FALSE, 0, 192, FilterConf_HBM[0], nullptr, &FilC_events[0]));
                    CL_CHECK(clWaitForEvents(one,(cl_event*)(&C2F_events[i])));
                    CL_CHECK(clWaitForEvents(one,(cl_event*)(&FilC_events[0])));
                    //KERNEL CALL
                    uint32_t dCount = Data_length / 64;
                    if ((Data_length % 64) != 0) {
                        dCount += 1;
                    }
                    kernelDD.setArg(0, buffer_in_HBM[0]);
                    kernelDD.setArg(1, filConf_in_HBM[0]);
                    kernelDD.setArg(2, buffer_out_HBM[0]);          //data0
                    kernelDD.setArg(3, buffer_out_HBM[2]);          //data1
                    kernelDD.setArg(4, buffer_out_HBM[4]);          //data2
                    kernelDD.setArg(5, buffer_out_HBM[6]);          //data3
                    kernelDD.setArg(6, buffer_out_HBM[8]);          //idx
                    kernelDD.setArg(7, buffer_out_HBM[10]);         //data4(meta)
                    kernelDD.setArg(8, sizeof(dCount), &dCount);
                    CL_CHECK(cmd_.enqueueTask(kernelDD, nullptr, &Comp_events[i]));
                    CL_CHECK(clWaitForEvents(one,(cl_event*)(&Comp_events[i])));


                    //FPGA_2_CPU
                    CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[0])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                    nullptr, &F2C_events[i]));
                    CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[2])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                    nullptr, &F2C_events[i+1]));
                    CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[4])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                    nullptr, &F2C_events[i+2]));
                    CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[6])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                    nullptr, &F2C_events[i+3]));
                    CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[8])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                    nullptr, &F2C_events[i+4]));
                    CL_CHECK(cmd_.enqueueMigrateMemObjects({(buffer_out_HBM[10])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                    nullptr, &F2C_events[i+5]));

                    CL_CHECK(cmd_.enqueueMigrateMemObjects({(filConf_in_HBM[0])}, CL_MIGRATE_MEM_OBJECT_HOST , 
                                                    nullptr, &F2C_events[i+6]));

                    CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[i])));
                    CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[i+1])));
                    CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[i+2])));
                    CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[i+3])));
                    CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[i+4])));
                    CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[i+5])));
                    CL_CHECK(clWaitForEvents(one,(cl_event*)(&F2C_events[i+6])));

                    _512b* ptr = reinterpret_cast<ap_uint<512>*>(FilterConf_HBM[0]);
                    _512b tNum = ptr[2];
                    int filrows = tNum.range(31,0);
                    filterRowCount[i] = filrows;
                    std::cout << "Total Numbers after Filteration: " << filterRowCount[i] << std::endl;

                    //data Copy
                    uint32_t dSize = filterRowCount[i];
                    uint32_t tRem = dSize%64;    //number not multiple of 64 numbers (16*4).
                    dSize = dSize/64;  //count of how many 512b(64bytes) to copy. 
                    if(tRem != 0)
                    {
                        dSize += 1;     //add one count.
                    }
                    dSize = dSize*64;       //each 512bit is 64bytes
                    uint32_t tTrackSize = dSize *1.2;
                    tRem = tTrackSize%16;  //128bit is 16bytes
                    if(tRem != 0)
                    {
                        tTrackSize += (16 - tRem);
                    }
                    std::cout << "dSize: " << dSize << std::endl;
                    std::cout << "tTrackSize: " << tTrackSize << std::endl;
                    
                    offsetD += dSize_prev;
                    offsetT += tTrackSize_prev;

                    std::cout << "offsetD: " << offsetD << std::endl;
                    std::cout << "offsetT: " << offsetT << std::endl;

                    t1 = std::thread(copy_data, data_out_HBM[0], dataOut[0], dSize, offsetD);
                    t2 = std::thread(copy_data, data_out_HBM[2], dataOut[1], dSize, offsetD);
                    t3 = std::thread(copy_data, data_out_HBM[4], dataOut[2], dSize, offsetD);
                    t4 = std::thread(copy_data, data_out_HBM[6], dataOut[3], dSize, offsetD);
                    t5 = std::thread(copy_data, data_out_HBM[8], trackOut, tTrackSize, offsetT);

                    t1.join();
                    t2.join();
                    t3.join();
                    t4.join();
                    t5.join();

                    dSize_prev = dSize;
                    tTrackSize_prev = tTrackSize;

                    // print_data(data_out_HBM[0], data_out_HBM[2], data_out_HBM[4], data_out_HBM[6], filterRowCount[i], 0);
                    // print_data(dataOut[0], dataOut[1], dataOut[2], dataOut[3], filterRowCount[i], offsetD);
                    // if(i > 1)
                    // {
                    //     print_data(dataOut[0], dataOut[1], dataOut[2], dataOut[3], filterRowCount[i], 0);
                    // }
                    

                    
                }
            }
            auto dfend = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(dfend - dfstart);
            double total_time = static_cast<double>(duration.count());
            std::cout << "Schedule Completed." << std::endl;

            (void)close(nvmeFd);

            if(FLAGS_VERIF)
            {
                std::cout << "Starting Data verif" << std::endl;
                int kernel_dout = 0;
                ap_uint<AXI_WIDTH> buf_out = 0;
                std::vector<int32_t> combinedData;

                int32_t* mData[4];

                mData[0] = reinterpret_cast<int32_t*>(dataOut[0]);
                mData[1] = reinterpret_cast<int32_t*>(dataOut[1]);
                mData[2] = reinterpret_cast<int32_t*>(dataOut[2]);
                mData[3] = reinterpret_cast<int32_t*>(dataOut[3]);

                // update_patch_data(reinterpret_cast<int32_t*>(dataOut[0]), 
                //             reinterpret_cast<int32_t*>(dataOut[1]), 
                //             reinterpret_cast<int32_t*>(dataOut[2]), 
                //             reinterpret_cast<int32_t*>(dataOut[3]),
                //             reinterpret_cast<int32_t*>(trackOut)
                //             );
                
                //combine data 
                for(int j = 0; j < T_ITER; j++)
                {
                    uint32_t remDiv = filterRowCount[j]%64;
                    uint64_t dOut_Size_A = filterRowCount[j]/64;   //I THINK ITS SOLVED FOR SR NOW.bcz of short repeat. else its nrows/16. One 512 can worst case contain 3 numbers.
                    if(remDiv!=0)
                    {
                        dOut_Size_A += 1;
                    }
                    // dOut_Size_A = dOut_Size_A*4;
                    std::cout << "dOut_Size_A:  " << dOut_Size_A << std::endl;

                    for(int i = 0; i < (dOut_Size_A); i ++)
                    {
                        for(int k = 0; k < 4; k++)
                        {
                            for(int z = 0; z < 16; z++)
                            {
                                kernel_dout = mData[k][(i*16)+z];
                                // std::cout << "kernel_dout: " << kernel_dout << std::endl;
                                combinedData.push_back(kernel_dout);
                            }
                        }   
                    }
                }
                // Sort the combined data in descending order
                // std::sort(combinedData.begin(), combinedData.end());    //for ascending order
                // std::sort(combinedData.begin(), combinedData.end(), std::greater<int32_t>());
                
                //Print out the data
                // for (const auto& element : combinedData) {
                //     std::cout << element << std::endl;
                // }
                std::cout << "Join Done" << std::endl;
                verif_sorted(combinedData, filterRowCount);
                // processFlags(Data_Index);
            }


            ////PROFILING RESULTS////
            std::cout << std::dec << "----CPU TIMER BASED CALCULATIONS----" << std::endl;
            std::cout << "TOTAL ITERATION COUNT: " << stripeCount*DATA_MUL << std::endl;
            std::cout << "END 2 END exec Total Time (ns): " << total_time << std::endl;
            double total_read = 0.0;
            
            #ifdef PRINT_DEBUG
                if(dataflow && (NITERS < 20))
                {
                    for (int s = 0; s < NITERS; s++)
                    {
                        total_read += time_async[s];
                        total_read += time_read[s];
                        total_read += time_C2F[s];
                        total_read += time_COMP[s];
                        total_read += time_F2C[s];
                        // total_read += time_fpga[s];
                        total_read += time_dCopy[s];
                        
                        std::cout << std::dec << "Total Async Call[" <<s<< "] Time (us): " << time_async[s] << std::endl;
                        // if(s < stripeCount)
                        // {
                        std::cout << std::dec << "Total Read[" <<s<< "] Time (us): " << time_read[s] << std::endl;
                            // std::cout << std::dec << "Total Write0s[" <<s<< "] Time (us): " << time_wr[s] << std::endl;
                        // }
                        // std::cout << std::dec << "Total FPGA[" <<s<< "] Time (us): " << time_fpga[s] << std::endl;
                        std::cout << std::dec << "Total C2F[" <<s<< "] Time (us): " << time_C2F[s] << std::endl;
                        std::cout << std::dec << "Total COMP[" <<s<< "] Time (us): " << time_COMP[s] << std::endl;
                        std::cout << std::dec << "Total F2C[" <<s<< "] Time (us): " << time_F2C[s] << std::endl;
                        std::cout << std::dec << "Total dCopy[" <<s<< "] Time (us): " << time_dCopy[s] << std::endl;
                    }
                    std::cout << std::dec << "Total Read Sum(us): " << total_read << std::endl;
                }
            #endif

            double totalOutputSize = 0;
            for(int i = 0; i < T_ITER; i++)
            {
                totalOutputSize += filterRowCount[i];
            }
            totalOutputSize = totalOutputSize * 4; //BYTES

            std::cout << std::dec << "Compressed Data File Size(MB): " << ((double)(Data_length*DATA_MUL)/(double)(1000.0*1000.0)) << std::endl;
            std::cout << std::dec << "Total Output Size(MB): " << (totalOutputSize/1000.0*1000.0) << std::endl;

            #ifdef PRINT_DEBUG
                std::cout << std::dec << "BANDWIDTH INPUT(trtime) (MB/s): " << (((double)(Data_length*DATA_MUL))/total_read) << std::endl;
                std::cout << std::dec << "BANDWIDTH OUTPUT(trtime) (MB/s): " << (totalOutputSize/total_read)<< std::endl;
                std::cout << std::dec << "BANDWIDTH INPUT (MB/s): " << (((double)(Data_length*DATA_MUL))/total_time)*1000.0 << std::endl;
                std::cout << std::dec << "BANDWIDTH OUTPUT (MB/s): " << (totalOutputSize/total_time)*1000.0 << std::endl;
            #else
                std::cout << std::dec << "BANDWIDTH INPUT (MB/s): " << (((double)(Data_length*DATA_MUL))/total_time)*1000.0 << std::endl;
                std::cout << std::dec << "BANDWIDTH OUTPUT (MB/s): " << (totalOutputSize/total_time)*1000.0 << std::endl;
            #endif
            
        
        
        // After using the buffers, free the allocated memory
        for(int i = 0; i < BUFFERS_IN; i++) {
            free(data_in_HBM[i]);
        }

        for(int i = 0; i < BUFFERS_IN; i++) {
            free(FilterConf_HBM[i]);
        }


        for (int i = 0; i < BUFFERS_OUT; ++i) {
            free(data_out_HBM[i]);
        }

        for (int i = 0; i < 4; ++i) {
            free(dataOut[i]);
        }
        free(trackOut);

        //////////////////////////
    }
    else
    {
        //run csim, sw_emu
        std::cout << "***CSIM/SW_EMU MODE RUN Single Stripe Single Column for Testing***" << std::endl;
        // wait_count = WAIT_MAX;
    }  
    return 0;
}

void verif_sorted(std::vector<int32_t> combinedData, uint32_t* FilRows)
{
    std::ifstream in_file(FLAGS_orig);

    if (!in_file) {
        throw std::runtime_error("Error: failed to open input file.");
    }

    int number;
    int rowIndex = 0;
    int numMatched = 0;
    int T_NUM = 0;
    bool oneT = true;

    for(int i = 0; i < DATA_MUL; i++)
    {

        while (in_file >> number) {
            // std::cout << "number: " << number << std::endl;
            // if (rowIndex >= FilRows[i]) {
            //     // throw std::runtime_error("Error: File contains more numbers than expected rows.");
            //     std::cout << "Error: File contains more numbers than expected rows." << std::endl;
            // }

            if (number != combinedData[rowIndex]) {
                throw std::runtime_error("Error: Data mismatch at index " + std::to_string(rowIndex + 1) +
                                        ". FPGA: " + std::to_string(combinedData[rowIndex]) +
                                        ", Actual: " + std::to_string(number));
            }
            ++numMatched;
            ++rowIndex;
        }
        in_file.clear();  // Clear EOF or any other error flags
        in_file.seekg(0, std::ios::beg);  // Move the pointer back to the beginning of the file
        //row idx to waste if filter is not multiple of 64.
        T_NUM += FilRows[i];
        // std::cout << "FilRows[i]: " << FilRows[i] << std::endl;
        // std::cout << "Before rowIndex: " << rowIndex << std::endl;
        uint32_t RCount = FilRows[i]/64;
        uint32_t remRows = (FilRows[i]%64);
        if(remRows!=0)
        {
            RCount+=1;
        }
        RCount = RCount*64;
        RCount = RCount*(i+1);
        rowIndex+=(RCount-rowIndex);

        
        // std::cout << "After rowIndex: " << rowIndex << std::endl;
        
        if(FilRows[i] != (numMatched/(i+1)))
        {
            if(oneT)
            {
                oneT = false;
                T_NUM -= FilRows[i]-numMatched;
            }
            else
            {
                T_NUM -= FilRows[i]-(numMatched/(i+1));
            }
        }

    }
    std::cout << "Number of Rows Matched: " << numMatched << std::endl;
    if(numMatched == T_NUM)
    {
        std::cout << "PASSED" << std::endl;
    }
    else
    {
        std::cout << "FAILED" << std::endl;
    }

}


void update_patch_data(int32_t *datain0, int32_t *datain1, int32_t *datain2, int32_t *datain3, int32_t *track)
{
    _128b *mTr;
    mTr = (_128b*)(track);

    uint16_t decType = 0;
    uint16_t runLength = 0;
    int32_t BV = 0;
    uint8_t PLL = 0;
    uint16_t gap_val = 0;
    int32_t patch_val = 0;

    int32_t totalCount = nrows;  // Assuming nrows is defined somewhere globally or passed in

    bool latchRL = 1;
    bool latchPA = 1;
    uint16_t TRL = 0;
    uint32_t data_count = 0;
    uint32_t crow = 0;
    uint32_t patch_Didx = 0;
    uint16_t offset_gap = 0;
    uint16_t gap_mod = 0;
    uint16_t prev_gap = 0;
    uint16_t dataPtr = 0;
    uint16_t subVal = 0;
    uint32_t fdiv = 0;
    uint16_t offset = 0;
    uint32_t pll_count = 0;
    uint32_t Data_idx = 0;

    while(crow < totalCount)
    {
        
        PLL = mTr->range(7, 0);      
        gap_val = mTr->range(23, 8);   
        patch_val = mTr->range(55, 24);  
        decType = mTr->range(71, 64);
        runLength = mTr->range(95, 80); 
        BV = mTr->range(127, 96);      

        mTr++;

        if(latchRL)
        {
            latchRL = 0;
            TRL = runLength;
        }

        if(runLength != 0)
        {

            if(decType != PATCHED)
            {
                data_count += 16;
                latchPA = 1;

                if(TRL <= 64)
                {
                    latchRL = 1;
                    crow += TRL;
                    TRL = 0;
                }
                else
                {
                    TRL -= 64;
                    crow += 64;
                }
            }
            else
            {
                if(latchPA)
                {
                    latchPA = 0;
                    patch_Didx = data_count;
                }

                if(PLL != 0)  
                {
                    pll_count += 1;

                    if(pll_count == PLL)
                    {
                        // printf("TRL when PLL done = %d\n", TRL);
                        crow += TRL;  
                        pll_count = 0;
                        latchRL = 1;
                        latchPA = 1;
                    }

                    if (gap_val == 255 && patch_val == 0)
                    {
                        offset_gap = gap_val;
                    }
                    else
                    {
                        gap_val += prev_gap;
                        gap_val += offset_gap;
                        offset_gap = 0;
                        prev_gap = gap_val;

                        gap_mod = gap_val % 64;
                        dataPtr = gap_mod / 16;

                        fdiv = gap_val / 64;
                        subVal = (fdiv * 64) + (dataPtr * 16);
                        offset = gap_val - subVal;

                        Data_idx = (fdiv * 16) + patch_Didx + offset;

                        switch (dataPtr)
                        {
                            case 0:
                                if (Data_idx < 0) printf("Error: Data_idx < 0\n");
                                datain0[Data_idx] += patch_val;
                                break;
                            case 1:
                                if (Data_idx < 0) printf("Error: Data_idx < 0\n");
                                datain1[Data_idx] += patch_val;
                                break;
                            case 2:
                                if (Data_idx < 0) printf("Error: Data_idx < 0\n");
                                datain2[Data_idx] += patch_val;
                                break;
                            case 3:
                                if (Data_idx < 0) printf("Error: Data_idx < 0\n");
                                datain3[Data_idx] += patch_val;
                                break;
                            default:
                                printf("Error: Invalid dataPtr = %d\n", dataPtr);
                                break;
                        }
                    }

                }
                else
                {
                    data_count += 16;
                    prev_gap = 0;

                    if(TRL <= 64)
                    {
                        TRL = TRL;
                    }
                    else
                    {
                        TRL -= 64;
                        crow += 64;
                    }
                }
            }
        }
        else
        {
            printf("Error: runLength is 0, exiting program!\n");
            exit(EXIT_FAILURE);  // Immediately exit the program with failure status
        }
    }
}


// void processFlags(const std::vector<_512b, tapa::aligned_allocator<_512b>>& data) {
//     // Open the file containing 0s and 1s
//     std::ifstream flagFile(filFLAG);

//     if (!flagFile.is_open()) {
//         std::cerr << "Error opening flags file." << std::endl;
//         return;
//     }

//     // Initialize counters
//     uint16_t bitCounter = 0;
//     uint32_t vectorIndex = 0;
//     bool NM_Flag = 0;
//     bool myData = 0;

//     uint32_t idx = 1;

//     // Read flags from the file and compare with the data
//     bool flag = 0;
//     while (flagFile >> flag) {
//         // Assuming flag is 0 or 1 in the file

//         // Extract the relevant 64 bits from the _512b vector
//         _512b dataChunk = data[vectorIndex];
//         myData = dataChunk.range(bitCounter,bitCounter);

//         // Compare the flag with the corresponding bit in the data
//         if (myData == flag) {
//             // std::cout << "Flag at position idx " << idx << " bitcounter " << bitCounter << " matches." << std::endl;
//         } else {
//             NM_Flag = 1;
//             // std::cout << "Flag at position idx " << idx << " bitcounter " << bitCounter << " does not match." << std::endl;
//         }

//         // Move to the next bit
//         bitCounter = (bitCounter + 1) % 64;

//         // Move to the next vector if necessary
//         if (bitCounter == 0) {
//             ++vectorIndex;
//         }

//         ++idx;
//     }

//     flagFile.close();
//     if(NM_Flag == 0)
//     {
//         std::cout << "PASSED, All flags matched: " << (idx-1) << std::endl;
//     }
//     else
//     {
//         std::cout << "FAILED, Flags have mismatch" << std::endl;
//     }
    
// }
