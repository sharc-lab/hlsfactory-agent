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

constexpr int kernel_total_size1_div_16 = kernel_total_size1 / 16;
constexpr int kernel_total_size2_div_16 = kernel_total_size2 / 16;

using int16_v16 = tapa::vec_t<ap_int<16>, 16>;
using int16_v32 = tapa::vec_t<ap_int<16>, 32>;
using int16_v64 = tapa::vec_t<ap_int<16>, 64>;

void measure_cycle(tapa::istream<bool>& fifo_fin, tapa::mmap<int> cycle_count){
    for(int cycle = 0;;cycle++){
        if(!fifo_fin.empty()){
            fifo_fin.read(nullptr);
            cycle_count[0] = cycle;
            break;
        }
    }
}

void top(
    tapa::mmap<int16_v16> X_map,
    tapa::async_mmap<int16_v16>& W1,
    tapa::async_mmap<int16_v16>& W2,
    tapa::async_mmap<int16_v16>& W3,
    tapa::async_mmap<int16_v16>& W4,
    tapa::mmap<ap_int<32>> offchip_decoder_conv3,
    tapa::mmap<ap_int<32>> offchip_decoder_conv4,
    tapa::mmap<int> acc_out,
    tapa::ostream<bool>& fifo_fin
) {
    // Encoder weights
    ap_int<16> kernel1[num_channel][num_channel][kernel_total_size1];
    #pragma HLS array_partition variable=kernel1 complete dim=3
    ap_int<16> kernel2[num_channel][num_channel][kernel_total_size2];
    #pragma HLS array_partition variable=kernel2 complete dim=3

    // Decoder weights
    ap_int<16> kernel3[num_channel][num_channel][kernel_total_size1];
    #pragma HLS array_partition variable=kernel3 complete dim=3
    ap_int<16> kernel4[num_channel][num_channel][kernel_total_size2];
    #pragma HLS array_partition variable=kernel4 complete dim=3

    // Encoder hidden buffers
    ap_int<16> hidden1_buffer[num_channel][hidden1_size][hidden1_size];
    #pragma HLS array_partition variable=hidden1_buffer cyclic factor=8 dim=2
    #pragma HLS array_partition variable=hidden1_buffer cyclic factor=8 dim=3

    ap_int<16> hidden2_buffer[num_channel][hidden2_size][hidden2_size];
    #pragma HLS array_partition variable=hidden2_buffer complete dim=1
    #pragma HLS array_partition variable=hidden2_buffer complete dim=2
    #pragma HLS array_partition variable=hidden2_buffer complete dim=3

    // Read in weights
    for (int c = 0; c < num_channel; c++){
        for (int r = 0; r < num_channel; r++){
            load_kernel_size1: for(int i = 0; i < kernel_total_size1_div_16; i++){
                #pragma HLS pipeline II=1
                const int addr = (c*num_channel + r) * kernel_total_size1_div_16 + i;

                // Kernel 1
                W1.read_addr.write(addr);
                int16_v16 tmp1 = W1.read_data.read(nullptr);
                for (int j = 0; j < 16; j++){
                    #pragma HLS unroll
                    kernel1[c][r][i*16+j] = tmp1[j];
                }
                // Kernel 3
                W3.read_addr.write(addr);
                int16_v16 tmp2 = W3.read_data.read(nullptr);
                for (int j = 0; j < 16; j++){
                    #pragma HLS unroll
                    kernel3[c][r][i*16+j] = tmp2[j];
                }
            }
            load_kernel_size2: for (int i = 0; i < kernel_total_size2_div_16; i++){
                #pragma HLS pipeline II=1
                const int addr = (c*num_channel + r) * kernel_total_size2_div_16 + i;

                // Kernel 2
                W2.read_addr.write(addr);
                int16_v16 tmp1 = W2.read_data.read(nullptr);
                for (int j = 0; j < 16; j++){
                    #pragma HLS unroll
                    kernel2[c][r][i*16+j] = tmp1[j];
                }
                // Kernel 4
                W4.read_addr.write(addr);
                int16_v16 tmp2 = W4.read_data.read(nullptr);
                for (int j = 0; j < 16; j++){
                    #pragma HLS unroll
                    kernel4[c][r][i*16+j] = tmp2[j];
                }
            }
        }
    }

    // Encoder Conv 1
    for (int i = 0; i < num_channel; i++) {
        for (int j = 0; j < hidden1_size; j++) {
            for (int k = 0; k < hidden1_size; k++) {
                hidden1_buffer[i][j][k] = ap_int<16>(0);
            }
        }
    }
    for (int r = 0; r < num_channel; r++){
        for (int c = 0; c < num_channel; c++){
            ap_int<16> X[input_height][input_width];
            #pragma HLS array_partition variable=X cyclic factor=8 dim=1
            #pragma HLS array_partition variable=X cyclic factor=8 dim=2

            for (int i = 0; i < (input_height >> 2); i++){
                for (int j = 0; j < (input_width >> 2); j++){
                    #pragma HLS pipeline II=2

                    const int addr = ((r * num_channel + c) * (input_height >> 4) + i) * (input_width >> 2) + j;
                    int16_v16 tmp = X_map[addr];

                    for (int k = 0; k < 4; k++){
                        #pragma HLS unroll
                        for (int kk = 0; kk < 4; kk++){
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
                    hidden1_buffer[r][i][j] += (tmp > 0) ? tmp : ap_int<16>(0);
                }
            }
        }
    }

    // Encoder Conv 2
    for (int i = 0; i < num_channel; i++) {
        for (int j = 0; j < hidden1_size; j++) {
            for (int k = 0; k < hidden1_size; k++) {
                hidden2_buffer[i][j][k] = ap_int<16>(0);
            }
        }
    }
    for(int c = 0; c < num_channel; c++){
        for (int i = 0; i < hidden2_size; i++) {
            for (int j = 0; j < hidden2_size; j++) {
                ap_int<16> acc;
                encoder_conv2_compute: for (int r = 0; r < num_channel; r++){
                    #pragma HLS pipeline II=1
                    ap_int<16> tmp = 0;
                    for (int ki = 0; ki < kernel_size2; ki++) {
                        #pragma HLS unroll
                        for (int kj = 0; kj < kernel_size2; kj++) {
                            #pragma HLS unroll
                            tmp += hidden1_buffer[c][i + ki][j + kj] * kernel2[c][r][ki * kernel_size2 + kj];
                        }
                    }
                    acc += tmp;
                }
                // ReLU and latent sample
                acc = (acc > 0) ? acc : ap_int<16>(0);
                const float factor = (c == 0) ? 0.05 : 0.02;
                acc += ap_int<16>((int)(hls::exp((acc >> 1)) * factor));
                hidden2_buffer[c][i][j] = acc;
            }
        }
    }

    // Sample + decoder conv3
    latent_sample_rep_loop: for (int rep = 0; rep < num_channel; rep++){
        latent_sample_f_loop: for (int f = 0; f < num_channel; f++){
            latent_sample_i_loop: for (int i = 0; i < hidden3_size; i++) {
                latent_sample_j_loop: for (int j = 0; j < hidden3_size; j++) {
                    ap_int<16> tmp = 0;
                    ap_int<16> tmp_acc[kernel_size1][kernel_size1];
                    #pragma HLS array_partition variable=tmp_acc complete dim=1
                    #pragma HLS array_partition variable=tmp_acc complete dim=2

                    latent_sample_compute: for (int c = 0; c < num_channel; c++){
                        #pragma HLS pipeline II=1
                        #pragma HLS dependence variable=hidden2_buffer inter false

                        for (int ki = 0; ki < kernel_size1; ki++) {
                            for (int kj = 0; kj < kernel_size1; kj++) {
                                int ii = i - ki;
                                int jj = j - kj;
                                if ((ii >= 0) && (ii < hidden2_size) && (jj >= 0) & (jj < hidden2_size)){
                                    tmp_acc[ki][kj] += hidden2_buffer[c][ii][jj] * kernel3[c][f][ki * kernel_size1 + kj];
                                }
                            }
                        }
                    }
                    for (int ki = 0; ki < kernel_size1; ki++) {
                        for (int kj = 0; kj < kernel_size1; kj++) {
                            tmp += tmp_acc[ki][kj];
                        }
                    }
                    const int addr = ((rep * num_channel + f)*hidden3_size + i)*hidden3_size + j;
                    offchip_decoder_conv3[addr] = ap_int<32>(tmp);
                }
            }
        }
    }

    // Decoder Conv4 Transpose
    for(int f = 0; f < num_channel; f++) {
        for(int c = 0; c < num_channel; c++) {

            ap_int<16> hidden_cache[hidden3_size][hidden3_size];
            #pragma HLS array_partition variable=hidden_cache cyclic factor=4 dim=1
            #pragma HLS array_partition variable=hidden_cache cyclic factor=4 dim=2

            for(int i = 0; i < hidden3_size; i++){
                for(int j = 0; j < hidden3_size; j++){
                    #pragma HLS pipeline II=2
                    const int addr = ((f*num_channel + c)*hidden3_size + i)*hidden3_size + j;
                    ap_int<16> tmp = ap_int<16>(offchip_decoder_conv3[addr]);
                    hidden_cache[i][j] = tmp;
                }
            }

            for (int i = 0; i < hidden4_size; i++) {
                decoder_conv4_compute: for (int j = 0; j < hidden4_size; j++) {
                    #pragma HLS pipeline II=2
                    ap_int<16> tmp = 0;
                    for (int ki = 0; ki < kernel_size1; ki++) {
                        for (int kj = 0; kj < kernel_size1; kj++) {
                            int ii = i - ki;
                            int jj = j - kj;
                            if(ii >= 0 && ii < hidden3_size && jj >= 0 && jj < hidden3_size){
                                tmp += hidden_cache[ii][jj] * kernel4[c][f][ki * kernel_size2 + kj];
                            }
                        }
                    }
                    const int addr = ((f * num_channel + c)*hidden4_size + i)*hidden4_size + j;
                    offchip_decoder_conv4[addr] = ap_int<32>(tmp);
                }
            }
        }
    }

    // Final ReLU
    for(int f = 0; f < num_channel; f++){
        ap_int<16> output_cache[hidden4_size][hidden4_size];

        for(int j = 0; j < hidden4_size; j++){
            for(int k = 0; k < hidden4_size; k++){
                output_cache[j][k] = 0;
            }
        }

        for (int i = 0; i < hidden4_size; i++) {
            for (int j = 0; j < hidden4_size; j++) {
                const int addr = ((f*num_channel + 0)*hidden4_size + i)*hidden4_size + j;
                ap_int<16> tmp = ap_int<16>(offchip_decoder_conv4[addr]);
                output_cache[i][j] += tmp;
            }
        }

        for (int i = 0; i < hidden4_size; i++) {
            for (int j = 0; j < hidden4_size; j++) {
                const int addr = ((f*num_channel + 1)*hidden4_size + i)*hidden4_size + j;
                ap_int<16> tmp = offchip_decoder_conv4[addr];
                tmp += output_cache[i][j];
                tmp = (tmp > 0) ? tmp : ap_int<16>(0);

                const int addr_out = (f*hidden4_size + i)*hidden4_size + j;
                int tmp_out = tmp;
                acc_out[addr_out] = tmp_out;
            }
        }
    }

    fifo_fin.write(true);
}

void VAE(
    tapa::mmap<int16_v16> X,
    tapa::mmap<int16_v16> W1,
    tapa::mmap<int16_v16> W2,
    tapa::mmap<int16_v16> W3,
    tapa::mmap<int16_v16> W4,
    tapa::mmap<ap_int<32>> offchip_decoder_conv3,
    tapa::mmap<ap_int<32>> offchip_decoder_conv4,
    tapa::mmap<int> acc_out,
    tapa::mmap<int> cycle_count
) {
    tapa::stream<bool> fifo_fin("fifo_fin");

    tapa::task()
        .invoke<tapa::join>(top, 
                            X, 
                            W1, 
                            W2, 
                            W3,
                            W4,
                            offchip_decoder_conv3,
                            offchip_decoder_conv4,
                            acc_out,
                            fifo_fin)
        .invoke<tapa::join>(measure_cycle, fifo_fin, cycle_count);
}
