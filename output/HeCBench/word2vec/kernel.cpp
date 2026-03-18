#include "kernel.h"

// --- from cbow.cu ---
extern "C"
void device_memset(real * array, int size){
    #pragma HLS INTERFACE m_axi port=array offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int idx = _bid_x * BLOCK_DIM_X + _tid_x;
            if (idx < size)
            array[idx] = 0;

        }
    }
}

void reduceInWarp(volatile float * f, int idInWarp){

  for (unsigned int i=THREADS_PER_WORD /2; i>32; i>>=1) {
    if (idInWarp < i) {
      f[idInWarp] += f[idInWarp + i];
    }
  }
  if (idInWarp < 32){
    f[idInWarp] += f[idInWarp + 32];
    f[idInWarp] += f[idInWarp + 16];
    f[idInWarp] += f[idInWarp + 8];
    f[idInWarp] += f[idInWarp + 4];
    f[idInWarp] += f[idInWarp + 2];
    f[idInWarp] += f[idInWarp + 1];
  }
}
extern "C"

void device_cbow(
    const int sentence_num,
    const int layer1_size,
    const int layer1_size_aligned,
    const int window,
    const int negative,
    const int table_size,
    const int vocab_size,
    const int * d_sen,
    const int * d_table,
    float * d_syn0,
    float * d_syn1neg,
    unsigned int * d_random)
{
    #pragma HLS INTERFACE s_axilite port=sentence_num
    #pragma HLS INTERFACE s_axilite port=layer1_size
    #pragma HLS INTERFACE s_axilite port=layer1_size_aligned
    #pragma HLS INTERFACE s_axilite port=window
    #pragma HLS INTERFACE s_axilite port=negative
    #pragma HLS INTERFACE s_axilite port=table_size
    #pragma HLS INTERFACE s_axilite port=vocab_size
    #pragma HLS INTERFACE m_axi port=d_sen offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_table offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_syn0 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_syn1neg offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=d_random offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int sentence_position = (_tid_x / THREADS_PER_WORD) + (BLOCK_DIM_X / THREADS_PER_WORD) * _bid_x;
            int idInWarp = _tid_x % THREADS_PER_WORD;

            float shared[4096];
            float * f = shared + (_tid_x / THREADS_PER_WORD) * THREADS_PER_WORD;
            float * neu1 = shared + BLOCK_SIZE + (_tid_x / THREADS_PER_WORD) * layer1_size_aligned;
            float * neu1e= shared + BLOCK_SIZE + (BLOCK_DIM_X / THREADS_PER_WORD) * layer1_size_aligned +
            (_tid_x / THREADS_PER_WORD) * layer1_size_aligned;

            if (sentence_position < MAX_SENTENCE_LENGTH) {
            unsigned int next_random = d_random[sentence_position];

            for (int sentence_idx = 0; sentence_idx < sentence_num; sentence_idx++) {

            for (int c = idInWarp; c < layer1_size; c+=THREADS_PER_WORD) {
            neu1[c] = 0;
            neu1e[c] = 0;
            }

            next_random = next_random * (unsigned int) 1664525 + 1013904223;
            int b = next_random % window;
            int word = d_sen[sentence_idx * MAX_SENTENCE_LENGTH + sentence_position];
            // in -> hidden
            int cw = 0;
            for (int a = b; a < window * 2 + 1 - b; a++)
            if (a != window) {
            int w = sentence_position - window + a;
            if (w < 0 || w>= MAX_SENTENCE_LENGTH) continue;
            int last_word = d_sen[sentence_idx * MAX_SENTENCE_LENGTH + w];
            for (int c = idInWarp; c < layer1_size; c+= THREADS_PER_WORD)
            neu1[c] += d_syn0[c + last_word * layer1_size_aligned];

            cw++;
            }

            if (cw) {
            for (int c = idInWarp; c < layer1_size; c+= THREADS_PER_WORD)
            neu1[c] /= cw;

            // NEGATIVE SAMPLING
            int target, label;
            float alpha =*((float *) &d_sen[MAX_SENTENCE_NUM * MAX_SENTENCE_LENGTH + sentence_idx]);

            if (negative > 0)

            for (int d = 0; d < negative + 1; d++) {

            if (d == 0) {
            target = word;
            label = 1;
            } else {
            next_random = next_random * (unsigned int) 1664525 + 1013904223;
            target = d_table[(next_random) % table_size];
            if (target == 0)
            target = next_random % (vocab_size - 1) + 1;
            if (target == word)
            continue;
            label = 0;
            }
            int l2 = target * layer1_size_aligned;
            f[idInWarp] = 0;

            for (int c = idInWarp; c < layer1_size; c+=THREADS_PER_WORD){
            f[idInWarp] += neu1[c] * d_syn1neg[c + l2];
            }

            // Do reduction here;
            reduceInWarp(f, idInWarp);

            float g;
            if (f[0] > MAX_EXP)
            g = (label - 1) * alpha;
            else if (f[0] < -MAX_EXP)
            g = (label - 0) * alpha;
            else
            g = (label - expTable[(int) ((f[0] + MAX_EXP)
            * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;

            for (int c = idInWarp; c < layer1_size; c+=THREADS_PER_WORD)
            neu1e[c] += g * d_syn1neg[c + l2];
            for (int c = idInWarp; c < layer1_size; c+=THREADS_PER_WORD)
            d_syn1neg[c + l2] += g * neu1[c];
            }

            // hidden -> in
            for (int a = b; a < window * 2 + 1 - b; a++)
            if (a != window) {
            int w = sentence_position - window + a;
            if (w < 0)
            continue;
            if (w >= MAX_SENTENCE_LENGTH)
            continue;
            int last_word = d_sen[sentence_idx * MAX_SENTENCE_LENGTH + w];

            for (int c = idInWarp; c < layer1_size; c+=THREADS_PER_WORD)
            d_syn0[c + last_word * layer1_size_aligned] += neu1e[c];
            }
            }
            }// End for sentence_idx

            // Update d_random
            if (idInWarp == 0 ) d_random[sentence_position] = next_random;
            }

        }
    }
}
