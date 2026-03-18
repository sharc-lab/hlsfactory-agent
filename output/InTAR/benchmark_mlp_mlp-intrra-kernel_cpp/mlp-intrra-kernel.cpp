#include <iostream>
#include <cmath>
#include <numeric>
#include <string>
#include <tapa.h>
#include <ap_int.h>
#include <hls_math.h>

constexpr int input_size = 256;     // Number of input features
constexpr int hidden_size1 = 1024;   // Number of neurons in the first hidden layer
constexpr int hidden_size1_div_2 = hidden_size1 / 2;
constexpr int hidden_size2 = 2048;   // Number of neurons in the second hidden layer
constexpr int output_size = 256;    // Number of output classes
constexpr int output_size_div_16 = output_size / 16;
constexpr int weight_size_cc0 = input_size * hidden_size1 / 32 + hidden_size1 * hidden_size2 /16;
constexpr int weight_size_cc1 = input_size * hidden_size1 / 32 + hidden_size2 * output_size /16;

using int16_v16 = tapa::vec_t<ap_int<16>, 16>;

struct ConfigInst {
    ap_uint<2> stage;
    ap_uint<11> i_bound;
    ap_uint<11> j_bound;
};

void send_inst_cc0(
    tapa::ostream<ConfigInst>& fifo_inst
){
    for(int stage = 0; stage < 2; stage++){
        ConfigInst inst;
        inst.stage = ap_uint<2>(stage);

        if(stage == 0){
            inst.i_bound = (hidden_size1 >> 5);
            inst.j_bound = input_size;
        } else {
            inst.i_bound = (hidden_size2 >> 4);
            inst.j_bound = hidden_size1;
        }

        fifo_inst.write(inst);
    }
}

void send_inst_cc1(
    tapa::ostream<ConfigInst>& fifo_inst
){
    for(int stage = 0; stage < 2; stage++){
        ConfigInst inst;
        inst.stage = ap_uint<2>(stage);

        if(stage == 0){
            inst.i_bound = (hidden_size1 >> 5);
            inst.j_bound = input_size;
        } else {
            inst.i_bound = (hidden_size2 >> 4);
            inst.j_bound = output_size;
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
    for(int i_req = 0, i_resp = 0; i_resp < (input_size >> 4);){
        #pragma HLS pipeline II=1 style=stp
        if((i_req < (input_size >> 4)) & !vec.read_addr.full()){
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

    for(int i_req = 0, i_resp = 0; i_resp < (output_size >> 4);){
        #pragma HLS pipeline II=1 style=stp
        if((i_req < (output_size >> 4)) & !fifo_in.empty() & !output_mtx.write_addr.full() & !output_mtx.write_data.full()){
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


void CC0_F1_F2(
    tapa::istream<int16_v16>& fifo_input,
    tapa::istream<int16_v16>& fifo_weight,
    tapa::istream<ConfigInst>& fifo_inst,
    tapa::istream<int16_v16>& fifo_from_CC1,
    tapa::ostream<int16_v16>& fifo_to_CC1
){

    ap_int<16> X[input_size];
    ap_int<16> scratchpad[hidden_size1];

    #pragma HLS array_partition variable=X cyclic factor=16
    #pragma HLS array_partition variable=scratchpad cyclic factor=32

    for(int i = 0; i < (input_size >> 4);){
        #pragma HLS pipeline II=1

        if(!fifo_input.empty()){
            int16_v16 tmp; fifo_input.try_read(tmp);
            for(int k = 0; k < 16; k++){
                #pragma HLS unroll
                X[i*16+k] = tmp[k];
            }
            i++;
        }
    }

    for(int st = 0; st < 2; st++){
        ConfigInst inst = fifo_inst.read();
        const int stage = inst.stage;
        const int i_bound = inst.i_bound;
        const int j_bound = inst.j_bound;

        for(int i = 0; i < i_bound; i++){
            
            int16_v16 sum;
            for(int k = 0; k < 16; k++){
                #pragma HLS unroll
                sum[k] = 0;
            }

            for(int j = 0; j < j_bound; j++){
                #pragma HLS pipeline II=1

                int16_v16 tmp = fifo_weight.read();

                ap_int<16> op1;
                if(stage == 0){
                    op1 = X[j];
                } else {
                    op1 = scratchpad[j];
                }

                for(int k = 0; k < 16; k++){
                    #pragma HLS unroll
                    sum[k] += op1 * tmp[k];
                }
            }

            if(stage == 0){
                int16_v16 tmp_acc1 = fifo_from_CC1.read();
                for(int j = 0; j < 16; j++){
                    #pragma HLS unroll
                    ap_int<16> sum_relu = (sum[j] < 0) ? ap_int<16>(0) : sum[j];
                    ap_int<16> tmp_relu = (tmp_acc1[j] < 0) ? ap_int<16>(0) : tmp_acc1[j];
                    scratchpad[i*32+j] = sum_relu;
                    scratchpad[i*32+j+16] = tmp_relu;
                }
            } else {
                fifo_to_CC1.write(sum);
            }
        }
    }
}

void CC1_F1_F3(
    tapa::istream<int16_v16>& fifo_input,
    tapa::istream<int16_v16>& fifo_weight,
    tapa::istream<ConfigInst>& fifo_inst,
    tapa::istream<int16_v16>& fifo_from_CC0,
    tapa::ostream<int16_v16>& fifo_to_CC0,
    tapa::ostream<int16_v16>& fifo_output
){
    ap_int<16> X[input_size];
    #pragma HLS array_partition variable=X cyclic factor=16

    ap_int<16> scratchpad[output_size];
    #pragma HLS array_partition variable=scratchpad cyclic factor=16


    for(int i = 0; i < (input_size >> 4);){
        #pragma HLS pipeline II=1

        if(!fifo_input.empty()){
            int16_v16 tmp; fifo_input.try_read(tmp);
            for(int k = 0; k < 16; k++){
                #pragma HLS unroll
                X[i*16+k] = tmp[k];
            }
            i++;
        }
    }


    for(int st = 0; st < 2; st++){
    
        ConfigInst inst = fifo_inst.read();
        const int stage = inst.stage;
        const int i_bound = inst.i_bound;
        const int j_bound = inst.j_bound;

        if(stage == 1) {
            for(int i = 0; i < (output_size >> 4); i++){
                for(int j = 0; j < 16; j++){
                    #pragma HLS unroll
                    scratchpad[i*16+j] = 0;
                }
            }
        }

        for(int i = 0; i < i_bound; i++){
            
            int16_v16 sum;

            if(stage == 0){
                for(int k = 0; k < 16; k++){
                    #pragma HLS unroll
                    sum[k] = 0;
                }
            } else {
                sum = fifo_from_CC0.read();
            }

            for(int j = 0; j < j_bound; j++){
                #pragma HLS pipeline II=1

                int16_v16 tmp = fifo_weight.read();

                ap_int<16> op_res_mtx[16];
                ap_int<16> op1_mtx[16];
                #pragma HLS array_partition variable=op_res_mtx complete
                #pragma HLS array_partition variable=op1_mtx complete

                for(int k = 0; k < 16; k++){
                    #pragma HLS unroll
                    if(stage == 0){
                        op_res_mtx[k] = sum[k];
                        op1_mtx[k] = X[j];
                    } else {
                        op_res_mtx[k] = scratchpad[(j%output_size_div_16)*16+k];
                        op1_mtx[k] = sum[j/output_size_div_16];
                    }
                }

                for(int k = 0; k < 16; k++){
                    #pragma HLS unroll

                    op_res_mtx[k] += op1_mtx[k] * tmp[k];

                }

                for(int k = 0; k < 16; k++){
                    #pragma HLS unroll
                    if(stage == 0){
                        sum[k] = op_res_mtx[k];
                    } else {
                        scratchpad[(j%output_size_div_16)*16+k] = op_res_mtx[k];
                    }
                }
            }

            if(stage == 0){
                fifo_to_CC0.write(sum);
            }
        }

    }

    for(int i = 0; i < (output_size >> 4); i++){
        #pragma HLS pipeline II=1

        int16_v16 tmp_out;
        for(int k = 0; k < 16; k++){
            #pragma HLS unroll
            tmp_out[k] = scratchpad[i*16+k];
        }
        fifo_output.write(tmp_out);
    }

}

// Helper function for ReLU activation
void relu(tapa::istream<int16_v16>& fifo_act_in, tapa::ostream<int16_v16>& fifo_act_out) {
    for(;;){
        if(!fifo_act_in.empty()){
            int16_v16 tmp; fifo_act_in.try_read(tmp);
            for(int i = 0; i < 16; i++){
                #pragma HLS unroll
                tmp[i] = (tmp[i] < 0) ? ap_int<16> (0) : tmp[i];
            }
            fifo_act_out.write(tmp);
        }
    }
}

void measure_cycle(tapa::istream<bool>& fifo_fin, tapa::mmap<int> cycle_count){
    for(int cycle = 0;;cycle++){
        if(!fifo_fin.empty()){
            fifo_fin.read(nullptr);
            cycle_count[0] = cycle;
            break;
        }
    }
}

// Multilayer Perceptron: Forward pass with two hidden layers
void MLP(
    tapa::mmap<int16_v16> X_acc0,
    tapa::mmap<int16_v16> X_acc1,
    tapa::mmap<int16_v16> W_acc0,
    tapa::mmap<int16_v16> W_acc1,
    tapa::mmap<int16_v16> acc1_out,
    tapa::mmap<int> cycle_count
) {
    tapa::stream<int16_v16> fifo_input_CC0("fifo_input_CC0");
    tapa::stream<int16_v16> fifo_input_CC1("fifo_input_CC1");
    tapa::stream<int16_v16> fifo_weight_CC0("fifo_weight_CC0");
    tapa::stream<int16_v16> fifo_weight_CC1("fifo_weight_CC1");

    tapa::stream<int16_v16> fifo_from_CC1_to_SFU("fifo_from_CC1_to_SFU");
    tapa::stream<int16_v16> fifo_from_CC0_to_SFU("fifo_from_CC0_to_SFU");
    tapa::stream<int16_v16> fifo_from_SFU_to_CC0("fifo_from_SFU_to_CC0");
    tapa::stream<int16_v16> fifo_from_SFU_to_CC1("fifo_from_SFU_to_CC1");

    tapa::stream<bool> fifo_fin("fifo_fin");
    tapa::stream<int16_v16> fifo_output("fifo_output");
    tapa::stream<int16_v16> fifo_output_relu("fifo_output_relu");

    tapa::stream<ConfigInst> fifo_inst_cc0("fifo_inst_cc0");
    tapa::stream<ConfigInst> fifo_inst_cc1("fifo_inst_cc1");

    tapa::task()
        .invoke<tapa::join>(read_W, weight_size_cc0, W_acc0, fifo_weight_CC0)
        .invoke<tapa::join>(read_W, weight_size_cc1, W_acc1, fifo_weight_CC1)
        .invoke<tapa::join>(read_X, X_acc0, fifo_input_CC0)
        .invoke<tapa::join>(read_X, X_acc1, fifo_input_CC1)
        .invoke<tapa::join>(send_inst_cc0, fifo_inst_cc0)
        .invoke<tapa::join>(send_inst_cc1, fifo_inst_cc1)
        .invoke<tapa::join>(CC0_F1_F2, fifo_input_CC0, fifo_weight_CC0, fifo_inst_cc0, fifo_from_SFU_to_CC0, fifo_from_CC0_to_SFU)
        .invoke<tapa::detach>(relu, fifo_from_CC0_to_SFU, fifo_from_SFU_to_CC1)
        .invoke<tapa::detach>(relu, fifo_from_CC1_to_SFU, fifo_from_SFU_to_CC0)
        .invoke<tapa::join>(CC1_F1_F3, fifo_input_CC1, fifo_weight_CC1, fifo_inst_cc1, fifo_from_SFU_to_CC1, fifo_from_CC1_to_SFU, fifo_output)
        .invoke<tapa::detach>(relu, fifo_output, fifo_output_relu)
        .invoke<tapa::join>(write_mtx, acc1_out, fifo_output_relu, fifo_fin)
        .invoke<tapa::join>(measure_cycle, fifo_fin, cycle_count);
    
}
