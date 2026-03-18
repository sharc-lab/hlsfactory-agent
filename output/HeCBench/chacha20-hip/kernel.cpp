#include "kernel.h"

// --- from main.cu ---
void hex_to_raw(const char* src, const int n /*src size*/, uint8_t* dst, const uint8_t* char_to_uint){
  for (int i = _tid_x; i < n/2; i = i + BLOCK_DIM_X) {
    uint8_t hi = char_to_uint[src[i*2 + 0]];
    uint8_t lo = char_to_uint[src[i*2 + 1]];
    dst[i] = (hi << 4) | lo;
  }
}
extern "C"

void test_keystreams (
    const char * text_key,
    const char * text_nonce,
    const char * text_keystream,
    const uint8_t * char_to_uint,
    uint8_t * raw_key,
    uint8_t * raw_nonce,
    uint8_t * raw_keystream,
    uint8_t * result,
    const int text_key_size,
    const int text_nonce_size,
    const int text_keystream_size)

{
    #pragma HLS INTERFACE m_axi port=text_key offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=text_nonce offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=text_keystream offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=char_to_uint offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=raw_key offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=raw_nonce offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=raw_keystream offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=text_key_size
    #pragma HLS INTERFACE s_axilite port=text_nonce_size
    #pragma HLS INTERFACE s_axilite port=text_keystream_size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1

        hex_to_raw(text_key, text_key_size, raw_key, char_to_uint);
        hex_to_raw(text_nonce, text_nonce_size, raw_nonce, char_to_uint);
        hex_to_raw(text_keystream, text_keystream_size, raw_keystream, char_to_uint);

        if (_tid_x == 0) {
        Chacha20 chacha(raw_key, raw_nonce);
        chacha.crypt(result, text_keystream_size / 2);
        }

    }
}
