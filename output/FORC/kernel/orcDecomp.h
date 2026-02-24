
#ifndef ORCDECOMP_H
#define ORCDECOMP_H

//////////////////////////////////////////////////////////////////////////
enum eHuffmanType { FIXED = 0, DYNAMIC = 1, FULL = 2 };
enum blockStatus { PENDING = 0, FINISH = 1 };
//////////////////////////////////////////////////////////////////////////
#define MULTIPLE_BYTES 8
typedef ap_uint<48> bitBufferType;
//////////////////////////////////////////////////////////////////////////

template<int SIZE_ST = 1>
void DecompHead(tapa::istream<_512b>& inStream,
                tapa::ostream<_512b>& outCstm,
                tapa::ostream<_512b>& outUCstm,
                tapa::ostream<ap_uint<20>>& outCMDstm,
                tapa::ostream<ap_uint<20>>& outUCMDstm,
                uint32_t data_count)
{
    _1024b sData = 0; 
    bool waste_cycle = false;
    uint16_t remainBits = 0;
    uint32_t dRead_Count = 0;
    uint16_t dOffset = 0;
    bool orig_data = 0;
    uint32_t dSendCount = 0;
    uint16_t hOffset = 0;
    ap_uint<24> sendCount = 0;
    bool is_last_iteration = false;
    bool is_block_start = false;
    bool exit_loop = false;
    ap_uint<20> metaData = 0;
    bool keep_read = false;
    bool data_read = true;
    bool headProc = true;
    bool proc_data = false;
    _512b sendData = 0;
    _1024b latchData = 0;
    ap_uint<24> header = 0;
    ap_uint<24> Dsize = 0;
    uint8_t dRemSize = 0;
    sData.range(511,0) = inStream.read();
    sData.range(1023,512) = inStream.read();
    dRead_Count = 2;

    DecompHead:for( ;; )
    {
        #pragma HLS PIPELINE II = 1
        if(headProc)
        {
            headProc = false;
            data_read = true;
            // proc_data = true;
            header = sData.range(hOffset+23,hOffset);
            dOffset = hOffset + 24;
            Dsize = header >> 1;    //in bytes
            orig_data = header & 1;

            dSendCount = Dsize >> 6;    //64bytes=512bits Dsize/64 = Dsize>>6
            dRemSize = Dsize&63;         //it equals to Dsize%64=Dsize&(n-1) only works with power of 2 of n
            remainBits = dRemSize*8;       //remaining bits

            if(dRemSize!=0)
            {
                dSendCount+=1;
            }

            #ifndef __SYNTHESIS__
                // std::cout << "Header: 0x" << std::setfill('0') 
                // << std::setw(64) << std::hex << headerData << std::endl << std::dec;
                std::cout << "Dsize: " << Dsize << std::endl;
                // std::cout << "Cur hOffset: " << hOffset << std::endl;
                // std::cout << "dOffset: " << dOffset << std::endl;
                // std::cout << "orig_data: " << orig_data << std::endl;
                // std::cout << "remainBits: " << remainBits << std::endl;
                // std::cout << "dSendCount: " << dSendCount << std::endl;
            #endif
            //next header offset
            waste_cycle = dOffset >= 512;
            if(waste_cycle)
            {
                dOffset -= 512;
            }
            else
            {
                _512b hData = sData.range(511+dOffset,dOffset);
                sendCount+=1;

                //metaData 
                metaData.range(0,0) = orig_data;           // Data Tracking Flag
                metaData.range(1,1) = 0;                   // End of Block for compressed/Uncomp data
                metaData.range(2,2) = 1;                   // Signal Start of Block Data
                metaData.range(3,3) = 0;                   // Signal End of Data
                metaData.range(19,4) = remainBits;
                outCMDstm.write(metaData);                   //Compressed Strm Metadata

                // Write to the appropriate output stream
                if(orig_data)
                {
                    outUCstm.write(hData);   // Uncompressed Data
                    outUCMDstm.write(metaData);                 //Uncompressed Strm Metadata, when data is compressed then metaData length will
                                                            //mismatch with the decompressed data.
                }
                else
                {
                    outCstm.write(hData);   // Compressed Data
                    outUCMDstm.write(metaData);                 //Uncompressed Strm Metadata, when data is compressed then metaData length will
                                                            //mismatch with the decompressed data.
                    // outCMDDstm.write(true);     //bcz of block start
                }
                #ifndef __SYNTHESIS__
                    // std::cout << "sendCount: " << sendCount << std::endl;
                    // std::cout << "dSendCount: " << dSendCount << std::endl;
                    // std::cout << "data_read: " << data_read << std::endl;
                    // std::cout << "orig_data: " << orig_data << std::endl;
                    // std::cout << "is_block_start: " << 1 << std::endl;
                    // std::cout << "is_last_iteration: " << 0 << std::endl;
                    // std::cout << "remainBits: " << remainBits << std::endl;
                    // std::cout << "exit_loop: " << 0 << std::endl;
                    // std::cout << "dRead_Count: " << dRead_Count << std::endl;
                    // std::cout << "data_count: " << data_count << std::endl << std::endl;
                    // std::cout << "hData: 0x" << std::setfill('0') 
                    // << std::setw(64) << std::hex << hData << std::dec << std::endl << std::endl;
            
                #endif

            }

            hOffset = dOffset+remainBits;           //next header offset
            
            if(hOffset >= 512)                      //if next header in middle and remainBits large but less than 512
            {
                keep_read = true;
                hOffset -= 512;
            }
            else 
            {
                if(remainBits == 0)
                {
                    keep_read = true;
                }
                else
                {
                    keep_read = false;
                }   
            }
        
            #ifndef __SYNTHESIS__
                // std::cout << "waste_cycle: " << waste_cycle << std::endl;
                // std::cout << "N hOffset: " << hOffset << std::endl;
                // std::cout << "N dOffset: " << dOffset << std::endl;
                // std::cout << "keep_read: " << keep_read << std::endl;
            #endif
        }
        else
        {
            latchData = sData >> dOffset;                           //discard header or used bits
            is_block_start = (sendCount == 0);
            is_last_iteration = ((sendCount + 1) == dSendCount);
            if(is_last_iteration)
            {
                sendCount = 0;
                exit_loop = (dRead_Count >= (data_count));
                sendData = (remainBits == 0) ? latchData.range(511, 0) : latchData.range(remainBits-1, 0);
                headProc = true;
                // proc_data = false;
                data_read = keep_read;
                // Dsend = false;

                #ifndef __SYNTHESIS__
                    // std::cout << "dRead_Count: " << dRead_Count << std::endl;
                    // std::cout << "data_count: " << data_count << std::endl;
                    // std::cout << "exit_loop: " << exit_loop << std::endl;
                    // std::cout << "dSendCount: " << dSendCount << std::endl;
                    // std::cout << "sendCount: " << sendCount << std::endl;

                    // std::cout << "sendData: " << std::setfill('0') 
                    // << std::setw(64) << std::hex << sendData << std::endl << std::dec;
                #endif
            }
            else
            {
                sendData = latchData.range(511,0);
                sendCount++;
                headProc = false;
                // proc_data = true;
                data_read = true;
            }
            
            //metaData 
            metaData.range(0,0) = orig_data;           // Data Tracking Flag
            metaData.range(1,1) = is_last_iteration;   // End of Block for compressed/Uncomp data
            metaData.range(2,2) = is_block_start;           // Signal Start of Block Data
            metaData.range(3,3) = exit_loop;           // Signal End of Data
            metaData.range(19,4) = remainBits;
            outCMDstm.write(metaData);                   //Compressed Strm Metadata
            

            // Write to the appropriate output stream
            if(orig_data)
            {
                outUCstm.write(sendData);   // Uncompressed Data
                outUCMDstm.write(metaData);                 //Uncompressed Strm Metadata, when data is compressed then metaData length will
                                                        //mismatch with the decompressed data.
            }
            else
            {
                outCstm.write(sendData);   // Compressed Data
                if(is_block_start)
                {
                    outUCMDstm.write(metaData);                 //Uncompressed Strm Metadata, when data is compressed then metaData length will
                                                            //mismatch with the decompressed data.
                }
                else if(exit_loop)
                {
                    outUCMDstm.write(metaData);
                }
            }   

            #ifndef __SYNTHESIS__
                // std::cout << "sendCount: " << sendCount << std::endl;
                // std::cout << "dSendCount: " << dSendCount << std::endl;
                // std::cout << "data_read: " << data_read << std::endl;
                // std::cout << "orig_data: " << orig_data << std::endl;
                // std::cout << "is_block_start: " << is_block_start << std::endl;
                // std::cout << "is_last_iteration: " << is_last_iteration << std::endl;
                // std::cout << "remainBits: " << remainBits << std::endl;
                // std::cout << "exit_loop: " << exit_loop << std::endl;
                // std::cout << "dRead_Count: " << dRead_Count << std::endl;
                // std::cout << "data_count: " << data_count << std::endl << std::endl;
                // std::cout << "sendData: " << std::setfill('0') 
                // << std::setw(64) << std::hex << sendData << std::endl << std::dec << std::endl;
            #endif
        }

        if(exit_loop)   //exit after sending
        {
            break;
        }

        if(data_read)
        {
            sData = (sData >> 512);
            sData.range(1023,512) = inStream.read();
            dRead_Count += 1;
        }
    }

    consumeLeftOver:while(dRead_Count < (data_count + 1))
    {
        inStream.read();    //consume left over
        dRead_Count+=1;
    }
    #ifndef __SYNTHESIS__
        // std::cout << "header proc ended" << std::endl;
    #endif

}



template<int SIZE_ST = 1>
void decompSender(tapa::istream<_512b>& inStream,
                tapa::istream<ap_uint<20>>& inMetaStream,
                tapa::ostream<uint8_t>& idxCombinerSend,
                tapa::ostreams<bool, DMUL>& outCEBoSstrm,
                tapa::ostreams<_512b, DMUL>& outCdata,
                tapa::ostreams<ap_uint<7>, DMUL>& outVdata)
{
    //start stop decompression and buffer data for decomp

    uint8_t strm_idx = 0;

    decompSender:for( ;; )
    {
        #pragma HLS PIPELINE II = 1 style=flp
        ap_uint<7> validCount = 32;
        ap_uint<20> metaData = inMetaStream.read();
        bool orig_data = metaData.range(0,0);    
        bool is_last_iteration = metaData.range(1,1);    
        bool is_block_start = metaData.range(2,2);  
        bool exit_loop = metaData.range(3,3);
        uint16_t dRemBits = metaData.range(19,4);
        // bool st_add = is_last_iteration;
        

        if(!orig_data)
        {
            if(is_last_iteration)
            {
                if(dRemBits != 0)
                {
                    uint8_t dRem16Count = dRemBits >> 4;                            //  dRem/16
                    uint8_t dRem16rem = dRemBits&15;                                // dRem%16
                    dRem16Count = (dRem16rem == 0) ? dRem16Count : (dRem16Count+1);
                    validCount.range(5,0) = dRem16Count;
                }
                validCount.range(6,6) = 1;                                          //block end
            }
            else
            {
                if(is_block_start)
                {
                    idxCombinerSend.write(strm_idx);
                    outCEBoSstrm[strm_idx].write(true);       //start zlib decomp 
                }
                // validCount = 32;                           //16 bits count     512/16 = 32
            }
            outCdata[strm_idx].write(inStream.read());      //zlib data
            outVdata[strm_idx].write(validCount);                  

            #ifndef __SYNTHESIS__
                // printf("strm_idx: %d\n", strm_idx);
            #endif

            if(exit_loop)
            {
                outCEBoSstrm[strm_idx].write(false);       //stop zlib decomp
                break;
            }
            else if(is_last_iteration)
            {
                strm_idx = (strm_idx == (DMUL-1)) ? 0 : (strm_idx + 1);
            }
        }
        else if(exit_loop)
        {
            outCEBoSstrm[strm_idx].write(false);       //stop zlib decomp
            break;
        }
    }
    //stop rest of decomps
    endS:for(int i = 0; i < DMUL; i++)
    {
        #pragma HLS UNROLL
        if(i != strm_idx)
        {
            outCEBoSstrm[i].write(false);  //end decomp
        }
    }

}



template<int SIZE_ST = 1>
void zlib_Sender(tapa::istream<_512b>& inStream,
                    tapa::istream<ap_uint<7>>& inVdata,
                    tapa::ostream<ap_uint<16>>& outCstrm,
                    tapa::ostream<bool>& outCEoSstrm)
{
    zlib_Sender:for( ;; )
    {
        if((!inStream.empty()) && (!inVdata.empty()))
        {
            _512b Cdata = inStream.read();
            ap_uint<7> Vdata = inVdata.read();
            bool block_end = Vdata.range(6,6);

            outCstrm.write(Cdata.range(15,0));
            bool iLast = (block_end && (Vdata < 2));    //if only one data to send and is last iteration
            outCEoSstrm.write(iLast);      
            Cdata = Cdata >> 16;

            downSize:for(int i = 0; i < (Vdata-1); i++)         //if one data to send loop wont run
            {
                #pragma HLS PIPELINE II=1
                bool iLast = (block_end && (i == (Vdata-2)));    //if only one data to send and is last iteration
                outCstrm.write(Cdata.range(15,0));
                outCEoSstrm.write(iLast);      
                Cdata = Cdata >> 16;
            }
        }
    }
}


template<int SIZE_ST = 1>
void decompData(tapa::istream<_72b>& inDecomStrm,
                tapa::ostream<_512b>& outDstm,
                tapa::ostream<ap_uint<8>>& outVDStm) 
{

    // Data variables
    _72b inData = 0;
    ap_uint<1024> send_Data = 0;
    ap_uint<16> total_bits = 0;
    ap_uint<64> dData = 0;
    ap_uint<12> valid_bits = 0;
    ap_uint<16> offset = 0;
    

    decompData:for ( ;; )
    {
        #pragma HLS PIPELINE II = 1
        if(!inDecomStrm.empty())
        {
            inData = inDecomStrm.read();

            if(inData == 0)     //last data of decomp
            {
                //Send remaining bits here
                // if total_bits is 0 data will also be 0.
                ap_uint<8> myBytes = 0;
                myBytes.range(7,7) = 1;
                myBytes.range(6,0) = (total_bits >> 3);                 //total_bits/8
                // myBits = (ap_uint<16>)(0x8000) | total_bits;          // 8000 = 32768
                outDstm.write(send_Data.range(511,0));
                outVDStm.write(myBytes);  //valid bits. binary-> 8000 | 200 , 200 = 512 8000 for the last bit to set to one to show its the last data

                #ifndef __SYNTHESIS__
                    // std::cout << "inData is 0: " << total_bits << std::endl;
                    // std::cout << "send_Data: " << std::setfill('0') << std::setw(64) 
                    //         << std::hex << send_Data.range(511,0) << std::endl << std::dec;
                #endif
                // Reset variables for the next block
                send_Data = 0;
                total_bits = 0;
                offset = 0;
            }
            else
            {
                ap_uint<8> strb = (uint32_t)(inData.range(7,0));
                dData = inData.range(71,8);
                valid_bits = (32 - __builtin_clz(strb));    //bytes
                valid_bits = valid_bits*8;                  //ideally they should be 64 all time until last cycle

                #ifndef __SYNTHESIS__
                    // std::cout << "strb: " << strb << std::endl;
                    // std::cout << "valid_bits: " << valid_bits << std::endl;
                    // std::cout << "Valid Data: " << std::setfill('0') << std::setw(8) 
                    //                 << std::hex << dData << std::endl << std::dec;
                #endif
                
                send_Data.range(offset+valid_bits-1,offset) = dData.range(valid_bits - 1, 0);
                
                total_bits += valid_bits;

                #ifndef __SYNTHESIS__
                    // std::cout << "total_bits: " << total_bits << std::endl;
                    // std::cout << "valid_bits: " << valid_bits << std::endl;
                    // std::cout << "send_Data: " << std::setfill('0') << std::setw(64) 
                    //         << std::hex << send_Data.range(511,0) << std::endl << std::dec;
                #endif
                
                if(total_bits >= 512)
                {   
                    //concat and send       
                    outDstm.write(send_Data.range(511,0));
                    outVDStm.write(64);                        //valid bytes.   
                    send_Data = send_Data >> 512;               //discard bits
                    total_bits -= 512;
                }
                offset = total_bits;
            }
        }
    }
}

template<int SIZE_ST = 1>
void DataCombiner(tapa::istream<ap_uint<20>>& inMDstrm,
                    tapa::istream<uint8_t>& compIdx,
                  tapa::istream<_512b>& inUCstrm,
                  tapa::istreams<_512b, DMUL>& inCstrm,
                  tapa::istreams<ap_uint<8>, DMUL>& inVbStrm,
                  tapa::ostream<_512b>& outDstrm,
                  tapa::ostream<bool>& outEDstrm) 
{
    // Declarations
    uint16_t dRemSize = 0;
    bool orig_data = false;
    bool is_last_iteration = false;
    bool is_block_start = false;
    bool exit_loop = false;
    ap_uint<20> metaData = 0;

    bool readMD = true;
    // bool readMDF = true;
    bool procData = false;
    _512b Rdata = 0;
    // bool dCombEnd = false;
    bool is_compL_iteration = false;
    ap_uint<1024> WrData = 0;
    ap_uint<16> validBits = 0;
    ap_uint<16> bitsInWrData = 0;
    bool dataReady = false;  // New flag to indicate when data is ready to be combined
    uint8_t strm_idx = 0;
    // uint16_t debug_Cnt = 0;

    metaData = inMDstrm.read();
    orig_data = metaData.range(0,0);
    is_last_iteration = metaData.range(1,1);
    is_block_start = metaData.range(2,2);
    exit_loop = metaData.range(3,3);
    dRemSize = metaData.range(19, 4);
    readMD = orig_data;

    if((!orig_data) && (is_block_start))
    {
        strm_idx = compIdx.read();
    }

    #ifndef __SYNTHESIS__
        // printf("strm_idx: %d \n", strm_idx);
    #endif 

    DataCombiner: for (;;)
    {
        #pragma HLS PIPELINE II = 1   
        // dCombEnd = exit_loop;     
        if(orig_data) 
        {
            Rdata = inUCstrm.read();
            validBits = 512;
            if(is_last_iteration)
            {
                if(dRemSize !=0)           //if 0 its 512
                {
                    validBits = dRemSize;
                }
            } 
            readMD = true;
        } 
        else
        {
            #ifndef __SYNTHESIS__
                // printf("ReadD, strm_idx: %d \n", strm_idx);
            #endif
            if(!exit_loop)
            {
                Rdata = inCstrm[strm_idx].read();
                ap_uint<8> Vb_full = inVbStrm[strm_idx].read();
                is_compL_iteration = Vb_full.range(7,7);
                validBits = Vb_full.range(6, 0) * 8;
                readMD = is_compL_iteration;
            }
            else
            {
                validBits = 0;
            }
        }

        #ifndef __SYNTHESIS__
            // std::cout << "is_last_iteration: " << is_last_iteration << std::endl;
            // std::cout << "is_compL_iteration: " << is_compL_iteration << std::endl;
            // std::cout << "orig_data: " << orig_data << std::endl;
            // std::cout << "exit_loop DC: " << exit_loop << std::endl;
            // std::cout << "dCombEnd: " << dCombEnd << std::endl;
            // std::cout << "validBits: " << validBits << std::endl;
            // std::cout << "readMD: " << readMD << std::endl;
            // std::cout << "bitsInWrDataB: " << bitsInWrData << std::endl;
            // std::cout << "Valid Data: " << std::setfill('0') << std::setw(64) 
            //                 << std::hex << Rdata << std::endl << std::dec;
        #endif

        if(validBits > 0)
        {
            WrData.range(bitsInWrData + validBits - 1, bitsInWrData) = Rdata.range(validBits - 1, 0);
        }
        
        bitsInWrData += validBits;
        
        #ifndef __SYNTHESIS__
            // std::cout << "validBits: " << validBits << std::endl;
            // std::cout << "dCombEnd: " << dCombEnd << std::endl;
            // std::cout << "debug_Cnt: " << debug_Cnt << std::endl;
            // std::cout << "bitsInWrDataA: " << bitsInWrData << std::endl;
            // std::cout << "WrData:" << std::setfill('0') 
            // << std::setw(64) << std::hex << WrData << std::endl << std::dec;
        #endif

        // Send data if enough bits are collected
        if(exit_loop)    //write remaining bits if any
        {
            outDstrm.write(WrData.range(511, 0));
            outEDstrm.write(true);
            bitsInWrData = 0;
            // debug_Cnt += 1;
            break;
        }
        else if(bitsInWrData >= 512)
        {
            outDstrm.write(WrData.range(511, 0));
            outEDstrm.write(false);
            WrData = WrData >> 512;
            bitsInWrData -= 512;
            // debug_Cnt += 1;
        }

        // Read metadata
        if (readMD) 
        {
            metaData = inMDstrm.read();
            orig_data = metaData.range(0,0);
            is_last_iteration = metaData.range(1,1);
            is_block_start = metaData.range(2,2);
            exit_loop = metaData.range(3,3);
            dRemSize = metaData.range(19, 4);
            if((!orig_data) && (is_block_start))
            {
                strm_idx = compIdx.read();
            }
        }
        #ifndef __SYNTHESIS__
            // std::cout << "WrData:" << std::setfill('0') 
            // << std::setw(64) << std::hex << WrData.range(511,0) << std::endl << std::dec;
            // std::cout << "dCombEnd: " << dCombEnd << std::endl;
            // std::cout << "debug_Cnt: " << debug_Cnt << std::endl;
            // std::cout << "bitsInWrData: " << bitsInWrData << std::endl;
            // std::cout << "WrData:" << std::setfill('0') 
            // << std::setw(64) << std::hex << WrData << std::endl << std::dec;
        #endif
        
    }
    // outDstrm.write(0);
    // outEDstrm.write(true);
}

#endif