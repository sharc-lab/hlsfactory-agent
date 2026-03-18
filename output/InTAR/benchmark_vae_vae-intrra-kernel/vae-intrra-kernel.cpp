#include <iostream>
#include <cmath>
#include <numeric>
#include <string>
#include <tapa.h>
#include <ap_int.h>
#include <hls_math.h>

// Hyperparameters
constexpr int num_channel = 2;
constexpr int input_height = 28; // Height of input image
constexpr int input_width = 28;  // Width of input image
constexpr int input_size = input_height * input_height * num_channel;
constexpr int kernel_size1 = 8;  // Kernel size for first convolution
constexpr int kernel_total_size1 = kernel_size1 * kernel_size1;
constexpr int kernel_size2 = 4;  // Kernel size for second convolution
constexpr int kernel_total_size2 = kernel_size2 * kernel_size2;
constexpr int hidden1_size = input_height - kernel_size1 + 1;
constexpr int hidden2_size = hidden1_size - kernel_size2 + 1;
constexpr int hidden3_size = hidden2_size + kernel_size1 - 1;
constexpr int hidden4_size = hidden3_size + kernel_size2 - 1;
constexpr int latent_dim = 324;   // Latent space dimensionality
constexpr int output_height = input_height;
constexpr int output_width = input_width;
constexpr int output_size = output_height * output_width * num_channel;

constexpr int weight_size_cc0 = (kernel_total_size1 + kernel_total_size2 + kernel_total_size1 * num_channel) * num_channel/16;
constexpr int weight_size_cc1 = (kernel_total_size1 + kernel_total_size2 + kernel_total_size2 * num_channel) * num_channel/16;


using int16_v16 = tapa::vec_t<ap_int<16>, 16>;
using int16_v32 = tapa::vec_t<ap_int<16>, 32>;
using int16_v64 = tapa::vec_t<ap_int<16>, 64>;

struct ConfigInst {
    ap_uint<3> stage;
    ap_uint<9> i_bound;
    ap_uint<9> j_bound;
    ap_uint<11> k_bound;
};

// Kernel

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
    tapa::async_mmap<int>& output_mtx,
    tapa::istream<ap_int<16>>& fifo_in,
    tapa::ostream<bool>& fifo_fin
){

    for(int i_req = 0, i_resp = 0; i_resp < output_size;){
        #pragma HLS pipeline II=1 style=stp
        if((i_req < output_size) & !fifo_in.empty() & !output_mtx.write_addr.full() & !output_mtx.write_data.full()){
            output_mtx.write_addr.try_write(i_req);
            ap_int<16> tmp; fifo_in.try_read(tmp);
            int tmp_out = tmp;
            output_mtx.write_data.try_write(tmp_out);
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


// Rely on HLS resource binding for this kernel

void CC0_Encoder_Decoder_Conv1(
    tapa::istream<int16_v16>& fifo_input,
    tapa::istream<int16_v16>& fifo_weight,
    tapa::ostream<ap_int<16>>& fifo_to_ctr_mem,
    tapa::istream<int16_v32>& fifo_from_ctr_mem,
    tapa::ostream<ap_int<16>>& fifo_to_latent_sample,
    tapa::istream<int16_v64>& fifo_from_latent_sample,
    tapa::ostream<ap_int<16>>& fifo_to_CC1
){

    ap_int<16> kernel1[num_channel][num_channel][kernel_total_size1];
    #pragma HLS array_partition variable=kernel1 complete dim=3

    ap_int<16> kernel2[num_channel][kernel_total_size2];
    #pragma HLS array_partition variable=kernel2 complete dim=2
    #pragma HLS array_partition variable=kernel2 complete dim=1


    for(int st = 0; st < 3; st++){
        
        if(st == 0){
            for(int c = 0; c < num_channel; c++){
                for(int i = 0; i < (kernel_total_size1 >> 4); i++){
                    #pragma HLS pipeline II=1
                    int16_v16 tmp = fifo_weight.read();
                    for(int j = 0; j < 16; j++){
                        #pragma HLS unroll
                        kernel1[c][0][i*16+j] = tmp[j];
                    }
                }
            }
        } else if (st == 1) {
            for(int c = 0; c < num_channel; c++){
                for(int i = 0; i < (kernel_total_size2 >> 4); i++){
                    #pragma HLS pipeline II=1
                    int16_v16 tmp = fifo_weight.read();
                    for(int j = 0; j < 16; j++){
                        #pragma HLS unroll
                        kernel2[c][i*16+j] = tmp[j];
                    }
                }
            }
        } else {
            //decoder conv transpose 1
            for(int c = 0; c < num_channel; c++){
                for(int f = 0; f < num_channel; f++){
                    for(int i = 0; i < (kernel_total_size1 >> 4); i++){
                        #pragma HLS pipeline II=1
                        int16_v16 tmp = fifo_weight.read();
                        for(int j = 0; j < 16; j++){
                            #pragma HLS unroll
                            kernel1[c][f][i*16+j] = tmp[j];
                        }
                    }
                }
            }
        }

        //encoder conv1
        if(st == 0 || st == 2){
            const int bound_rep = (st == 0) ? 1 : num_channel;
            const int bound_ij = (st == 0) ? hidden1_size : hidden3_size;
            const int bound_c = (st == 1) ? 1 : num_channel;

            for(int rep = 0; rep < bound_rep; rep++){
                for(int f = 0; f < num_channel; f++){
                    ap_int<16> X[input_height][input_width];

                    #pragma HLS array_partition variable=X cyclic factor=8 dim=1
                    #pragma HLS array_partition variable=X cyclic factor=8 dim=2

                    if(st == 0) {
                        for(int i = 0; i < (input_height >> 2); i++){
                            for(int j = 0; j < (input_width >> 2); j++){
                                #pragma HLS pipeline II=1

                                int16_v16 tmp = fifo_input.read();
                                for(int k = 0; k < 4; k++){
                                    #pragma HLS unroll
                                    for(int kk = 0; kk < 4; kk++){
                                        #pragma HLS unroll
                                        X[i*4+k][j*4+kk] = tmp[k*4+kk];
                                    }
                                }
                            }
                        }
                    }

                    for (int i = 0; i < bound_ij; i++) {
                        for (int j = 0; j < bound_ij; j++) {
                            ap_int<16> tmp = 0;
                            for(int c = 0; c < bound_c; c++){
                                #pragma HLS pipeline II=1
                                int16_v64 inp; 
                                if(st == 2) inp = fifo_from_latent_sample.read();
                                int16_v64 op1; int16_v64 op2;
                                for(int ki = 0; ki < kernel_size1; ki++){
                                    #pragma HLS unroll
                                    for(int kj = 0; kj < kernel_size1; kj++){
                                        #pragma HLS unroll
                                        if(st == 0){
                                            op1[ki*kernel_size1+kj] = X[i + ki][j + kj];
                                            op2[ki * kernel_size1 + kj] = kernel1[f][0][ki * kernel_size1 + kj];
                                        } else {
                                            op1[ki*kernel_size1+kj] = inp[ki*kernel_size1+kj];
                                            op2[ki * kernel_size1 + kj] = kernel1[c][f][ki * kernel_size1 + kj];
                                        }
                                    }
                                }
                                for (int ki = 0; ki < kernel_size1; ki++) {
                                    #pragma HLS unroll
                                    for (int kj = 0; kj < kernel_size1; kj++) {
                                        #pragma HLS unroll
                                        tmp += op1[ki*kernel_size1+kj] * op2[ki*kernel_size1+kj];
                                    }
                                }
                            }
                            if(st == 0) {
                                fifo_to_ctr_mem.write(tmp);
                            } else {
                                fifo_to_CC1.write(tmp);
                            }
                        }
                    }
                }
            }
        } else if(st == 1) {
            for (int i = 0; i < hidden2_size; i++) {
                for (int j = 0; j < hidden2_size; j++) {
                    #pragma HLS pipeline II=1
                    ap_int<16> tmp = 0;
                    int16_v32 inp = fifo_from_ctr_mem.read();
                    for(int c = 0; c < num_channel; c++){
                        for (int ki = 0; ki < kernel_size2; ki++) {
                            for (int kj = 0; kj < kernel_size2; kj++) {
                                tmp += inp[c*kernel_total_size2+ki*kernel_size2+kj] * kernel2[c][ki * kernel_size2 + kj];
                            }
                        }
                    }
                    fifo_to_latent_sample.write(tmp);
                }
            }
        }
    }
}

void central_mem_cache(
    tapa::istream<ap_int<16>>& fifo_from_CC0,
    tapa::istream<ap_int<16>>& fifo_from_CC1,
    tapa::ostream<int16_v32>& fifo_to_CC0,
    tapa::ostream<int16_v32>& fifo_to_CC1
){
    ap_int<16> hidden_cache[num_channel][hidden1_size][hidden1_size];
    #pragma HLS array_partition variable=hidden_cache complete dim=1
    #pragma HLS array_partition variable=hidden_cache cyclic factor=4 dim=2
    #pragma HLS array_partition variable=hidden_cache cyclic factor=4 dim=3

    for(int i = 0; i < hidden1_size; i++){
        for(int j = 0; j < hidden1_size; j++){
            #pragma HLS pipeline II=1
            ap_int<16> tmp1 = fifo_from_CC0.read();
            ap_int<16> tmp2 = fifo_from_CC1.read();
            hidden_cache[0][i][j] = tmp1;
            hidden_cache[1][i][j] = tmp2;
        }
    }

    for(int i = 0; i < hidden1_size; i++){
        for(int j = 0; j < hidden1_size; j++){
            #pragma HLS pipeline II=1
            #pragma HLS dependence variable=hidden_cache type=inter false
            ap_int<16> tmp1 = fifo_from_CC0.read();
            ap_int<16> tmp2 = fifo_from_CC1.read();
            ap_int<16> res1 = hidden_cache[0][i][j] + tmp1;
            ap_int<16> res2 = hidden_cache[1][i][j] + tmp2;
            hidden_cache[0][i][j] = (res1 > 0) ? res1 : ap_int<16>(0);
            hidden_cache[1][i][j] = (res2 > 0) ? res2 : ap_int<16>(0);
        }
    }

    for (int i = 0; i < hidden2_size; i++) {
        for (int j = 0; j < hidden2_size; j++) {
            #pragma HLS pipeline II=1
            int16_v32 inp;
            for(int c = 0; c < num_channel; c++){
                for (int ki = 0; ki < kernel_size2; ki++) {
                    for (int kj = 0; kj < kernel_size2; kj++) {
                        inp[c*kernel_total_size2+ki*kernel_size2+kj] = hidden_cache[c][i+ki][j+kj];
                    }
                }
            }
            fifo_to_CC0.write(inp);
            fifo_to_CC1.write(inp);
        }
    }
}

void latent_sample(
    tapa::istream<ap_int<16>>& fifo_from_CC0,
    tapa::istream<ap_int<16>>& fifo_from_CC1,
    tapa::ostream<int16_v64>& fifo_to_CC0
){
    ap_int<16> hidden_cache[num_channel][hidden2_size][hidden2_size];
    #pragma HLS array_partition variable=hidden_cache complete dim=1
    #pragma HLS array_partition variable=hidden_cache complete dim=2
    #pragma HLS array_partition variable=hidden_cache complete dim=3

    for(int i = 0; i < hidden2_size; i++){
        for(int j = 0; j < hidden2_size; j++){
            #pragma HLS pipeline II=1
            ap_int<16> res1 = fifo_from_CC0.read();
            ap_int<16> res2 = fifo_from_CC1.read();
            res1 = (res1 > 0) ? res1 : ap_int<16>(0);
            res2 = (res2 > 0) ? res2 : ap_int<16>(0);
            hidden_cache[0][i][j] = res1 + ap_int<16>((int)(hls::exp((res2 >> 1)) * 0.05));
            hidden_cache[1][i][j] = res1 + ap_int<16>((int)(hls::exp((res2 >> 1)) * 0.02));
        }
    }

    for(int rep = 0; rep < num_channel; rep++){
        for(int f = 0; f < num_channel; f++){
            for (int i = 0; i < hidden3_size; i++) {
                for (int j = 0; j < hidden3_size; j++) {
                    for(int c = 0; c < num_channel; c++){
                        #pragma HLS pipeline II=1
                        int16_v64 inp;
                        for (int ki = 0; ki < kernel_size1; ki++) {
                            for (int kj = 0; kj < kernel_size1; kj++) {
                                int ii = i - ki;
                                int jj = j - kj;
                                if((ii >= 0) & (ii < hidden2_size) & (jj >= 0) & (jj < hidden2_size)){
                                    inp[ki*kernel_size1+kj] = hidden_cache[c][ii][jj];
                                } else {
                                    inp[ki*kernel_size1+kj] = 0;
                                }
                            }
                        }
                        fifo_to_CC0.write(inp);
                    }
                }
            }
        }
    }
}

void CC1_Encoder_Decoder_Conv2(
    tapa::istream<int16_v16>& fifo_input,
    tapa::istream<int16_v16>& fifo_weight,
    tapa::ostream<ap_int<16>>& fifo_to_ctr_mem,
    tapa::istream<int16_v32>& fifo_from_ctr_mem,
    tapa::ostream<ap_int<16>>& fifo_to_latent_sample,
    tapa::istream<ap_int<16>>& fifo_from_CC0,
    tapa::ostream<ap_int<16>>& fifo_output
){
    ap_int<16> kernel1[num_channel][kernel_total_size1];
    #pragma HLS array_partition variable=kernel1 complete dim=2

    ap_int<16> kernel2[num_channel][num_channel][kernel_total_size2];
    #pragma HLS array_partition variable=kernel2 complete dim=3
    #pragma HLS array_partition variable=kernel2 complete dim=1


    for(int c = 0; c < num_channel; c++){
        for(int i = 0; i < (kernel_total_size1 >> 4); i++){
            #pragma HLS pipeline II=1
            int16_v16 tmp = fifo_weight.read();
            for(int j = 0; j < 16; j++){
                #pragma HLS unroll
                kernel1[c][i*16+j] = tmp[j];
            }
        }
    }

    //encoder conv1

    for(int c = 0; c < num_channel; c++){
        ap_int<16> X[input_height][input_width];

        #pragma HLS array_partition variable=X cyclic factor=8 dim=1
        #pragma HLS array_partition variable=X cyclic factor=8 dim=2

        for(int i = 0; i < (input_height >> 2); i++){
            for(int j = 0; j < (input_width >> 2); j++){
                #pragma HLS pipeline II=1

                int16_v16 tmp = fifo_input.read();
                for(int k = 0; k < 4; k++){
                    #pragma HLS unroll
                    for(int kk = 0; kk < 4; kk++){
                        #pragma HLS unroll
                        X[i*4+k][j*4+kk] = tmp[k*4+kk];
                    }
                }
            }
        }

        for (int i = 0; i < hidden1_size; i++) {
            for (int j = 0; j < hidden1_size; j++) {
                #pragma HLS pipeline II=1
                ap_int<16> tmp = 0;
                for (int ki = 0; ki < kernel_size1; ki++) {
                    for (int kj = 0; kj < kernel_size1; kj++) {
                        tmp += X[i + ki][j + kj] * kernel1[c][ki * kernel_size1 + kj];
                    }
                }
                fifo_to_ctr_mem.write(tmp);
            }
        }
    }

    for(int st = 0; st < 2; st++){

        if(st == 0){
            for(int c = 0; c < num_channel; c++){
                for(int i = 0; i < (kernel_total_size2 >> 4); i++){
                    #pragma HLS pipeline II=1
                    int16_v16 tmp = fifo_weight.read();
                    for(int j = 0; j < 16; j++){
                        #pragma HLS unroll
                        kernel2[c][1][i*16+j] = tmp[j];
                    }
                }
            }
        } else {
            for(int c = 0; c < num_channel; c++){
                for(int f = 0; f < num_channel; f++){
                    for(int i = 0; i < (kernel_total_size2 >> 4); i++){
                        #pragma HLS pipeline II=1

                        int16_v16 tmp = fifo_weight.read();
                        for(int j = 0; j < 16; j++){
                            #pragma HLS unroll
                            kernel2[c][f][i*16+j] = tmp[j];
                        }
                    }
                }
            }
        }

        const int f_bound = (st == 0) ? 1 : num_channel;
        const int c_bound = (st == 0) ? 1 : num_channel;
        const int inner_c = (st == 0) ? num_channel : 1;
        const int bound_ij = (st == 0) ? hidden2_size : hidden4_size;

        for(int f = 0; f < f_bound; f++){
            for(int c = 0; c < c_bound; c++){
                ap_int<16> hidden_cache[hidden3_size][hidden3_size];
                #pragma HLS array_partition variable=hidden_cache cyclic factor=4 dim=1
                #pragma HLS array_partition variable=hidden_cache cyclic factor=4 dim=2

                if(st == 1){
                    for(int i = 0; i < hidden3_size; i++){
                        for(int j = 0; j < hidden3_size; j++){
                            #pragma HLS pipeline II=1
                            ap_int<16> tmp = fifo_from_CC0.read();
                            hidden_cache[i][j] = (tmp > 0) ? tmp : ap_int<16>(0);
                        }
                    }
                }

                for (int i = 0; i < bound_ij; i++) {
                    for (int j = 0; j < bound_ij; j++) {
                        ap_int<16> tmp = 0;
                        int16_v32 inp;
                        if(st == 0) inp = fifo_from_ctr_mem.read();
                        for(int c_in = 0; c_in < inner_c; c_in++){
                            #pragma HLS pipeline II=1
                            int16_v16 op1; int16_v16 op2;
                            for (int ki = 0; ki < kernel_size2; ki++) {
                                for (int kj = 0; kj < kernel_size2; kj++) {
                                    const int ii = i - ki;
                                    const int jj = j - kj;
                                    if(st == 0){
                                        op1[ki * kernel_size2 + kj] = inp[c_in*kernel_total_size2+ki*kernel_size2+kj];
                                        op2[ki * kernel_size2 + kj] = kernel2[c_in][1][ki * kernel_size2 + kj];
                                    } else if(ii >= 0 && ii < hidden3_size && jj >= 0 && jj < hidden3_size){
                                        op1[ki * kernel_size2 + kj] = hidden_cache[ii][jj];
                                        op2[ki * kernel_size2 + kj] = kernel2[c][f][ki * kernel_size2 + kj];
                                    } else {
                                        op1[ki * kernel_size2 + kj] = 0;
                                        op2[ki * kernel_size2 + kj] = 0;
                                    }
                                }
                            }

                            for (int ki = 0; ki < kernel_size2; ki++) {
                                for (int kj = 0; kj < kernel_size2; kj++) {
                                    tmp += op1[ki * kernel_size2 + kj] * op2[ki * kernel_size2 + kj];
                                }
                            }
                        }
                        if (st == 0) {
                            fifo_to_latent_sample.write(tmp);
                        } else {
                            fifo_output.write(tmp);
                        }
                    }
                }
            }
        }
    }
}

void final_relu(tapa::istream<ap_int<16>>& fifo_in, tapa::ostream<ap_int<16>>& fifo_out){

    for(int f = 0; f < num_channel; f++){
        ap_int<16> output_cache[hidden4_size][hidden4_size];

        for(int j = 0; j < hidden4_size; j++){
            for(int k = 0; k < hidden4_size; k++){
                output_cache[j][k] = 0;
            }
        }

        for (int i = 0; i < hidden4_size; i++) {
            for (int j = 0; j < hidden4_size; j++) {
                ap_int<16> tmp = fifo_in.read();
                output_cache[i][j] += tmp;
            }
        }

        for (int i = 0; i < hidden4_size; i++) {
            for (int j = 0; j < hidden4_size; j++) {
                ap_int<16> tmp = fifo_in.read();
                tmp += output_cache[i][j];
                tmp = (tmp > 0) ? tmp : ap_int<16>(0);
                fifo_out.write(tmp);
            }
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

void VAE(
    tapa::mmap<int16_v16> X_acc0,
    tapa::mmap<int16_v16> X_acc1,
    tapa::mmap<int16_v16> W_acc0,
    tapa::mmap<int16_v16> W_acc1,
    tapa::mmap<int> acc1_out,
    tapa::mmap<int> cycle_count
) {
    tapa::stream<int16_v16> fifo_input_CC0("fifo_input_CC0");
    tapa::stream<int16_v16> fifo_input_CC1("fifo_input_CC1");
    tapa::stream<int16_v16> fifo_weight_CC0("fifo_weight_CC0");
    tapa::stream<int16_v16> fifo_weight_CC1("fifo_weight_CC1");

    tapa::stream<ap_int<16>> fifo_to_ctr_mem_cc0("fifo_to_ctr_mem_cc0");
    tapa::stream<int16_v32> fifo_from_ctr_mem_cc0("fifo_from_ctr_mem_cc0");
    tapa::stream<ap_int<16>> fifo_to_latent_sample_cc0("fifo_to_latent_sample_cc0");
    tapa::stream<int16_v64> fifo_from_latent_sample("fifo_from_latent_sample");
    tapa::stream<ap_int<16>> fifo_to_CC1("fifo_to_CC1");

    tapa::stream<ap_int<16>> fifo_to_ctr_mem_cc1("fifo_to_ctr_mem_cc1");
    tapa::stream<int16_v32> fifo_from_ctr_mem_cc1("fifo_from_ctr_mem_cc1");
    tapa::stream<ap_int<16>> fifo_to_latent_sample_cc1("fifo_to_latent_sample_cc1");
    tapa::stream<ap_int<16>> fifo_output("fifo_output");

    tapa::stream<bool> fifo_fin("fifo_fin");
    tapa::stream<ap_int<16>> fifo_output_relu("fifo_output_relu");

    tapa::task()
        .invoke<tapa::join>(read_W, weight_size_cc0, W_acc0, fifo_weight_CC0)
        .invoke<tapa::join>(read_W, weight_size_cc1, W_acc1, fifo_weight_CC1)
        .invoke<tapa::join>(read_X, X_acc0, fifo_input_CC0)
        .invoke<tapa::join>(read_X, X_acc1, fifo_input_CC1)
        .invoke<tapa::join>(
            CC0_Encoder_Decoder_Conv1, 
            fifo_input_CC0, 
            fifo_weight_CC0, 
            fifo_to_ctr_mem_cc0, 
            fifo_from_ctr_mem_cc0,
            fifo_to_latent_sample_cc0,
            fifo_from_latent_sample,
            fifo_to_CC1
        )
        .invoke<tapa::join>(central_mem_cache, fifo_to_ctr_mem_cc0, fifo_to_ctr_mem_cc1, fifo_from_ctr_mem_cc0, fifo_from_ctr_mem_cc1)
        .invoke<tapa::join>(latent_sample, fifo_to_latent_sample_cc0, fifo_to_latent_sample_cc1, fifo_from_latent_sample)
        .invoke<tapa::join>(
            CC1_Encoder_Decoder_Conv2, 
            fifo_input_CC1, 
            fifo_weight_CC1, 
            fifo_to_ctr_mem_cc1,
            fifo_from_ctr_mem_cc1,
            fifo_to_latent_sample_cc1,
            fifo_to_CC1,
            fifo_output
        )
        .invoke<tapa::join>(final_relu, fifo_output, fifo_output_relu)
        .invoke<tapa::join>(write_mtx, acc1_out, fifo_output_relu, fifo_fin)
        .invoke<tapa::join>(measure_cycle, fifo_fin, cycle_count);
}