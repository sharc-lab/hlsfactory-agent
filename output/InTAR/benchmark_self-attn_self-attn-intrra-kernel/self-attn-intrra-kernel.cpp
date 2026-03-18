#include <iostream>
#include <cmath>
#include <numeric>
#include <string>
#include <tapa.h>
#include <ap_int.h>
#include <hls_math.h>

constexpr int seq_len = 256;
constexpr int D = 1024;
constexpr int D_head = 1024;
constexpr int D_head_div_2 = D_head / 2;
constexpr int D_head_div_4 = D_head / 4;
constexpr int D_head_div_8 = D_head / 8;
constexpr int D_head_div_16 = D_head / 16;
constexpr int weight_size_cc0 = D * D_head / 16;
constexpr int weight_size_cc1 = D * D_head / 8;
constexpr int input_size = seq_len * D / 16;
constexpr int output_size = seq_len * D_head / 32;
constexpr int merge_size = (D / 32) * 3;
constexpr float scale = 1/32.0;

using int16_v16 = tapa::vec_t<ap_int<16>, 16>;

struct ConfigInst {
    ap_uint<3> stage;
    ap_uint<9> i_bound;
    ap_uint<9> j_bound;
    ap_uint<11> k_bound;
};

// Kernel

void send_inst_cc0(
    tapa::ostream<ConfigInst>& fifo_inst
){
    for(int stage = 0; stage < 4; stage++){
        ConfigInst inst;
        inst.stage = ap_uint<3>(stage);

        if(stage == 0 || stage == 2){
            inst.i_bound = (D_head >> 5);
            inst.j_bound = seq_len;
            inst.k_bound = D;
        } else if(stage == 1) {
            inst.i_bound = seq_len;
            inst.j_bound = seq_len;
            inst.k_bound = (D_head >> 4);
        } else {
            inst.i_bound = seq_len;
            inst.j_bound = (D_head >> 5);
            inst.k_bound = seq_len;
        }

        fifo_inst.write(inst);
    }
}

void send_inst_cc1(
    tapa::ostream<ConfigInst>& fifo_inst
){
    for(int stage = 0; stage < 3; stage++){
        ConfigInst inst;
        inst.stage = ap_uint<3>(stage);

        if(stage == 0){
            inst.i_bound = merge_size;
            inst.j_bound = seq_len;
            inst.k_bound = D;
        } else if(stage == 1) {
            inst.i_bound = (D_head >> 5);
            inst.j_bound = seq_len;
            inst.k_bound = D;
        } else {
            inst.i_bound = seq_len;
            inst.j_bound = (D_head >> 5);
            inst.k_bound = seq_len;
        }

        fifo_inst.write(inst);
    }
}

void read_W(
    const int w_size,
    tapa::async_mmap<int16_v16>& vec,
    tapa::ostream<int16_v16>& fifo_out
){
    for(int i_req = 0, i_resp = 0; i_resp < w_size;){
        #pragma HLS pipeline II=1 style=stp
        if((i_req < w_size) & !vec.read_addr.full()){
            vec.read_addr.write(i_req);
            i_req++;
        }
        int16_v16 tmp_o; 
        bool success = vec.read_data.try_read(tmp_o);
        if(success){
            fifo_out.write(tmp_o);
            i_resp++;
        }
    }
}

void read_X(
    tapa::async_mmap<int16_v16>& vec,
    tapa::ostream<int16_v16>& fifo_out
){
    for(int i_req = 0, i_resp = 0; i_resp < input_size;){
        #pragma HLS pipeline II=1 style=stp
        if((i_req < input_size) & !vec.read_addr.full()){
            vec.read_addr.write(i_req);
            i_req++;
        }
        int16_v16 tmp_o; 
        bool success = vec.read_data.try_read(tmp_o);
        if(success){
            fifo_out.write(tmp_o);
            i_resp++;
        }
    }
}

void write_mtx(
    tapa::async_mmap<int16_v16>& output_mtx,
    tapa::istream<int16_v16>& fifo_in,
    tapa::ostream<bool>& fifo_fin
){

    for(int i_req = 0, i_resp = 0; i_resp < output_size;){
        #pragma HLS pipeline II=1 style=stp
        if((i_req < output_size) & !fifo_in.empty() & !output_mtx.write_addr.full() & !output_mtx.write_data.full()){
            output_mtx.write_addr.try_write(i_req);
            int16_v16 tmp; fifo_in.try_read(tmp);
            output_mtx.write_data.try_write(tmp);
            ++i_req;
        }
        bool success = false;
        auto resp = output_mtx.write_resp.read(success);
        if(success){
            i_resp += unsigned(resp)+1;
        }
    }

    fifo_fin.write(true);
} 

void CC0_QAV_Proj(
    tapa::istream<int16_v16>& fifo_input,
    tapa::istream<int16_v16>& fifo_weight,
    tapa::istream<ConfigInst>& fifo_inst,
    tapa::istream<int16_v16>& fifo_from_CC1,
    tapa::ostream<ap_int<16>>& fifo_to_SFU,
    tapa::istream<ap_int<16>>& fifo_from_SFU,
    tapa::ostream<int16_v16>& fifo_output
){
    // stage 1: compute Q
    ap_int<16> X[seq_len][D];
    ap_uint<64> scratchpad[seq_len][D_head_div_4];

    #pragma HLS array_partition variable=X cyclic dim=2 factor=16
    #pragma HLS array_partition variable=scratchpad cyclic dim=2 factor=8
    #pragma HLS bind_storage variable=scratchpad type=ram_2p impl=uram

    for(int i = 0; i < seq_len; i++){
        for(int j = 0; j < (D >> 4);){
            #pragma HLS pipeline II=1
            #pragma HLS loop_tripcount min=D_head_div_16 max=D_head_div_16

            if(!fifo_input.empty()){
                int16_v16 tmp; fifo_input.try_read(tmp);
                for(int k = 0; k < 16; k++){
                    #pragma HLS unroll
                    X[i][j*16+k] = tmp[k];
                }
                j++;
            }
        }
    }

    for(int st = 0; st < 4; st++){
        ConfigInst inst = fifo_inst.read();
        const int stage = inst.stage;
        const int i_bound = inst.i_bound;
        const int j_bound = inst.j_bound;
        const int k_bound = inst.k_bound;

        for(int i = 0; i < i_bound; i++){
            ap_int<16> weight[16][D];
            #pragma HLS array_partition variable=weight complete dim=1 

            ap_int<16> cache_k[D_head];
            #pragma HLS array_partition variable=cache_k cyclic factor=16

            if(stage == 0 || stage == 2){
                for(int k = 0; k < D;){
                    #pragma HLS pipeline II=1
                    #pragma HLS loop_tripcount min=D max=D

                    if(!fifo_weight.empty()){
                        int16_v16 tmp; fifo_weight.try_read(tmp);
                        for(int l = 0; l < 16; l++){
                            #pragma HLS unroll
                            weight[l][k] = tmp[l];
                        }
                        k++;
                    }
                }
            } else if(stage == 1){
                for(int j = 0; j < (D_head >> 4);){
                    #pragma HLS pipeline II=1
                    #pragma HLS loop_tripcount min=D_head_div_16 max=D_head_div_16

                    if(!fifo_from_CC1.empty()){
                        int16_v16 tmp; fifo_from_CC1.try_read(tmp);
                        for(int k = 0; k < 16; k++){
                            cache_k[k] = tmp[k];
                        }
                        j++;
                    }
                }
            }

            for(int j = 0; j < j_bound; j++){

                int16_v16 sum;

                for(int k = 0; k < 16; k++){
                    #pragma HLS unroll
                    sum[k] = 0;
                }

            compute:
                for(int k = 0; k < k_bound; k++){
                    #pragma HLS pipeline II=1

                    ap_int<16> op1_mtx[16];
                    ap_int<16> op2_mtx[16];
                    #pragma HLS array_partition variable=op1_mtx complete
                    #pragma HLS array_partition variable=op2_mtx complete

                    ap_int<16> tmp = 0;
                    if(stage == 3) tmp = fifo_from_SFU.read();

                    for(int l = 0; l < 16; l++){
                        #pragma HLS unroll
                        if(stage == 0 || stage == 2){
                            op1_mtx[l] = X[j][k];
                            op2_mtx[l] = weight[l][k];
                        } else if(stage == 1){
                            op1_mtx[l] = cache_k[k*16+l];
                            op2_mtx[l] = ap_int<16>(scratchpad[j][k*4+l/4]((l%4)*16+15, (l%4)*16));
                        } else {
                            op1_mtx[l] = tmp;
                            op2_mtx[l] = ap_int<16>(scratchpad[j][k*4+l/4]((l%4)*16+15, (l%4)*16));
                        }
                    }

                    for(int l = 0; l < 16; l++){
                        #pragma HLS unroll

                        ap_int<16> op1 = op1_mtx[l];
                        ap_int<16> op2 = op2_mtx[l];

                        sum[l] += op1 * op2;
                    }
                }

                if(stage == 1){

                reduce:
                    for(int k = 1; k < 16; k++){
                        #pragma HLS pipeline II=1
                        sum[0] += sum[k];
                    }
                }

                if(stage == 0){
                    int16_v16 tmp_acc1 = fifo_from_CC1.read();
                store:
                    for(int k = 0; k < 4; k++){
                        #pragma HLS unroll
                        ap_uint<64> tmp_a = 0;
                        ap_uint<64> tmp_b = 0;
                        for(int l = 0; l < 4; l++){
                            #pragma HLS unroll
                            tmp_a(l*16+15, l*16) = sum[k*4+l];
                            tmp_b(l*16+15, l*16) = tmp_acc1[k*4+l];
                        }
                        scratchpad[j][i*8+k] = tmp_a;
                        scratchpad[j][i*8+k+4] = tmp_b;
                    }
                } else if(stage == 1){
                    fifo_to_SFU.write((sum[0] >> 5));
                } else if(stage == 2){
                    for(int k = 0; k < 4; k++){
                        #pragma HLS unroll
                        ap_uint<64> tmp = 0;
                        for(int l = 0; l < 4; l++){
                            #pragma HLS unroll
                            tmp(l*16+15, l*16) = sum[k*4+l];
                        }
                        scratchpad[i][j*4+k] = tmp; 
                    }
                } else {
                    fifo_output.write(sum);
                }
            }
        }
    }
}

void CC1_QKV_Proj(
    tapa::istream<int16_v16>& fifo_input,
    tapa::istream<int16_v16>& fifo_weight,
    tapa::istream<ConfigInst>& fifo_inst,
    tapa::ostream<int16_v16>& fifo_to_CC0,
    tapa::istream<ap_int<16>>& fifo_from_SFU,
    tapa::ostream<int16_v16>& fifo_output
){
    // stage 1: compute Q
    //stage 2: compute K
    ap_int<16> X[seq_len][D];
    ap_uint<64> scratchpad[seq_len][D_head_div_8];
    #pragma HLS array_partition variable=X cyclic dim=2 factor=16
    #pragma HLS array_partition variable=scratchpad cyclic dim=2 factor=8
    #pragma HLS bind_storage variable=scratchpad type=ram_2p impl=uram

    for(int i = 0; i < seq_len; i++){
        for(int j = 0; j < (D >> 4);){
            #pragma HLS pipeline II=1
            #pragma HLS loop_tripcount min=D_head_div_16 max=D_head_div_16

            if(!fifo_input.empty()){
                int16_v16 tmp; fifo_input.try_read(tmp);
                for(int k = 0; k < 16; k++){
                    X[i][j*16+k] = tmp[k];
                }
                j++;
            }
        }
    }

    for(int st = 0; st < 3; st++){

        ConfigInst inst = fifo_inst.read();
        const int stage = inst.stage;
        const int i_bound = inst.i_bound;
        const int j_bound = inst.j_bound;
        const int k_bound = inst.k_bound;

        for(int i = 0; i < i_bound; i++){
            ap_int<16> weight[16][D];
            #pragma HLS array_partition variable=weight complete dim=1 

            if(stage < 2){
                for(int k = 0; k < D;){
                    #pragma HLS pipeline II=1
                    
                    if(!fifo_weight.empty()){
                        int16_v16 tmp; fifo_weight.try_read(tmp);
                        for(int l = 0; l < 16; l++){
                            #pragma HLS unroll
                            weight[l][k] = tmp[l];
                        }
                        k++;
                    }
                }
            }

            for(int j = 0; j < j_bound; j++){

                int16_v16 sum;

                for(int k = 0; k < 16; k++){
                    #pragma HLS unroll
                    sum[k] = 0;
                }

            compute:
                for(int k = 0; k < k_bound; k++){
                    #pragma HLS pipeline II=1

                    ap_int<16> op1_mtx[16];
                    ap_int<16> op2_mtx[16];
                    #pragma HLS array_partition variable=op1_mtx complete
                    #pragma HLS array_partition variable=op2_mtx complete

                    ap_int<16> tmp = 0;
                    if(stage == 2) tmp = fifo_from_SFU.read();

                    for(int l = 0; l < 16; l++){
                        #pragma HLS unroll
                        if(stage < 2){
                            op1_mtx[l] = X[j][k];
                            op2_mtx[l] = weight[l][k];
                        } else {
                            op1_mtx[l] = tmp;
                            op2_mtx[l] = ap_int<16>(scratchpad[j][k*4+l/4]((l%4)*16+15, (l%4)*16));
                        }
                    }

                    for(int l = 0; l < 16; l++){
                        #pragma HLS unroll

                        ap_int<16> op1 = op1_mtx[l];
                        ap_int<16> op2 = op2_mtx[l];

                        sum[l] += op1 * op2;
                    }
                }

                if(stage == 0){
                    fifo_to_CC0.write(sum);
                } else if (stage == 1){
                    for(int k = 0; k < 4; k++){
                        #pragma HLS unroll
                        ap_uint<64> tmp = 0;
                        for(int l = 0; l < 4; l++){
                            #pragma HLS unroll
                            tmp(l*16+15, l*16) = sum[k*4+l];
                        }
                        scratchpad[i][j*4+k] = tmp; 
                    }
                } else {
                    fifo_output.write((sum >> 6));
                }
            }
        }
    }
}

void SFU_softmax_exp(
    tapa::istream<ap_int<16>>& fifo_from_CC0,
    tapa::ostream<float>& fifo_out
){
    
    for(int i = 0; i < seq_len; i++){
        for(int j = 0; j < seq_len; j++){
            #pragma HLS pipeline II=1

            ap_int<16> tmp = fifo_from_CC0.read();
            float score = hls::exp(tmp);
            fifo_out.write(score);
        }
    }
}

void SFU_softmax_norm(
    tapa::istream<float>& fifo_from_SFU,
    tapa::ostream<ap_int<16>>& fifo_to_CC0,
    tapa::ostream<ap_int<16>>& fifo_to_CC1
){
    float scaled_attn[seq_len][seq_len];
    
    for(int i = 0; i < seq_len; i++){
        float sum[8];
        #pragma HLS array_partition variable=sum cyclic factor=4
        for(int j = 0; j < 8; j++){
            #pragma HLS unroll
            sum[j] = 0;
        }
        for(int j = 0; j < seq_len; j++){
            #pragma HLS pipeline II=1
            #pragma HLS dependence false variable=sum
            #pragma HLS dependence true variable=sum distance=8

            float score = fifo_from_SFU.read();
            sum[j%8] += score;
            scaled_attn[i][j] = score;
        }
        for(int j = 0; j < 4; j++){
            #pragma HLS unroll
            sum[j] += sum[j+4];
        }
        for(int j = 0; j < 2; j++){
            #pragma HLS unroll
            sum[j] += sum[j+2];
        }
        sum[0] += sum[1];
        sum[0] = 1 / sum[0];
        for(int j = 0; j < seq_len; j++){
            #pragma HLS pipeline II=1
            scaled_attn[i][j] *= sum[0];
        }
    }

    // send back to CC
    for(int i = 0; i < seq_len; i++){
        for(int j = 0; j < (D_head >> 5); j++){
            for(int k = 0; k < seq_len; k++){
                #pragma HLS pipeline II=1
                ap_int<16> cast_val = ap_int<16>((int)(scaled_attn[i][k] * 64));
                fifo_to_CC0.write(cast_val);
                fifo_to_CC1.write(cast_val);
            }
        }
    }
}

void measure_cycle(tapa::istreams<bool, 2>& fifo_fin, tapa::mmap<int> cycle_count){
    for(int cycle = 0;;cycle++){
        bool flag_cont = false;
        for(int i = 0; i < 2; i++){
            flag_cont |= fifo_fin[i].empty();
        }
        if(!flag_cont){
            for(int i = 0; i < 2; i++){
                fifo_fin[i].read(nullptr);
            }
            cycle_count[0] = cycle;
            break;
        }
    }
}

// Self-attention computation
void selfAttention(
    tapa::mmap<int16_v16> X_acc0,
    tapa::mmap<int16_v16> X_acc1,
    tapa::mmap<int16_v16> W_acc0,
    tapa::mmap<int16_v16> W_acc1,
    tapa::mmap<int16_v16> acc0_out,
    tapa::mmap<int16_v16> acc1_out,
    tapa::mmap<int> cycle_count
) {
   
    tapa::stream<int16_v16> fifo_input_CC0("fifo_input_CC0");
    tapa::stream<int16_v16> fifo_input_CC1("fifo_input_CC1");
    tapa::stream<int16_v16> fifo_weight_CC0("fifo_weight_CC0");
    tapa::stream<int16_v16> fifo_weight_CC1("fifo_weight_CC1");

    tapa::stream<ap_int<16>> fifo_from_CC0_to_SFU("fifo_from_CC0_to_SFU");
    tapa::stream<ap_int<16>> fifo_from_SFU_to_CC0("fifo_from_SFU_to_CC0");
    tapa::stream<ap_int<16>> fifo_from_SFU_to_CC1("fifo_from_SFU_to_CC1");
    tapa::stream<int16_v16> fifo_from_CC1_to_CC0("fifo_from_CC1_to_CC0");

    tapa::stream<float, seq_len> fifo_between_SFU("fifo_between_SFU");

    tapa::stream<int16_v16> fifo_output_CC0("fifo_output_CC0");
    tapa::stream<int16_v16> fifo_output_CC1("fifo_output_CC1");

    tapa::streams<bool, 2> fifo_fin("fifo_fin"); 

    tapa::stream<ConfigInst> fifo_inst_cc0("fifo_inst_cc0");
    tapa::stream<ConfigInst> fifo_inst_cc1("fifo_inst_cc1");

    tapa::task()
        .invoke<tapa::join>(read_W, weight_size_cc0, W_acc0, fifo_weight_CC0)
        .invoke<tapa::join>(read_W, weight_size_cc1, W_acc1, fifo_weight_CC1)
        .invoke<tapa::join>(read_X, X_acc0, fifo_input_CC0)
        .invoke<tapa::join>(read_X, X_acc1, fifo_input_CC1)
        .invoke<tapa::join>(send_inst_cc0, fifo_inst_cc0)
        .invoke<tapa::join>(send_inst_cc1, fifo_inst_cc1)
        .invoke<tapa::join>(CC0_QAV_Proj, fifo_input_CC0, fifo_weight_CC0, fifo_inst_cc0, fifo_from_CC1_to_CC0, fifo_from_CC0_to_SFU, fifo_from_SFU_to_CC0, fifo_output_CC0)
        .invoke<tapa::join>(SFU_softmax_exp, fifo_from_CC0_to_SFU, fifo_between_SFU)
        .invoke<tapa::join>(SFU_softmax_norm, fifo_between_SFU, fifo_from_SFU_to_CC0, fifo_from_SFU_to_CC1)
        .invoke<tapa::join>(CC1_QKV_Proj, fifo_input_CC1, fifo_weight_CC1, fifo_inst_cc1, fifo_from_CC1_to_CC0, fifo_from_SFU_to_CC1, fifo_output_CC1)
        .invoke<tapa::join>(write_mtx, acc0_out, fifo_output_CC0, fifo_fin)
        .invoke<tapa::join>(write_mtx, acc1_out, fifo_output_CC1, fifo_fin)
        .invoke<tapa::join>(measure_cycle, fifo_fin, cycle_count);
}