#include "kernel.h"

// --- from main.cu ---
extern "C"
void log_probs_kernel(
    float*       log_probs,
    const T*     logits,
    const int*   ids,
    const int*   lengths,
    const int    max_input_length,
    const int    batch_size,
    const int    vocab_size,
    const int    vocab_size_padded)
{
    #pragma HLS INTERFACE m_axi port=log_probs offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=logits offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=ids offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=lengths offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=max_input_length
    #pragma HLS INTERFACE s_axilite port=batch_size
    #pragma HLS INTERFACE s_axilite port=vocab_size
    #pragma HLS INTERFACE s_axilite port=vocab_size_padded
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                // Calculate the log probability from logits.
                //   log_probs[t, :] = log(softmax(logits))[ids[t + 1, :]]
                //
                // log_probs: [batch_size, max_length -1],
                //     log probabilities of each token.
                // logits: [batch_size, max_length, vocab_size_padded]
                // lengths: [batch_size], sequence lengths
                // ids: [max_length, batch_size], token ids.
                // batch_size: [1], batch_size. in case of beam > 1, batch x beam.
                // vocab_size: [1], vocab_size,
                // vocab_size: [1], vocab_size_padded, padded vocab size.

                const bool IS_FP16   = std::is_same<T, half>::value;
                const T    MAX_T_VAL = (IS_FP16) ? HALF_FLT_MAX : FLT_MAX;

                int tidx = _tid_x; // vocab dim
                int step = _bid_x;  // step dim
                int bidx = _bid_y;  // batch dim

                float s_max_logit;

                if (bidx < batch_size && step < lengths[bidx] - 1) {
                // Compute the address of logits to data for the current batch
                int step_offset  = step * vocab_size_padded;
                int batch_offset = bidx * max_input_length * vocab_size_padded;
                logits += step_offset + batch_offset;

                // Find max(logits)
                float local_max = -MAX_T_VAL;
                float val       = -MAX_T_VAL;
                for (int i = tidx; i < vocab_size; i += BLOCK_DIM_X) {
                val       = static_cast<float>(logits[i]);
                local_max = fmaxf(local_max, val);
                }

                float max_val = blockReduceMax<float>(local_max);
                if (tidx == 0) {
                s_max_logit = max_val;
                }

                // Calculate the denominator: sum_i exp(logits[i])
                float local_sum_exp = 0.0f;
                for (int i = tidx; i < vocab_size; i += BLOCK_DIM_X) {
                val = expf(static_cast<float>(logits[i]) - s_max_logit);
                local_sum_exp += val;
                }

                float sum_exp = blockReduceSum<float>(local_sum_exp);
                if (tidx == 0) {
                int idx = step + bidx * (max_input_length - 1);
                // log_probs[step, ...] is the log probability of a token at step t + 1.
                int token_idx = step + 1 + bidx * max_input_length;
                log_probs[idx] = static_cast<float>(logits[ids[token_idx]]) - s_max_logit - logf(sum_exp + 1e-9f);
                }
                }

            }
        }
    }
}
extern "C"

void accumulate_log_probs(
          float* cum_log_probs,
    const float* log_probs,
    const int*   lengths,
    const int    max_input_length,
    const int    batch_size)
{
    #pragma HLS INTERFACE m_axi port=cum_log_probs offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=log_probs offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=lengths offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=max_input_length
    #pragma HLS INTERFACE s_axilite port=batch_size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                // Accumulate the log probability along the sequence dimension.
                //   cum_log_probs[j] = sum_i log(softmax(logits))[ids[i,j]]
                //
                // cum_log_probs: [batch_size], cumulative log probability
                // log_probs: [batch_size, max_length - 1],
                //   log probability of each token
                // lengths: [batch_size], sequence lengths
                // batch_size: [1], batch_size. in case of beam > 1, batch x beam.

                int bidx = _bid_x;   // batch dim
                int tidx = _tid_x;  // step dim
                int length = lengths[bidx];

                // reposition logits to data for the current batch.
                log_probs += bidx * (max_input_length - 1);
                float local_accum = 0.0f;
                for (int step = tidx; step < length - 1; step += BLOCK_DIM_X) {
                local_accum += static_cast<float>(log_probs[step]);
                }
                float accum = blockReduceSum<float>(local_accum);
                if (tidx == 0) {
                cum_log_probs[bidx] = accum;
                }

            }
        }
    }
}
