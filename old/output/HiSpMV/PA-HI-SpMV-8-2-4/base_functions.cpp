#include "spmv.h"

void MM2S_A(tapa::async_mmap<channelA_t>& mmap,
    tapa::ostreams<uint64_t, PES_PER_CH>& streams,
    const uint32_t offset, const uint32_t len, const uint16_t rp_time) 
{
    for(uint16_t rp = 0; rp < rp_time; rp++) {
        for(uint32_t i_req = 0, i_resp = 0; i_resp < len;) {
        #pragma HLS pipeline II=1
            channelA_t tmp;
            if ((i_req < len) & !mmap.read_addr.full()) {
                mmap.read_addr.try_write(offset + i_req);
                ++i_req;
            }

            bool fifos_full = false;
            for(int p = 0; p < PES_PER_CH; p++)
            #pragma HLS UNROLL
                fifos_full |= streams[p].full();

            if (!fifos_full & !mmap.read_data.empty()) {
                mmap.read_data.try_read(tmp);
                for(int p = 0; p < PES_PER_CH; p++) 
                #pragma HLS UNROLL
                    streams[p].try_write(tmp[p]);
                ++i_resp;
            }
        }
    }
    printf("MM2S_A\n");
}

void MM2S_B(tapa::async_mmap<channelB_t>& mmap,
    tapa::ostream<channelB_t>& stream,
    const uint16_t num_tiles_r, const uint32_t len, const uint16_t rp_time) 
{
    for (uint32_t n = 0; n < num_tiles_r * rp_time; n++) {
        for(uint32_t i_req = 0, i_resp = 0; i_resp < len; ) {
        #pragma HLS pipeline II=1
            if ((i_req < len) & !mmap.read_addr.full()) {
                mmap.read_addr.try_write(i_req);
                ++i_req;
            }

            if (!stream.full() & !mmap.read_data.empty()) {
                channelB_t tmp;
                mmap.read_data.try_read(tmp);
                stream.try_write(tmp);
                ++i_resp;
            }
        }
    }
    printf("MM2S_B\n");
}

void MM2S_C(tapa::async_mmap<channelC_t>& mmap,
    tapa::ostream<channelC_t>& stream,
    const uint32_t num_rows_per_pe, const uint16_t rp_time) 
{
    uint32_t len = (num_rows_per_pe * NUM_PES / FP32_PER_CH) / NUM_C_CH;
    for(uint16_t rp = 0; rp < rp_time; rp++) {
        for(uint32_t i_req = 0, i_resp = 0; i_resp < len; ) {
        #pragma HLS pipeline II=1
            if ((i_req < len) & !mmap.read_addr.full()) {
                mmap.read_addr.try_write(i_req);
                ++i_req;
            }

            if (!stream.full() & !mmap.read_data.empty()) {
                channelC_t tmp;
                mmap.read_data.try_read(tmp);
                stream.try_write(tmp);
                ++i_resp;
            }
        }
    }
    printf("MM2S_C\n");
}

void S2MM_C(tapa::istream<channelC_t>& stream,
    tapa::async_mmap<channelC_t>& mmap, 
    const uint32_t num_rows_per_pe, const uint16_t rp_time) 
{
    uint32_t len = (num_rows_per_pe * NUM_PES / FP32_PER_CH) / NUM_C_CH;
    for(uint16_t rp = 0; rp < rp_time; rp++) {
        for(int i_req = 0, i_resp = 0; i_resp < len;) {
        #pragma HLS pipeline II=1
            channelC_t tmp;
            // issue write requests
            if ((i_req < len) && !stream.empty() && !mmap.write_addr.full() && !mmap.write_data.full()) {
                tmp = stream.read(nullptr);
                mmap.write_addr.try_write(i_req);
                mmap.write_data.try_write(tmp);
                ++i_req;
            }

            // receive acks of write success
            if (!mmap.write_resp.empty()) 
                i_resp += unsigned(mmap.write_resp.read(nullptr)) + 1;
        }
    }
    printf("S2MM_C\n");
}

void LoadB(tapa::istreams<channelB_t, NUM_B_CH>& b_in, 
        tapa::ostreams<channelB_t, NUM_B_CH>& b_out,
        obuffersB_t& local_B, //mem core
        const int B_len, const int num_tiles_rp_time)
{
    uint32_t c = 0;
    load_b:
    for (int i = 0; i < num_tiles_rp_time; i++) {
        if(c == B_len)
            c = 0;

        for (int i = 0; i < LOAD_GROUP_SIZE; i++) {
            auto section = local_B[i].create_section();
            local_B[i].acquire(section);    
        }

        
        for(uint16_t w = 0; (w < W_WINDOW) && (c < (B_len)) ; w++, c++) {
        #pragma HLS loop_tripcount min=0 max=512
        #pragma HLS PIPELINE II=1
            channelB_t temp[NUM_B_CH];
            for(uint8_t ch = 0; ch < NUM_B_CH; ch++) 
            #pragma HLS UNROLL
                temp[ch] = b_in[ch].read();

            for(uint8_t q = 0; q < (NUM_B_CH * FP32_PER_CH); q++) {
            #pragma HLS UNROLL
                for(int l = 0; l < LOAD_GROUP_SIZE; l++) {
                    auto section = local_B[l].create_section();
                    auto& buf_ref = section();
                    buf_ref[w*(NUM_B_CH * FP32_PER_CH) + q] = temp[q/FP32_PER_CH][q%FP32_PER_CH];
                }
            }
                
            for(uint8_t ch = 0; ch < NUM_B_CH; ch++) 
            #pragma HLS UNROLL
                b_out[ch].write(temp[ch]);
        }

        for (int i = 0; i < LOAD_GROUP_SIZE; i++) {
            auto section = local_B[i].create_section();
            section.release_section(); 
        }
    }
    printf("PEG load\n");
}

void DummyReadB(tapa::istream<channelB_t>& b_in) {
    for(;;) 
    #pragma HLS PIPELINE II=1
        channelB_t tmp = b_in.read();
}

void ComputeAB(tapa::istreams<uint64_t, 2>& a_in, tapa::ostream<flags_pkt>& c_flags, 
    tapa::ostreams<uint16_t, 2>& c_row, tapa::ostreams<float, 2>& c_val, 
    ibufferB_t& local_B, //mem core
    const uint32_t A_len,
    const uint32_t num_rows_per_pe,
    const uint16_t rp_time,
    const bool DENSE_MODE) 
{
    auto section = local_B.create_section();
    compute:
    for (uint16_t r = 1; r <= rp_time ; r++) {
        uint32_t l = 1; 
        for(bool last = false; !last; ) {
            local_B.acquire(section);
            auto& buf_ref = section();

#ifdef BUILD_DENSE_OVERLAY
            uint16_t row_cnt = 0;
            uint16_t col_cnt = 0;
#endif
            for (bool tileEnd = false; !(tileEnd); l++) {
            #pragma HLS loop_tripcount min=1 max=100000
            #pragma HLS PIPELINE II=1
                uint64_t temp_in[2];
                for(int p = 0; p < 2; p++) 
                #pragma HLS UNROLL
                    temp_in[p] = a_in[p].read();

				float val_out[2];
				uint16_t row_out[2];
#ifdef BUILD_DENSE_OVERLAY
                float b_val[2];
                for(int p = 0; p < 2; p++) {
                #pragma HLS UNROLL
                    uint16_t col_id = DENSE_MODE ? col_cnt + p : (temp_in[p] >> 32) & 0x3FFF;
                    b_val[p] = buf_ref[col_id];
                }

                for(int p = 0; p < 2; p++) 
                #pragma HLS UNROLL
                    row_out[p] = DENSE_MODE ? row_cnt | (1 << 15) : (temp_in[p] >> 48) & 0xFFFF;

                
                for(int p = 0; p < 2; p++) {
                #pragma HLS UNROLL
                    if (DENSE_MODE) {
                        uint32_t temp0 = ((uint32_t)(temp_in[p] & 0xFFFFFFFF));
                        uint32_t temp1 = ((uint32_t)((temp_in[p] >> 32) & 0xFFFFFFFF));
                        float a_val0 = *(float*)&temp0;
                        float a_val1 = *(float*)&temp1;
                        val_out[p] = (a_val0 * b_val[0]) + (a_val1 * b_val[1]);
                    }
                    else {
                        uint32_t temp = ((uint32_t)(temp_in[p] & 0xFFFFFFFF));
                        float a_val = *(float*)&temp;
                        val_out[p] = a_val * b_val[p];
                    }
                }
                row_cnt++;
                if (row_cnt == num_rows_per_pe) {
                    row_cnt = 0;
                    col_cnt += 2;
                }
                last = (l == A_len);
                tileEnd = DENSE_MODE ? (col_cnt == B_WINDOW) || last : (temp_in[0] >> 47) & 1;
                flags_pkt flags_out;
                flags_out.sharedRow = DENSE_MODE ? false : (temp_in[0] >> 46) & 1;
                flags_out.tileEnd = tileEnd;
                flags_out.last = last && (r == rp_time);
#else
				for(int p = 0; p < 2; p++) {
                    uint64_t a = temp_in[p];
                    uint32_t val_bits = a & 0xFFFFFFFF;
                    float val = *(float*)(&val_bits);
                    uint16_t col_id = (a >> 32) & 0x3FFF;
                    row_out[p] = (a >> 48) & 0xFFFF;
					val_out[p] = val * buf_ref[col_id];
                }
				last = (l == A_len);
				tileEnd = (temp_in[0] >> 47) & 1;
				flags_pkt flags_out;
                flags_out.sharedRow = (temp_in[0] >> 46) & 1;
                flags_out.tileEnd   = tileEnd;
                flags_out.last      = last && (r == rp_time);
#endif
                for(int p = 0; p < 2; p++) {
				#pragma HLS UNROLL
                    c_val[p] << val_out[p];
                    c_row[p] << row_out[p];
                }
                c_flags << flags_out;
            }
            section.release_section();
        }
    }
    printf("Compute AB\n");
}


void PreAccumulator(tapa::istreams<uint16_t, 2>& c_row, tapa::istreams<float, 2>& c_val,
        tapa::istream<flags_pkt>& c_flags, tapa::ostreams<Cnoc_pkt, 2>& c_out) { 
#ifdef BUILD_PRE_ACCUMULATOR
    float val_buff_part[2][II_DIST];
    ap_uint<24> row_buff_part[2][II_DIST];
    #pragma HLS bind_storage variable=val_buff_part type=RAM_2P impl=LUTRAM
    #pragma HLS array_partition variable=val_buff_part type=complete dim=0

    #pragma HLS bind_storage variable=row_buff_part type=RAM_2P impl=LUTRAM
    #pragma HLS array_partition variable=row_buff_part type=complete dim=0
#endif

    main:
    for (bool last = false;!(last);) {

#ifdef BUILD_PRE_ACCUMULATOR
        init:
        for (uint8_t p = 0; p < 2; p++) {
        #pragma HLS UNROLL
            for (uint8_t k = 0; k < II_DIST; k++) {
            #pragma HLS UNROLL
                row_buff_part[p][k] = 0;
                val_buff_part[p][k] = 0;
            }
        }
#endif

        compute:
        for (bool tileEnd = false; !(tileEnd);) {
        #pragma HLS PIPELINE II=1
            uint16_t row_in[2]; 
            float val_in[2]; 
            for(int p = 0; p < 2; p++) {
                row_in[p] = c_row[p].read();
                val_in[p] = c_val[p].read();
            }
            flags_pkt flags_in = c_flags.read();
            
            uint8_t shared_bank = row_in[0] & ((1U << LOG_2_NUM_PES) - 1);
            uint16_t shared_row16 = row_in[1];

            for(int p = 0; p < 2; p++) {
            #pragma HLS UNROLL
                uint16_t row16 = row_in[p];
                float val = val_in[p];

                uint8_t bank = (flags_in.sharedRow) ? shared_bank : 0;
                uint16_t row = (flags_in.sharedRow) ? shared_row16 : row16;
                bool rowEnd = (row >> 15) & 1;

#ifdef BUILD_PRE_ACCUMULATOR                
                ap_uint<24> curr_row = ((ap_uint<24>)((row & 0x7FFF) | (flags_in.sharedRow << 15)) << LOG_2_NUM_PES) | (bank & ((1U << LOG_2_NUM_PES) - 1));

                for (uint8_t l = 0; l < II_DIST-1; l++) {
                #pragma HLS UNROLL
                    row_buff_part[p][l] = row_buff_part[p][l+1]; 
                    val_buff_part[p][l] = val_buff_part[p][l+1]; 
                }

                val_buff_part[p][II_DIST-1] = val;
                row_buff_part[p][II_DIST-1] = curr_row;

                float temp[II_DIST];
                for (uint8_t l = 0; l < II_DIST; l++) 
                #pragma HLS UNROLL
                    temp[l] = (row_buff_part[p][l] == curr_row) ? val_buff_part[p][l] : 0;    

                for (uint8_t l = 1; l < II_DIST; l++) 
                #pragma HLS UNROLL
                    temp[0] += temp[l];
#endif

                Cnoc_pkt curr_out;
                curr_out.dummy = !(rowEnd);
                curr_out.sharedRow = flags_in.sharedRow;
                curr_out.last = flags_in.last;
                curr_out.tileEnd = flags_in.tileEnd;

#ifdef BUILD_PRE_ACCUMULATOR
                curr_out.row16 = (uint16_t)((curr_row >> LOG_2_NUM_PES) & 0x7FFF);
                curr_out.bank = (uint8_t)(curr_row & ((1U << LOG_2_NUM_PES) - 1));
                curr_out.val = temp[0];
#else
                curr_out.row16 = (uint16_t)(row & 0x7FFF);
                curr_out.bank = (uint8_t)(bank & ((1U << LOG_2_NUM_PES) - 1));
                curr_out.val = val;
#endif

                c_out[p] << curr_out;
            }

            tileEnd = flags_in.tileEnd;
            last = flags_in.last;
        }
    }
    printf("PreAccumulator\n");
}

#ifdef BUILD_ROW_DIST_NETWORK
template<bool m, bool sw>
void ADD(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
    tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
    for(bool last = false; !last; ) {
    #pragma HLS Pipeline II=1
        Cnoc_pkt curr_in[2];
        curr_in[0] = c_in0.read();
        curr_in[1] = c_in1.read();
        
        float sum = curr_in[0].val + curr_in[1].val;
        bool shared_cond = ((curr_in[0].sharedRow && curr_in[1].sharedRow) && !(curr_in[0].dummy || curr_in[1].dummy));

        float temp[2];
        bool dummy[2];
        bool i = sw?((curr_in[0].bank >> (LOG_2_NUM_PES - 1)) & 1) : m;
        temp[i] = shared_cond ? sum : curr_in[i].val;
        temp[!i] = shared_cond ? 0 : curr_in[!i].val;
        dummy[i] = shared_cond ? false : curr_in[i].dummy;
        dummy[!i] = shared_cond ? true : curr_in[!i].dummy;

        Cnoc_pkt curr_out[2];
        for(int i = 0; i < 2; i++) {
            curr_out[i].last = curr_in[i].last;
            curr_out[i].bank = curr_in[i].bank;
            curr_out[i].dummy = dummy[i];
            curr_out[i].tileEnd = curr_in[i].tileEnd;
            curr_out[i].sharedRow = curr_in[i].sharedRow;
            curr_out[i].row16 = curr_in[i].row16;
            curr_out[i].val = temp[i];
        }

        c_out0 << curr_out[0];
        c_out1 << curr_out[1];

        last = curr_in[0].last & curr_in[1].last;    
    }
}

template<bool m, int n>
void SWB(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
    tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
    for(bool last = false; !last; ) {
    #pragma HLS Pipeline II=1
        Cnoc_pkt curr_in[2];
        Cnoc_pkt curr_out[2];
        curr_in[0] = c_in0.read();
        curr_in[1] = c_in1.read();

        bool target = (curr_in[m].bank >> n) & 1;
        bool sharedRow = curr_in[m].sharedRow;
        bool i = m ? (target || !sharedRow) : (target && sharedRow);
        curr_out[i] = curr_in[m];
        curr_out[!i] = curr_in[!m];

        c_out0 << curr_out[0];
        c_out1 << curr_out[1];

        last = curr_in[0].last & curr_in[1].last;    
    }
}

void SSW(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
    tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
    for(bool last = false; !last; ) {
    #pragma HLS Pipeline II=1
        Cnoc_pkt curr_in0 = c_in0.read();
        Cnoc_pkt curr_in1 = c_in1.read();
        Cnoc_pkt curr_out[2];

        bool i = curr_in0.sharedRow;

        curr_out[i] = curr_in0;
        curr_out[!i] = curr_in1;


        c_out0 << curr_out[0];
        c_out1 << curr_out[1];

        last = curr_in0.last & curr_in1.last;    
    }
}
#endif

void AccumBuffer(tapa::istream<Cnoc_pkt>& c_in, 
    tapa::ostream<float>& c_out, 
    const uint32_t num_rows_per_pe, const uint16_t num_tiles_c, const uint16_t rp_time) {

    float BUFF_C[MAX_ROWS_PER_PE];
    #pragma HLS bind_storage variable=BUFF_C type=RAM_2P impl=URAM

    float circbuff_val[8];
	uint16_t circbuff_row[8];

    #pragma HLS bind_storage variable=circbuff_val type=RAM_2P impl=LUTRAM
	#pragma HLS bind_storage variable=circbuff_row type=RAM_2P impl=LUTRAM
    //initialise 
    for(uint16_t rp = 0; rp < rp_time; rp++)
    {
        uint32_t r = 0;
        init0:
        for (uint32_t i = 0; (i < MAX_ROWS_PER_PE) && (i < num_rows_per_pe);) {
        #pragma HLS PIPELINE II=1
		#pragma HLS bind_op variable=i op=add impl=dsp
            BUFF_C[i] = 0;
			i = i + 1;
        }

        ap_uint<3> w_idx = II_DIST;
        ap_uint<3> r_idx = 0;
        main:
        for(;(r < num_rows_per_pe);) {
            for (uint16_t i = 0; i < num_tiles_c; i++) {
                init:
                for (int j = 0; j < 8; j++) {
                #pragma HLS PIPELINE
                    circbuff_val[j] = 0;
					circbuff_row[j] = 0xFFFF;
                }

                acc:
                for (bool tileEnd = false; !(tileEnd); r_idx++, w_idx++) {
                #pragma HLS PIPELINE II=1
                #pragma HLS DEPENDENCE true type=inter variable=circbuff_val direction=RAW distance=II_DIST
                #pragma HLS DEPENDENCE false type=inter variable=BUFF_C 
                    Cnoc_pkt curr_in = c_in.read();
                    if (!curr_in.dummy) {
                        #pragma HLS bind_op variable=circbuff_val op=fadd latency=FP_ACC_LATENCY impl=fabric
                        circbuff_val[w_idx] = (curr_in.val) + ((circbuff_row[r_idx] == curr_in.row16) ? circbuff_val[r_idx] : BUFF_C[curr_in.row16]);
                        BUFF_C[curr_in.row16] = circbuff_val[w_idx];
                        circbuff_row[w_idx] =  curr_in.row16;
					}
                    tileEnd = curr_in.tileEnd;
                }
            }
            out:
            for (uint32_t i = 0; (i < MAX_ROWS_PER_PE) && (r < num_rows_per_pe);) {
            #pragma HLS PIPELINE II=1
				c_out << BUFF_C[i];
				BUFF_C[i] = 0;

			#pragma HLS bind_op variable=i op=add impl=dsp
				i = i + 1;
			#pragma HLS bind_op variable=r op=add impl=dsp
				r = r + 1;
            }
        }
    }
    printf("Result Buffer\n");
}

void Arbiter_C(tapa::istreams<float, NUM_PES>& c_ab_in, tapa::ostreams<channelC_t, NUM_C_CH>& c_ab_out, const uint32_t num_rows_per_pe, const uint16_t rp_time) {
	channelC_t tmp_in[NUM_C_CH];
	#pragma HLS aggregate variable=tmp_in
	for (uint32_t i = 0; i < (num_rows_per_pe * rp_time) ; i++) {
        for(uint8_t j = 0; j < (NUM_PES / FP32_PER_CH) / NUM_C_CH; j++) {
			for(int  jj = 0; jj < FP32_PER_CH * NUM_C_CH; jj++) 
            #pragma HLS UNROLL 
                c_ab_in[j * FP32_PER_CH * NUM_C_CH + jj] >> tmp_in[jj / FP32_PER_CH][jj % FP32_PER_CH];
			for(int c = 0; c < NUM_C_CH; c++)
            #pragma HLS UNROLL
                c_ab_out[c] << tmp_in[c];
		}
	}
}

void Compute_C(tapa::istream<channelC_t>& c_in, tapa::istream<channelC_t>& c_ab, 
    tapa::ostream<channelC_t>& c_out, 
    const float alpha, const float beta, const uint32_t num_rows_per_pe, const uint16_t rp_time) {
    channelC_t tmp_in0, tmp_in1, tmp_out;
    #pragma HLS aggregate variable=tmp_in0
    #pragma HLS aggregate variable=tmp_in1
    #pragma HLS aggregate variable=tmp_out
    for (uint32_t i = 0; i < (num_rows_per_pe * rp_time); i++) {
        for(uint8_t j = 0; j < NUM_PES / FP32_PER_CH / NUM_C_CH; j++) {
            #pragma HLS PIPELINE II=1
            c_in >> tmp_in0;
			c_ab >> tmp_in1;
            for(int  jj = 0; jj < FP32_PER_CH; jj++) 
            #pragma HLS UNROLL
                tmp_out[jj] = (beta * tmp_in0[jj]) + (alpha * tmp_in1[jj]); 
            c_out << tmp_out;
        }
    }
    printf("Compute C\n");
}

#ifdef BUILD_ROW_DIST_NETWORK
void ADD_0(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
    tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
    ADD<0,0>(c_in0, c_in1, c_out0, c_out1);
}

void ADD_1(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
    tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
    ADD<1,0>(c_in0, c_in1, c_out0, c_out1);
}

void ADD_SWB(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
    tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
    ADD<0,1>(c_in0, c_in1, c_out0, c_out1);
}

void SWB0_0(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<0,0>(c_in0, c_in1, c_out0, c_out1);
}

void SWB1_0(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<1,0>(c_in0, c_in1, c_out0, c_out1);
}

void SWB0_1(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<0,1>(c_in0, c_in1, c_out0, c_out1);
}

void SWB1_1(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<1,1>(c_in0, c_in1, c_out0, c_out1);
}

void SWB0_2(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<0,2>(c_in0, c_in1, c_out0, c_out1);
}

void SWB1_2(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<1,2>(c_in0, c_in1, c_out0, c_out1);
}

void SWB0_3(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<0,3>(c_in0, c_in1, c_out0, c_out1);
}

void SWB1_3(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<1,3>(c_in0, c_in1, c_out0, c_out1);
}

void SWB0_4(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<0,4>(c_in0, c_in1, c_out0, c_out1);
}

void SWB1_4(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<1,4>(c_in0, c_in1, c_out0, c_out1);
}

void SWB0_5(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<0,5>(c_in0, c_in1, c_out0, c_out1);
}

void SWB1_5(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<1,5>(c_in0, c_in1, c_out0, c_out1);
}

void SWB0_6(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<0,6>(c_in0, c_in1, c_out0, c_out1);
}

void SWB1_6(tapa::istream<Cnoc_pkt>& c_in0, tapa::istream<Cnoc_pkt>& c_in1,
	tapa::ostream<Cnoc_pkt>& c_out0, tapa::ostream<Cnoc_pkt>& c_out1) {
	SWB<1,6>(c_in0, c_in1, c_out0, c_out1);
}
#endif