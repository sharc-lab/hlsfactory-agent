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
void SpMV(tapa::mmaps<channelA_t, NUM_A_CH> A,
          tapa::mmaps<channelB_t, NUM_B_CH> b,
          tapa::mmaps<channelC_t, NUM_C_CH> c_in,
          tapa::mmaps<channelC_t, NUM_C_CH> c_out,
          const float alpha, const float beta,
          const uint32_t A_off, const uint32_t A_len,
          const uint32_t num_rows_per_pe, const uint32_t B_len,
          const uint16_t num_tiles_r, const uint16_t num_tiles_c,
          const uint32_t num_tiles_rp_time, const uint16_t rp_time,
          const bool DENSE_MODE)
{

    tapa::streams<uint64_t, NUM_PES, FIFO_DEPTH> FIFO_A_IN("a_in");
    tapa::streams<uint16_t, NUM_PES, FIFO_DEPTH> FIFO_C_ROW("c_row");
    tapa::streams<float, NUM_PES, FIFO_DEPTH> FIFO_C_VAL("c_val");
    tapa::streams<flags_pkt, NUM_PES_HALF, FIFO_DEPTH> FIFO_C_FLAG("c_flag");
    
    tapa::streams<channelB_t, ((NUM_PES_HALF/LOAD_GROUP_SIZE) + 1) * NUM_B_CH, FIFO_DEPTH> FIFO_B_IN("b_in");

    buffersB_t BUFF_B; 

#ifdef BUILD_ROW_DIST_NETWORK
    tapa::streams<Cnoc_pkt, NUM_PES, FIFO_DEPTH> FIFO_C_SHF("c_shf");
#endif
    tapa::streams<Cnoc_pkt, NUM_PES, FIFO_DEPTH> FIFO_C_BUF("c_buf");

    tapa::streams<float, NUM_PES, FIFO_DEPTH> FIFO_C_ARB("c_arb");
	tapa::streams<channelC_t, NUM_C_CH, FIFO_DEPTH> FIFO_C_AB("c_ab");
    tapa::streams<channelC_t, NUM_C_CH, FIFO_DEPTH> FIFO_C_IN("c_in");
    tapa::streams<channelC_t, NUM_C_CH, FIFO_DEPTH> FIFO_C_OUT("c_out");

#ifdef BUILD_ROW_DIST_NETWORK
	tapa::stream<Cnoc_pkt, 72> s_0_0("s_0_0");
	tapa::stream<Cnoc_pkt, 2> s_1_0("s_1_0");
	tapa::stream<Cnoc_pkt, 2> s_2_0("s_2_0");
	tapa::stream<Cnoc_pkt, 10> s_3_0("s_3_0");
	tapa::stream<Cnoc_pkt, 10> s_4_0("s_4_0");
	tapa::stream<Cnoc_pkt, 2> s_5_0("s_5_0");
	tapa::stream<Cnoc_pkt, 2> s_6_0("s_6_0");
	tapa::stream<Cnoc_pkt, 20> s_7_0("s_7_0");
	tapa::stream<Cnoc_pkt, 20> s_8_0("s_8_0");
	tapa::stream<Cnoc_pkt, 2> s_9_0("s_9_0");
	tapa::stream<Cnoc_pkt, 2> s_10_0("s_10_0");
	tapa::stream<Cnoc_pkt, 10> s_11_0("s_11_0");
	tapa::stream<Cnoc_pkt, 10> s_12_0("s_12_0");
	tapa::stream<Cnoc_pkt, 2> s_13_0("s_13_0");
	tapa::stream<Cnoc_pkt, 2> s_14_0("s_14_0");
	tapa::stream<Cnoc_pkt, 30> s_15_0("s_15_0");
	tapa::stream<Cnoc_pkt, 30> s_16_0("s_16_0");
	tapa::stream<Cnoc_pkt, 2> s_17_0("s_17_0");
	tapa::stream<Cnoc_pkt, 2> s_18_0("s_18_0");
	tapa::stream<Cnoc_pkt, 10> s_19_0("s_19_0");
	tapa::stream<Cnoc_pkt, 10> s_20_0("s_20_0");
	tapa::stream<Cnoc_pkt, 2> s_21_0("s_21_0");
	tapa::stream<Cnoc_pkt, 2> s_22_0("s_22_0");
	tapa::stream<Cnoc_pkt, 20> s_23_0("s_23_0");
	tapa::stream<Cnoc_pkt, 20> s_24_0("s_24_0");
	tapa::stream<Cnoc_pkt, 2> s_25_0("s_25_0");
	tapa::stream<Cnoc_pkt, 2> s_26_0("s_26_0");
	tapa::stream<Cnoc_pkt, 10> s_27_0("s_27_0");
	tapa::stream<Cnoc_pkt, 10> s_28_0("s_28_0");
	tapa::stream<Cnoc_pkt, 2> s_29_0("s_29_0");
	tapa::stream<Cnoc_pkt, 2> s_30_0("s_30_0");
	tapa::stream<Cnoc_pkt, 40> s_31_0("s_31_0");
	tapa::stream<Cnoc_pkt, 40> s_32_0("s_32_0");
	tapa::stream<Cnoc_pkt, 2> s_33_0("s_33_0");
	tapa::stream<Cnoc_pkt, 2> s_34_0("s_34_0");
	tapa::stream<Cnoc_pkt, 10> s_35_0("s_35_0");
	tapa::stream<Cnoc_pkt, 10> s_36_0("s_36_0");
	tapa::stream<Cnoc_pkt, 2> s_37_0("s_37_0");
	tapa::stream<Cnoc_pkt, 2> s_38_0("s_38_0");
	tapa::stream<Cnoc_pkt, 20> s_39_0("s_39_0");
	tapa::stream<Cnoc_pkt, 20> s_40_0("s_40_0");
	tapa::stream<Cnoc_pkt, 2> s_41_0("s_41_0");
	tapa::stream<Cnoc_pkt, 2> s_42_0("s_42_0");
	tapa::stream<Cnoc_pkt, 10> s_43_0("s_43_0");
	tapa::stream<Cnoc_pkt, 10> s_44_0("s_44_0");
	tapa::stream<Cnoc_pkt, 2> s_45_0("s_45_0");
	tapa::stream<Cnoc_pkt, 2> s_46_0("s_46_0");
	tapa::stream<Cnoc_pkt, 30> s_47_0("s_47_0");
	tapa::stream<Cnoc_pkt, 30> s_48_0("s_48_0");
	tapa::stream<Cnoc_pkt, 2> s_49_0("s_49_0");
	tapa::stream<Cnoc_pkt, 2> s_50_0("s_50_0");
	tapa::stream<Cnoc_pkt, 10> s_51_0("s_51_0");
	tapa::stream<Cnoc_pkt, 10> s_52_0("s_52_0");
	tapa::stream<Cnoc_pkt, 2> s_53_0("s_53_0");
	tapa::stream<Cnoc_pkt, 2> s_54_0("s_54_0");
	tapa::stream<Cnoc_pkt, 20> s_55_0("s_55_0");
	tapa::stream<Cnoc_pkt, 20> s_56_0("s_56_0");
	tapa::stream<Cnoc_pkt, 2> s_57_0("s_57_0");
	tapa::stream<Cnoc_pkt, 2> s_58_0("s_58_0");
	tapa::stream<Cnoc_pkt, 10> s_59_0("s_59_0");
	tapa::stream<Cnoc_pkt, 10> s_60_0("s_60_0");
	tapa::stream<Cnoc_pkt, 2> s_61_0("s_61_0");
	tapa::stream<Cnoc_pkt, 2> s_62_0("s_62_0");
	tapa::stream<Cnoc_pkt, 72> s_63_0("s_63_0");
	tapa::stream<Cnoc_pkt, 68> s_1_1("s_1_1");
	tapa::stream<Cnoc_pkt, 8> s_2_1("s_2_1");
	tapa::stream<Cnoc_pkt, 8> s_5_1("s_5_1");
	tapa::stream<Cnoc_pkt, 68> s_6_1("s_6_1");
	tapa::stream<Cnoc_pkt, 68> s_9_1("s_9_1");
	tapa::stream<Cnoc_pkt, 8> s_10_1("s_10_1");
	tapa::stream<Cnoc_pkt, 8> s_13_1("s_13_1");
	tapa::stream<Cnoc_pkt, 68> s_14_1("s_14_1");
	tapa::stream<Cnoc_pkt, 68> s_17_1("s_17_1");
	tapa::stream<Cnoc_pkt, 8> s_18_1("s_18_1");
	tapa::stream<Cnoc_pkt, 8> s_21_1("s_21_1");
	tapa::stream<Cnoc_pkt, 68> s_22_1("s_22_1");
	tapa::stream<Cnoc_pkt, 68> s_25_1("s_25_1");
	tapa::stream<Cnoc_pkt, 8> s_26_1("s_26_1");
	tapa::stream<Cnoc_pkt, 8> s_29_1("s_29_1");
	tapa::stream<Cnoc_pkt, 68> s_30_1("s_30_1");
	tapa::stream<Cnoc_pkt, 68> s_33_1("s_33_1");
	tapa::stream<Cnoc_pkt, 8> s_34_1("s_34_1");
	tapa::stream<Cnoc_pkt, 8> s_37_1("s_37_1");
	tapa::stream<Cnoc_pkt, 68> s_38_1("s_38_1");
	tapa::stream<Cnoc_pkt, 68> s_41_1("s_41_1");
	tapa::stream<Cnoc_pkt, 8> s_42_1("s_42_1");
	tapa::stream<Cnoc_pkt, 8> s_45_1("s_45_1");
	tapa::stream<Cnoc_pkt, 68> s_46_1("s_46_1");
	tapa::stream<Cnoc_pkt, 68> s_49_1("s_49_1");
	tapa::stream<Cnoc_pkt, 8> s_50_1("s_50_1");
	tapa::stream<Cnoc_pkt, 8> s_53_1("s_53_1");
	tapa::stream<Cnoc_pkt, 68> s_54_1("s_54_1");
	tapa::stream<Cnoc_pkt, 68> s_57_1("s_57_1");
	tapa::stream<Cnoc_pkt, 8> s_58_1("s_58_1");
	tapa::stream<Cnoc_pkt, 8> s_61_1("s_61_1");
	tapa::stream<Cnoc_pkt, 68> s_62_1("s_62_1");
	tapa::stream<Cnoc_pkt, 58> s_2_2("s_2_2");
	tapa::stream<Cnoc_pkt, 2> s_3_1("s_3_1");
	tapa::stream<Cnoc_pkt, 2> s_4_1("s_4_1");
	tapa::stream<Cnoc_pkt, 58> s_5_2("s_5_2");
	tapa::stream<Cnoc_pkt, 54> s_3_2("s_3_2");
	tapa::stream<Cnoc_pkt, 8> s_4_2("s_4_2");
	tapa::stream<Cnoc_pkt, 58> s_10_2("s_10_2");
	tapa::stream<Cnoc_pkt, 2> s_11_1("s_11_1");
	tapa::stream<Cnoc_pkt, 2> s_12_1("s_12_1");
	tapa::stream<Cnoc_pkt, 58> s_13_2("s_13_2");
	tapa::stream<Cnoc_pkt, 8> s_11_2("s_11_2");
	tapa::stream<Cnoc_pkt, 54> s_12_2("s_12_2");
	tapa::stream<Cnoc_pkt, 58> s_18_2("s_18_2");
	tapa::stream<Cnoc_pkt, 2> s_19_1("s_19_1");
	tapa::stream<Cnoc_pkt, 2> s_20_1("s_20_1");
	tapa::stream<Cnoc_pkt, 58> s_21_2("s_21_2");
	tapa::stream<Cnoc_pkt, 54> s_19_2("s_19_2");
	tapa::stream<Cnoc_pkt, 8> s_20_2("s_20_2");
	tapa::stream<Cnoc_pkt, 58> s_26_2("s_26_2");
	tapa::stream<Cnoc_pkt, 2> s_27_1("s_27_1");
	tapa::stream<Cnoc_pkt, 2> s_28_1("s_28_1");
	tapa::stream<Cnoc_pkt, 58> s_29_2("s_29_2");
	tapa::stream<Cnoc_pkt, 8> s_27_2("s_27_2");
	tapa::stream<Cnoc_pkt, 54> s_28_2("s_28_2");
	tapa::stream<Cnoc_pkt, 58> s_34_2("s_34_2");
	tapa::stream<Cnoc_pkt, 2> s_35_1("s_35_1");
	tapa::stream<Cnoc_pkt, 2> s_36_1("s_36_1");
	tapa::stream<Cnoc_pkt, 58> s_37_2("s_37_2");
	tapa::stream<Cnoc_pkt, 54> s_35_2("s_35_2");
	tapa::stream<Cnoc_pkt, 8> s_36_2("s_36_2");
	tapa::stream<Cnoc_pkt, 58> s_42_2("s_42_2");
	tapa::stream<Cnoc_pkt, 2> s_43_1("s_43_1");
	tapa::stream<Cnoc_pkt, 2> s_44_1("s_44_1");
	tapa::stream<Cnoc_pkt, 58> s_45_2("s_45_2");
	tapa::stream<Cnoc_pkt, 8> s_43_2("s_43_2");
	tapa::stream<Cnoc_pkt, 54> s_44_2("s_44_2");
	tapa::stream<Cnoc_pkt, 58> s_50_2("s_50_2");
	tapa::stream<Cnoc_pkt, 2> s_51_1("s_51_1");
	tapa::stream<Cnoc_pkt, 2> s_52_1("s_52_1");
	tapa::stream<Cnoc_pkt, 58> s_53_2("s_53_2");
	tapa::stream<Cnoc_pkt, 54> s_51_2("s_51_2");
	tapa::stream<Cnoc_pkt, 8> s_52_2("s_52_2");
	tapa::stream<Cnoc_pkt, 58> s_58_2("s_58_2");
	tapa::stream<Cnoc_pkt, 2> s_59_1("s_59_1");
	tapa::stream<Cnoc_pkt, 2> s_60_1("s_60_1");
	tapa::stream<Cnoc_pkt, 58> s_61_2("s_61_2");
	tapa::stream<Cnoc_pkt, 8> s_59_2("s_59_2");
	tapa::stream<Cnoc_pkt, 54> s_60_2("s_60_2");
	tapa::stream<Cnoc_pkt, 44> s_4_3("s_4_3");
	tapa::stream<Cnoc_pkt, 2> s_7_1("s_7_1");
	tapa::stream<Cnoc_pkt, 2> s_8_1("s_8_1");
	tapa::stream<Cnoc_pkt, 44> s_11_3("s_11_3");
	tapa::stream<Cnoc_pkt, 40> s_7_2("s_7_2");
	tapa::stream<Cnoc_pkt, 8> s_8_2("s_8_2");
	tapa::stream<Cnoc_pkt, 44> s_20_3("s_20_3");
	tapa::stream<Cnoc_pkt, 2> s_23_1("s_23_1");
	tapa::stream<Cnoc_pkt, 2> s_24_1("s_24_1");
	tapa::stream<Cnoc_pkt, 44> s_27_3("s_27_3");
	tapa::stream<Cnoc_pkt, 8> s_23_2("s_23_2");
	tapa::stream<Cnoc_pkt, 40> s_24_2("s_24_2");
	tapa::stream<Cnoc_pkt, 44> s_36_3("s_36_3");
	tapa::stream<Cnoc_pkt, 2> s_39_1("s_39_1");
	tapa::stream<Cnoc_pkt, 2> s_40_1("s_40_1");
	tapa::stream<Cnoc_pkt, 44> s_43_3("s_43_3");
	tapa::stream<Cnoc_pkt, 40> s_39_2("s_39_2");
	tapa::stream<Cnoc_pkt, 8> s_40_2("s_40_2");
	tapa::stream<Cnoc_pkt, 44> s_52_3("s_52_3");
	tapa::stream<Cnoc_pkt, 2> s_55_1("s_55_1");
	tapa::stream<Cnoc_pkt, 2> s_56_1("s_56_1");
	tapa::stream<Cnoc_pkt, 44> s_59_3("s_59_3");
	tapa::stream<Cnoc_pkt, 8> s_55_2("s_55_2");
	tapa::stream<Cnoc_pkt, 40> s_56_2("s_56_2");
	tapa::stream<Cnoc_pkt, 30> s_8_3("s_8_3");
	tapa::stream<Cnoc_pkt, 2> s_15_1("s_15_1");
	tapa::stream<Cnoc_pkt, 2> s_16_1("s_16_1");
	tapa::stream<Cnoc_pkt, 30> s_23_3("s_23_3");
	tapa::stream<Cnoc_pkt, 20> s_15_2("s_15_2");
	tapa::stream<Cnoc_pkt, 8> s_16_2("s_16_2");
	tapa::stream<Cnoc_pkt, 30> s_40_3("s_40_3");
	tapa::stream<Cnoc_pkt, 2> s_47_1("s_47_1");
	tapa::stream<Cnoc_pkt, 2> s_48_1("s_48_1");
	tapa::stream<Cnoc_pkt, 30> s_55_3("s_55_3");
	tapa::stream<Cnoc_pkt, 8> s_47_2("s_47_2");
	tapa::stream<Cnoc_pkt, 20> s_48_2("s_48_2");
	tapa::stream<Cnoc_pkt, 10> s_16_3("s_16_3");
	tapa::stream<Cnoc_pkt, 2> s_31_1("s_31_1");
	tapa::stream<Cnoc_pkt, 2> s_32_1("s_32_1");
	tapa::stream<Cnoc_pkt, 10> s_47_3("s_47_3");
	tapa::stream<Cnoc_pkt, 8> s_31_2("s_31_2");
	tapa::stream<Cnoc_pkt, 8> s_32_2("s_32_2");
	tapa::stream<Cnoc_pkt, 2> s_16_4("s_16_4");
	tapa::stream<Cnoc_pkt, 22> s_31_3("s_31_3");
	tapa::stream<Cnoc_pkt, 22> s_32_3("s_32_3");
	tapa::stream<Cnoc_pkt, 2> s_47_4("s_47_4");
	tapa::stream<Cnoc_pkt, 8> s_15_3("s_15_3");
	tapa::stream<Cnoc_pkt, 8> s_16_5("s_16_5");
	tapa::stream<Cnoc_pkt, 2> s_8_4("s_8_4");
	tapa::stream<Cnoc_pkt, 12> s_15_4("s_15_4");
	tapa::stream<Cnoc_pkt, 12> s_16_6("s_16_6");
	tapa::stream<Cnoc_pkt, 2> s_23_4("s_23_4");
	tapa::stream<Cnoc_pkt, 8> s_47_5("s_47_5");
	tapa::stream<Cnoc_pkt, 8> s_48_3("s_48_3");
	tapa::stream<Cnoc_pkt, 2> s_40_4("s_40_4");
	tapa::stream<Cnoc_pkt, 12> s_47_6("s_47_6");
	tapa::stream<Cnoc_pkt, 12> s_48_4("s_48_4");
	tapa::stream<Cnoc_pkt, 2> s_55_4("s_55_4");
	tapa::stream<Cnoc_pkt, 2> s_7_3("s_7_3");
	tapa::stream<Cnoc_pkt, 2> s_8_5("s_8_5");
	tapa::stream<Cnoc_pkt, 2> s_4_4("s_4_4");
	tapa::stream<Cnoc_pkt, 8> s_7_4("s_7_4");
	tapa::stream<Cnoc_pkt, 8> s_8_6("s_8_6");
	tapa::stream<Cnoc_pkt, 2> s_11_4("s_11_4");
	tapa::stream<Cnoc_pkt, 2> s_23_5("s_23_5");
	tapa::stream<Cnoc_pkt, 2> s_24_3("s_24_3");
	tapa::stream<Cnoc_pkt, 2> s_20_4("s_20_4");
	tapa::stream<Cnoc_pkt, 8> s_23_6("s_23_6");
	tapa::stream<Cnoc_pkt, 8> s_24_4("s_24_4");
	tapa::stream<Cnoc_pkt, 2> s_27_4("s_27_4");
	tapa::stream<Cnoc_pkt, 2> s_39_3("s_39_3");
	tapa::stream<Cnoc_pkt, 2> s_40_5("s_40_5");
	tapa::stream<Cnoc_pkt, 2> s_36_4("s_36_4");
	tapa::stream<Cnoc_pkt, 8> s_39_4("s_39_4");
	tapa::stream<Cnoc_pkt, 8> s_40_6("s_40_6");
	tapa::stream<Cnoc_pkt, 2> s_43_4("s_43_4");
	tapa::stream<Cnoc_pkt, 2> s_55_5("s_55_5");
	tapa::stream<Cnoc_pkt, 2> s_56_3("s_56_3");
	tapa::stream<Cnoc_pkt, 2> s_52_4("s_52_4");
	tapa::stream<Cnoc_pkt, 8> s_55_6("s_55_6");
	tapa::stream<Cnoc_pkt, 8> s_56_4("s_56_4");
	tapa::stream<Cnoc_pkt, 2> s_59_4("s_59_4");
	tapa::stream<Cnoc_pkt, 2> s_3_3("s_3_3");
	tapa::stream<Cnoc_pkt, 2> s_4_5("s_4_5");
	tapa::stream<Cnoc_pkt, 2> s_2_3("s_2_3");
	tapa::stream<Cnoc_pkt, 4> s_3_4("s_3_4");
	tapa::stream<Cnoc_pkt, 4> s_4_6("s_4_6");
	tapa::stream<Cnoc_pkt, 2> s_5_3("s_5_3");
	tapa::stream<Cnoc_pkt, 2> s_11_5("s_11_5");
	tapa::stream<Cnoc_pkt, 2> s_12_3("s_12_3");
	tapa::stream<Cnoc_pkt, 2> s_10_3("s_10_3");
	tapa::stream<Cnoc_pkt, 4> s_11_6("s_11_6");
	tapa::stream<Cnoc_pkt, 4> s_12_4("s_12_4");
	tapa::stream<Cnoc_pkt, 2> s_13_3("s_13_3");
	tapa::stream<Cnoc_pkt, 2> s_19_3("s_19_3");
	tapa::stream<Cnoc_pkt, 2> s_20_5("s_20_5");
	tapa::stream<Cnoc_pkt, 2> s_18_3("s_18_3");
	tapa::stream<Cnoc_pkt, 4> s_19_4("s_19_4");
	tapa::stream<Cnoc_pkt, 4> s_20_6("s_20_6");
	tapa::stream<Cnoc_pkt, 2> s_21_3("s_21_3");
	tapa::stream<Cnoc_pkt, 2> s_27_5("s_27_5");
	tapa::stream<Cnoc_pkt, 2> s_28_3("s_28_3");
	tapa::stream<Cnoc_pkt, 2> s_26_3("s_26_3");
	tapa::stream<Cnoc_pkt, 4> s_27_6("s_27_6");
	tapa::stream<Cnoc_pkt, 4> s_28_4("s_28_4");
	tapa::stream<Cnoc_pkt, 2> s_29_3("s_29_3");
	tapa::stream<Cnoc_pkt, 2> s_35_3("s_35_3");
	tapa::stream<Cnoc_pkt, 2> s_36_5("s_36_5");
	tapa::stream<Cnoc_pkt, 2> s_34_3("s_34_3");
	tapa::stream<Cnoc_pkt, 4> s_35_4("s_35_4");
	tapa::stream<Cnoc_pkt, 4> s_36_6("s_36_6");
	tapa::stream<Cnoc_pkt, 2> s_37_3("s_37_3");
	tapa::stream<Cnoc_pkt, 2> s_43_5("s_43_5");
	tapa::stream<Cnoc_pkt, 2> s_44_3("s_44_3");
	tapa::stream<Cnoc_pkt, 2> s_42_3("s_42_3");
	tapa::stream<Cnoc_pkt, 4> s_43_6("s_43_6");
	tapa::stream<Cnoc_pkt, 4> s_44_4("s_44_4");
	tapa::stream<Cnoc_pkt, 2> s_45_3("s_45_3");
	tapa::stream<Cnoc_pkt, 2> s_51_3("s_51_3");
	tapa::stream<Cnoc_pkt, 2> s_52_5("s_52_5");
	tapa::stream<Cnoc_pkt, 2> s_50_3("s_50_3");
	tapa::stream<Cnoc_pkt, 4> s_51_4("s_51_4");
	tapa::stream<Cnoc_pkt, 4> s_52_6("s_52_6");
	tapa::stream<Cnoc_pkt, 2> s_53_3("s_53_3");
	tapa::stream<Cnoc_pkt, 2> s_59_5("s_59_5");
	tapa::stream<Cnoc_pkt, 2> s_60_3("s_60_3");
	tapa::stream<Cnoc_pkt, 2> s_58_3("s_58_3");
	tapa::stream<Cnoc_pkt, 4> s_59_6("s_59_6");
	tapa::stream<Cnoc_pkt, 4> s_60_4("s_60_4");
	tapa::stream<Cnoc_pkt, 2> s_61_3("s_61_3");
	tapa::stream<Cnoc_pkt, 2> s_1_2("s_1_2");
	tapa::stream<Cnoc_pkt, 2> s_2_4("s_2_4");
	tapa::stream<Cnoc_pkt, 2> s_5_4("s_5_4");
	tapa::stream<Cnoc_pkt, 2> s_6_2("s_6_2");
	tapa::stream<Cnoc_pkt, 2> s_9_2("s_9_2");
	tapa::stream<Cnoc_pkt, 2> s_10_4("s_10_4");
	tapa::stream<Cnoc_pkt, 2> s_13_4("s_13_4");
	tapa::stream<Cnoc_pkt, 2> s_14_2("s_14_2");
	tapa::stream<Cnoc_pkt, 2> s_17_2("s_17_2");
	tapa::stream<Cnoc_pkt, 2> s_18_4("s_18_4");
	tapa::stream<Cnoc_pkt, 2> s_21_4("s_21_4");
	tapa::stream<Cnoc_pkt, 2> s_22_2("s_22_2");
	tapa::stream<Cnoc_pkt, 2> s_25_2("s_25_2");
	tapa::stream<Cnoc_pkt, 2> s_26_4("s_26_4");
	tapa::stream<Cnoc_pkt, 2> s_29_4("s_29_4");
	tapa::stream<Cnoc_pkt, 2> s_30_2("s_30_2");
	tapa::stream<Cnoc_pkt, 2> s_33_2("s_33_2");
	tapa::stream<Cnoc_pkt, 2> s_34_4("s_34_4");
	tapa::stream<Cnoc_pkt, 2> s_37_4("s_37_4");
	tapa::stream<Cnoc_pkt, 2> s_38_2("s_38_2");
	tapa::stream<Cnoc_pkt, 2> s_41_2("s_41_2");
	tapa::stream<Cnoc_pkt, 2> s_42_4("s_42_4");
	tapa::stream<Cnoc_pkt, 2> s_45_4("s_45_4");
	tapa::stream<Cnoc_pkt, 2> s_46_2("s_46_2");
	tapa::stream<Cnoc_pkt, 2> s_49_2("s_49_2");
	tapa::stream<Cnoc_pkt, 2> s_50_4("s_50_4");
	tapa::stream<Cnoc_pkt, 2> s_53_4("s_53_4");
	tapa::stream<Cnoc_pkt, 2> s_54_2("s_54_2");
	tapa::stream<Cnoc_pkt, 2> s_57_2("s_57_2");
	tapa::stream<Cnoc_pkt, 2> s_58_4("s_58_4");
	tapa::stream<Cnoc_pkt, 2> s_61_4("s_61_4");
	tapa::stream<Cnoc_pkt, 2> s_62_2("s_62_2");
#endif

    tapa::task()
        .invoke<tapa::join, NUM_A_CH>(MM2S_A, A, FIFO_A_IN, A_off, A_len, rp_time)
        .invoke<tapa::join, NUM_B_CH>(MM2S_B, b, FIFO_B_IN, num_tiles_r, B_len, rp_time)
        .invoke<tapa::join, NUM_PES_HALF/LOAD_GROUP_SIZE>(LoadB, FIFO_B_IN, FIFO_B_IN, BUFF_B, B_len, num_tiles_rp_time)
        .invoke<tapa::join, NUM_PES_HALF>(ComputeAB, FIFO_A_IN, FIFO_C_FLAG, FIFO_C_ROW, FIFO_C_VAL, BUFF_B, A_len, num_rows_per_pe, rp_time, DENSE_MODE)
        .invoke<tapa::detach, NUM_B_CH>(DummyReadB, FIFO_B_IN)
#ifdef BUILD_ROW_DIST_NETWORK
        .invoke<tapa::join, NUM_PES_HALF>(PreAccumulator, FIFO_C_ROW, FIFO_C_VAL, FIFO_C_FLAG, FIFO_C_SHF)
		.invoke(ADD_1, FIFO_C_SHF[0], FIFO_C_SHF[1], s_0_0, s_1_0)/*0*/
		.invoke(ADD_0, FIFO_C_SHF[2], FIFO_C_SHF[3], s_2_0, s_3_0)/*1*/
		.invoke(ADD_1, FIFO_C_SHF[4], FIFO_C_SHF[5], s_4_0, s_5_0)/*2*/
		.invoke(ADD_0, FIFO_C_SHF[6], FIFO_C_SHF[7], s_6_0, s_7_0)/*3*/
		.invoke(ADD_1, FIFO_C_SHF[8], FIFO_C_SHF[9], s_8_0, s_9_0)/*4*/
		.invoke(ADD_0, FIFO_C_SHF[10], FIFO_C_SHF[11], s_10_0, s_11_0)/*5*/
		.invoke(ADD_1, FIFO_C_SHF[12], FIFO_C_SHF[13], s_12_0, s_13_0)/*6*/
		.invoke(ADD_0, FIFO_C_SHF[14], FIFO_C_SHF[15], s_14_0, s_15_0)/*7*/
		.invoke(ADD_1, FIFO_C_SHF[16], FIFO_C_SHF[17], s_16_0, s_17_0)/*8*/
		.invoke(ADD_0, FIFO_C_SHF[18], FIFO_C_SHF[19], s_18_0, s_19_0)/*9*/
		.invoke(ADD_1, FIFO_C_SHF[20], FIFO_C_SHF[21], s_20_0, s_21_0)/*10*/
		.invoke(ADD_0, FIFO_C_SHF[22], FIFO_C_SHF[23], s_22_0, s_23_0)/*11*/
		.invoke(ADD_1, FIFO_C_SHF[24], FIFO_C_SHF[25], s_24_0, s_25_0)/*12*/
		.invoke(ADD_0, FIFO_C_SHF[26], FIFO_C_SHF[27], s_26_0, s_27_0)/*13*/
		.invoke(ADD_1, FIFO_C_SHF[28], FIFO_C_SHF[29], s_28_0, s_29_0)/*14*/
		.invoke(ADD_0, FIFO_C_SHF[30], FIFO_C_SHF[31], s_30_0, s_31_0)/*15*/
		.invoke(ADD_1, FIFO_C_SHF[32], FIFO_C_SHF[33], s_32_0, s_33_0)/*16*/
		.invoke(ADD_0, FIFO_C_SHF[34], FIFO_C_SHF[35], s_34_0, s_35_0)/*17*/
		.invoke(ADD_1, FIFO_C_SHF[36], FIFO_C_SHF[37], s_36_0, s_37_0)/*18*/
		.invoke(ADD_0, FIFO_C_SHF[38], FIFO_C_SHF[39], s_38_0, s_39_0)/*19*/
		.invoke(ADD_1, FIFO_C_SHF[40], FIFO_C_SHF[41], s_40_0, s_41_0)/*20*/
		.invoke(ADD_0, FIFO_C_SHF[42], FIFO_C_SHF[43], s_42_0, s_43_0)/*21*/
		.invoke(ADD_1, FIFO_C_SHF[44], FIFO_C_SHF[45], s_44_0, s_45_0)/*22*/
		.invoke(ADD_0, FIFO_C_SHF[46], FIFO_C_SHF[47], s_46_0, s_47_0)/*23*/
		.invoke(ADD_1, FIFO_C_SHF[48], FIFO_C_SHF[49], s_48_0, s_49_0)/*24*/
		.invoke(ADD_0, FIFO_C_SHF[50], FIFO_C_SHF[51], s_50_0, s_51_0)/*25*/
		.invoke(ADD_1, FIFO_C_SHF[52], FIFO_C_SHF[53], s_52_0, s_53_0)/*26*/
		.invoke(ADD_0, FIFO_C_SHF[54], FIFO_C_SHF[55], s_54_0, s_55_0)/*27*/
		.invoke(ADD_1, FIFO_C_SHF[56], FIFO_C_SHF[57], s_56_0, s_57_0)/*28*/
		.invoke(ADD_0, FIFO_C_SHF[58], FIFO_C_SHF[59], s_58_0, s_59_0)/*29*/
		.invoke(ADD_1, FIFO_C_SHF[60], FIFO_C_SHF[61], s_60_0, s_61_0)/*30*/
		.invoke(ADD_0, FIFO_C_SHF[62], FIFO_C_SHF[63], s_62_0, s_63_0)/*31*/
		.invoke(ADD_1, s_1_0, s_2_0, s_1_1, s_2_1)/*32*/
		.invoke(ADD_0, s_5_0, s_6_0, s_5_1, s_6_1)/*33*/
		.invoke(ADD_1, s_9_0, s_10_0, s_9_1, s_10_1)/*34*/
		.invoke(ADD_0, s_13_0, s_14_0, s_13_1, s_14_1)/*35*/
		.invoke(ADD_1, s_17_0, s_18_0, s_17_1, s_18_1)/*36*/
		.invoke(ADD_0, s_21_0, s_22_0, s_21_1, s_22_1)/*37*/
		.invoke(ADD_1, s_25_0, s_26_0, s_25_1, s_26_1)/*38*/
		.invoke(ADD_0, s_29_0, s_30_0, s_29_1, s_30_1)/*39*/
		.invoke(ADD_1, s_33_0, s_34_0, s_33_1, s_34_1)/*40*/
		.invoke(ADD_0, s_37_0, s_38_0, s_37_1, s_38_1)/*41*/
		.invoke(ADD_1, s_41_0, s_42_0, s_41_1, s_42_1)/*42*/
		.invoke(ADD_0, s_45_0, s_46_0, s_45_1, s_46_1)/*43*/
		.invoke(ADD_1, s_49_0, s_50_0, s_49_1, s_50_1)/*44*/
		.invoke(ADD_0, s_53_0, s_54_0, s_53_1, s_54_1)/*45*/
		.invoke(ADD_1, s_57_0, s_58_0, s_57_1, s_58_1)/*46*/
		.invoke(ADD_0, s_61_0, s_62_0, s_61_1, s_62_1)/*47*/
		.invoke(SSW, s_2_1, s_3_0, s_2_2, s_3_1)/*48*/
		.invoke(SSW, s_4_0, s_5_1, s_4_1, s_5_2)/*49*/
		.invoke(ADD_1, s_3_1, s_4_1, s_3_2, s_4_2)/*50*/
		.invoke(SSW, s_10_1, s_11_0, s_10_2, s_11_1)/*51*/
		.invoke(SSW, s_12_0, s_13_1, s_12_1, s_13_2)/*52*/
		.invoke(ADD_0, s_11_1, s_12_1, s_11_2, s_12_2)/*53*/
		.invoke(SSW, s_18_1, s_19_0, s_18_2, s_19_1)/*54*/
		.invoke(SSW, s_20_0, s_21_1, s_20_1, s_21_2)/*55*/
		.invoke(ADD_1, s_19_1, s_20_1, s_19_2, s_20_2)/*56*/
		.invoke(SSW, s_26_1, s_27_0, s_26_2, s_27_1)/*57*/
		.invoke(SSW, s_28_0, s_29_1, s_28_1, s_29_2)/*58*/
		.invoke(ADD_0, s_27_1, s_28_1, s_27_2, s_28_2)/*59*/
		.invoke(SSW, s_34_1, s_35_0, s_34_2, s_35_1)/*60*/
		.invoke(SSW, s_36_0, s_37_1, s_36_1, s_37_2)/*61*/
		.invoke(ADD_1, s_35_1, s_36_1, s_35_2, s_36_2)/*62*/
		.invoke(SSW, s_42_1, s_43_0, s_42_2, s_43_1)/*63*/
		.invoke(SSW, s_44_0, s_45_1, s_44_1, s_45_2)/*64*/
		.invoke(ADD_0, s_43_1, s_44_1, s_43_2, s_44_2)/*65*/
		.invoke(SSW, s_50_1, s_51_0, s_50_2, s_51_1)/*66*/
		.invoke(SSW, s_52_0, s_53_1, s_52_1, s_53_2)/*67*/
		.invoke(ADD_1, s_51_1, s_52_1, s_51_2, s_52_2)/*68*/
		.invoke(SSW, s_58_1, s_59_0, s_58_2, s_59_1)/*69*/
		.invoke(SSW, s_60_0, s_61_1, s_60_1, s_61_2)/*70*/
		.invoke(ADD_0, s_59_1, s_60_1, s_59_2, s_60_2)/*71*/
		.invoke(SSW, s_4_2, s_7_0, s_4_3, s_7_1)/*72*/
		.invoke(SSW, s_8_0, s_11_2, s_8_1, s_11_3)/*73*/
		.invoke(ADD_1, s_7_1, s_8_1, s_7_2, s_8_2)/*74*/
		.invoke(SSW, s_20_2, s_23_0, s_20_3, s_23_1)/*75*/
		.invoke(SSW, s_24_0, s_27_2, s_24_1, s_27_3)/*76*/
		.invoke(ADD_0, s_23_1, s_24_1, s_23_2, s_24_2)/*77*/
		.invoke(SSW, s_36_2, s_39_0, s_36_3, s_39_1)/*78*/
		.invoke(SSW, s_40_0, s_43_2, s_40_1, s_43_3)/*79*/
		.invoke(ADD_1, s_39_1, s_40_1, s_39_2, s_40_2)/*80*/
		.invoke(SSW, s_52_2, s_55_0, s_52_3, s_55_1)/*81*/
		.invoke(SSW, s_56_0, s_59_2, s_56_1, s_59_3)/*82*/
		.invoke(ADD_0, s_55_1, s_56_1, s_55_2, s_56_2)/*83*/
		.invoke(SSW, s_8_2, s_15_0, s_8_3, s_15_1)/*84*/
		.invoke(SSW, s_16_0, s_23_2, s_16_1, s_23_3)/*85*/
		.invoke(ADD_1, s_15_1, s_16_1, s_15_2, s_16_2)/*86*/
		.invoke(SSW, s_40_2, s_47_0, s_40_3, s_47_1)/*87*/
		.invoke(SSW, s_48_0, s_55_2, s_48_1, s_55_3)/*88*/
		.invoke(ADD_0, s_47_1, s_48_1, s_47_2, s_48_2)/*89*/
		.invoke(SSW, s_16_2, s_31_0, s_16_3, s_31_1)/*90*/
		.invoke(SSW, s_32_0, s_47_2, s_32_1, s_47_3)/*91*/
		.invoke(ADD_SWB, s_31_1, s_32_1, s_31_2, s_32_2)/*92*/
		.invoke(SSW, s_16_3, s_31_2, s_16_4, s_31_3)/*93*/
		.invoke(SSW, s_32_2, s_47_3, s_32_3, s_47_4)/*94*/
		.invoke(SWB1_4, s_15_2, s_16_4, s_15_3, s_16_5)/*95*/
		.invoke(SSW, s_8_3, s_15_3, s_8_4, s_15_4)/*96*/
		.invoke(SSW, s_16_5, s_23_3, s_16_6, s_23_4)/*97*/
		.invoke(SWB0_4, s_47_4, s_48_2, s_47_5, s_48_3)/*98*/
		.invoke(SSW, s_40_3, s_47_5, s_40_4, s_47_6)/*99*/
		.invoke(SSW, s_48_3, s_55_3, s_48_4, s_55_4)/*100*/
		.invoke(SWB1_3, s_7_2, s_8_4, s_7_3, s_8_5)/*101*/
		.invoke(SSW, s_4_3, s_7_3, s_4_4, s_7_4)/*102*/
		.invoke(SSW, s_8_5, s_11_3, s_8_6, s_11_4)/*103*/
		.invoke(SWB0_3, s_23_4, s_24_2, s_23_5, s_24_3)/*104*/
		.invoke(SSW, s_20_3, s_23_5, s_20_4, s_23_6)/*105*/
		.invoke(SSW, s_24_3, s_27_3, s_24_4, s_27_4)/*106*/
		.invoke(SWB1_3, s_39_2, s_40_4, s_39_3, s_40_5)/*107*/
		.invoke(SSW, s_36_3, s_39_3, s_36_4, s_39_4)/*108*/
		.invoke(SSW, s_40_5, s_43_3, s_40_6, s_43_4)/*109*/
		.invoke(SWB0_3, s_55_4, s_56_2, s_55_5, s_56_3)/*110*/
		.invoke(SSW, s_52_3, s_55_5, s_52_4, s_55_6)/*111*/
		.invoke(SSW, s_56_3, s_59_3, s_56_4, s_59_4)/*112*/
		.invoke(SWB1_2, s_3_2, s_4_4, s_3_3, s_4_5)/*113*/
		.invoke(SSW, s_2_2, s_3_3, s_2_3, s_3_4)/*114*/
		.invoke(SSW, s_4_5, s_5_2, s_4_6, s_5_3)/*115*/
		.invoke(SWB0_2, s_11_4, s_12_2, s_11_5, s_12_3)/*116*/
		.invoke(SSW, s_10_2, s_11_5, s_10_3, s_11_6)/*117*/
		.invoke(SSW, s_12_3, s_13_2, s_12_4, s_13_3)/*118*/
		.invoke(SWB1_2, s_19_2, s_20_4, s_19_3, s_20_5)/*119*/
		.invoke(SSW, s_18_2, s_19_3, s_18_3, s_19_4)/*120*/
		.invoke(SSW, s_20_5, s_21_2, s_20_6, s_21_3)/*121*/
		.invoke(SWB0_2, s_27_4, s_28_2, s_27_5, s_28_3)/*122*/
		.invoke(SSW, s_26_2, s_27_5, s_26_3, s_27_6)/*123*/
		.invoke(SSW, s_28_3, s_29_2, s_28_4, s_29_3)/*124*/
		.invoke(SWB1_2, s_35_2, s_36_4, s_35_3, s_36_5)/*125*/
		.invoke(SSW, s_34_2, s_35_3, s_34_3, s_35_4)/*126*/
		.invoke(SSW, s_36_5, s_37_2, s_36_6, s_37_3)/*127*/
		.invoke(SWB0_2, s_43_4, s_44_2, s_43_5, s_44_3)/*128*/
		.invoke(SSW, s_42_2, s_43_5, s_42_3, s_43_6)/*129*/
		.invoke(SSW, s_44_3, s_45_2, s_44_4, s_45_3)/*130*/
		.invoke(SWB1_2, s_51_2, s_52_4, s_51_3, s_52_5)/*131*/
		.invoke(SSW, s_50_2, s_51_3, s_50_3, s_51_4)/*132*/
		.invoke(SSW, s_52_5, s_53_2, s_52_6, s_53_3)/*133*/
		.invoke(SWB0_2, s_59_4, s_60_2, s_59_5, s_60_3)/*134*/
		.invoke(SSW, s_58_2, s_59_5, s_58_3, s_59_6)/*135*/
		.invoke(SSW, s_60_3, s_61_2, s_60_4, s_61_3)/*136*/
		.invoke(SWB1_1, s_1_1, s_2_3, s_1_2, s_2_4)/*137*/
		.invoke(SWB0_1, s_5_3, s_6_1, s_5_4, s_6_2)/*138*/
		.invoke(SWB1_1, s_9_1, s_10_3, s_9_2, s_10_4)/*139*/
		.invoke(SWB0_1, s_13_3, s_14_1, s_13_4, s_14_2)/*140*/
		.invoke(SWB1_1, s_17_1, s_18_3, s_17_2, s_18_4)/*141*/
		.invoke(SWB0_1, s_21_3, s_22_1, s_21_4, s_22_2)/*142*/
		.invoke(SWB1_1, s_25_1, s_26_3, s_25_2, s_26_4)/*143*/
		.invoke(SWB0_1, s_29_3, s_30_1, s_29_4, s_30_2)/*144*/
		.invoke(SWB1_1, s_33_1, s_34_3, s_33_2, s_34_4)/*145*/
		.invoke(SWB0_1, s_37_3, s_38_1, s_37_4, s_38_2)/*146*/
		.invoke(SWB1_1, s_41_1, s_42_3, s_41_2, s_42_4)/*147*/
		.invoke(SWB0_1, s_45_3, s_46_1, s_45_4, s_46_2)/*148*/
		.invoke(SWB1_1, s_49_1, s_50_3, s_49_2, s_50_4)/*149*/
		.invoke(SWB0_1, s_53_3, s_54_1, s_53_4, s_54_2)/*150*/
		.invoke(SWB1_1, s_57_1, s_58_3, s_57_2, s_58_4)/*151*/
		.invoke(SWB0_1, s_61_3, s_62_1, s_61_4, s_62_2)/*152*/
		.invoke(SWB1_0, s_0_0, s_1_2, FIFO_C_BUF[0], FIFO_C_BUF[1])/*153*/
		.invoke(SWB0_0, s_2_4, s_3_4, FIFO_C_BUF[2], FIFO_C_BUF[3])/*154*/
		.invoke(SWB1_0, s_4_6, s_5_4, FIFO_C_BUF[4], FIFO_C_BUF[5])/*155*/
		.invoke(SWB0_0, s_6_2, s_7_4, FIFO_C_BUF[6], FIFO_C_BUF[7])/*156*/
		.invoke(SWB1_0, s_8_6, s_9_2, FIFO_C_BUF[8], FIFO_C_BUF[9])/*157*/
		.invoke(SWB0_0, s_10_4, s_11_6, FIFO_C_BUF[10], FIFO_C_BUF[11])/*158*/
		.invoke(SWB1_0, s_12_4, s_13_4, FIFO_C_BUF[12], FIFO_C_BUF[13])/*159*/
		.invoke(SWB0_0, s_14_2, s_15_4, FIFO_C_BUF[14], FIFO_C_BUF[15])/*160*/
		.invoke(SWB1_0, s_16_6, s_17_2, FIFO_C_BUF[16], FIFO_C_BUF[17])/*161*/
		.invoke(SWB0_0, s_18_4, s_19_4, FIFO_C_BUF[18], FIFO_C_BUF[19])/*162*/
		.invoke(SWB1_0, s_20_6, s_21_4, FIFO_C_BUF[20], FIFO_C_BUF[21])/*163*/
		.invoke(SWB0_0, s_22_2, s_23_6, FIFO_C_BUF[22], FIFO_C_BUF[23])/*164*/
		.invoke(SWB1_0, s_24_4, s_25_2, FIFO_C_BUF[24], FIFO_C_BUF[25])/*165*/
		.invoke(SWB0_0, s_26_4, s_27_6, FIFO_C_BUF[26], FIFO_C_BUF[27])/*166*/
		.invoke(SWB1_0, s_28_4, s_29_4, FIFO_C_BUF[28], FIFO_C_BUF[29])/*167*/
		.invoke(SWB0_0, s_30_2, s_31_3, FIFO_C_BUF[30], FIFO_C_BUF[31])/*168*/
		.invoke(SWB1_0, s_32_3, s_33_2, FIFO_C_BUF[32], FIFO_C_BUF[33])/*169*/
		.invoke(SWB0_0, s_34_4, s_35_4, FIFO_C_BUF[34], FIFO_C_BUF[35])/*170*/
		.invoke(SWB1_0, s_36_6, s_37_4, FIFO_C_BUF[36], FIFO_C_BUF[37])/*171*/
		.invoke(SWB0_0, s_38_2, s_39_4, FIFO_C_BUF[38], FIFO_C_BUF[39])/*172*/
		.invoke(SWB1_0, s_40_6, s_41_2, FIFO_C_BUF[40], FIFO_C_BUF[41])/*173*/
		.invoke(SWB0_0, s_42_4, s_43_6, FIFO_C_BUF[42], FIFO_C_BUF[43])/*174*/
		.invoke(SWB1_0, s_44_4, s_45_4, FIFO_C_BUF[44], FIFO_C_BUF[45])/*175*/
		.invoke(SWB0_0, s_46_2, s_47_6, FIFO_C_BUF[46], FIFO_C_BUF[47])/*176*/
		.invoke(SWB1_0, s_48_4, s_49_2, FIFO_C_BUF[48], FIFO_C_BUF[49])/*177*/
		.invoke(SWB0_0, s_50_4, s_51_4, FIFO_C_BUF[50], FIFO_C_BUF[51])/*178*/
		.invoke(SWB1_0, s_52_6, s_53_4, FIFO_C_BUF[52], FIFO_C_BUF[53])/*179*/
		.invoke(SWB0_0, s_54_2, s_55_6, FIFO_C_BUF[54], FIFO_C_BUF[55])/*180*/
		.invoke(SWB1_0, s_56_4, s_57_2, FIFO_C_BUF[56], FIFO_C_BUF[57])/*181*/
		.invoke(SWB0_0, s_58_4, s_59_6, FIFO_C_BUF[58], FIFO_C_BUF[59])/*182*/
		.invoke(SWB1_0, s_60_4, s_61_4, FIFO_C_BUF[60], FIFO_C_BUF[61])/*183*/
		.invoke(SWB0_0, s_62_2, s_63_0, FIFO_C_BUF[62], FIFO_C_BUF[63])/*184*/
#else
		.invoke<tapa::join, NUM_PES_HALF>(PreAccumulator, FIFO_C_ROW, FIFO_C_VAL, FIFO_C_FLAG, FIFO_C_BUF)
#endif
        .invoke<tapa::join, NUM_PES>(AccumBuffer, FIFO_C_BUF, FIFO_C_ARB, num_rows_per_pe, num_tiles_c, rp_time)
		.invoke(Arbiter_C, FIFO_C_ARB, FIFO_C_AB, num_rows_per_pe, rp_time)
        .invoke<tapa::join, NUM_C_CH>(MM2S_C, c_in, FIFO_C_IN, num_rows_per_pe, rp_time)
        .invoke<tapa::join, NUM_C_CH>(Compute_C, FIFO_C_IN, FIFO_C_AB, FIFO_C_OUT, alpha, beta, num_rows_per_pe, rp_time)
        .invoke<tapa::join, NUM_C_CH>(S2MM_C, FIFO_C_OUT, c_out, num_rows_per_pe, rp_time);
}