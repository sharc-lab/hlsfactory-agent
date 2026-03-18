#include <iostream>
#include <cmath>
#include <numeric>
#include <string>
#include <tapa.h>
#include <ap_int.h>
#include <hls_math.h>

constexpr int input_size = 256;     // Number of input features
constexpr int hidden_size1 = 1024;   // Number of neurons in the first hidden layer
constexpr int hidden_size2 = 2048;   // Number of neurons in the second hidden layer
constexpr int output_size = 256;    // Number of output classes

constexpr int weight_size1 = input_size * hidden_size1 / 16;
constexpr int weight_size2 = hidden_size1 * hidden_size2 / 16;
constexpr int weight_size3 = hidden_size2 * output_size / 16;

using int16_v16 = tapa::vec_t<ap_int<16>, 16>;

template <typename data_t>
inline void bh(tapa::istream<data_t> & fifo_in) {
#pragma HLS inline
    bh: for (;;) {
#pragma HLS pipeline II=1 style=stp
        data_t tmp; fifo_in.try_read(tmp);
    }
}

void black_hole_int16_v16(tapa::istream<int16_v16>& fifo_in) {
    bh(fifo_in);
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

void layer1_3(
    tapa::istream<int16_v16>& fifo_input1,
    tapa::istream<int16_v16>& fifo_input2,
    tapa::istream<int16_v16>& fifo_weight1,
    tapa::istream<int16_v16>& fifo_weight2,
    tapa::ostream<int16_v16>& fifo_output1,
    tapa::ostream<int16_v16>& fifo_output2
) {
    ap_int<16> X[hidden_size2];
    #pragma HLS array_partition variable=X cyclic factor=16

    for (int layer = 0; layer < 2; layer++) {
        const int in_size = (layer == 0) ? input_size : hidden_size2;
        const int hidden_size = (layer == 0) ? hidden_size1 : output_size;

        for (int i = 0; i < (in_size >> 4);) {
            #pragma HLS pipeline II=1
            if (layer == 0) {
                if (!fifo_input1.empty()) {
                    int16_v16 tmp; fifo_input1.try_read(tmp);
                    for (int k = 0; k < 16; k++) {
                        #pragma HLS unroll
                        X[i*16+k] = tmp[k];
                    }
                    i++;
                }
            } else if (layer == 1) {
                if (!fifo_input2.empty()) {
                    int16_v16 tmp; fifo_input2.try_read(tmp);
                    for (int k = 0; k < 16; k++) {
                        #pragma HLS unroll
                        X[i*16+k] = tmp[k];
                    }
                    i++;
                }
            }

        }

        for (int i = 0; i < (hidden_size >> 4); i++) {
            int16_v16 sum;
            for (int j = 0; j < in_size; j++) {
                #pragma HLS pipeline II=1
                int16_v16 tmp;
                if (layer == 0) {
                    tmp = fifo_weight1.read();
                } else if (layer == 1) {
                    tmp = fifo_weight2.read();
                }
                for (int k = 0; k < 16; k++) {
                    #pragma HLS unroll
                    sum[k] = X[j] * tmp[k];
                }
            }
            if (layer == 0) {
                fifo_output1.write(sum);
            } else if (layer == 1) {
                fifo_output2.write(sum);
            }
        }
    }
}

void layer2(
    tapa::istream<int16_v16>& fifo_input,
    tapa::istream<int16_v16>& fifo_weight,
    tapa::ostream<int16_v16>& fifo_output
) {
    ap_int<16> X[hidden_size1];
    #pragma HLS array_partition variable=X cyclic factor=16

    for (int i = 0; i < (hidden_size1 >> 4);) {
        #pragma HLS pipeline II=1

        if (!fifo_input.empty()) {
            int16_v16 tmp; fifo_input.try_read(tmp);
            for (int k = 0; k < 16; k++) {
                #pragma HLS unroll
                X[i*16+k] = tmp[k];
            }
            i++;
        }
    }

    for (int i = 0; i < (hidden_size2 >> 4); i++) {
        int16_v16 sum;
        for (int j = 0; j < hidden_size1; j++) {
            #pragma HLS pipeline II=1

            int16_v16 tmp = fifo_weight.read();
            for (int k = 0; k < 16; k++) {
                #pragma HLS unroll
                sum[k] = X[j] * tmp[k];
            }
        }
        fifo_output.write(sum);
    }
}

// Helper function for ReLU activation
void relu(
    const int N,
    tapa::istream<int16_v16>& fifo_act_in,
    tapa::ostream<int16_v16>& fifo_act_out
) {
    for(int n = 0; n < (N >> 4);){
        if(!fifo_act_in.empty()){
            int16_v16 tmp; fifo_act_in.try_read(tmp);
            for(int i = 0; i < 16; i++){
                #pragma HLS unroll
                tmp[i] = (tmp[i] < 0) ? ap_int<16> (0) : tmp[i];
            }
            fifo_act_out.write(tmp);
            n++;
        }
    }
}

void MLP(
    tapa::mmap<int16_v16> X,
    tapa::mmap<int16_v16> W1,
    tapa::mmap<int16_v16> W2,
    tapa::mmap<int16_v16> W3,
    tapa::mmap<int16_v16> data_out,
    tapa::mmap<int> cycle_count
) {
    tapa::stream<int16_v16> fifo_X("fifo_X");
    tapa::stream<int16_v16> fifo_W1("fifo_W1");
    tapa::stream<int16_v16> fifo_W2("fifo_W2");
    tapa::stream<int16_v16> fifo_W3("fifo_W3");

    tapa::stream<int16_v16> fifo_layer1("fifo_layer1");
    tapa::stream<int16_v16> fifo_relu1("fifo_relu1");

    tapa::stream<int16_v16> fifo_layer2("fifo_layer2");
    tapa::stream<int16_v16> fifo_relu2("fifo_relu2");

    tapa::stream<int16_v16> fifo_layer3("fifo_layer3");
    tapa::stream<int16_v16> fifo_relu3("fifo_relu3");

    tapa::stream<bool> fifo_fin("fifo_fin");

    tapa::task()
        .invoke<tapa::join>(read_X, X, fifo_X)
        .invoke<tapa::join>(read_W, weight_size1, W1, fifo_W1)
        .invoke<tapa::join>(read_W, weight_size2, W2, fifo_W2)
        .invoke<tapa::join>(read_W, weight_size3, W3, fifo_W3)

        // Layer 1
        .invoke<tapa::join>(layer1_3, fifo_X, fifo_relu2, fifo_W1, fifo_W3, fifo_layer1, fifo_layer3)
        .invoke<tapa::join>(relu, hidden_size1, fifo_layer1, fifo_relu1)
        .invoke<tapa::join>(relu, output_size, fifo_layer3, fifo_relu3)
        // Layer 2
        .invoke<tapa::join>(layer2, fifo_relu1, fifo_W2, fifo_layer2)
        .invoke<tapa::join>(relu, hidden_size2, fifo_layer2, fifo_relu2)


        .invoke<tapa::join>(write_mtx, data_out, fifo_relu3, fifo_fin)

        .invoke<tapa::join>(measure_cycle, fifo_fin, cycle_count)
    ;
}
