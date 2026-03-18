#ifndef ZLIB_TAPA_H
#define ZLIB_TAPA_H

template <class SIZE_DT = uint8_t>
void lzProcessingUnit(tapa::istream<ap_uint<17> >& inStream,
                        tapa::istream<bool>& inDEos,
                        tapa::ostream<bool>& outLMDEos,
                        tapa::ostream<bool>& outLUDEos,
                      tapa::ostream<SIZE_DT>& litLenStream,
                      tapa::ostream<SIZE_DT>& matchLenStream,
                      tapa::ostream<ap_uint<16> >& offsetStream,
                      tapa::ostream<ap_uint<10> >& outStream) {
    
    const int c_maxLitLen = 128;

    while(inDEos.read())
    {
        #ifndef __SYNTHESIS__
            // std::cout << "Block Start" << std::endl;
        #endif
        outLMDEos.write(true);
        outLUDEos.write(true);
        ap_uint<17> inValue, nextValue;
        uint16_t offset = 0;
        uint16_t matchLen = 0;
        uint8_t litLen = 0;
        uint8_t outLitLen = 0;
        ap_uint<10> lit = 0;

        nextValue = inStream.read();
        #ifndef __SYNTHESIS__
            // std::cout << "nextValue: " << std::hex << nextValue << std::endl;
        #endif
        bool eosFlag = nextValue.range(0, 0);
        bool lastLiteral = false;
        bool isLiteral = true;
    lzProcessing:
        for (; eosFlag == false;) {
    #pragma HLS PIPELINE II = 1
            inValue = nextValue;
            nextValue = inStream.read();
            eosFlag = nextValue.range(0, 0);
            #ifndef __SYNTHESIS__
                // std::cout << "inValue: " << std::hex << inValue << std::endl;
                // std::cout << "nextValue: " << std::hex << nextValue << std::endl;
            #endif

            bool outFlag, outStreamFlag;
            if (inValue.range(16, 9) == 0xFF && isLiteral) {
                #ifndef __SYNTHESIS__
                    // std::cout << "runA " << std::endl;
                #endif
                outStreamFlag = true;
                outLitLen = litLen + 1;
                if (litLen == c_maxLitLen - 1) {
                    #ifndef __SYNTHESIS__
                        // std::cout << "runAA " << std::endl;
                    #endif
                    outFlag = true;
                    matchLen = 0;
                    offset = 1; // dummy value
                    litLen = 0;
                } else {
                    #ifndef __SYNTHESIS__
                        // std::cout << "runAB " << std::endl;
                    #endif
                    outFlag = false;
                    litLen++;
                }
            } else {
                #ifndef __SYNTHESIS__
                    // std::cout << "runZ " << std::endl;
                #endif
                if (isLiteral) {
                    #ifndef __SYNTHESIS__
                        // std::cout << "runZA " << std::endl;
                    #endif
                    matchLen = inValue.range(16, 1);
                    isLiteral = false;
                    outFlag = false;
                    outStreamFlag = false;
                } else {
                    #ifndef __SYNTHESIS__
                        // std::cout << "runZB " << std::endl;
                    #endif
                    offset = inValue.range(16, 1);
                    isLiteral = true;
                    outFlag = true;
                    outLitLen = litLen;
                    litLen = 0;
                    outStreamFlag = false;
                }
            }

            #ifndef __SYNTHESIS__
                // std::cout << "eosFlag: " << eosFlag << std::endl;
                // std::cout << "outStreamFlag: " << outStreamFlag << std::endl;
                // std::cout << "outFlag: " << outFlag << std::endl;
                // std::cout << "matchLen: " << matchLen << std::endl;
                // std::cout << "offset: " << offset << std::endl;
                // std::cout << "litLen: " << (uint16_t)litLen << std::endl;
                // std::cout << "outLitLen: " << (uint16_t)outLitLen << std::endl;
                // std::cout << "isLiteral: " << isLiteral << std::endl;
                // std::cout << "lastLiteral: " << lastLiteral << std::endl;
            #endif


            if (outStreamFlag) {
                lit.range(9, 2) = inValue.range(8, 1);
                if (nextValue.range(16, 9) == 0xFF) {
                    lit.range(1, 0) = 0;
                } else {
                    lit.range(1, 0) = 1;
                }
                lastLiteral = true;
                outStream << lit;
            } else if (lastLiteral) {
                outStream << 3;
                lastLiteral = false;
            }

            #ifndef __SYNTHESIS__
                // std::cout << "lit: " << lit << std::endl;
            #endif

            if (outFlag) {
                litLenStream << outLitLen;
                offsetStream << offset;
                matchLenStream << matchLen;
            }
        }

        if (litLen) {
            litLenStream << litLen;
            offsetStream << 0;
            matchLenStream << 0;
        }

        // Terminate condition
        outStream << 2;
        offsetStream << 0;
        matchLenStream << 0;
        litLenStream << 0;
    }
    outLMDEos.write(false);
    outLUDEos.write(false);
}


template <int PARALLEL_BYTES>
void lzLiteralUpsizer(tapa::istream<ap_uint<10> >& inStream, 
                        tapa::istream<bool>& inDEos,
                        tapa::ostream<ap_uint<PARALLEL_BYTES * 8> >& litStream) {
    const uint8_t c_parallelBit = PARALLEL_BYTES * 8;
    // const uint8_t c_maxLitLen = 128;
    while(inDEos.read())
    {
        ap_uint<c_parallelBit> outBuffer;
        ap_uint<4> idx = 0;
        ap_uint<2> status = 0;
        ap_uint<10> val;
        bool done = false;
    lzliteralUpsizer:
        while (status != 2) {
    #pragma HLS PIPELINE II = 1
            status = 0;
            val = inStream.read();
            status = val.range(1, 0);
            outBuffer.range((idx + 1) * 8 - 1, idx * 8) = val.range(9, 2);
            idx++;

            if ((status & 1) || (idx == 8)) {
                if (status != 3) {
                    litStream << outBuffer;
                }
                idx = 0;
            }

            #ifndef __SYNTHESIS__
                // std::cout << "outBuffer: " << std::hex << outBuffer << std::dec << std::endl;
            #endif

        }
        if (idx > 1) {
            litStream << outBuffer;
            idx = 0;
        }
        #ifndef __SYNTHESIS__
            // std::cout << "outBuffer: " << std::hex << outBuffer << std::dec << std::endl;
        #endif
    }
}

template <int PARALLEL_BYTES, int HISTORY_SIZE, class SIZE_DT = uint8_t, class SIZE_OFFSET = ap_uint<16> >
void lzMultiByteDecompress(tapa::istream<SIZE_DT>& litlenStream,
                           tapa::istream<ap_uint<PARALLEL_BYTES * 8> >& litStream,
                           tapa::istream<SIZE_OFFSET>& offsetStream,
                           tapa::istream<SIZE_DT>& matchlenStream,
                           tapa::istream<bool>& inDEos,
                           tapa::ostream<ap_uint<(PARALLEL_BYTES * 8) + PARALLEL_BYTES> >& outStream) {
    const uint8_t c_parallelBit = PARALLEL_BYTES * 8;
    const uint8_t c_lowOffset = 8 * PARALLEL_BYTES;
    const uint8_t c_veryLowOffset = 2 * PARALLEL_BYTES;

    const uint16_t c_ramHistSize = HISTORY_SIZE / PARALLEL_BYTES;
    const uint8_t c_regHistSize = (2 * c_lowOffset) / PARALLEL_BYTES;

    enum lzDecompressStates { READ_LIT_LEN, WRITE_LITERAL, READ_OFFSET, READ_MATCH, NO_OP };

    ap_uint<c_parallelBit> ramHistory[2][c_ramHistSize];
#pragma HLS dependence variable = ramHistory inter false
#pragma HLS BIND_STORAGE variable = ramHistory type = RAM_2P impl = URAM
#pragma HLS ARRAY_PARTITION variable = ramHistory dim = 1 complete

    ap_uint<c_parallelBit> regHistory[2][c_regHistSize];
// full partition  to infer as reg
#pragma HLS ARRAY_PARTITION variable = regHistory dim = 0 complete

    while(inDEos.read())
    {
        enum lzDecompressStates next_state = READ_LIT_LEN; // start from Read Literal Length

    //     ap_uint<c_parallelBit> ramHistory[2][c_ramHistSize];
    // #pragma HLS dependence variable = ramHistory inter false
    // #pragma HLS BIND_STORAGE variable = ramHistory type = RAM_2P impl = URAM
    // #pragma HLS ARRAY_PARTITION variable = ramHistory dim = 1 complete

    //     ap_uint<c_parallelBit> regHistory[2][c_regHistSize];
    // // full partition  to infer as reg
    // #pragma HLS ARRAY_PARTITION variable = regHistory dim = 0 complete

        SIZE_DT lit_len = 0;
        SIZE_DT orig_lit_len = 0;
        uint32_t output_cnt = 0;
        SIZE_OFFSET match_loc = 0;
        SIZE_DT match_len = 0;
        SIZE_OFFSET write_idx = 0;
        SIZE_OFFSET output_index = 0;

        ap_uint<c_parallelBit> outValue;

        uint8_t incr_output_index = 0;
        bool outStreamFlag = false;

        SIZE_OFFSET offset = 0;
        ap_uint<c_parallelBit> outStreamValue = 0;
        ap_uint<2 * PARALLEL_BYTES * 8> output_window;
        uint8_t parallelBits = 0;

        bool matchDone = false;
        uint16_t read_idx = match_loc / PARALLEL_BYTES;
        uint16_t byte_loc = (match_loc % PARALLEL_BYTES) % PARALLEL_BYTES;

    lz_decompress:
        for (; matchDone == false;) {
    #pragma HLS PIPELINE II = 1
            ap_uint<2 * c_parallelBit> localValue;
            ap_uint<c_parallelBit> lowValue, highValue;

            // always reading to make better timing
            ap_uint<c_parallelBit> lowValueReg = regHistory[0][(read_idx + 0) % c_regHistSize];
            ap_uint<c_parallelBit> highValueReg = regHistory[1][(read_idx + 1) % c_regHistSize];
            ap_uint<c_parallelBit> lowValueRam = ramHistory[0][(read_idx + 0) % c_ramHistSize];
            ap_uint<c_parallelBit> highValueRam = ramHistory[1][(read_idx + 1) % c_ramHistSize];

            if (offset < c_lowOffset) {
                lowValue = lowValueReg;
                highValue = highValueReg;
            } else {
                lowValue = lowValueRam;
                highValue = highValueRam;
            }

            localValue.range(c_parallelBit - 1, 0) = lowValue;
            localValue.range(2 * c_parallelBit - 1, c_parallelBit) = highValue;

            if (next_state == READ_LIT_LEN) {
                incr_output_index = 0;
                orig_lit_len = litlenStream.read();
                lit_len = orig_lit_len;
                if (lit_len) {
                    next_state = WRITE_LITERAL;
                } else {
                    next_state = READ_OFFSET;
                }
                output_cnt += lit_len;
            } else if (next_state == WRITE_LITERAL) {
                outValue = litStream.read();
                SIZE_DT localLitLen = lit_len;
                if (localLitLen <= PARALLEL_BYTES) {
                    incr_output_index = lit_len;
                    lit_len = 0;
                    offset = offsetStream.read();
                    match_len = matchlenStream.read();
                    match_loc = output_cnt - offset;
                    if (orig_lit_len == 0 && match_len == 0) {
                        matchDone = true;
                    } else if (match_len == 0) {
                        next_state = READ_LIT_LEN;
                    } else if ((offset > 0) & (offset < c_veryLowOffset)) {
                        parallelBits = 1;
                        if (offset < PARALLEL_BYTES) {
                            next_state = NO_OP;
                        } else {
                            next_state = READ_MATCH;
                        }
                    } else {
                        parallelBits = PARALLEL_BYTES;
                        next_state = READ_MATCH;
                    }
                    output_cnt += match_len;
                } else {
                    incr_output_index = PARALLEL_BYTES;
                    lit_len -= PARALLEL_BYTES;
                    next_state = WRITE_LITERAL;
                }
            } else if (next_state == READ_OFFSET) {
                incr_output_index = 0;
                offset = offsetStream.read();
                match_len = matchlenStream.read();
                match_loc = output_cnt - offset;
                if (orig_lit_len == 0 && match_len == 0) {
                    matchDone = true;
                } else if (match_len == 0) {
                    next_state = READ_LIT_LEN;
                } else if ((offset > 0) & (offset < c_veryLowOffset)) {
                    parallelBits = 1;
                    next_state = READ_MATCH;
                } else {
                    parallelBits = PARALLEL_BYTES;
                    next_state = READ_MATCH;
                }
                output_cnt += match_len;
            } else if (next_state == READ_MATCH) {
                outValue = localValue >> (byte_loc * 8);
                SIZE_DT localMatchLen = match_len;
                if (match_len <= parallelBits) {
                    incr_output_index = match_len;
                    match_loc += match_len;
                    match_len = 0;
                    orig_lit_len = litlenStream.read();
                    lit_len = orig_lit_len;
                    if (lit_len) {
                        next_state = WRITE_LITERAL;
                    } else {
                        next_state = READ_OFFSET;
                    }
                    output_cnt += lit_len;
                } else {
                    incr_output_index = parallelBits;
                    match_loc += parallelBits;
                    match_len -= parallelBits;
                    next_state = READ_MATCH;
                }
            } else if (next_state == NO_OP) {
                incr_output_index = 0;
                // Adding NO_OP as workaround for low offset case as
                // for very low offset case, results are not matching
                next_state = READ_MATCH;
            } else {
                assert(0);
            }

            read_idx = match_loc / PARALLEL_BYTES;
            byte_loc = (match_loc % PARALLEL_BYTES) % PARALLEL_BYTES;

            output_window.range((output_index + PARALLEL_BYTES) * 8 - 1, output_index * 8) = outValue;
            output_index += incr_output_index;

            uint8_t localOutputIdx = output_index - PARALLEL_BYTES;
            bool outputIdxFlag = ((output_index >= PARALLEL_BYTES));

            outStreamValue = output_window.range(c_parallelBit - 1, 0);
            regHistory[0][write_idx % c_regHistSize] = outStreamValue;
            regHistory[1][write_idx % c_regHistSize] = outStreamValue;
            ramHistory[0][write_idx % c_ramHistSize] = outStreamValue;
            ramHistory[1][write_idx % c_ramHistSize] = outStreamValue;

            bool outStreamFlag = false;
            if (outputIdxFlag) {
                write_idx++;
                output_window >>= PARALLEL_BYTES * 8;
                output_index = localOutputIdx;
                outStreamFlag = true;
            }

            if (outStreamFlag) {
                ap_uint<(PARALLEL_BYTES * 8) + PARALLEL_BYTES> val;
                val.range((PARALLEL_BYTES * 8) + PARALLEL_BYTES - 1, PARALLEL_BYTES) = outStreamValue;
                val.range(PARALLEL_BYTES - 1, 0) = -1;
                outStream << val;

                #ifndef __SYNTHESIS__
                    // std::cout << "val: " << std::hex << val << std::dec << std::endl;
                    // std::cout << "outStreamValue: " << std::hex << outStreamValue << std::dec << std::endl;
                #endif
            }

            

        }

        // Write out if there is remaining left over data in output buffer
        // to outStream
        if (output_index) {
            outStreamValue = output_window.range(c_parallelBit - 1, 0);
            ap_uint<(PARALLEL_BYTES * 8) + PARALLEL_BYTES> val;
            val.range((PARALLEL_BYTES * 8) + PARALLEL_BYTES - 1, PARALLEL_BYTES) = outStreamValue;
            val.range(PARALLEL_BYTES - 1, 0) = ((1 << output_index) - 1);
            outStream << val;
            #ifndef __SYNTHESIS__
                // std::cout << "val: " << std::hex << val << std::dec << std::endl;
                // std::cout << "outStreamValue: " << std::hex << outStreamValue << std::dec << std::endl;
            #endif
        }
        outStream << 0;
    }
}


void loadBitStream(bitBufferType& bitbuffer,
                   ap_uint<6>& bits_cntr,
                   tapa::istream<ap_uint<16> >& inStream,
                   tapa::istream<bool>& inEos,
                   bool& done) {
#pragma HLS INLINE off
    while (bits_cntr < 32 && (done == false)) {
    loadBitStream:
        uint16_t tmp_dt = (uint16_t)inStream.read();
        bitbuffer += (bitBufferType)(tmp_dt) << bits_cntr;
        done = inEos.read();
        bits_cntr += 16;
    }
}

void discardBitStream(bitBufferType& bitbuffer, ap_uint<6>& bits_cntr, ap_uint<6> n_bits) {
    bitbuffer >>= n_bits;
    bits_cntr -= n_bits;
}

template <typename T>
T reg(T d) {
#pragma HLS PIPELINE II = 1
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INLINE off
    return d;
}

inline uint8_t huffmanBytegenStatic(bitBufferType& _bitbuffer,
                                    ap_uint<6>& bits_cntr,
                                    tapa::ostream<ap_uint<17> >& outStream,
                                    tapa::istream<bool>& inEos,
                                    tapa::istream<ap_uint<16> >& inStream,
                                    const uint8_t* array_codes_op,
                                    const uint8_t* array_codes_bits,
                                    const uint16_t* array_codes_val,
                                    bool& done) {
#pragma HLS INLINE
    uint16_t used = 512;
    uint16_t lit_mask = 511;
    uint16_t dist_mask = 31;
    bitBufferType bitbuffer = reg<bitBufferType>(_bitbuffer);
    ap_uint<10> lidx = bitbuffer & lit_mask;
    uint8_t current_op = reg<uint8_t>(array_codes_op[lidx]);
    uint8_t current_bits = reg<uint8_t>(array_codes_bits[lidx]);
    uint16_t current_val = reg<uint16_t>(array_codes_val[lidx]);
    bool is_length = true;
    ap_uint<17> tmpVal;
    uint8_t ret = 0;

    bool huffDone = false;
ByteGenStatic:
    for (; !huffDone;) {
#pragma HLS PIPELINE II = 1
        ap_uint<4> len1 = current_bits;
        ap_uint<4> len2 = 0;
        ap_uint<4> ml_op = current_op;
        uint64_t bitbuffer1 = bitbuffer >> current_bits;
        ap_uint<9> bitbuffer3 = bitbuffer >> current_bits;
        uint64_t bitbuffer2 = bitbuffer >> (current_bits + ml_op);
        bits_cntr -= current_bits;

        if (current_op == 0) {
            tmpVal.range(8, 1) = (uint8_t)(current_val);
            tmpVal.range(16, 9) = 0XFF;
            tmpVal.range(0, 0) = 0;
            outStream << tmpVal;
            lidx = bitbuffer3;
            is_length = true;
        } else if (current_op & 16) {
            uint16_t len = (uint16_t)(current_val);
            len += (uint16_t)bitbuffer1 & ((1 << ml_op) - 1);
            len2 = ml_op;
            bits_cntr -= ml_op;
            tmpVal.range(16, 1) = len;
            tmpVal.range(0, 0) = 0;
            outStream << tmpVal;
            uint16_t array_offset = (is_length) ? used : 0;
            ap_uint<9> mask = (is_length) ? dist_mask : lit_mask;
            lidx = array_offset + (bitbuffer2 & mask);
            is_length = !(is_length);
        } else if (current_op & 32) {
            if (is_length) {
                ret = blockStatus::PENDING;
            } else {
                ret = blockStatus::FINISH;
            }
            huffDone = true;
        }

        if ((done == true) && (bits_cntr < 32)) {
            ret = blockStatus::FINISH;
            huffDone = true;
        }
        if ((bits_cntr < 32) && (done == false)) {
            uint16_t inValue = inStream.read();
            done = inEos.read();
            bitbuffer = (bitbuffer >> (len1 + len2)) | (bitBufferType)(inValue) << bits_cntr;
            bits_cntr += (uint8_t)16;
        } else {
            bitbuffer >>= (len1 + len2);
        }
        current_op = array_codes_op[lidx];
        current_bits = array_codes_bits[lidx];
        current_val = array_codes_val[lidx];
    }
    _bitbuffer = bitbuffer;
    return ret;
}


void code_generator_array_dyn(
    uint8_t curr_table, uint16_t* lens, ap_uint<9> codes, uint32_t* table, uint32_t* table_extra, uint32_t bits) {
/**
 * @brief This module regenerates the code values based on bit length
 * information present in block preamble. Output generated by this module
 * presents operation, bits and value for each literal, match length and
 * distance.
 *
 * @param curr_table input current module to process i.e., literal or
 * distance table etc
 * @param lens input bit length information
 * @param codes input number of codes
 * @param table_op output operation per active symbol (literal or distance)
 * @param table_bits output bits to process per symbol (literal or distance)
 * @param table_val output value per symbol (literal or distance)
 * @param bits represents the start of the table
 * @param used presents next valid entry in table
 */
#pragma HLS INLINE REGION
    uint16_t sym = 0;
    uint16_t min, max = 0;
    uint16_t extra_idx = 0;
    uint32_t root = bits;
    uint16_t curr;
    uint16_t drop;
    uint16_t huff = 0;
    uint16_t incr;
    int16_t fill;
    uint16_t low;
    uint16_t mask;

    const uint16_t c_maxbits = 15;
    uint8_t code_data_op = 0;
    uint8_t code_data_bits = 0;
    uint16_t code_data_val = 0;

    uint8_t* nptr_op;
    uint8_t* nptr_bits;
    uint16_t* nptr_val;
    uint32_t* nptr;
    uint32_t* nptr_extra;

    const uint16_t* base;
    const uint16_t* extra;
    uint16_t match;
    uint16_t count[c_maxbits + 1] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#pragma HLS ARRAY_PARTITION variable = count

    uint16_t offs[c_maxbits + 1];
#pragma HLS ARRAY_PARTITION variable = offs

    uint16_t codeBuffer[512];
#pragma HLS DEPENDENCE false inter variable = codeBuffer

    const uint16_t lbase[32] = {3,  4,  5,  6,  7,  8,  9,  10,  11,  13,  15,  17,  19,  23, 27, 31,
                                35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0,  0,  0};
    const uint16_t lext[32] = {16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18,  18,
                               19, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 16, 77, 202, 0};
    const uint16_t dbase[32] = {1,    2,    3,    4,    5,    7,     9,     13,    17,  25,   33,
                                49,   65,   97,   129,  193,  257,   385,   513,   769, 1025, 1537,
                                2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0,   0};
    const uint16_t dext[32] = {16, 16, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22,
                               23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 64, 64};
cnt_lens:
    for (ap_uint<9> i = 0; i < codes; i++) {
#pragma HLS PIPELINE II = 1
#pragma HLS UNROLL FACTOR = 2
        uint16_t val = lens[i];
        if (val > max) {
            max = val;
        }
        count[val]++;
    }

min_loop:
    for (min = 1; min < max; min++) {
#pragma HLS PIPELINE II = 1
        if (count[min] != 0) break;
    }

    int left = 1;

    offs[1] = 0;
offs_loop:
    for (uint16_t i = 1; i < c_maxbits; i++)
#pragma HLS PIPELINE II = 1
        offs[i + 1] = offs[i] + count[i];

codes_loop:
    for (ap_uint<9> i = 0; i < codes; i++) {
#pragma HLS DEPENDENCE false inter variable = codeBuffer
#pragma HLS PIPELINE II = 1
#pragma HLS UNROLL FACTOR = 2
        if (lens[i] != 0) codeBuffer[offs[lens[i]]++] = (uint16_t)i;
    }

    switch (curr_table) {
        case 1:
            base = extra = codeBuffer;
            match = 20;
            break;
        case 2:
            base = lbase;
            extra = lext;
            match = 257;
            break;
        case 3:
            base = dbase;
            extra = dext;
            match = 0;
    }

    uint16_t len = min;

    nptr = table;
    nptr_extra = table_extra;

    curr = root;
    drop = 0;
    low = 65535;   //(uint32_t)(-1);
    mask = (1 << root) - 1;
    bool is_extra = false;

code_gen:
    for (;;) {
        code_data_bits = (uint8_t)(len - drop);

        if (codeBuffer[sym] + 1 < match) {
            code_data_op = (uint8_t)0;
            code_data_val = codeBuffer[sym];
        } else if (codeBuffer[sym] >= match) {
            code_data_op = (uint8_t)(extra[codeBuffer[sym] - match]);
            code_data_val = base[codeBuffer[sym] - match];
        } else {
            code_data_op = (uint8_t)(96);
            code_data_val = 0;
        }

        incr = 1 << (len - drop);
        fill = 1 << curr;
        min = fill;

        uint32_t code_val = ((uint32_t)code_data_op << 24) | ((uint32_t)code_data_bits << 16) | code_data_val;
        uint16_t fill_itr = fill / incr;
        uint16_t fill_idx = huff >> drop;
    fill:
        for (uint16_t i = 0; i < fill_itr; i++) {
#pragma HLS DEPENDENCE false inter variable = nptr
#pragma HLS DEPENDENCE false inter variable = nptr_extra
#pragma HLS PIPELINE II = 1
#pragma HLS UNROLL FACTOR = 2
            if (is_extra) {
                nptr_extra[fill_idx] = code_val;
            } else {
                nptr[fill_idx] = code_val;
            }

            fill_idx += incr;
        }

        fill = 0;

        incr = 1 << (len - 1);

        while (huff & incr) incr >>= 1;

        if (incr != 0) {
            huff &= incr - 1;
            huff += incr;
        } else
            huff = 0;

        sym++;

        if (--(count[len]) == 0) {
            if (len == max) break;
            len = lens[codeBuffer[sym]];
        }

        if (len > root && (huff & mask) != low) {
            if (drop == 0) {
                drop = root;
                min = 0;
                is_extra = true;
            }

            extra_idx += min;
            nptr_extra += min;
            curr = len - drop;
            left = (int)(1 << curr);

            uint16_t sum = curr + drop;
        left:
            for (int i = curr; i + drop < max; i++, curr++) {
#pragma HLS PIPELINE II = 1
#pragma HLS UNROLL FACTOR = 2
                left -= count[sum];
                if (left <= 0) break;
                left <<= 1;
                sum++;
            }

            low = huff & mask;
            table[low] = ((uint32_t)curr << 24) | ((uint32_t)root << 16) | extra_idx;
        }
    }
}

uint8_t huffmanBytegen(bitBufferType& _bitbuffer,
                       ap_uint<6>& bits_cntr,
                       tapa::ostream<ap_uint<17> >& outStream,
                       tapa::istream<bool>& inEos,
                       tapa::istream<ap_uint<16> >& inStream,
                       const uint32_t* array_codes,
                       const uint32_t* array_codes_extra,
                       const uint32_t* array_codes_dist,
                       const uint32_t* array_codes_dist_extra,
                       bool& done) {
#pragma HLS INLINE
    uint16_t lit_mask = 511; // Adjusted according to 8 bit
    uint16_t dist_mask = 511;
    bitBufferType bitbuffer = reg<bitBufferType>(_bitbuffer);
    //    bitBufferType bitbuffer = _bitbuffer;
    ap_uint<9> lidx = bitbuffer;
    ap_uint<9> lidx1;
    ap_uint<32> current_array_val = reg<ap_uint<32> >(array_codes[lidx]);
    uint8_t current_op = current_array_val.range(31, 24);
    uint8_t current_bits = current_array_val.range(23, 16);
    uint16_t current_val = current_array_val.range(15, 0);
    bool is_length = true;
    ap_uint<32> tmpVal;
    uint8_t ret = 0;
    bool dist_extra = false;
    bool len_extra = false;

    bool huffDone = false;
ByteGen:
    for (; !huffDone;) {
#pragma HLS PIPELINE II = 1
        ap_uint<4> len1 = current_bits;
        ap_uint<4> len2 = 0;
        ap_uint<4> ml_op = current_op;
        uint8_t current_op1 = (current_op == 0 || current_op >= 64) ? 1 : current_op;
        ap_uint<64> bitbuffer1 = bitbuffer >> current_bits;
        ap_uint<9> bitbuffer3 = bitbuffer >> current_bits;
        lidx1 = bitbuffer1.range(current_op1 - 1, 0) + current_val;
        ap_uint<9> bitbuffer2 = bitbuffer.range(current_bits + ml_op + 8, current_bits + ml_op);
        bits_cntr -= current_bits;
        dist_extra = false;
        len_extra = false;

        if (current_op == 0) {
            tmpVal.range(8, 1) = (uint8_t)(current_val);
            tmpVal.range(16, 9) = 0xFF;
            tmpVal.range(0, 0) = 0;
            // std::cout << (char)(current_val);
            outStream << tmpVal;
            lidx = bitbuffer3;
            is_length = true;
            #ifndef __SYNTHESIS__
                // std::cout << "runA" << std::endl;
                // std::cout << "tmpVal: " << std::hex << tmpVal << std::dec << std::endl;
                // std::cout << "bits_cntr: " << bits_cntr << std::endl;
            #endif

        } else if (current_op & 16) {
            #ifndef __SYNTHESIS__
                // std::cout << "runB" << std::endl;
            #endif
            uint16_t len = (uint16_t)(current_val);
            len += (uint16_t)bitbuffer1 & ((1 << ml_op) - 1);
            len2 = ml_op;
            bits_cntr -= ml_op;
            tmpVal.range(0, 0) = 0;
            tmpVal.range(16, 1) = len;
            outStream << tmpVal;
            lidx = bitbuffer2;
            is_length = !(is_length);
            #ifndef __SYNTHESIS__
                // std::cout << "runB" << std::endl;
                // std::cout << "tmpVal: " << std::hex << tmpVal << std::dec << std::endl;
                // std::cout << "bits_cntr: " << bits_cntr << std::endl;
            #endif
        } else if ((current_op & 64) == 0) {
            #ifndef __SYNTHESIS__
                // std::cout << "runC" << std::endl;
            #endif
            if (is_length) {
                len_extra = true;
            } else {
                dist_extra = true;
            }
        } else if (current_op & 32) {
            #ifndef __SYNTHESIS__
                // std::cout << "runD" << std::endl;
            #endif
            if (is_length) {
                ret = blockStatus::PENDING;
            } else {
                ret = blockStatus::FINISH;
            }
            huffDone = true;
        }
        if ((done == true) && (bits_cntr < 16)) {   //if ((done == true) && (bits_cntr < 32))
            huffDone = true;
            ret = blockStatus::FINISH;
        }
        if (bits_cntr < 32 && (done == false)) {
            uint16_t inValue = inStream.read();
            done = inEos.read();
            bitbuffer = (bitbuffer >> (len1 + len2)) | (bitBufferType)(inValue) << bits_cntr;
            #ifndef __SYNTHESIS__
                // std::cout << "inValue: " << std::hex << inValue << std::dec << std::endl;
            #endif
            bits_cntr += (uint8_t)16;
        } else {
            bitbuffer >>= (len1 + len2);
        }

        #ifndef __SYNTHESIS__
            // std::cout << "inValue: " << std::hex << inValue << std::dec << std::endl;
            // std::cout << "bitbuffer: " << std::hex << bitbuffer << std::dec << std::endl;
            // std::cout << "len_extra: " << len_extra << std::dec << std::endl;
            // std::cout << "dist_extra: " << dist_extra << std::dec << std::endl;
            // std::cout << "is_length: " << is_length << std::dec << std::endl;
            // std::cout << "done: " << std::hex << done << std::dec << std::endl;
            // std::cout << "huffDone: " << std::hex << huffDone << std::dec << std::endl;
        #endif

        if (len_extra) {
            ap_uint<32> val = array_codes_extra[lidx1];
            current_op = val.range(31, 24);
            current_bits = val.range(23, 16);
            current_val = val.range(15, 0);
        } else if (dist_extra) {
            ap_uint<32> val = array_codes_dist_extra[lidx1];
            current_op = val.range(31, 24);
            current_bits = val.range(23, 16);
            current_val = val.range(15, 0);
        } else if (is_length) {
            ap_uint<32> val = array_codes[lidx];
            current_op = val.range(31, 24);
            current_bits = val.range(23, 16);
            current_val = val.range(15, 0);
        } else {
            ap_uint<32> val = array_codes_dist[lidx];
            current_op = val.range(31, 24);
            current_bits = val.range(23, 16);
            current_val = val.range(15, 0);
        }
    }
    _bitbuffer = bitbuffer;
    return ret;
}

// tapa::istream<ap_uint<16> >& inStream,
// tapa::istream<bool>& inEos,
// tapa::istream<bool>& inDEos,
// tapa::ostream<bool>& outDEos,
// tapa::ostream<ap_uint<16> >& outStream

template <eHuffmanType DECODER = FULL>
void huffmanDecoder(tapa::istream<ap_uint<16> >& inStream,
                    tapa::istream<bool>& inEos,
                    tapa::istream<bool>& inDEos,
                    tapa::ostream<bool>& outDEos,
                    tapa::ostream<ap_uint<17> >& outStream) {


    const bool include_fixed_block = (DECODER == FIXED || DECODER == FULL);
    const bool include_dynamic_block = (DECODER == DYNAMIC || DECODER == FULL);
    const ap_uint<5> order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    const uint16_t c_tcodesize = 2048;
while(inDEos.read())
{
    outDEos.write(true);
    bitBufferType bitbuffer = 0;
    ap_uint<6> bits_cntr = 0;
    bool isMultipleFiles = false;
    bool done = false;
    // loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);
    while (done == false) {
        uint8_t current_op = 0;
        uint8_t current_bits = 0;
        uint16_t current_val = 0;
        ap_uint<32> current_table_val;

        uint8_t len = 0;

        

        bool dynamic_last = 0;
        ap_uint<9> dynamic_nlen = 0;
        ap_uint<9> dynamic_ndist = 0;
        ap_uint<5> dynamic_ncode = 0;
        ap_uint<9> dynamic_curInSize = 0;
        uint16_t dynamic_lens[512];

        uint8_t copy = 0;

        bool blocks_processed = false;

        

        uint32_t array_codes[512];
        uint32_t array_codes_extra[512];
        uint32_t array_codes_dist[512];
        uint32_t array_codes_dist_extra[512];

        bool isGzip = false;

        
        bool skip_fname = false;
        // details::loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);
        // uint16_t magic_number = bitbuffer & 0xFFFF;
        // details::discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)16);
        // if (magic_number == 0x8b1f) {
        //     // GZIP Header Processing
        //     // Deflate mode & file name flag
        //     isGzip = true;
        //     isMultipleFiles = false;
        //     details::loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);
        //     uint16_t lcl_tmp = bitbuffer & 0xFFFF;
        //     details::discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)16);

        //     // Check for fnam content
        //     skip_fname = (lcl_tmp >> 8) ? true : false;
        //     details::loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);

        //     // MTIME - must
        //     // XFL (2 for high compress, 4 fast)
        //     // OS code (3Unix, 0Fat)
        //     details::discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)32);
        //     details::loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);

        //     // MTIME - must
        //     // XFL (2 for high compress, 4 fast)
        //     // OS code (3Unix, 0Fat)
        //     details::discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)16);
        //     // If FLG is set to zero by using -n
        //     if (skip_fname) {
        //     // Read file name
        //     read_fname:
        //         do {
        //             #pragma HLS PIPELINE II = 1
        //             if (bits_cntr < 16 && (done == false)) {
        //                 uint16_t tmp_data = inStream.read();
        //                 bitbuffer += (bitBufferType)(tmp_data << bits_cntr);
        //                 done = inEos.read();
        //                 bits_cntr += 16;
        //             }
        //             lcl_tmp = bitbuffer & 0xFF;
        //             details::discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)8);
        //         } while (lcl_tmp != 0);
        //     }
        // } else if ((magic_number & 0x00FF) != 0x0078) {
        //     blocks_processed = true;
        // }

        if (isMultipleFiles) blocks_processed = true;
        while (!blocks_processed && (done == false)) {
            // one block per iteration
            // check if the following block is stored block or compressed block
            isMultipleFiles = true;
            loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);
            // read the last bit in bitbuffer to check if this is last block
            dynamic_last = bitbuffer & 1;
            bitbuffer >>= 1; // dump the bit read
            ap_uint<2> cb_type = (uint8_t)(bitbuffer)&3;
            bitbuffer >>= 2;
            bits_cntr -= 3; // previously dumped 1 bit + current dumped 2 bits

            #ifndef __SYNTHESIS__ 
                // std::cout << "cb_type: " << cb_type << std::endl;
            #endif

            if (cb_type == 0) { // stored block
                bitbuffer >>= bits_cntr & 7;
                bits_cntr -= bits_cntr & 7;

                loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);
                uint16_t store_length = bitbuffer & 0xffff;
                discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)32);

                if (DECODER == FULL) {
                    ap_uint<17> tmpVal = 0;
                strd_blk_cpy:
                    for (uint16_t i = 0; i < store_length; i++) {
#pragma HLS PIPELINE II = 1
                        if (bits_cntr < 8 && (done == false)) {
                            uint16_t tmp_dt = (uint16_t)inStream.read();
                            bitbuffer += (bitBufferType)(tmp_dt) << bits_cntr;
                            done = inEos.read();
                            bits_cntr += 16;
                        }
                        tmpVal.range(8, 1) = bitbuffer;
                        tmpVal.range(16, 9) = 0xFF;
                        tmpVal.range(0, 0) = 0;
                        outStream << tmpVal;
                        discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)8);
                    }
                }

            } else if (cb_type == 2) {       // dynamic huffman compressed block
                if (include_dynamic_block) { // compile if decoder should be dynamic/full
                                             // Read 14 bits HLIT(5-bits), HDIST(5-bits) and HCLEN(4-bits)
                    loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);
                    dynamic_nlen = (bitbuffer & ((1 << 5) - 1)) + 257; // Max 288
                    discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)5);

                    dynamic_ndist = (bitbuffer & ((1 << 5) - 1)) + 1; // Max 30
                    discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)5);

                    dynamic_ncode = (bitbuffer & ((1 << 4) - 1)) + 4; // Max 19
                    discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)4);

                    dynamic_curInSize = 0;

                dyn_len_bits:
                    while (dynamic_curInSize < dynamic_ncode) {
#pragma HLS PIPELINE II = 1
                        if ((bits_cntr < 16) && (done == false)) {
                            uint16_t tmp_data = inStream.read();
                            bitbuffer += (bitBufferType)(tmp_data << bits_cntr);
                            done = inEos.read();
                            bits_cntr += 16;
                        }
                        dynamic_lens[order[dynamic_curInSize++]] = (uint16_t)(bitbuffer & ((1 << 3) - 1));
                        discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)3);
                    }

                    while (dynamic_curInSize < 19) dynamic_lens[order[dynamic_curInSize++]] = 0;
                    code_generator_array_dyn(1, dynamic_lens, 19, array_codes, array_codes_extra, 7);

                    dynamic_curInSize = 0;
                    uint32_t dlenb_mask = ((1 << 7) - 1);

                    loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);
                    current_table_val = array_codes[(bitbuffer & dlenb_mask)];
                // Figure out codes for LIT/ML and DIST
                bitlen_gen:
                    while (dynamic_curInSize < dynamic_nlen + dynamic_ndist || (copy != 0)) {
#pragma HLS PIPELINE II = 1
                        //#pragma HLS dependence variable = dynamic_lens inter false

                        current_bits = current_table_val.range(23, 16);
                        current_op = current_table_val.range(24, 31);
                        current_val = current_table_val.range(15, 0);

                        if (current_val < 16 && (copy == 0)) {
                            bitbuffer >>= current_bits;
                            bits_cntr -= current_bits;
                            len = current_val;
                            copy = 1;
                        } else if (current_val == 16 && (copy == 0)) {
                            bitbuffer >>= current_bits;

                            if (dynamic_curInSize == 0) blocks_processed = true;

                            // len = dynamic_lens[dynamic_curInSize - 1];
                            copy = 3 + (bitbuffer & 3);      // use 2 bits
                            bitbuffer >>= 2;                 // dump 2 bits
                            bits_cntr -= (current_bits + 2); // update bits_cntr
                        } else if (current_val == 17 && (copy == 0)) {
                            bitbuffer >>= current_bits;
                            len = 0;
                            copy = 3 + (bitbuffer & 7); // use 3 bits
                            bitbuffer >>= 3;
                            bits_cntr -= (current_bits + 3);
                        } else if (copy == 0) {
                            bitbuffer >>= current_bits;
                            len = 0;
                            copy = 11 + (bitbuffer & ((1 << 7) - 1)); // use 7 bits
                            bitbuffer >>= 7;
                            bits_cntr -= (current_bits + 7);
                        }

                        // std::cout << "lens[" << dynamic_curInSize <<"] = " << (uint16_t)len << std::endl;
                        dynamic_lens[dynamic_curInSize++] = (uint16_t)len;
                        copy -= 1;
                        current_table_val = reg<ap_uint<32> >(array_codes[(bitbuffer & dlenb_mask)]);
                        if ((bits_cntr < 32) && (done == false)) {
                            uint16_t tmp_data = inStream.read();
                            bitbuffer += (bitBufferType)(tmp_data) << bits_cntr;
                            done = inEos.read();
                            bits_cntr += 16;
                        }
                    } // End of while
                    code_generator_array_dyn(2, dynamic_lens, dynamic_nlen, array_codes, array_codes_extra, 9);

                    code_generator_array_dyn(3, dynamic_lens + dynamic_nlen, dynamic_ndist, array_codes_dist,
                                                      array_codes_dist_extra, 9);
                    // BYTEGEN dynamic state
                    // ********************************
                    //  Create Packets Below
                    //  [LIT|ML|DIST|DIST] --> 32 Bit
                    //  Read data from inStream - 8bits
                    //  at a time. Decode the literals,
                    //  ML, Distances based on tables
                    // ********************************

                    // Read from inStream
                    loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);

                    uint8_t ret =
                        huffmanBytegen(bitbuffer, bits_cntr, outStream, inEos, inStream, array_codes,
                                                array_codes_extra, array_codes_dist, array_codes_dist_extra, done);

                    if (ret == blockStatus::FINISH) blocks_processed = true;

                } else {
                    blocks_processed = true;
                }
            } else if (cb_type == 1) {     // fixed huffman compressed block
                if (include_fixed_block) { // compile if decoder should be fixed/full
#include "fixed_codes.hpp"
                    // ********************************
                    //  Create Packets Below
                    //  [LIT|ML|DIST|DIST] --> 32 Bit
                    //  Read data from inStream - 8bits
                    //  at a time. Decode the literals,
                    //  ML, Distances based on tables
                    // ********************************
                    // Read from inStream
                    loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);

                    // ByteGeneration module
                    uint8_t ret =
                        huffmanBytegenStatic(bitbuffer, bits_cntr, outStream, inEos, inStream, fixed_litml_op,
                                                      fixed_litml_bits, fixed_litml_val, done);

                    if (ret == blockStatus::FINISH) blocks_processed = true;

                } else {
                    blocks_processed = true;
                }
            } else {
                blocks_processed = true;
            }
            if (dynamic_last) blocks_processed = true;
        } // While end
        // Checksum 4Bytes
        // if (isGzip) {
        //     details::loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);
        //     ap_uint<6> leftOverBits = 32;
        //     details::discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)leftOverBits);

        //     details::loadBitStream(bitbuffer, bits_cntr, inStream, inEos, done);
        //     leftOverBits = 32 + (bits_cntr % 8);
        //     details::discardBitStream(bitbuffer, bits_cntr, (ap_uint<6>)leftOverBits);
        // } else 
        {
        consumeLeftOverData:
            while (done == false) {
                #ifndef __SYNTHESIS__
                    // std::cout << "Consume left over" << std::endl;
                #endif
                inStream.read();
                done = inEos.read();
            }
        }
    }
    outStream << 1; // Adding Dummy Data for last end of stream case
}
outDEos.write(false);
}

#endif
