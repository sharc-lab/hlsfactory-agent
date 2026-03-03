#ifndef ORC_FILTER_H
#define ORC_FILTER_H

template<int SIZE_ST = 1>
void FilterData(tapa::istream<_512b>& inConfStrm,
                tapa::istream<_512b>& inAllStrm,
                tapa::istream<uint16_t>& inFilIdx,
                tapa::istream<bool>& eFd_strm,
                tapa::ostream<_512b>& outFilterStrm,
                tapa::ostream<uint16_t>& outIdxStrm,
                // tapa::ostream<uint32_t>& numCnt,        //total count
                tapa::ostream<uint8_t>& dataCnt,        //per pump count
                tapa::ostream<bool>& eFDout_strm,
                int F_idx
                )
{
    _512b filter_conf = 0;
    _512b Data_Read = 0;
    ap_uint<16> filIdxRead = 0;
    _512b filData = 0;
    // bool read_conf = 0; 
    bool idx_flag = 0;
    bool range_flag = 0;
    uint8_t RROP = 0;
    uint8_t LROP = 0;
    int RR = 0;
    int LR = 0;
    // ap_uint<24> numCount = 0;
    uint8_t  dCnt = 0;
    ap_uint<16> FilIdx = 0;

    int32_t Din[PEs] = {0};
    int32_t Dout[PEs] = {0};
    bool FG_LR[PEs] = {0};
    bool FG_RR[PEs] = {0};


    filter_conf = inConfStrm.read();    
    idx_flag = filter_conf.range(1,0);
    range_flag = filter_conf.range(9,8);
    RROP = filter_conf.range(23,16);
    LROP = filter_conf.range(31,24);
    RR = filter_conf.range(63,32);
    LR = filter_conf.range(95,64);
    // read_conf = 1;  //read once
 
    filterD:for( ;; )
    {
        #pragma HLS pipeline II = 1
        
        if(!eFd_strm.empty())
        {
            if(eFd_strm.read())
            {
                //reset the vars
                dCnt = 0;

                initOut:for (int i = 0; i < PEs; ++i) {
                    #pragma HLS UNROLL
                    Dout[i] = 0;
                }

                //Read the Data
                Data_Read = inAllStrm.read();

                //Split The Rows
                splitR:for(int i = 0; i < PEs; i++)
                {
                    #pragma HLS UNROLL
                    Din[i] = (int)(Data_Read.range((i*32)+31,i*32));
                }

                //send end of stream
                eFDout_strm.write(true);

                if(idx_flag == 0)
                {
                    //Apply Filter Conditions

                    //Apply the  Left Range Filter
                    switch (LROP)
                    {
                        case FOP_LT:
                            LFOP_LT:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_LR[i] = (Din[i] < LR);
                            }
                            break;
                        
                        case FOP_LTE:
                            LFOP_LTE:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_LR[i] = (Din[i] <= LR);
                            }
                            break;

                        case FOP_EQ:
                            LFOP_EQ:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_LR[i] = (Din[i] == LR);
                            }
                            break;

                        case FOP_NE:
                            LFOP_NE:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_LR[i] = (Din[i] != LR);
                            }
                            break;
                        
                        case FOP_GT:
                            LFOP_GT:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_LR[i] = (Din[i] > LR);
                            }
                            break;

                        case FOP_GTE:
                            LFOP_GTE:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_LR[i] = (Din[i] >= LR);
                            }
                            break;
                        
                        default:
                            LFOP_ERR:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_LR[i] = 0;
                            }   
                            break;
                    }
                    
                    //Apply the  Right Range Filter
                    switch (RROP)
                    {
                        case FOP_LT:
                            RFOP_LT:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_RR[i] = (Din[i] < RR);
                            }
                            break;
                        
                        case FOP_LTE:
                            RFOP_LTE:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_RR[i] = (Din[i] <= RR);
                            }
                            break;

                        case FOP_EQ:
                            RFOP_EQ:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_RR[i] = (Din[i] == RR);
                            }
                            break;

                        case FOP_NE:
                            RFOP_NE:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_RR[i] = (Din[i] != RR);
                            }
                            break;
                        
                        case FOP_GT:
                            RFOP_GT:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_RR[i] = (Din[i] > RR);
                            }
                            break;

                        case FOP_GTE:
                            RFOP_GTE:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_RR[i] = (Din[i] >= RR);
                            }
                            break;
                        
                        default:
                            RFOP_ERR:for(int i = 0; i < PEs; i++)
                            {
                                #pragma HLS UNROLL
                                FG_RR[i] = 0;
                            }   
                            break;
                    }

                    //reset out
                    FilIdx = 0;
                    
                    //Prepare Data To Write
                    FiltDWR:for(int i = 0; i < PEs; i++)
                    {
                        #pragma HLS UNROLL
                        if(FG_LR[i] && FG_RR[i])
                        {
                            //Track Index Here
                            Dout[dCnt] = Din[i];
                            // numCount += 1;
                            dCnt += 1;
                            FilIdx.range(i,i) = 1;

                            #ifndef __SYNTHESIS__
                                
                                // if(F_idx == 0)
                                // {
                                //     std::cout << "Within Range" << std::endl;
                                //     std::cout << "Idx: " << i << std::endl;
                                //     std::cout << "Num: " << Din[i] << std::endl;
                                //     std::cout << "Data Actual: " << Data_Read.range(31,0) << std::endl;
                                // }

                            #endif

                        }
                    }
                    
                    outIdxStrm.write(FilIdx);

                }
                else
                {
                    //Read the index
                    filIdxRead = inFilIdx.read();

                    //Apply Index Filtering
                    //Dont write the count and the indexes
                    FilIdxD:for(int i = 0; i < PEs; i++)
                    {
                        #pragma HLS UNROLL
                        if(filIdxRead.range(i,i))
                        {
                            //Track Index Here
                            Dout[dCnt] = Din[i];
                            dCnt += 1;
                        }
                    }

                    #ifndef __SYNTHESIS__
                        // printf("dCnt: %d \n", dCnt);
                        // std::cout << "dCnt: " << (uint16_t)(dCnt) << std::endl;
                    #endif

                }

                //Pack the Data
                packDfil:for(int i = 0; i < PEs; i++)
                {
                    #pragma HLS UNROLL
                    filData.range((i*32)+31,i*32) = (uint32_t)(Dout[i]);
                }
                
                //per pump count
                dataCnt.write(dCnt);

                #ifndef __SYNTHESIS__
                    // printf("dCnt: %d \n", dCnt);
                    // std::cout << "dCnt: " << (uint16_t)(dCnt) << std::endl;
                #endif

                //send out
                outFilterStrm.write(filData);
                
            }
            else
            {
                eFDout_strm.write(false);
                #ifndef __SYNTHESIS__
                    // printf("End of filter data \n");
                #endif
                // if(idx_flag == 0)
                // {
                //     numCnt.write(numCount);
                // }
                break;
            }
        }
    }
}

template<int SIZE_ST = 1>
void br_s0e(tapa::istreams<_512b, 2>& inFilStrm,
            tapa::istreams<bool, 2>& eFDin_strm,
            tapa::istreams<uint8_t, 2>& inData_count,
            tapa::ostream<_1024b>& outFilStrm,
            tapa::ostream<uint8_t>& outData_count,
            tapa::ostream<bool>& eBR0out_strm
            )
{
    _1024b dout = 0;
    uint8_t c0 = 0;
    uint32_t c0_shifter = 0;
    uint8_t cSum = 0;

    bool tmp = eFDin_strm[0].read();      //read one extra here
    bool tmp1 = eFDin_strm[1].read();      //read one extra here

    br_s0e:for( ;; )
    {    
        #pragma HLS PIPELINE II=1

        if(!inFilStrm[0].empty() && !inFilStrm[1].empty() && 
            !inData_count[0].empty() && !inData_count[1].empty() &&
            !eFDin_strm[0].empty() && !eFDin_strm[1].empty()
            )
        {
            ///Impl type 1
            // dout = inFilStrm[1].read();
            // c0 = inData_count[0].read();
            // c0_shifter = c0*32;

            // dout = dout >> c0_shifter;
            // dout |= inFilStrm[0].read();
            ///Impl type 1

            ///Impl type 2
            dout = inFilStrm[0].read();
            c0 = inData_count[0].read();
            c0_shifter = c0*32;

            dout.range(1023,c0_shifter) = inFilStrm[1].read();
            ///Impl type 2

            cSum = c0 + inData_count[1].read();
            outData_count.write(cSum);
            outFilStrm.write(dout);

            bool tmp2 = eFDin_strm[1].read();
            if(!eFDin_strm[0].read())
            {
                eBR0out_strm.write(false);
                break;
            }
            else
            {
                eBR0out_strm.write(true);
            }

        }

    }

}

template<int SIZE_ST = 1>
void br_s1(tapa::istreams<_1024b, 2>& inFilStrm1,
            tapa::istreams<bool, 2>& eBR0in_strm,
            tapa::istreams<uint8_t, 2>& inData_count1,
            tapa::ostream<_2048b>& outFilStrm1,
            tapa::ostream<bool>& eBR1out_strm,
            tapa::ostream<uint8_t>& outData_count1
            )
{
    _2048b dout = 0;
    uint8_t c0 = 0;
    uint32_t c0_shifter = 0;
    uint8_t cSum = 0;

    br_s1:for( ;; )
    {    
        #pragma HLS PIPELINE II=1

        if(!inFilStrm1[0].empty() && !inFilStrm1[1].empty() && 
            !inData_count1[0].empty() && !inData_count1[1].empty() && 
            !eBR0in_strm[0].empty() && !eBR0in_strm[1].empty()
            )
        {
            ///Impl type 1
            // dout = inFilStrm[1].read();
            // c0 = inData_count[0].read();
            // c0_shifter = c0*32;

            // dout = dout >> c0_shifter;
            // dout |= inFilStrm[0].read();
            ///Impl type 1

            ///Impl type 2
            dout = inFilStrm1[0].read();
            c0 = inData_count1[0].read();
            c0_shifter = c0*32;

            dout.range(2047,c0_shifter) = inFilStrm1[1].read();
            ///Impl type 2

            cSum = c0 + inData_count1[1].read();
            outData_count1.write(cSum);
            outFilStrm1.write(dout);

            bool tmp = eBR0in_strm[1].read();
            if(eBR0in_strm[0].read())
            {
                eBR1out_strm.write(true);
            }
            else
            {
                eBR1out_strm.write(false);
                break;
            }


        }

    }
}

template<int SIZE_ST = 1>
void br_WrTracker(tapa::istream<_2048b>& inFilStrm2,
                 tapa::istream<bool>& inEstrm,
                 tapa::istream<uint8_t>& inData_count2,
                 tapa::ostream<ap_uint<24>>& store_writeCount,
                 tapa::ostream<uint32_t>& numCnt,        //total count
                 tapa::ostreams<_512b, 4>& Dout
                )
{

    _2048b inData = 0;
    // _2048b tempData = 0;
    _2048b outData = 0;
    _2048b buf_data = 0;

    uint8_t runLength = 0;
    int8_t rRL = 0;
    uint16_t bufShift = 0;
    uint16_t dShift = 0;
    int8_t diffRL = 0;
    ap_uint<26> numCount = 0;
    bool last_data = false;
    ap_uint<24> write_count = 0;
    //reset the data writer
    // rst_StrmStore.write(1);

    br_WrTr:for( ;; )
    {
        #pragma HLS PIPELINE II=1
        
        if(!inFilStrm2.empty() && !inData_count2.empty() && !inEstrm.empty())
        {
            inData = inFilStrm2.read();
            runLength = inData_count2.read();       //this is not the runlength its the count of filtered data
            last_data = !(inEstrm.read());
            
            numCount += runLength;

            dShift = rRL*32;                    //rRL*32 or rRL << 5   , 50 *32 ,  57 * 32
            rRL += runLength;
            diffRL = rRL - 64;                  // 53 - 64 = -11 , 67 - 64 = 3
            bufShift = (runLength - diffRL) * 32;   // 10-3= 7*32

            #ifndef __SYNTHESIS__
                // std::cout << "runLength: " << (int16_t)(runLength) << std::endl;
                // std::cout << "rRL: " << (int16_t)(rRL) << std::endl;
                // std::cout << "diffRL: " << (int16_t)(diffRL) << std::endl;
            #endif

            //shift and concat
            // tempData = inData << dShift;
            outData |= (inData << dShift);

            if(diffRL >= 0 || last_data)
            {
                write_count += 1;
                rRL = diffRL;   // rRL = 3
                write_out:for(int i = 0; i < 4; i++)
                {
                    #pragma HLS UNROLL
                    Dout[i].write(outData.range((i*512)+511,i*512));
                }

                outData = 0;
                buf_data = inData >> bufShift;
                outData = buf_data;

                #ifndef __SYNTHESIS__
                    // std::cout << "diffRL: " << (int16_t)(diffRL) << std::endl;
                #endif

                if(last_data)
                {
                    numCnt.write(numCount);
                    store_writeCount.write(write_count);
                    write_last:for(int i = 0; i < 4; i++)
                    {
                        #pragma HLS UNROLL
                        Dout[i].write(outData.range((i*512)+511,i*512));
                    }
                    break;
                }
            }
        }
    }
}

#endif
