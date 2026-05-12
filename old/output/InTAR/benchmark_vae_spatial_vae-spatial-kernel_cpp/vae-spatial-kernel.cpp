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
constexpr int input_size = input_height * input_height * num_channel * num_channel;
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

constexpr int weight_size_kernel1 = (kernel_total_size1 * num_channel) * num_channel/16;
constexpr int weight_size_kernel2 = (kernel_total_size2 * num_channel) * num_channel/16;


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


void Encoder_Conv1(
    tapa::istream<int16_v16>& fifo_input,
    tapa::istream<int16_v16>& fifo_weight,
    tapa::ostream<ap_int<16>>& fifo_output
) {
    ap_int<16> kernel1[num_channel][num_channel][kernel_total_size1];
    #pragma HLS array_partition variable=kernel1 complete dim=3

    for(int c = 0; c < num_channel; c++){
        for(int r = 0; r < num_channel; r++){
            for(int i = 0; i < (kernel_total_size1 >> 4); i++){
                #pragma HLS pipeline II=1
                int16_v16 tmp = fifo_weight.read();
                for(int j = 0; j < 16; j++){
                    #pragma HLS unroll
                    kernel1[c][r][i*16+j] = tmp[j];
                }
            }
        }
    }

    for(int r = 0; r < num_channel; r++){
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
                        #pragma HLS unroll
                        for (int kj = 0; kj < kernel_size1; kj++) {
                            #pragma HLS unroll
                            tmp += X[i + ki][j + kj] * kernel1[c][r][ki * kernel_size1 + kj];
                        }
                    }
                    fifo_output.write(tmp);
                }
            }
        }
    }
}

void relu_conv1(
    tapa::istream<ap_int<16>>& fifo_input,
    tapa::ostream<ap_int<16>>& fifo_output
) {
    ap_int<16> hidden[hidden1_size][hidden1_size];
    for(int r = 0; r < num_channel; r++){
        for(int i = 0; i < hidden1_size; i++){
            for(int j = 0; j < hidden1_size; j++){
                ap_int<16> tmp = fifo_input.read();
                hidden[i][j] = tmp;
            }
        }
        for(int i = 0; i < hidden1_size; i++){
            for(int j = 0; j < hidden1_size; j++){
                ap_int<16> tmp = fifo_input.read();
                ap_int<16> total = tmp + hidden[i][j];
                ap_int<16> total_pos = (total > 0) ? total : ap_int<16>(0);
                fifo_output.write(total_pos);
            }
        }
    }
}

void Encoder_Conv2(
    tapa::istream<ap_int<16>>& fifo_input,
    tapa::istream<int16_v16>& fifo_weight,
    tapa::ostream<ap_int<16>>& fifo_output
) {
    ap_int<16> kernel2[num_channel][num_channel][kernel_total_size2];
    #pragma HLS array_partition variable=kernel2 complete dim=3

    for(int c = 0; c < num_channel; c++){
        for(int r = 0; r < num_channel; r++){
            for(int i = 0; i < (kernel_total_size2 >> 4); i++){
                #pragma HLS pipeline II=1
                int16_v16 tmp = fifo_weight.read();
                for(int j = 0; j < 16; j++){
                    #pragma HLS unroll
                    kernel2[c][r][i*16+j] = tmp[j];
                }
            }
        }
    }
    for(int c = 0; c < num_channel; c++){
        ap_int<16> X[hidden1_size][hidden1_size];

        #pragma HLS array_partition variable=X cyclic factor=8 dim=1
        #pragma HLS array_partition variable=X cyclic factor=8 dim=2

        for(int i = 0; i < hidden1_size; i++){
            for(int j = 0; j < hidden1_size; j++){
                auto tmp = fifo_input.read();
                X[i][j] = tmp;
            }
        }
        
        for (int i = 0; i < hidden2_size; i++) {
            for (int j = 0; j < hidden2_size; j++) {
                for(int r = 0; r < num_channel; r++){
                    #pragma HLS pipeline II=1
                    ap_int<16> tmp = 0;
                    for (int ki = 0; ki < kernel_size2; ki++) {
                        #pragma HLS unroll
                        for (int kj = 0; kj < kernel_size2; kj++) {
                            #pragma HLS unroll
                            tmp += X[i + ki][j + kj] * kernel2[c][r][ki * kernel_size2 + kj];
                        }
                    }
                    fifo_output.write(tmp);
                }
            }
        }
    }
}

void latent_sample(
    tapa::istream<ap_int<16>>& fifo_input,
    tapa::ostream<int16_v64>& fifo_output
){
    ap_int<16> hidden_cache[num_channel][hidden2_size][hidden2_size];
    #pragma HLS array_partition variable=hidden_cache complete dim=1
    #pragma HLS array_partition variable=hidden_cache complete dim=2
    #pragma HLS array_partition variable=hidden_cache complete dim=3

    for(int i = 0; i < hidden2_size; i++){
        for(int j = 0; j < hidden2_size; j++){
            for(int r = 0; r < num_channel; r++){
                hidden_cache[r][i][j] = fifo_input.read();
            }
        }
    }

    for(int i = 0; i < hidden2_size; i++){
        for(int j = 0; j < hidden2_size; j++){
            
            ap_int<16> res[2];
            for(int r = 0; r < num_channel; r++){
                #pragma HLS pipeline II=1
                res[r] = fifo_input.read();
            }
            res[0] += hidden_cache[0][i][j];
            res[1] += hidden_cache[1][i][j];
            res[0] = (res[0] > 0) ? res[0] : ap_int<16>(0);
            res[1] = (res[1] > 0) ? res[1] : ap_int<16>(0);
            hidden_cache[0][i][j] = res[0] + ap_int<16>((int)(hls::exp((res[0] >> 1)) * 0.05));
            hidden_cache[1][i][j] = res[1] + ap_int<16>((int)(hls::exp((res[1] >> 1)) * 0.02));
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
                        fifo_output.write(inp);
                    }
                }
            }
        }
    }
}

void Decoder_Conv3(
    tapa::istream<int16_v64>& fifo_input,
    tapa::istream<int16_v16>& fifo_weight,
    tapa::ostream<ap_int<16>>& fifo_output
) {
    ap_int<16> kernel1[num_channel][num_channel][kernel_total_size1];
    #pragma HLS array_partition variable=kernel1 complete dim=3
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

    for(int rep = 0; rep < num_channel; rep++){
        for(int f = 0; f < num_channel; f++){
            for (int i = 0; i < hidden3_size; i++) {
                for (int j = 0; j < hidden3_size; j++) {
                    ap_int<16> tmp = 0;
                    for(int c = 0; c < num_channel; c++){
                        #pragma HLS pipeline II=1
                        int16_v64 inp = fifo_input.read();
                        for (int ki = 0; ki < kernel_size1; ki++) {
                            for (int kj = 0; kj < kernel_size1; kj++) {
                                tmp += inp[ki*kernel_size2+kj] * kernel1[c][f][ki * kernel_size1 + kj];
                            }
                        }
                    }
                    fifo_output.write(tmp);
                }
            }
        }
    }
}

void Decoder_Conv4(
    tapa::istream<ap_int<16>>& fifo_input,
    tapa::istream<int16_v16>& fifo_weight,
    tapa::ostream<ap_int<16>>& fifo_output
) {
    ap_int<16> kernel2[num_channel][num_channel][kernel_total_size2];
    #pragma HLS array_partition variable=kernel2 complete dim=3
//decoder conv transpose 2
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

    for(int f = 0; f < num_channel; f++){
        for(int c = 0; c < num_channel; c++){
            
            ap_int<16> hidden_cache[hidden3_size][hidden3_size];
            #pragma HLS array_partition variable=hidden_cache cyclic factor=4 dim=1
            #pragma HLS array_partition variable=hidden_cache cyclic factor=4 dim=2

            for(int i = 0; i < hidden3_size; i++){
                for(int j = 0; j < hidden3_size; j++){
                    #pragma HLS pipeline II=1
                    ap_int<16> tmp = fifo_input.read();
                    hidden_cache[i][j] = tmp;
                }
            }

            for (int i = 0; i < hidden4_size; i++) {
                for (int j = 0; j < hidden4_size; j++) {
                    #pragma HLS pipeline II=1
                    ap_int<16> tmp = 0;
                    for (int ki = 0; ki < kernel_size1; ki++) {
                        for (int kj = 0; kj < kernel_size1; kj++) {
                            int ii = i - ki;
                            int jj = j - kj;
                            if(ii >= 0 && ii < hidden3_size && jj >= 0 && jj < hidden3_size){
                                tmp += hidden_cache[ii][jj] * kernel2[c][f][ki * kernel_size2 + kj];
                            }
                        }
                    }
                    fifo_output.write(tmp);
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
    tapa::mmap<int16_v16> X,
    tapa::mmap<int16_v16> W1,
    tapa::mmap<int16_v16> W2,
    tapa::mmap<int16_v16> W3,
    tapa::mmap<int16_v16> W4,
    tapa::mmap<int> acc_out,
    tapa::mmap<int> cycle_count
) {
    tapa::stream<int16_v16> fifo_input("fifo_input");
    tapa::stream<int16_v16> fifo_weight1("fifo_weight1");
    tapa::stream<int16_v16> fifo_weight2("fifo_weight2");
    tapa::stream<int16_v16> fifo_weight3("fifo_weight3");
    tapa::stream<int16_v16> fifo_weight4("fifo_weight4");
    tapa::stream<ap_int<16>> fifo_conv1_to_relu1("fifo_conv1_to_relu1");
    tapa::stream<ap_int<16>> fifo_relu1_to_conv2("fifo_relu1_to_conv2");
    tapa::stream<ap_int<16>> fifo_conv2_to_latent("fifo_conv2_to_latent");
    tapa::stream<int16_v64> fifo_latent_to_conv3("fifo_latent_to_conv3");
    tapa::stream<ap_int<16>> fifo_conv3_to_conv4("fifo_conv3_to_conv4");
    tapa::stream<ap_int<16>> fifo_conv4_to_relu2("fifo_conv4_to_relu2");


    tapa::stream<bool> fifo_fin("fifo_fin");
    tapa::stream<ap_int<16>> fifo_output_relu("fifo_output_relu");

    tapa::task()
        .invoke<tapa::join>(read_W, weight_size_kernel1, W1, fifo_weight1)
        .invoke<tapa::join>(read_W, weight_size_kernel2, W2, fifo_weight2)
        .invoke<tapa::join>(read_W, weight_size_kernel1, W3, fifo_weight3)
        .invoke<tapa::join>(read_W, weight_size_kernel2, W4, fifo_weight4)
        .invoke<tapa::join>(read_X, X, fifo_input)
        .invoke<tapa::join>(Encoder_Conv1, fifo_input, fifo_weight1, fifo_conv1_to_relu1)
        .invoke<tapa::join>(relu_conv1, fifo_conv1_to_relu1, fifo_relu1_to_conv2)
        .invoke<tapa::join>(Encoder_Conv2, fifo_relu1_to_conv2, fifo_weight2, fifo_conv2_to_latent)
        .invoke<tapa::join>(latent_sample, fifo_conv2_to_latent, fifo_latent_to_conv3)
        .invoke<tapa::join>(Decoder_Conv3, fifo_latent_to_conv3, fifo_weight3, fifo_conv3_to_conv4)
        .invoke<tapa::join>(Decoder_Conv4, fifo_conv3_to_conv4, fifo_weight4, fifo_conv4_to_relu2)
        .invoke<tapa::join>(final_relu, fifo_conv4_to_relu2, fifo_output_relu)
        .invoke<tapa::join>(write_mtx, acc_out, fifo_output_relu, fifo_fin)
        .invoke<tapa::join>(measure_cycle, fifo_fin, cycle_count);
}