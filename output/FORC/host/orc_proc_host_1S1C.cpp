#include "orc_proc_host_1S1C.h"

void orc_proc(tapa::mmap<_512b> input_port, 
                    tapa::mmap<_512b> FilterConf_port,
                    tapa::mmap<_512b> output_port0_32b_8b, 
                    tapa::mmap<_512b> output_port1_16b_8b,
                    tapa::mmap<_512b> output_port2_16b_8b,
                    tapa::mmap<_512b> output_port3_8b,
                    tapa::mmap<_512b> data_Idx,
                    tapa::mmap<_512b> output_port4_Track,
                    // uint32_t wait_count, 
                    uint32_t data_count
                    // uint32_t KRNL_Data_Write
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

void processFlags(const std::vector<_512b, tapa::aligned_allocator<_512b>>& data);

void verif_sorted(std::vector<int32_t> combinedData, int FilRows);

void update_patch_data(int32_t *datain0, int32_t *datain1, int32_t *datain2, int32_t *datain3, int32_t *track);


int main(int argc, char* argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);

    // std::string read_file = "/localhdd/awa159/orc_decoder/DIRECT_DECODER/32_bit_data_orc.bin";
    // std::string check_file = "/localhdd/awa159/orc_decoder/DIRECT_DECODER/32_bit_data.bin";
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
        uint64_t stripeCount = reader->getNumberOfStripes();
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

        comp_file.seekg(0, std::ios::end);
        Data_length = static_cast<uint64_t>(comp_file.tellg());  // Cast tellg() to uint64_t
        comp_file.seekg(0, std::ios::beg);
        
        Data_offset = 0;        
    }
    std::cout << "Data_offset: " << Data_offset << std::endl;
    std::cout << "Data_length: " << Data_length << std::endl;
    std::cout << "Number of rows are: " << nrows << std::endl;    

    uint32_t dCount = Data_length / 64;
    if ((Data_length % 64) != 0) {
        dCount += 1;
    }

    ///////DECLARE READ WRITE HOST PTRs////////
        uint32_t remDiv = nrows%RSIZE_DIV;
        uint64_t dOut_Size_A = nrows/RSIZE_DIV;   //I THINK ITS SOLVED FOR SR NOW.bcz of short repeat. else its nrows/16. One 512 can worst case contain 3 numbers.
        if(remDiv!=0)
        {
            dOut_Size_A = (dOut_Size_A + 1);
        }
        // dOut_Size_A = dOut_Size_A*4;
        std::cout << "dOut_Size_A:  " << dOut_Size_A << std::endl;

        std::vector<_512b, tapa::aligned_allocator<_512b>> data_in(dCount);
        std::vector<_512b, tapa::aligned_allocator<_512b>> data_out(dOut_Size_A);   //contain SR as well. by default it should be dOut_Size_A/4
        std::vector<_512b, tapa::aligned_allocator<_512b>> data_out1(dOut_Size_A);   
        std::vector<_512b, tapa::aligned_allocator<_512b>> data_out2(dOut_Size_A);
        std::vector<_512b, tapa::aligned_allocator<_512b>> data_out3(dOut_Size_A);
        std::vector<_512b, tapa::aligned_allocator<_512b>> idx_data(dOut_Size_A);
        std::vector<_512b, tapa::aligned_allocator<_512b>> track_data(dOut_Size_A*1.2);   //Divide by 4 bcz all info is there *2 bcz it has PLL info
        std::vector<_512b, tapa::aligned_allocator<_512b>> filterConf(3);

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

        filterConf[0] = filconf;

        // if(idx_flag == 1)
        // {
            uint32_t MyremDiv = nrows%64;
            uint32_t readCnt = nrows/64;
            if(MyremDiv!=0)
            {
                readCnt += 1;
            }
            filterConf[1] = readCnt;
            std::cout << "readCnt:  " << readCnt << std::endl;
        // }
        

        // uint64_t dOut_Size = (nrows * 4);   //for 32 bit data with 4 bytes
        // uint32_t dOut_Size_H = dOut_Size/2;
        // uint32_t outDataSize = (dOut_Size_A*16);      //*64 bcz of 512bits divide 4 bcz its divided 64/4=16
        // uint32_t outTrackSize = dOut_Size_A*16;   //*16 bcz of 128bits
        // std::cout << "outDataSize:  " << outDataSize/1024.0/1024.0 << std::endl;
        // std::cout << "outTrackSize:  " << outTrackSize/1024.0/1024.0 << std::endl;
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
    
    int64_t kernel_time_ns = 0;


    // LOG(FATAL) << "NO HARDWARE PATH PROVIDED";
    // return -1;
    ////
        // struct aiocb aio_rf;
        // async_readnorm(&aio_rf, (void *)(data_in.data()), nvmeFd, ORIG_file_size_bytes, Data_offset); 
        // while( aio_error(&aio_rf) == EINPROGRESS ) {;}
        // int aio_result = aio_error(&aio_rf);
        // if (aio_result != 0) {
        //     perror("aio_error");
        //     std::cerr << "Asynchronous I/O error: " << strerror(aio_result) << std::endl;
        //     return 1;
        // }
        // int bytes_read = aio_return(&aio_rf);

        off_t offset = Data_offset;
        if (lseek(nvmeFd, offset, SEEK_SET) == -1) {
            perror("Error setting file offset");
            close(nvmeFd);
            return 1;
        }
        ssize_t bytes_read = read(nvmeFd, (void *)(data_in.data()), Data_length);

        std::cout << "Bytes Read:" << bytes_read << std::endl;
        int NRCOUNT = 1;
        // KRNL_file_size_bytes = KRNL_file_size_bytes/64;
        for (int k = 0; k < NRCOUNT; k++)
        {
            // read_write_mmap
            kernel_time_ns = tapa::invoke(
                orc_proc, FLAGS_bitstream, 
                tapa::read_only_mmap<_512b>(data_in),
                tapa::read_write_mmap<_512b> (filterConf),
                tapa::write_only_mmap<_512b>(data_out),
                tapa::write_only_mmap<_512b>(data_out1),
                tapa::write_only_mmap<_512b>(data_out2),
                tapa::write_only_mmap<_512b>(data_out3),
                tapa::write_only_mmap<_512b>(idx_data),
                tapa::write_only_mmap<_512b>(track_data),
                // wait_count,
                dCount
                // KRNL_Data_Write
            );
            // std::cout << "Kernel Exec time(ms): " << (kernel_time_ns) * 1e-6 << std::endl;
        }

        // filterConf[2] = filterConf[2];   //no need for this now rows are written for both cases

        // orc::ReaderOptions readerOpts;
        // std::unique_ptr<orc::Reader> reader =
        //     orc::createReader(orc::readFile(c2FileData, readerOpts.getReaderMetrics()), readerOpts);

        // nvme_file = c2FileData;
        // std::cout << "{ \"name\": \"" << c2FileData << "\",\n";
        // uint64_t numberColumns = reader->getType().getMaximumColumnId() + 1;

        // std::cout << "\n  \"file length\": " << reader->getFileLength() << ",\n";

        // std::cout << "  \"rows\": " << reader->getNumberOfRows() << ",\n";
        // uint64_t stripeCount = reader->getNumberOfStripes();
        // std::cout << "  \"stripe count\": " << stripeCount << ",\n";


        // for (uint64_t str = 0; str < reader->getStripe(0)->getNumberOfStreams(); ++str) {
        // if (str != 0) {
        //     std::cout << ",\n";
        // }
        // std::unique_ptr<orc::StreamInformation> stream = reader->getStripe(0)->getStreamInformation(str);
        // std::cout << "        { \"id\": " << str << ", \"column\": " << stream->getColumnId()
        //     << ", \"kind\": \"" << streamKindToString(stream->getKind())
        //     << "\", \"offset\": " << stream->getOffset() << ", \"length\": " << stream->getLength()
        //     << " }";

        //     if(stream->getKind() == 1) 
        //     {
        //         std::cout << "\n \nData stream found" << std::endl;
        //         Data_offset = stream->getOffset();
        //         Data_length = stream->getLength();
        //     }
        // }
        // reader.reset(); //
        // std::cout << "Data_offset: " << Data_offset << std::endl;
        // std::cout << "Data_length: " << Data_length << std::endl;

        // ORIG_file_size_bytes = Data_length;

        // std::cout << "C2 File size is: " << ORIG_file_size_bytes << std::endl;

        // file_size_rem = ORIG_file_size_bytes%64;
        // if(file_size_rem!=0)
        // {
        //     KRNL_file_size_bytes = ORIG_file_size_bytes + (64 - file_size_rem);
        // }
        // else
        // {
        //     KRNL_file_size_bytes = ORIG_file_size_bytes;
        // }
        // KRNL_file_size_bytes = KRNL_file_size_bytes + 576;  //the pipeline depth of FPGA 64*9 = 576
        // KRNL_file_size_count = KRNL_file_size_bytes/64;

        // nvme_file = c2FileData;
        // nvmeFd = open(nvme_file.c_str(), O_RDONLY); // | O_DIRECT | O_SYNC  O_RDONLY  O_RDWR
        // if (nvmeFd < 0) {
        //     std::cerr << "ERROR: open " << nvme_file << "failed: " << std::endl;
        //     return EXIT_FAILURE;
        // }

        // offset = Data_offset;
        // if (lseek(nvmeFd, offset, SEEK_SET) == -1) {
        //     perror("Error setting file offset");
        //     close(nvmeFd);
        //     return 1;
        // }
        // std::vector<_512b, tapa::aligned_allocator<_512b>> data_in1(KRNL_file_size_count);
        // bytes_read = read(nvmeFd, static_cast<void*>(data_in1.data()), ORIG_file_size_bytes);

        // uint32_t remDiv = nrows%64;
        // uint32_t readCnt = nrows/64;
        // if(remDiv!=0)
        // {
        //     readCnt += 1;
        // }
        // std::cout << "nrows: " << nrows << std::endl;
        // std::cout << "readCnt: " << readCnt << std::endl;
        // filterConf[1] = readCnt;
        
        // filconf = (uint32_t)(LR);
        // filconf = filconf << 32;    //4
        // filconf = filconf | (uint32_t)(RR);
        // filconf = filconf << 8;     //1
        // filconf = filconf | LROP;
        // filconf = filconf << 8;     //1
        // filconf = filconf | RROP;
        // filconf = filconf << 8;     //1
        // filconf = filconf | range_flag;
        // filconf = filconf << 8;     //1
        // filconf = filconf | 1;      //idx data

        // filterConf[0] = filconf;

        // kernel_time_ns = tapa::invoke(
        //     orc_proc, FLAGS_bitstream, 
        //     tapa::read_only_mmap<_512b>(data_in1),
        //     tapa::read_write_mmap<_512b> (filterConf),
        //     tapa::write_only_mmap<_512b>(data_out),
        //     tapa::write_only_mmap<_512b>(data_out1),
        //     tapa::write_only_mmap<_512b>(data_out2),
        //     tapa::write_only_mmap<_512b>(data_out3),
        //     tapa::write_only_mmap<_512b>(track_data),
        //     wait_count,
        //     KRNL_file_size_count
        //     // KRNL_Data_Write
        // );


    _512b tNum = filterConf[2];
    int filrows = tNum.range(31,0);
    std::cout << "Total Numbers after Filteration: " << filrows << std::endl;

    std::cout << "Kernel Exec time(ms): " << (kernel_time_ns) * 1e-6 << std::endl;
    std::cout << "Data Input Size (MB): " << (float)(Data_length/(1000.0)/1000.0) << std::endl;
    std::cout << "Data Output Size (MB): " << (float)((nrows*4)/(1000.0*1000.0)) << std::endl;
    std::cout << "Kernel Input throughput(MB/s): " << ((float)(Data_length)/(float)(kernel_time_ns))*1000.0 << std::endl;
    std::cout << "Kernel Output throughput(MB/s): " << ((float)(nrows*4)/(float)(kernel_time_ns))*1000.0 << std::endl;
    std::cout << "Kernel Output(Fil Rows) throughput(MB/s): " << ((float)(filrows*4)/(float)(kernel_time_ns))*1000.0 << std::endl;

    //Read Data Idx
    int kernel_dout = 0;
    ap_uint<AXI_WIDTH> buf_out = 0;
    int32_t* data_OUT = reinterpret_cast<int32_t*>(aligned_alloc(4096, nrows*sizeof(int32_t)));
    std::vector<int32_t> combinedData;

    
    // if(filrows == 0)
    // {
    //     filrows = 468316;
    // }
    //Data Verification
    if(FLAGS_VERIF)
    {
        std::cout << "Starting Data verif" << std::endl;
        update_patch_data(reinterpret_cast<int32_t*>(data_out.data()), 
                    reinterpret_cast<int32_t*>(data_out1.data()), 
                    reinterpret_cast<int32_t*>(data_out2.data()), 
                    reinterpret_cast<int32_t*>(data_out3.data()),
                    reinterpret_cast<int32_t*>(track_data.data())
                    );
        // verif_all
        // verif_allNmeta
        // verif_all(reinterpret_cast<int32_t*>(data_out.data()), 
        //             reinterpret_cast<int32_t*>(data_out1.data()), 
        //             reinterpret_cast<int32_t*>(data_out2.data()), 
        //             reinterpret_cast<int32_t*>(data_out3.data()),
        //             reinterpret_cast<int32_t*>(track_data.data())
        //             );
        
        //combine data 
            for(int i = 0; i < (dOut_Size_A); i ++)
            {
                buf_out = data_out[i];
                for(int j = 0; j < 16; j++)
                {
                    kernel_dout = buf_out.range(31,0);
                    combinedData.push_back(kernel_dout);
                    buf_out = buf_out >> 32;
                }
                buf_out = data_out1[i];
                for(int j = 0; j < 16; j++)
                {
                    kernel_dout = buf_out.range(31,0);
                    combinedData.push_back(kernel_dout);
                    buf_out = buf_out >> 32;
                }
                buf_out = data_out2[i];
                for(int j = 0; j < 16; j++)
                {
                    kernel_dout = buf_out.range(31,0);
                    combinedData.push_back(kernel_dout);
                    buf_out = buf_out >> 32;
                }
                buf_out = data_out3[i];
                for(int j = 0; j < 16; j++)
                {
                    kernel_dout = buf_out.range(31,0);
                    combinedData.push_back(kernel_dout);
                    buf_out = buf_out >> 32;
                }
            }

        // Sort the combined data in descending order
        // std::sort(combinedData.begin(), combinedData.end());    //for ascending order
        // std::sort(combinedData.begin(), combinedData.end(), std::greater<int32_t>());
        
        //Print out the data
        // for (const auto& element : combinedData) {
        //     std::cout << element << std::endl;
        // }

        verif_sorted(combinedData, filrows);
        // processFlags(Data_Index);
    }

    // compare data_in with data_out
    #if 0
        bool d_check = 0;
        int j = 0;
        // for(int i = 0; i < kernel_d_count; i++)
        // {
        //     j = i%32==0 ? j+1 : j;
        //     if(data_in[j] != data_out[i])
        //     {
        //         d_check = 1;
        //         std::cout << "Data mismatch at: " << i << std::endl;
        //         break;
        //     }        
        //     ++j;
        // }
        // if(d_check)
        // {
        //     std::cout << "---Failed---" << std::endl;
        // }
        // else
        // {
        //     std::cout << "---Passed---" << std::endl;
        // }
    #endif


    // Printing out data
    #if 0
        for(int i = 0; i < (dOut_Size_A/4); i ++)
        {
            buf_out = data_out[i];
            std::cout << "Hex value D0 Out: " << std::setfill('0') 
            << std::setw(64) << std::hex << buf_out << std::endl << std::dec;

            buf_out = data_out1[i];
            std::cout << "Hex value D1 Out: " << std::setfill('0') 
            << std::setw(64) << std::hex << buf_out << std::endl << std::dec;

            buf_out = data_out2[i];
            std::cout << "Hex value D2 Out: " << std::setfill('0') 
            << std::setw(64) << std::hex << buf_out << std::endl << std::dec;

            buf_out = data_out3[i];
            std::cout << "Hex value D3 Out: " << std::setfill('0') 
            << std::setw(64) << std::hex << buf_out << std::endl << std::dec;
        }
        
    #endif

    //print all numbers
    #if 0
        for(int i = 0; i < (dOut_Size_A); i ++)
        {
            buf_out = data_out[i];
            for(int j = 0; j < 16; j++)
            {
                kernel_dout = buf_out.range(31,0);
                std::cout << kernel_dout << std::endl;
                buf_out = buf_out >> 32;
            }
            buf_out = data_out1[i];
            for(int j = 0; j < 16; j++)
            {
                kernel_dout = buf_out.range(31,0);
                std::cout << kernel_dout << std::endl;
                buf_out = buf_out >> 32;
            }
            buf_out = data_out2[i];
            for(int j = 0; j < 16; j++)
            {
                kernel_dout = buf_out.range(31,0);
                std::cout << kernel_dout << std::endl;
                buf_out = buf_out >> 32;
            }
            buf_out = data_out3[i];
            for(int j = 0; j < 16; j++)
            {
                kernel_dout = buf_out.range(31,0);
                std::cout << kernel_dout << std::endl;
                buf_out = buf_out >> 32;
            }
        }
    #endif

    if (close(nvmeFd) == -1) {
        perror("Error closing file");
        return 1; // or handle the error in your specific way
    }

    return 0;
}

void verif_sorted(std::vector<int32_t> combinedData, int FilRows)
{
    std::ifstream in_file(FLAGS_orig);

    if (!in_file) {
        throw std::runtime_error("Error: failed to open input file.");
    }

    int number;
    int rowIndex = 0;

    while (in_file >> number) {
        // std::cout << "number: " << number << std::endl;
        if (rowIndex >= FilRows) {
            // throw std::runtime_error("Error: File contains more numbers than expected rows.");
            std::cout << "Error: File contains more numbers than expected rows." << std::endl;
        }

        if (number != combinedData[rowIndex]) {
            throw std::runtime_error("Error: Data mismatch at index " + std::to_string(rowIndex + 1) +
                                     ". FPGA: " + std::to_string(combinedData[rowIndex]) +
                                     ", Actual: " + std::to_string(number));
        }

        ++rowIndex;
    }

    // if (rowIndex < FilRows) {
    //     throw std::runtime_error("Error: File contains fewer numbers than expected rows.");
    // }

    std::cout << "All rows match. Passed " << rowIndex << " rows." << std::endl;

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


void processFlags(const std::vector<_512b, tapa::aligned_allocator<_512b>>& data) {
    // Open the file containing 0s and 1s
    std::ifstream flagFile(filFLAG);

    if (!flagFile.is_open()) {
        std::cerr << "Error opening flags file." << std::endl;
        return;
    }

    // Initialize counters
    uint16_t bitCounter = 0;
    uint32_t vectorIndex = 0;
    bool NM_Flag = 0;
    bool myData = 0;

    uint32_t idx = 1;

    // Read flags from the file and compare with the data
    bool flag = 0;
    while (flagFile >> flag) {
        // Assuming flag is 0 or 1 in the file

        // Extract the relevant 64 bits from the _512b vector
        _512b dataChunk = data[vectorIndex];
        myData = dataChunk.range(bitCounter,bitCounter);

        // Compare the flag with the corresponding bit in the data
        if (myData == flag) {
            // std::cout << "Flag at position idx " << idx << " bitcounter " << bitCounter << " matches." << std::endl;
        } else {
            NM_Flag = 1;
            // std::cout << "Flag at position idx " << idx << " bitcounter " << bitCounter << " does not match." << std::endl;
        }

        // Move to the next bit
        bitCounter = (bitCounter + 1) % 64;

        // Move to the next vector if necessary
        if (bitCounter == 0) {
            ++vectorIndex;
        }

        ++idx;
    }

    flagFile.close();
    if(NM_Flag == 0)
    {
        std::cout << "PASSED, All flags matched: " << (idx-1) << std::endl;
    }
    else
    {
        std::cout << "FAILED, Flags have mismatch" << std::endl;
    }
    
}
