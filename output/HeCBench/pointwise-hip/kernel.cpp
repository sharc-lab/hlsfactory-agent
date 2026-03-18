#include "kernel.h"

// --- from main.cu ---
extern "C"
void elementwise(int hiddenSize, int miniBatch,
    const float * tmp_h,
    const float * tmp_i,
    const float * bias,
    float * linearGates,
    float * h_out,
    float * i_out,
    const float * c_in,
    float * c_out)
{
    #pragma HLS INTERFACE s_axilite port=hiddenSize
    #pragma HLS INTERFACE s_axilite port=miniBatch
    #pragma HLS INTERFACE m_axi port=tmp_h offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=tmp_i offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bias offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=linearGates offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=h_out offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=i_out offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=c_in offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=c_out offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            int numElements = miniBatch * hiddenSize;

            if (index >= numElements) return;

            int batch = index / hiddenSize;
            int gateIndex = (index % hiddenSize) + 4 * batch * hiddenSize;

            float g[4];

            for (int i = 0; i < 4; i++) {
            g[i] = tmp_i[i * hiddenSize + gateIndex] + tmp_h[i * hiddenSize + gateIndex];
            g[i] += bias[i * hiddenSize + index % hiddenSize] + bias[(i + 4) * hiddenSize + index % hiddenSize];
            linearGates[gateIndex + i * hiddenSize] = g[i];
            }

            float in_gate     = 1.f / (1.f + expf(-g[0]));
            float forget_gate = 1.f / (1.f + expf(-g[1]));
            float in_gate2    = tanhf(g[2]);
            float out_gate    = 1.f / (1.f + expf(-g[3]));

            float val = (forget_gate * c_in[index]) + (in_gate * in_gate2);

            c_out[index] = val;

            val = out_gate * tanhf(val);

            h_out[index] = val;
            i_out[index] = val;

        }
    }
}

float LCG_random(unsigned int * seed) {
  const unsigned int m = 2147483648;
  const unsigned int a = 26757677;
  const unsigned int c = 1;
  *seed = (a * (*seed) + c) % m;
  return (float) (*seed) / (float) m;
}
extern "C"

void init (float* data, int size) {
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index >= size) return;
            unsigned int seed = index ^ size;
            data[index] = LCG_random(&seed);

        }
    }
}
