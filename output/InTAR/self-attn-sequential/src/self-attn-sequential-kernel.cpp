#include <iostream>
#include <ap_int.h>
#include <tapa.h>
// #include <glog/logging.h>

using namespace std;

#define N 256
#define D 1024
#define VEC_LEN 16
#define SCALE_FACTOR 32

typedef ap_int<16> type_t;
using vec_t = tapa::vec_t<type_t, VEC_LEN>;
constexpr int input_size = N * (D / VEC_LEN);

void measure_cycle(tapa::istreams<bool, 1>& fifo_fin, tapa::mmap<int> cycle_count){
    measure_cycle_loop: for (int cycle = 0;;cycle++){
        bool flag_cont = false;
        for(int i = 0; i < 1; i++){
            flag_cont |= fifo_fin[i].empty();
        }
        if(!flag_cont){
            measure_cycle_loop_count: for (int i = 0; i < 1; i++){
                fifo_fin[i].read(nullptr);
            }
            cycle_count[0] = cycle;
            break;
        }
    }
}

void vector_mac(const vec_t& a, const vec_t& b, type_t& c){
#pragma HLS inline
    for (int i = 0; i < VEC_LEN; i++){
        c += a[i] * b[i];
    }
}


void attention_top(
    tapa::async_mmap<vec_t>& in_glb,
    tapa::async_mmap<vec_t>& WQ_glb,  // flattened with column major order
    tapa::async_mmap<vec_t>& WK_glb,  // flattened with column major order
    tapa::async_mmap<vec_t>& WV_glb,  // flattened with column major order
    tapa::async_mmap<vec_t>& offchip_Q,
    tapa::async_mmap<vec_t>& offchip_K,
    tapa::async_mmap<vec_t>& offchip_V,
    tapa::async_mmap<vec_t>& out_glb,
    tapa::ostream<bool>& fifo_fin
){
    // FIXME: do not use computation in array declaration
    // use constexpr in the example which will be evaluated at compile time and plugin
    vec_t input[input_size];  // segment the input into block matrix with N by (D / VEC_LEN) blocks and flattened
    vec_t tmp_vec;

    type_t S[N][N];  // scores Q * K^T

    // Read and cache input
    read_input_loop: for(int i_req = 0, i_resp = 0; i_resp < input_size;){
        #pragma HLS pipeline II=1 style=stp
        if((i_req < input_size) & !in_glb.read_addr.full()){
            in_glb.read_addr.write(i_req);
            i_req++;
        }
        bool success = in_glb.read_data.try_read(tmp_vec);
        if(success){
            input[i_resp] = tmp_vec;
            i_resp++;
        }
    }

    // LOG(INFO) << "Input read and cached";

    vec_t acc[N];
    // initialize acc to 0
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < VEC_LEN; k++) {
            acc[i][k] = 0;
        }
    }

    // Q = WQ * input
    for (int j = 0; j < D; j++){        
        for (int k_req = 0, k_resp = 0; k_resp < D / VEC_LEN;){
            // FIXME; check blocking/non-blocking
            if (k_req < D / VEC_LEN && !WQ_glb.read_addr.full()){
                WQ_glb.read_addr.write(j * D / VEC_LEN + k_req);
                k_req++;
            }
            bool success = WQ_glb.read_data.try_read(tmp_vec);
            if(success){                
                for (int i = 0; i < N; i++){  // input is already cached, can directly read
                    // FIXME: Do not use funcitno, just copy the code here
                    vector_mac(input[i * (D / VEC_LEN) + k_resp], tmp_vec, acc[i][j % VEC_LEN]);  // FIXME: Since VEC_LEN is 16, we can efficiently use the last 4 bits to index the vector for the mod operation. 
                }
                k_resp++;
            }
        }
        // increment first then use it to do the modulo, so we don't need to use (j_req + 1) % VEC_LEN
        if ((j + 1) % VEC_LEN == 0){
            // write a column to offchip
            for (int i_req = 0, i_resp = 0; i_resp < N;){
                if (i_req < N && !offchip_Q.write_addr.full() && !offchip_Q.write_data.full()){
                    offchip_Q.write_addr.write(i_req * (D / VEC_LEN) + ((j-1) / VEC_LEN));
                    offchip_Q.write_data.try_write(acc[i_req]);
                    i_req++;
                }
                bool success = false;
                auto resp = offchip_Q.write_resp.read(success);
                if(success){
                    i_resp += unsigned(resp)+1;
                }
            }

            // clear out acc to 0
            for (int i = 0; i < N; i++) {
                for (int k = 0; k < VEC_LEN; k++) {
                    acc[i][k] = 0;
                }
            }
        }
    }

    // LOG(INFO) << "Q computed";

    // K = WK * input
    for (int j = 0; j < D;){
        for (int k_req = 0, k_resp = 0; k_resp < D / VEC_LEN;){
            if (k_req < D / VEC_LEN && !WK_glb.read_addr.full()){
                WK_glb.read_addr.write(j * D / VEC_LEN + k_req);
                k_req++;
                
            }
            bool success = WK_glb.read_data.try_read(tmp_vec);
            if(success){                
                for (int i = 0; i < N; i++){  // input is already cached, can directly read
                    vector_mac(input[i * (D / VEC_LEN) + k_resp], tmp_vec, acc[i][j % VEC_LEN]);  // FIXME: Since VEC_LEN is 16, we can efficiently use the last 4 bits to index the vector for the mod operation. 
                }
                k_resp++;
            }
        }
        j++;  // increment first then use it to do the modulo, so we don't need to use (j_req + 1) % VEC_LEN
        if (j % VEC_LEN == 0){
            // write a column to offchip
            for (int i_req = 0, i_resp = 0; i_resp < N;){
                if (i_req < N && !offchip_K.write_addr.full() && !offchip_K.write_data.full()){
                    offchip_K.write_addr.write(i_req * (D / VEC_LEN) + ((j-1) / VEC_LEN));
                    offchip_K.write_data.try_write(acc[i_req]);
                    i_req++;
                }
                bool success = false;
                auto resp = offchip_K.write_resp.read(success);
                if(success){
                    i_resp += unsigned(resp)+1;
                }
            }

            // clear out acc to 0
            for (int i = 0; i < N; i++) {
                for (int k = 0; k < VEC_LEN; k++) {
                    acc[i][k] = 0;
                }
            }
        }
    }

    // LOG(INFO) << "K computed";

    // V = WV * input
    for (int j = 0; j < D;){
        for (int k_req = 0, k_resp = 0; k_resp < D / VEC_LEN;){
            if (k_req < D / VEC_LEN && !WV_glb.read_addr.full()){
                WV_glb.read_addr.write(j * D / VEC_LEN + k_req);
                k_req++;
                
            }
            bool success = WV_glb.read_data.try_read(tmp_vec);
            if(success){                
                for (int i = 0; i < N; i++){  // input is already cached, can directly read
                    vector_mac(input[i * (D / VEC_LEN) + k_resp], tmp_vec, acc[i][j % VEC_LEN]);  // FIXME: Since VEC_LEN is 16, we can efficiently use the last 4 bits to index the vector for the mod operation. 
                }
                k_resp++;
            }
        }
        j++;  // increment first then use it to do the modulo, so we don't need to use (j_req + 1) % VEC_LEN
        if (j % VEC_LEN == 0){
            // write a column to offchip
            for (int i_req = 0, i_resp = 0; i_resp < N;){
                if (i_req < N && !offchip_V.write_addr.full() && !offchip_V.write_data.full()){
                    offchip_V.write_addr.write(i_req * (D / VEC_LEN) + ((j-1) / VEC_LEN));
                    offchip_V.write_data.try_write(acc[i_req]);
                    i_req++;
                }
                bool success = false;
                auto resp = offchip_V.write_resp.read(success);
                if(success){
                    i_resp += unsigned(resp)+1;
                }
            }

            // clear out acc to 0
            for (int i = 0; i < N; i++) {
                for (int k = 0; k < VEC_LEN; k++) {
                    acc[i][k] = 0;
                }
            }
        }
    }

    // LOG(INFO) << "V computed";

    // scores = Q * K^T
    vec_t tmp_q[D / VEC_LEN];
    vec_t tmp_k;
    for (int i = 0; i < N; i++){
        // first cache all the q row
        for (int kq_req = 0, kq_resp = 0; kq_resp < D / VEC_LEN;){
            if (kq_req < D / VEC_LEN && !offchip_Q.read_addr.full()){
                offchip_Q.read_addr.write(i * (D / VEC_LEN) + kq_req);
                kq_req++;
            }
            bool q_success = offchip_Q.read_data.try_read(tmp_q[kq_resp]);
            if (q_success){
                kq_resp++;
            }
        }
        
        for (int j = 0; j < N; j++){
            S[i][j] = 0;

            // accumulate according to the k column read
            for (int kk_req = 0, kk_resp = 0; kk_resp < D / VEC_LEN;){
                if (kk_req < D / VEC_LEN && !offchip_K.read_addr.full()){
                    offchip_K.read_addr.write(j * (D / VEC_LEN) + kk_req);
                    kk_req++;
                }
                bool k_success = offchip_K.read_data.try_read(tmp_k);
                if (k_success){
                    vector_mac(tmp_q[kk_resp], tmp_k, S[i][j]);
                    kk_resp++;
                }
            }
        }
    }

    // LOG(INFO) << "S computed";
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            S[i][j] = S[i][j] / sqrt(D); // scaled attention
        }
    }

    // softmax
    for (size_t i = 0; i < N; ++i) {
        float sum = 0.0;

        for (size_t j = 0; j < N; ++j) {
            S[i][j] = (type_t) exp(float(S[i][j]));
            sum += S[i][j];
        }
        for (size_t j = 0; j < N; ++j) {
            S[i][j] = S[i][j] / sum * SCALE_FACTOR;
        }
    }

    // LOG(INFO) << "S softmaxed";

    // output = S * V
    for (int j = 0; j < D / VEC_LEN;){
        vec_t tmp_v[N];
        vec_t tmp_out[N];
        for (int i_req = 0, i_resp = 0; i_resp < N;){
            if (i_req < N && !offchip_V.read_addr.full()){
                offchip_V.read_addr.write(i_req * (D / VEC_LEN) + j);
                i_req++;
            }
            bool v_success = offchip_V.read_data.try_read(tmp_v[i_resp]);
            if (v_success){
                i_resp++;
            }
        }
        // compute a matrix mult between S and tmp_v with dimension N by N and N by D / VEC_LEN respectively
        for (int i = 0; i < N; i++){
            for (int j = 0; j < D / VEC_LEN; j++){
                tmp_out[i][j] = 0;
                for (int k = 0; k < N; k++){
                    tmp_out[i][j] += S[i][k] * tmp_v[k][j];
                }
                tmp_out[i][j] = tmp_out[i][j] / SCALE_FACTOR;  // add back the scaling in the softmax
            }
        }

        // write to output
        for (int i_req = 0, i_resp = 0; i_resp < N;){
            if (i_req < N && !out_glb.write_addr.full() && !out_glb.write_data.full()){
                out_glb.write_addr.write(i_req * (D / VEC_LEN) + j);
                out_glb.write_data.try_write(tmp_out[i_req]);
                i_req++;
            }
            bool success = false;
            auto resp = out_glb.write_resp.read(success);
            if(success){
                i_resp += unsigned(resp)+1;
            }
        }
        j++;
    }

    // LOG(INFO) << "Output computed";

    // terminate the kernel
    fifo_fin.write(true);
}

// Self-attention computation
void selfAttention(
    tapa::mmap<vec_t> in_glb,
    tapa::mmap<vec_t> WQ_glb,
    tapa::mmap<vec_t> WK_glb,
    tapa::mmap<vec_t> WV_glb,
    tapa::mmap<vec_t> offchip_Q,
    tapa::mmap<vec_t> offchip_K,
    tapa::mmap<vec_t> offchip_V,
    tapa::mmap<vec_t> out_glb,
    tapa::mmap<int> cycle_count
) {

    tapa::streams<bool, 1> fifo_fin("fifo_fin");

    tapa::task()
        .invoke<tapa::join>(
            attention_top,
            in_glb,
            WQ_glb,
            WK_glb,
            WV_glb,
            offchip_Q,
            offchip_K,
            offchip_V,
            out_glb,
            fifo_fin
        )
        .invoke<tapa::join>(
            measure_cycle,
            fifo_fin,
            cycle_count
        )
    ;
}
