
#include "orc_proc.h"
#include "orcDecomp.h"
#include "zlibTapa.h"
#include "orc_decoder.h"
#include "orc_filter.h"


// Data Reading
void mmap2s(tapa::async_mmap<_512b>& input_port,
            tapa::ostream<_512b>& outStream,
            uint32_t data_count)
{
    mmap2s:for(uint32_t i_req = 0, i_resp = 0; i_resp < data_count;)
    {
        #pragma HLS PIPELINE II = 1

        if ((i_req < data_count) && 
            (input_port.read_addr.try_write(i_req))) {
            ++i_req;
        }
        
        if((!input_port.read_data.empty()))
        {
            outStream.write(input_port.read_data.read(nullptr));
            ++i_resp;
        }
    }
    outStream.write(0);
}

//orc_zlib_decomp
void orcDecompHeadProc(tapa::istream<_512b>& inStream,
                tapa::ostream<_512b>& outCstm,
                tapa::ostream<_512b>& outUCstm,
                tapa::ostream<ap_uint<20>>& outCMDstm,
                tapa::ostream<ap_uint<20>>& outUCMDstm,
                uint32_t data_count)
{
    DecompHead<1>(inStream, outCstm, outUCstm, outCMDstm, outUCMDstm, data_count);
}

void orcDecompSender(tapa::istream<_512b>& inStream,
                    tapa::istream<ap_uint<20>>& inMetaStream,
                    tapa::ostream<uint8_t>& idxCombinerSend,
                    tapa::ostreams<bool, DMUL>& outCEBoSstrm,
                    tapa::ostreams<_512b, DMUL>& outCdata,
                    tapa::ostreams<ap_uint<7>, DMUL>& outVdata)
{
    decompSender<1>(inStream, inMetaStream, idxCombinerSend, outCEBoSstrm, outCdata, outVdata);
}

void zSBuf(tapa::istream<_512b>& inDstrm,
            tapa::istream<ap_uint<7>>& inVstrm,
            tapa::ostream<_512b>& oDstrm,
            tapa::ostream<ap_uint<7>>& oVstrm)
{
    zSBuf:for( ;; )
    {
        #pragma HLS PIPELINE II=1
        if(!inDstrm.empty())
        {
            oDstrm.write(inDstrm.read());
        }
        if(!inVstrm.empty())
        {
            oVstrm.write(inVstrm.read());
        }
    }
}


void orcZlibSender(tapa::istream<_512b>& inStream,
                    tapa::istream<ap_uint<7>>& inVdata,
                    tapa::ostream<ap_uint<16>>& outCstrm,
                    tapa::ostream<bool>& outCEoSstrm)
{
    zlib_Sender<1>(inStream, inVdata, outCstrm, outCEoSstrm);
}

void huffmanDecoderLL_W(tapa::istream<ap_uint<16> >& inStream,
                      tapa::istream<bool>& inEos,
                      tapa::istream<bool>& inEDos,
                      tapa::ostream<bool>& outDEos,
                      tapa::ostream<ap_uint<17> >& outStream)
{
    const eHuffmanType c_decoderType = (eHuffmanType)(2);   //DYNAMIC(1), FULL(2)
    huffmanDecoder<c_decoderType>(inStream, inEos, inEDos, outDEos, outStream);
}

void lzProcessingUnitLL_W(tapa::istream<ap_uint<17> >& inStream,
                        tapa::istream<bool>& inDEos,
                        tapa::ostream<bool>& outPPDEos,
                        tapa::ostream<bool>& outLUDEos,
                        tapa::ostream<ap_uint<9>>& litLenStream,
                        tapa::ostream<ap_uint<9>>& matchLenStream,
                        tapa::ostream<ap_uint<16> >& offsetStream,
                        tapa::ostream<ap_uint<10> >& outStream)
{
    lzProcessingUnit<ap_uint<9> >(inStream, inDEos, outPPDEos, outLUDEos, litLenStream,
                                 matchLenStream, offsetStream, outStream);
}

void lzLiteralUpsizerLL_W(tapa::istream<ap_uint<10> >& inStream, 
                            tapa::istream<bool>& inDEos,
                            tapa::ostream<ap_uint<8 * 8> >& litStream)
{
    lzLiteralUpsizer<MULTIPLE_BYTES>(inStream, inDEos, litStream);
}

void lzMultiByteDecompressLL_W(tapa::istream<ap_uint<9>>& litlenStream,
                                tapa::istream<ap_uint<8 * 8> >& litStream,
                                tapa::istream<ap_uint<16>>& offsetStream,
                                tapa::istream<ap_uint<9>>& matchlenStream,
                                tapa::istream<bool>& inDEos,
                                tapa::ostream<ap_uint<(8 * 8) + 8> >& outStream)
{
    const uint32_t HISTORY_SIZE = (32*1024);
    lzMultiByteDecompress<MULTIPLE_BYTES, HISTORY_SIZE, ap_uint<9> >(litlenStream, litStream, offsetStream, 
                                                                        matchlenStream, inDEos, outStream);
}

void orcZlib_top(tapa::istream<ap_uint<16>>& inZstrm,
                tapa::istream<bool>& inZEoSstrm,
                tapa::istream<bool>& inZEDoSstrm,
                tapa::ostream<_72b>& outDtream)
{ 
    tapa::stream<ap_uint<17>, 1024> bitunpackstream("bitunpackstream");      //256
    tapa::stream<ap_uint<9>, 16> matchLenStream("matchLenStream");            //16
    tapa::stream<ap_uint<9>, 16> litLenStream("litLenStream");                //16
    tapa::stream<ap_uint<16>, 16> offsetStream("offsetStream");                //16
    tapa::stream<ap_uint<10>, 16> lzProcOutStream("lzProcOutStream");         //16
    tapa::stream<ap_uint<64>, 32> litStream("litStream");                       //32

    tapa::stream<bool, 1024> outHEoS("outHEoS");                             //256
    tapa::stream<bool, 16> outPPUEoS("outPPUEoS");
    tapa::stream<bool, 16> outLUEoS("outLUEoS");
              
    tapa::task()
        .invoke(huffmanDecoderLL_W, inZstrm, inZEoSstrm, inZEDoSstrm, outHEoS, bitunpackstream)
        .invoke(lzProcessingUnitLL_W, bitunpackstream, outHEoS, outPPUEoS, outLUEoS, litLenStream, matchLenStream, offsetStream, lzProcOutStream)
        .invoke(lzLiteralUpsizerLL_W, lzProcOutStream, outLUEoS, litStream)
        .invoke(lzMultiByteDecompressLL_W, litLenStream, litStream, offsetStream, matchLenStream, outPPUEoS, outDtream);
}

void zlib_dataCombiner(tapa::istream<_72b>& inDecomStrm,
                        tapa::ostream<_512b>& outDstm,
                        tapa::ostream<ap_uint<8>>& outVDStm)
{
    decompData<1>(inDecomStrm, outDstm, outVDStm);
}

void DDBuf(tapa::istream<_512b>& inDstrm,
            tapa::istream<ap_uint<8>>& inMDStrm,
            tapa::ostream<_512b>& outDstm,
            tapa::ostream<ap_uint<8>>& outVDStm)
{
    DDBuf:for( ;; )
    {
        #pragma HLS PIPELINE II = 1
        if(!inDstrm.empty())
        {
            outDstm.write(inDstrm.read());
        }
        if(!inMDStrm.empty())
        {
             outVDStm.write(inMDStrm.read());
        }
    }
}


void orcDecompCombiner(tapa::istream<ap_uint<20>>& inMDstrm,
                        tapa::istream<uint8_t>& compIdx,
                        tapa::istream<_512b>& inUCstrm,
                        tapa::istreams<_512b, DMUL>& inCstrm,
                        tapa::istreams<ap_uint<8>, DMUL>& inVbStrm,
                        tapa::ostream<_512b>& outDstrm,
                        tapa::ostream<bool>& outEDstrm)
{
    DataCombiner<1>(inMDstrm, compIdx, inUCstrm, inCstrm, inVbStrm, outDstrm, outEDstrm);
}

void orcDecompTop(tapa::istream<_512b>& inStream, 
                    tapa::istream<ap_uint<7>>& inVstrm, 
                    tapa::istream<bool>& outZEDoSstrm, 
                    tapa::ostream<_512b>& outData, 
                    tapa::ostream<ap_uint<8>>& outMetaD)
{
    tapa::stream<_512b, DECOMP_DEPTH> outCstm("outCstm"); 
    tapa::stream<ap_uint<7>, DECOMP_DEPTH> outVstm("outVstm"); 

    tapa::stream<ap_uint<16>, 32> outZstrm("outZstrm");                //32*16 = 512
    tapa::stream<bool, 32> outZEoSstrm("outZEoSstrm");
    tapa::stream<_72b, 32> outZDstream("outZDstream");      //32

    tapa::stream<_512b, DECOMP_DEPTH> outDstm("outDstm");      //32
    tapa::stream<ap_uint<8>, DECOMP_DEPTH> outVDStm("outVDStm");      //32

    tapa::task()
        .invoke<tapa::detach>(zSBuf, inStream, inVstrm, outCstm, outVstm)
        .invoke<tapa::detach>(orcZlibSender, outCstm, outVstm, outZstrm, outZEoSstrm)
        .invoke<tapa::join>(orcZlib_top, outZstrm, outZEoSstrm, outZEDoSstrm, outZDstream)
        .invoke<tapa::detach>(zlib_dataCombiner, outZDstream, outDstm, outVDStm)
        .invoke<tapa::detach>(DDBuf, outDstm, outVDStm, outData, outMetaD);
}

//orcDeomp to OrcDecoder Connector
void orcDDconnector(tapa::istream<_512b>& data_in,
                    tapa::istream<bool>& dataEosIn,
                    tapa::ostream<_512b>& data_out
                    )
{
    orcDDconnector:for(;;)
    {
        #pragma HLS PIPELINE II=1
        if((!data_in.empty()) && (!dataEosIn.empty()))
        {
            data_out.write(data_in.read());
            bool tmp = dataEosIn.read();
            if(tmp)
            {
                break;
            }
        }
    }

    pipelineFlush:for(uint8_t i = 0; i < 9; i++)
    {
        #pragma HLS PIPELINE II = 1
        data_out.write(0);
    }
}

//orc_decoder
void orcDecHeadProc(tapa::istream<_512b>& data_in,
                    tapa::ostream<_2048b>& outAll_Lstrm,
                    tapa::ostream<_256b> &out_strmC_Track,
                    tapa::ostream<_512b>& PA_DATA,
                    tapa::ostream<uint32_t>& PA_metaDATA,
                    tapa::ostream<bool>& D_strm_e)
{
    load<1>(data_in, outAll_Lstrm, out_strmC_Track, PA_DATA, PA_metaDATA, D_strm_e);
}


void orcDecSender(tapa::istream<_2048b>& data_in,
                    tapa::istream<_256b>& meta_in,
                    tapa::istream<bool>& D_strm_e,
                    tapa::ostreams<_512b,4>& outAll_Pstrm,
                    tapa::ostream<uint64_t>& outSR_Pstrm,
                    tapa::ostreams<_256b, 4>& meta_out,
                    tapa::ostream<uint8_t>& SR_meta_out,
                    tapa::ostream<uint64_t>& All_meta_out,
                    tapa::ostream<ap_uint<24>>& Dec_type_Out,
                    tapa::ostream<bool>& MD_strm_e)
{
    data_Sender<1>(data_in, meta_in, D_strm_e, outAll_Pstrm, outSR_Pstrm, 
                    meta_out, SR_meta_out, All_meta_out, Dec_type_Out, MD_strm_e);
}

void SRcompute(tapa::istream<uint64_t>& outSR_Pstrm,
                tapa::istream<uint8_t>& meta_out,
                tapa::ostream<_320b>& SR_Dout)
{
    compSR<1>(outSR_Pstrm, meta_out, SR_Dout);
}

void common_DI_compute(tapa::istream<_512b>& in_Cstrm,
                        tapa::istream<_256b>& in_Trstrm,
                        tapa::ostream<uint32_t>& out_Cmeta_strm,
                        tapa::ostream<_512b>& out_Cstrm,
                        tapa::ostream<_512b>& DI_strm,
                        tapa::ostream<_512b>& PA_strm,
                        tapa::ostream<uint32_t>& PA_meta_strm,
                        uint32_t compD_ID)
{
    compute_delta<1>(in_Cstrm, in_Trstrm, out_Cmeta_strm, out_Cstrm, DI_strm, PA_strm, PA_meta_strm, compD_ID);
}

void PA_metadata(tapa::istream<_512b>& PA_Strm,
                tapa::istream<uint32_t>& PA_meta_Strm,
                tapa::ostream<uint64_t>& metaOutPA_Strm)   //PLL, IDX, VAL
{
    PA_meta_proc<1>(PA_Strm, PA_meta_Strm, metaOutPA_Strm);
}

void PA_data_proc(tapa::istream<_512b>& PA_Strm,
                tapa::istream<uint32_t>& PA_BV_Strm,
                tapa::ostream<_512b>& dataOutPA_Strm)
{
    PA_sum_out<1>(PA_Strm, PA_BV_Strm, dataOutPA_Strm);
}

void metadata_aligner(tapa::istream<uint64_t>& PA_metaStrm,
                tapa::istream<bool>& MD_strm_e,
                tapa::istream<uint64_t>& All_meta_Strm,
                tapa::ostream<bool>& MDD_strm_e,
                tapa::ostream<_128b>& meta_Out)
{
    Meta_Aligner<1>(PA_metaStrm, MD_strm_e, All_meta_Strm, MDD_strm_e, meta_Out);
}

void Meta_Writer(tapa::istream<_128b>& in_metaStrm,
                tapa::istream<bool>& MDD_strm_e,
                tapa::async_mmap<_512b>& metaPort_Out
                )
{
    uint32_t i_req_0 = 0, i_resp_0 = 0;
    _128b metaData = 0;
    _512b FinalMetaData = 0;
    uint8_t i = 0;

    uint32_t f_req = 0;

    // uint32_t write_count = metaWR_Cnt; //metaWR_Cnt; //2147483647;
    // uint32_t Rwrite_count = 0;
    // (i_resp_0 < write_count)
    store_meta:for( ;  ; )
    {
        #pragma HLS pipeline II=1

        //reset Stream
            // if(!rst_StrmDS.empty())
            // {
            //     bool tmp = rst_StrmDS.read();
            //     i_req_0 = 0;
            // }

            
        // issue write requests
        if((!in_metaStrm.empty()) &&
            (!metaPort_Out.write_addr.full()) &&
            (!metaPort_Out.write_data.full())
            )
        {
            metaData = in_metaStrm.read();

            #ifndef __SYNTHESIS__
                // std::cout << "MetaRead " << std::endl;
            #endif

            FinalMetaData.range((i*128)+127,(i*128)) = metaData;

            if(i == 3)
            {
                i = 0;
                metaPort_Out.write_addr.try_write(i_req_0);
                metaPort_Out.write_data.try_write(FinalMetaData);
                ++i_req_0;
                FinalMetaData = 0;
            }
            else
            {
                i++;
            }
        }
        else if(!MDD_strm_e.empty())
        {
            //empty the stream
            bool tmp = MDD_strm_e.read();
            //write the uncomplete set of data, it can be duplicated one so no problem.
            if((!metaPort_Out.write_addr.full()) &&
            (!metaPort_Out.write_data.full())
            )
            {
                i = 0;
                metaPort_Out.write_addr.try_write(i_req_0);
                metaPort_Out.write_data.try_write(FinalMetaData);
                ++i_req_0;
                f_req = i_req_0;
                FinalMetaData = 0;
            }
            // // receive acks of write success
            // if (!metaPort_Out.write_resp.empty()) {

            //     i_resp_0 = unsigned(metaPort_Out.write_resp.read(nullptr)) + 1;

            // }
            // break;
        }

        // receive acks of write success
        if (!metaPort_Out.write_resp.empty()) {

            i_resp_0 += unsigned(metaPort_Out.write_resp.read(nullptr)) + 1;
            if(f_req == i_resp_0)
            {
                break;
            }

        }
    }
}

void delta_proc0(tapa::istream<_512b>& delta_Strm,
                tapa::ostream<_512b>& dataOut_Strm,
                tapa::ostream<uint32_t>& CarryOut_Strm)
{
    delta_sumNC_in<1>(delta_Strm, dataOut_Strm, CarryOut_Strm);
}

void delta_proc1(tapa::istream<_512b>& delta_Strm,
                tapa::ostream<_512b>& dataOut_Strm)
{
    delta_sumNC_all<1>(delta_Strm, dataOut_Strm);
}

void delta_proc2(tapa::istream<_512b>& delta_Strm,
                tapa::istream<uint32_t>& CarryIn_Strm,
                tapa::ostream<_512b>& dataOut_Strm,
                tapa::ostream<uint32_t>& CarryOut_Strm,
                tapa::ostream<uint32_t>& CarryOut_Strm1)
{
    delta_sum_2out<1>(delta_Strm, CarryIn_Strm, dataOut_Strm, CarryOut_Strm, CarryOut_Strm1);
}

void delta_proc3(tapa::istream<_512b>& delta_Strm,
                tapa::istream<uint32_t>& CarryIn_Strm,
                tapa::ostream<_512b>& dataOut_Strm,
                tapa::ostream<uint32_t>& CarryOut_Strm)
{
    delta_sum_1out<1>(delta_Strm, CarryIn_Strm, dataOut_Strm, CarryOut_Strm);
}

void delta_proc4(tapa::istream<_512b>& delta_Strm,
                tapa::istream<uint32_t>& CarryIn_Strm,
                tapa::ostream<_512b>& dataOut_Strm)
{
    delta_sum_0out<1>(delta_Strm, CarryIn_Strm, dataOut_Strm);
}

void delta_proc5(tapa::istreams<_512b, 4>& delta_Strm,
                tapa::istreams<uint32_t, 4>& meta_Strm,
                tapa::istream<uint32_t>& CarryIn_Strm,
                tapa::ostreams<_512b, 4>& dataOut_Strm)
{
    delta_Fsum<1>(delta_Strm, meta_Strm, CarryIn_Strm, dataOut_Strm);
}


void orc_dataAligner(tapa::istream<ap_uint<24>>& inTrackStrm,
                tapa::istreams<_512b, 4>& inDEStrm, 
                tapa::istreams<_512b, 4>& inDIStrm,
                tapa::istreams<_512b, 4>& inPAStrm,
                tapa::istream<_320b>& inSRStrm,
                tapa::ostreams<_512b, 4>& outWRStrm,
                tapa::ostream<ap_uint<24>>& outTrackStrm)
{
    Data_Aligner<1>(inTrackStrm, inDEStrm, inDIStrm, inPAStrm, inSRStrm, outWRStrm, outTrackStrm);
}

void orc_BR(tapa::istreams<_512b, 4>& inStrm, 
                tapa::istream<ap_uint<24>>& inMeta,
                tapa::ostreams<_512b, 4>& outStrm,
                tapa::ostreams<bool, 4>& e_strm)
{
    brDecData<1>(inStrm, inMeta, outStrm, e_strm);
}

//orc_filer
void Filter_Config(tapa::mmap<_512b> FilterConf_port,
                    tapa::istream<uint64_t>& inIdxStrm,
                    tapa::istream<bool>& eFC_strm,
                    tapa::async_mmap<_512b> &data_Idx,
                    tapa::ostreams<_512b, 4>& FilterConfig_out,
                    tapa::ostreams<uint16_t, 4>& FilterIdx_out
                    )
{
    _512b filter_conf = 0;
    ap_uint<64> FilIdxFlags = 0;
    uint32_t total_count = 0;

    uint64_t idxData = 0;
    uint8_t idx_flag = 0;
    // bool endCh = 0;

    // #pragma HLS bind_storage variable=Didx_Data type=RAM_S2P impl=uram

    // uint32_t i_reqWr = 0;
    // uint32_t i_req_idx = 0;

    #ifndef __SYNTHESIS__
        uint32_t tmpWrVar = 0;
    #endif

    #ifndef __SYNTHESIS__ 
        // std::cout << "Read Conf FC" << std::endl;
    #endif

    filter_conf = FilterConf_port[0];
    total_count = FilterConf_port[1];

    idx_flag = filter_conf.range(7,0);

    #ifndef __SYNTHESIS__
        std::cout << "FC total_count: " << total_count << std::endl;
        std::cout << "FC idx_flag: " << (uint16_t)(idx_flag) << std::endl;
    #endif


    filter_Wr:for(int i = 0; i < 4; i++)
    {
        #pragma HLS UNROLL
        FilterConfig_out[i].write(filter_conf); //might need to reduce the port width
    }

    #ifndef __SYNTHESIS__
        // std::cout << "Idx_Flag: " << filter_conf.range(7,0) << std::endl;
        // std::cout << "Range_Flag: " << filter_conf.range(15,8) << std::endl;
        // std::cout << "RROP: " << filter_conf.range(23,16) << std::endl;
        // std::cout << "LROP: " << filter_conf.range(31,24) << std::endl;
        // std::cout << "RR: " << filter_conf.range(63,32) << std::endl;
        // std::cout << "LR: " << filter_conf.range(95,64) << std::endl;
    #endif


    if(idx_flag == 1)
    {   
        // Check the index port. Read it and append data in 512 bit output streams to the Filter
        Read_Idx:for(uint32_t i_reqR = 0, i_respR = 0; i_respR < total_count;)
        {
            #pragma HLS PIPELINE II = 1

            // FilIdxFlags = Didx_Data[i_req];
            // FilIdxFlags = data_Idx[i_req];

            if ((i_reqR < total_count) && 
            (data_Idx.read_addr.try_write(i_reqR))) {
            ++i_reqR;
            }

            if((!data_Idx.read_data.empty()))
            {
                FilIdxFlags = data_Idx.read_data.read(nullptr);
                ++i_respR;

                sendIdx:for(int i = 0; i < 4; i++)
                {
                    #pragma HLS UNROLL
                    FilterIdx_out[i].write(FilIdxFlags.range((i*16)+15,i*16));
                }   
            }

            #ifndef __SYNTHESIS__
                // std::cout << "FilIdxFlags[" << i_req << "]: " << FilIdxFlags << std::endl;
            #endif

                 
        }

        bool tmpF = eFC_strm.read();        //Consume end of stream
        // uint32_t tmp = fCntStrm.read();    //consume the stream no need to write back   
    }
    else
    {   
        bool endCh = 0;
        
        wr_idx:for(uint32_t i_reqW = 0, i_respW = 0; i_respW < total_count;)
        {
            #pragma HLS PIPELINE II=1

            if((!inIdxStrm.empty()) &&
                (!eFC_strm.empty()) &&
                (!data_Idx.write_addr.full()) &&
                (!data_Idx.write_data.full())
                ){
            endCh = eFC_strm.read();
            data_Idx.write_addr.try_write(i_reqW);
            data_Idx.write_data.try_write(inIdxStrm.read(nullptr));
            ++i_reqW;
            }

            // receive acks of write success
            if (!data_Idx.write_resp.empty()) {
                i_respW += unsigned(data_Idx.write_resp.read(nullptr)) + 1;
            }

            #ifndef __SYNTHESIS__
                // std::cout << "i_respW: " << i_respW << std::endl;
            #endif
            
        }
        
        endCh = eFC_strm.read();    //read 1 early
    }
}

void orcFilterData(tapa::istream<_512b>& inConfStrm,
                tapa::istream<_512b>& inAllStrm,
                tapa::istream<uint16_t>& inFilIdx,
                tapa::istream<bool>& eFd_strm,
                tapa::ostream<_512b>& outFilterStrm,
                tapa::ostream<uint16_t>& outIdxStrm,
                tapa::ostream<uint8_t>& dataCnt,        //per pump count
                tapa::ostream<bool>& eFDout_strm,
                int F_idx)
{
    FilterData<1>(inConfStrm, inAllStrm, inFilIdx, eFd_strm, outFilterStrm, outIdxStrm, dataCnt, eFDout_strm, F_idx);
}

void filterBR1(tapa::istreams<_512b, 2>& inFilStrm,
                tapa::istreams<bool, 2>& eFDin_strm,
                tapa::istreams<uint8_t, 2>& inData_count,
                tapa::ostream<_1024b>& outFilStrm,
                tapa::ostream<uint8_t>& outData_count,
                tapa::ostream<bool>& eBR0out_strm)
{
    br_s0e<1>(inFilStrm, eFDin_strm, inData_count, outFilStrm, outData_count, eBR0out_strm);
}

void filterBR2(tapa::istreams<_1024b, 2>& inFilStrm1,
            tapa::istreams<bool, 2>& eBR0in_strm,
            tapa::istreams<uint8_t, 2>& inData_count1,
            tapa::ostream<_2048b>& outFilStrm1,
            tapa::ostream<bool>& eBR1out_strm,
            tapa::ostream<uint8_t>& outData_count1)
{
    br_s1<1>(inFilStrm1, eBR0in_strm, inData_count1, outFilStrm1, eBR1out_strm, outData_count1);
}

void filterBR3(tapa::istream<_2048b>& inFilStrm2,
                 tapa::istream<bool>& inEstrm,
                 tapa::istream<uint8_t>& inData_count2,
                 tapa::ostream<ap_uint<24>>& store_writeCount,
                 tapa::ostream<uint32_t>& numCnt,        //total count
                 tapa::ostreams<_512b, 4>& Dout)
{
    br_WrTracker<1>(inFilStrm2, inEstrm, inData_count2, store_writeCount, numCnt, Dout);
}

void concat_IdxCnt(tapa::istream<uint32_t>& numCnt,
                    tapa::istreams<uint16_t, 4>& inIdxStrm,
                    tapa::ostream<uint64_t>& outIdxStrm,
                    tapa::ostream<bool>& eo_strm,
                    tapa::mmap<_512b> FilterConf_port
                    )
{
    ap_uint<64> FilIdxBuf = 0;
    uint32_t finalCount = 0;
    uint32_t i_req_idx = 0;

    ap_uint<64> Dwrite = 0;
    ap_uint<10> SRdata = 0;
    ap_uint<24> Dmeta_Idx = 0;
    // uint8_t dec_type = 0;
    // uint16_t runLength = 0;
    int kk = 0;

    #ifndef __SYNTHESIS__
        uint32_t debugVar = 0;
    #endif

    concat_IdxCnt:for( ;; )
    {
        #pragma HLS PIPELINE II = 1

        if(!inIdxStrm[0].empty() && !inIdxStrm[1].empty() && 
            !inIdxStrm[2].empty() && !inIdxStrm[3].empty() 
            // && !metaIdx.empty()
            )
        {
           
            prepFilIdx:for(int i = 0; i < 4; i++)
            {
                #pragma HLS UNROLL
                FilIdxBuf.range((i*16)+15,i*16) = inIdxStrm[i].read();
            }

            outIdxStrm.write(FilIdxBuf);
            eo_strm.write(true);           
            
            
            #ifndef __SYNTHESIS__
                debugVar += 1;
                // std::cout << "i_req_idx: " << i_req_idx << std::endl;
            #endif

        }
        else if(!numCnt.empty())
        {
            // finalCountStrm.write(finalCount);
            eo_strm.write(false);
            // Didx_Data[i_req_idx] = Dwrite;  //extra write incase of some elements of SR are left
            finalCount = numCnt.read();         //Filtered Rows Count
            FilterConf_port[2] = (_512b)(finalCount);
            
            #ifndef __SYNTHESIS__
                debugVar += 1;
                // std::cout << "Total eoWrites: " << debugVar << std::endl;
                // std::cout << "eo_strm is false" << std::endl;
                // std::cout << "finalCount: " << finalCount << std::endl;
                printf("finalCount: %d \n", finalCount);
            #endif


            break;

        } 
    }
}
 
//Data Writing
void store_all(tapa::istreams<_512b, 4>& inAllStrm,
                tapa::istream<ap_uint<24>>& store_writeCount,
                tapa::async_mmap<_512b> &OUT0_32b_8b,
                tapa::async_mmap<_512b> &OUT1_16b_8b,
                tapa::async_mmap<_512b> &OUT2_16b_8b,
                tapa::async_mmap<_512b> &OUT3_8b
            )
{

    uint32_t i_req_0 = 0, i_resp_0 = 0,
        i_req_1 = 0, i_resp_1 = 0,
        i_req_2 = 0, i_resp_2 = 0,
        i_req_3 = 0, i_resp_3 = 0;

    uint32_t write_count = 2147483647;    //max of 32bits, its max of 24bits in theory

    store_loop32:for( ; ; )
    {
        #pragma HLS pipeline II=1

        //------PORT0_32Bits _ 8bits--------
            // issue write requests
            if((!inAllStrm[0].empty()) &&
                (!OUT0_32b_8b.write_addr.full()) &&
                (!OUT0_32b_8b.write_data.full())
                ){
            OUT0_32b_8b.write_addr.try_write(i_req_0);
            OUT0_32b_8b.write_data.try_write(inAllStrm[0].read(nullptr));
            ++i_req_0;
            }

            // receive acks of write success
            if (!OUT0_32b_8b.write_resp.empty()) {
                i_resp_0 += unsigned(OUT0_32b_8b.write_resp.read(nullptr)) + 1;

                #ifndef __SYNTHESIS__
                    // std::cout << "i_resp_0: " << i_resp_0 << std::endl;
                #endif
            }


        //------PORT1_16Bits _ 8bits--------
            // issue write requests
            if ((!inAllStrm[1].empty()) &&
                (!OUT1_16b_8b.write_addr.full()) &&
                (!OUT1_16b_8b.write_data.full())
                ) {
            OUT1_16b_8b.write_addr.try_write(i_req_1);
            OUT1_16b_8b.write_data.try_write(inAllStrm[1].read(nullptr));
            ++i_req_1;
            }

            // receive acks of write success
            if (!OUT1_16b_8b.write_resp.empty()) {
                i_resp_1 += unsigned(OUT1_16b_8b.write_resp.read(nullptr)) + 1;
            }
        

        //------PORT2_16Bits _ 8bits--------
            // issue write requests
            if ((!inAllStrm[2].empty()) &&
                (!OUT2_16b_8b.write_addr.full()) &&
                (!OUT2_16b_8b.write_data.full())
                ) {
            OUT2_16b_8b.write_addr.try_write(i_req_2);
            OUT2_16b_8b.write_data.try_write(inAllStrm[2].read(nullptr));
            ++i_req_2;
            }

            // receive acks of write success
            if (!OUT2_16b_8b.write_resp.empty()) {
                i_resp_2 += unsigned(OUT2_16b_8b.write_resp.read(nullptr)) + 1;
            }

        //------PORT3_8Bits--------
            // issue write requests
            if ((!inAllStrm[3].empty()) &&
                (!OUT3_8b.write_addr.full()) &&
                (!OUT3_8b.write_data.full())
                ) {
            OUT3_8b.write_addr.try_write(i_req_3);
            OUT3_8b.write_data.try_write(inAllStrm[3].read(nullptr));
            ++i_req_3;
            }

            // receive acks of write success
            if (!OUT3_8b.write_resp.empty()) {
                i_resp_3 += unsigned(OUT3_8b.write_resp.read(nullptr)) + 1;
            }

        //End Loop Write

            if(!store_writeCount.empty())
            {
                write_count = store_writeCount.read() + 1;  //as there is one extra write in the last task
                // write_count += 1;   
                #ifndef __SYNTHESIS__
                    std::cout << "write_count: " << write_count << std::endl;
                #endif

            }

            #ifndef __SYNTHESIS__
                // std::cout << "i_resp_0: " << i_resp_0 << std::endl;
                // std::cout << "i_resp_1: " << i_resp_1 << std::endl;
                // std::cout << "i_resp_2: " << i_resp_2 << std::endl;
                // std::cout << "i_resp_3: " << i_resp_3 << std::endl;
            #endif

            if((i_resp_0 == write_count) && (i_resp_1 == write_count) && (i_resp_2 == write_count) && (i_resp_3 == write_count))
            {
                break;
            }
    }
}

void orc_proc(tapa::mmap<_512b> input_port, 
                tapa::mmap<_512b> FilterConf_port, 
                tapa::mmap<_512b> output_port0_32b_8b, 
                tapa::mmap<_512b> output_port1_16b_8b,
                tapa::mmap<_512b> output_port2_16b_8b,
                tapa::mmap<_512b> output_port3_8b,
                tapa::mmap<_512b> data_Idx,
                tapa::mmap<_512b> output_port4_Track,
                uint32_t data_count)
{
    //Data Reading
    tapa::stream<_512b, 8> outLstrm("outLstrm");

    //Orc Decomp
    tapa::stream<_512b, 16> outCstm("outCstm");         //32
    tapa::stream<ap_uint<20>, 16> outCMDstm("outCMDstm");
    tapa::stream<_512b, 24> outUCstm("outUCstm");          //256
    tapa::stream<ap_uint<20>, 24> outUCMDstm("outUCMDstm");
    tapa::stream<_512b, 16> outData("outData");
    tapa::stream<bool, 16> OutEoSdata("OutEoSdata");
    tapa::stream<uint8_t, 32> idxCombinerSend("idxCombinerSend");

    tapa::streams<bool, DMUL, 32> outCEBoSstrm("outCEBoSstrm");
    tapa::streams<_512b, DMUL, 32> outCdata("outCdata");
    tapa::streams<ap_uint<7>, DMUL, 32> outVdata("outVdata");
    tapa::streams<_512b, DMUL, 32> outDstm("outDstm");
    tapa::streams<ap_uint<8>, DMUL, 32> outVDStm("outVDStm");
    
    //Decomp to Decoder
    tapa::stream<_512b, 8> data_out("data_out");

    //Header Parsing
    tapa::stream<_2048b, 8> outAll_Lstrm("outAll_Lstrm");
    tapa::stream<_256b, 8> out_strmC_Track("out_strmC_Track");
    tapa::stream<_512b, 8> PA_DATA("PA_DATA");
    tapa::stream<uint32_t, 8> PA_meta_Data("PA_meta_Data");   
    tapa::stream<bool, 2> D_strm_e("D_strm_e");      //load to Data Sender
    tapa::streams<_512b, 4, 8> outAll_Pstrm("outAll_Pstrm");
    tapa::stream<uint64_t, 8> outSR_Pstrm("outSR_Pstrm");
    tapa::streams<_256b, 4, 8> meta_out("meta_out");
    tapa::stream<uint8_t, 8> SR_meta_out("SR_meta_out");
    tapa::stream<uint64_t, 16> All_meta_out("All_meta_out");
    tapa::stream<ap_uint<24>, 32> out_DA_Track("out_DA_Track");
    tapa::stream<bool, 2> MD_strm_e("MD_strm_e");    //Data Sender to Meta Aligner
    //Patch MetaData
    tapa::stream<uint64_t, 8> metaOutPA_Strm("metaOutPA_Strm");
    //Shift_Unzigzag and Direct Decoder
    tapa::streams<uint32_t, 4, 32> out_Cmeta_strm("out_Cmeta_strm");
    tapa::streams<_512b, 4, 8> out_all_Cstrm("out_all_Cstrm");
    tapa::streams<_512b, 4, 32> DIOut_Strm("DIOut_Strm");
    tapa::streams<_512b, 4, 8> PA_acc_strm("PA_acc_strm");
    tapa::streams<uint32_t, 4, 8> PA_acc_meta("PA_acc_meta");
    //Short Repeat
    tapa::stream<_320b, 32> SR_Dout("SR_Dout");
    //Patched Base
    tapa::streams<_512b, 4, 32> dataOutPA_Strm("dataOutPA_Strm");
    //Delta Decoder
    tapa::streams<_512b, 4, 32> MydataOut("MydataOut");
    tapa::streams<uint32_t, 2, 32> CarryOut_Strm("CarryOut_Strm");
    tapa::streams<_512b, 3, 32> deltaOut_Strm("deltaOut_Strm");
    tapa::stream<uint32_t, 32> CarryOut0_Strm1("CarryOut0_Strm1");
    tapa::stream<uint32_t, 32> CarryOut1_Strm1("CarryOut1_Strm1");
    tapa::stream<_512b, 32> dataOut_Strm1("dataOut_Strm1");
    tapa::stream<uint32_t, 32> CarryOut0_Strm2("CarryOut0_Strm2");
    tapa::streams<_512b, 4, 32> DEOut_Strm("DEOut_Strm");
    //Data Aligner
    tapa::streams<_512b, 4, 8> dataAlign_Out("dataAlign_Out");
    tapa::stream<ap_uint<24>, 8> metaBrStrm("metaBrStrm");
    //Bubble Remover
    tapa::streams<_512b, 4, 8> br_Strms("br_Strms");
    tapa::streams<bool, 4, 8> e_strm("e_strm");
    //Meta Writer
    tapa::stream<_128b, 8> meta_Writer_Out("meta_Writer_Out");
    tapa::stream<bool, 2> MDD_strm_e("MDD_strm_e");  //Meta Aligner to Meta Writer

    //Filter Config
    tapa::stream<uint64_t, 32> FinIdxStrm("FinIdxStrm");
    tapa::stream<bool, 32> e_confStrm("e_confStrm");  //concat_IdxCnt to FilterConfig
    tapa::streams<_512b, 4, 2> FilterConfigData("FilterConfigData");
    tapa::streams<uint16_t, 4, 32> FilterIdxData("FilterIdxData");
    tapa::streams<uint16_t, 4, 32> outIdxStrm("outIdxStrm");
    tapa::stream<ap_uint<24>, 2> store_writeCount("store_writeCount");
    tapa::stream<uint32_t, 2> PartialnumCnt("PartialnumCnt");
    //Data Filtering
    tapa::streams<_512b, 4, 32> outFilterStrm("outFilterStrm");
    tapa::streams<uint8_t, 4, 32> dataCntFD("dataCntFD");
    tapa::streams<bool, 4, 32> eFD_strm("eFD_strm"); 
    //Bubble Removing
    tapa::streams<_1024b, 2, 8> outBR0_Strm("outBR0_Strm");  
    tapa::streams<uint8_t, 2, 8> outData_count("outData_count");
    tapa::streams<bool, 2, 8> eBR0out_strm("eBR0out_strm");
    tapa::stream<_2048b, 8> outBR1_Strm("outBR1_Strm");
    tapa::stream<bool, 8> eBR1out_strm("eBR1out_strm");
    tapa::stream<uint8_t, 8> outBR1Data_count("outBR1Data_count");

    //Data Writing
    tapa::streams<_512b, 4, 8> BR_WR_OUT("BR_WR_OUT");

  tapa::task()
    /////--------Data Reading--------//////
    //Data Reading
    .invoke(mmap2s, input_port, outLstrm, data_count)

    /////--------Orc Decompression--------//////
    .invoke(orcDecompHeadProc, outLstrm, outCstm, outUCstm, outCMDstm, outUCMDstm, data_count)
    .invoke(orcDecompSender, outCstm, outCMDstm, idxCombinerSend, outCEBoSstrm, outCdata, outVdata)                                 //inc depth 
    .invoke<tapa::join, DMUL>(orcDecompTop, outCdata, outVdata, outCEBoSstrm, outDstm, outVDStm)
    .invoke(orcDecompCombiner, outUCMDstm, idxCombinerSend, outUCstm, outDstm, outVDStm, outData, OutEoSdata)

    /////--------Orc Decomp to Decoder Connector--------//////
    .invoke(orcDDconnector, outData, OutEoSdata, data_out)

    /////--------Orc Decoder--------//////
    //Header Parsing
    .invoke(orcDecHeadProc, data_out, outAll_Lstrm, out_strmC_Track, PA_DATA, PA_meta_Data, D_strm_e)
    .invoke(orcDecSender, outAll_Lstrm, out_strmC_Track, D_strm_e, outAll_Pstrm, outSR_Pstrm, meta_out, SR_meta_out, All_meta_out, out_DA_Track, MD_strm_e)
    //Patch Metadata
    .invoke<tapa::detach>(PA_metadata, PA_DATA, PA_meta_Data, metaOutPA_Strm)
    .invoke(metadata_aligner, metaOutPA_Strm, MD_strm_e, All_meta_out, MDD_strm_e, meta_Writer_Out)
    //Shift_Unzigzag and Direct Decoder
    .invoke<tapa::detach>(common_DI_compute, outAll_Pstrm[0], meta_out[0], out_Cmeta_strm[0], out_all_Cstrm[0], DIOut_Strm[0], PA_acc_strm[0], PA_acc_meta[0], 0)
    .invoke<tapa::detach>(common_DI_compute, outAll_Pstrm[1], meta_out[1], out_Cmeta_strm[1], out_all_Cstrm[1], DIOut_Strm[1], PA_acc_strm[1], PA_acc_meta[1], 1)
    .invoke<tapa::detach>(common_DI_compute, outAll_Pstrm[2], meta_out[2], out_Cmeta_strm[2], out_all_Cstrm[2], DIOut_Strm[2], PA_acc_strm[2], PA_acc_meta[2], 2)
    .invoke<tapa::detach>(common_DI_compute, outAll_Pstrm[3], meta_out[3], out_Cmeta_strm[3], out_all_Cstrm[3], DIOut_Strm[3], PA_acc_strm[3], PA_acc_meta[3], 3)
    //Short Repeat
    .invoke<tapa::detach>(SRcompute, outSR_Pstrm, SR_meta_out, SR_Dout)
    //Patched Based
    .invoke<tapa::detach, 4>(PA_data_proc, PA_acc_strm, PA_acc_meta, dataOutPA_Strm)
    //Delta Decoder
    .invoke<tapa::detach>(delta_proc0, out_all_Cstrm[0], MydataOut[0], CarryOut_Strm[0])
    .invoke<tapa::detach>(delta_proc1, out_all_Cstrm[1], deltaOut_Strm[0])
    .invoke<tapa::detach>(delta_proc0, out_all_Cstrm[2], deltaOut_Strm[1], CarryOut_Strm[1])
    .invoke<tapa::detach>(delta_proc1, out_all_Cstrm[3], deltaOut_Strm[2])
    .invoke<tapa::detach>(delta_proc2, deltaOut_Strm[0], CarryOut_Strm[0], MydataOut[1], CarryOut0_Strm1, CarryOut1_Strm1)
    .invoke<tapa::detach>(delta_proc4, deltaOut_Strm[2], CarryOut_Strm[1], dataOut_Strm1)
    .invoke<tapa::detach>(delta_proc4, deltaOut_Strm[1], CarryOut0_Strm1, MydataOut[2])
    .invoke<tapa::detach>(delta_proc3, dataOut_Strm1, CarryOut1_Strm1, MydataOut[3], CarryOut0_Strm2)
    .invoke<tapa::detach>(delta_proc5, MydataOut, out_Cmeta_strm, CarryOut0_Strm2, DEOut_Strm)
    //Data Aligner
    .invoke(orc_dataAligner, out_DA_Track, DEOut_Strm, DIOut_Strm, dataOutPA_Strm, SR_Dout, dataAlign_Out, metaBrStrm)
    //Bubble remover
    .invoke(orc_BR, dataAlign_Out, metaBrStrm, br_Strms, e_strm)

    /////--------Orc Filter--------//////
    //Filter Data
    .invoke(Filter_Config, FilterConf_port, FinIdxStrm, e_confStrm, data_Idx, FilterConfigData, FilterIdxData)    //read
    // .invoke(orcFilterTop, FilterConfigData, br_Strms, FilterIdxData, e_strm, outIdxStrm, store_writeCount, PartialnumCnt, BR_WR_OUT)    
    //Filtering
    .invoke(orcFilterData, FilterConfigData[0], br_Strms[0], FilterIdxData[0], e_strm[0], outFilterStrm[0], outIdxStrm[0], dataCntFD[0], eFD_strm[0], 0)
    .invoke(orcFilterData, FilterConfigData[1], br_Strms[1], FilterIdxData[1], e_strm[1], outFilterStrm[1], outIdxStrm[1], dataCntFD[1], eFD_strm[1], 1)
    .invoke(orcFilterData, FilterConfigData[2], br_Strms[2], FilterIdxData[2], e_strm[2], outFilterStrm[2], outIdxStrm[2], dataCntFD[2], eFD_strm[2], 2)
    .invoke(orcFilterData, FilterConfigData[3], br_Strms[3], FilterIdxData[3], e_strm[3], outFilterStrm[3], outIdxStrm[3], dataCntFD[3], eFD_strm[3], 3)
    
    //Bubble Removing
    .invoke(filterBR1, outFilterStrm, eFD_strm, dataCntFD, outBR0_Strm[0], outData_count[0], eBR0out_strm[0])
    .invoke(filterBR1, outFilterStrm, eFD_strm, dataCntFD, outBR0_Strm[1], outData_count[1], eBR0out_strm[1])
    .invoke(filterBR2, outBR0_Strm, eBR0out_strm, outData_count, outBR1_Strm, eBR1out_strm, outBR1Data_count)
    .invoke(filterBR3, outBR1_Strm, eBR1out_strm, outBR1Data_count, store_writeCount, PartialnumCnt, BR_WR_OUT)

    .invoke(concat_IdxCnt, PartialnumCnt, outIdxStrm, FinIdxStrm, e_confStrm, FilterConf_port)

    /////--------Data Writing--------//////
    //Data Writing
    .invoke(store_all, BR_WR_OUT, store_writeCount, output_port0_32b_8b, output_port1_16b_8b, output_port2_16b_8b, output_port3_8b)    //write
    .invoke(Meta_Writer, meta_Writer_Out, MDD_strm_e, output_port4_Track);

}
