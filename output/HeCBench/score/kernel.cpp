#include "kernel.h"

// --- from main.cu ---
T clamp(T value, T lower, T upper) { return min(max(value, lower), upper); }
extern "C"

void findTopK(int* indices_, 
              int* count_, 
              const T* scores_,
              const float threshold,
              const int classwise_topK,
              const size_t num_classes,
              const size_t num_priors)
{
    #pragma HLS INTERFACE m_axi port=indices_ offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=count_ offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=scores_ offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=threshold
    #pragma HLS INTERFACE s_axilite port=classwise_topK
    #pragma HLS INTERFACE s_axilite port=num_classes
    #pragma HLS INTERFACE s_axilite port=num_priors
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=bins complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                /* We need to sort boxes based on their confidence scores. The confidence scores fall in
                * the range [0.0, 1.0]. We break the range into bins and perform count sort. This is an
                * approximate algorithm.
                *
                * Each block handles a particular class of a particular batch item.
                */
                const auto c = _bid_x;
                const auto b = _bid_y;

                // indices: [batch_size, num_classes, classwise_topK]
                // count: [batch_size, num_classes]
                // scores: [batch_size, num_classes, num_priors]

                auto indices = indices_ + (b * num_classes + c) * classwise_topK;
                auto count = count_ + b * num_classes + c;
                auto scores = scores_ + (b * num_classes + c) * num_priors;

                /* We do not require a large number of bins to find the top K confidence scores. We will use
                * a reasonable number of bins which will fit in the shared memory.
                *
                * Note that smaller scores will have a smaller index, i.e. the `bins` are ordered in
                * ascending order.
                */

                int bins[BINS];

                #pragma unroll
                for (int unroll = 0; unroll < BINS / BLOCK_SIZE; unroll++)
                bins[unroll * BLOCK_SIZE + _tid_x] = 0;

                for (int i = _tid_x; i < num_priors; i = i + BLOCK_SIZE)
                {
                const float confidence = scores[i];
                if (confidence > threshold)
                {
                float conf_scaled = __fdividef(confidence - threshold, 1.f - threshold);
                int bin_index = conf_scaled * BINS;

                /* We store counts of confidence scores in the bins. Our ultimate goal is to store the indices
                * of the `classwise_topK` confidence values in the `indices` array.
                *
                * We use a little trick to parallelize the process of filling up the `indices` array.
                * We want every thread in the block to participate in the process. To do so, we want the
                * bins array to be shifted by one place to the left. We will be computing the suffix sum
                * of the bins array later. Details and reasons for doing so will be explained later.
                */
                bin_index = clamp<int>(bin_index, 0, BINS - 1) - 1; // shift left by one

                if (bin_index >= 0)
                (bins[bin_index] += 1);
                }
                }

                constexpr int WARP_SIZE = 32; /* must be equal to warpSize */

                if (_tid_x < WARP_SIZE)
                {
                /* We can compute suffix sum of an array in groups of N numbers.
                * Let N be 4 for this example.
                *
                * 1) Last 4 numbers
                *                      1   2   3   4   |   5   6   7   8   |   9   10  11  12
                * group suffix sum:                                            42  33  23  12
                *
                * 2) Middle 4 numbers
                *                      1   2   3   4   |   5   6   7   8   |   9   10  11  12
                * group suffix sum:                    |   26  21  15  8   |
                *
                * We add `42` (first element in the previous group) to each element to get:
                *
                *                      1   2   3   4   |   5   6   7   8   |   9   10  11  12
                *                                      |   68  63  57  50  |   42  33  23  12
                * 3) First 4 numbers
                *
                *                      1   2   3   4   |   5   6   7   8   |   9   10  11  12
                * group suffix sum:    10  9   7   4   |
                *
                * We add `68` (first element in the previous group) to each element to get:
                *
                *                      1   2   3   4   |   5   6   7   8   |   9   10  11  12
                * group suffix sum:    78  77  75  72  |   68  63  57  50  |   42  33  23  12
                *
                * What we are left with now is the suffix sum of the entire array.
                *
                * We use the aforementioned logic in the code below but work in groups of `warpSize`.
                */

                /* We calculate suffix sums WARP_SIZE elements at a time starting from the right end.
                * Hence, we will need BINS / WARP_SIZE number of iterations.
                *
                * Each iteration uses shuffle instructions to exchange data between threads. Shuffle
                * instructions cannot be used in warp-divergent code. If the bins are a multiple of
                * the warpSize, all the threads in the warp will participate.
                */
                static_assert(BINS % WARP_SIZE == 0, "number of bins must be a multiple of warp size");

                const int thread_id = _tid_x;
                const int inverse_lane_id = WARP_SIZE - thread_id - 1;

                int previous_group_first_element = 0;
                for (int iter = BINS / WARP_SIZE - 1; iter >= 0; iter--)
                {
                const int idx = iter * WARP_SIZE + thread_id;
                auto value = bins[idx];

                for (int i = 1; i < WARP_SIZE; i *= 2)
                {
                auto n = 0;
                if (inverse_lane_id >= i)
                value += n;
                }

                value += previous_group_first_element;
                bins[idx] = value;

                previous_group_first_element = 0;
                }
                }

                if (_tid_x == 0) *count = 0;

                for (int i = _tid_x; i < num_priors; i = i + BLOCK_SIZE)
                {
                const float confidence = scores[i];
                if (confidence > threshold)
                {
                float conf_scaled = __fdividef(confidence - threshold, 1.f - threshold);
                int bin_index = conf_scaled * BINS;
                bin_index = clamp<int>(bin_index, 0, BINS - 1);

                /* This bounding box is eligible to be selected unless it does not fall in
                * the `classwise_topK`. If it did, we would have to compute the location where it needs
                * to be stored.
                *
                * Suppose we had just 4 bins and say the following were the counts:
                * BIN0 2
                * BIN1 1
                * BIN2 3
                * BIN3 0 (last bin is always zero as we shift left by one while populating the bins)
                *
                * We will try our best to store the boxes in a sorted order in the `indices` array.
                * This requires that the boxes in later bins (higher confidence scores) must be
                * stored earlier.
                *
                * We compute the suffix sum of the array. This gives us:
                * BIN0 6
                * BIN1 4
                * BIN2 3
                * BIN3 0
                *
                * The bins now give us the location in the `indices` array from which the indices of the
                * scores corresponding to that bin would be stored. We atomically increment the bin count
                * everytime we store a box corresponding to that bin. Therefore, the value in the bins
                * gives the index in the `indices` array where the next box corresponding to that bin  must
                * be put.
                */

                const int idx = (bins[bin_index] += 1);
                if (idx < classwise_topK)
                {
                indices[idx] = i;
                (count[0] += 1);
                }
                }
                }

            }
        }
    }
}
