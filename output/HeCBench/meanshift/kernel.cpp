#include "kernel.h"

// --- from main.cu ---
  extern "C"
  void mean_shift(const float *data, float *data_next) {
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=data_next offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t tid = _bid_x * BLOCK_DIM_X + _tid_x;
            if (tid < N) {
            size_t row = tid * D;
            float new_position[D] = {0.f};
            float tot_weight = 0.f;
            for (size_t i = 0; i < N; ++i) {
            size_t row_n = i * D;
            float sq_dist = 0.f;
            for (size_t j = 0; j < D; ++j) {
            sq_dist += (data[row + j] - data[row_n + j]) * (data[row + j] - data[row_n + j]);
            }
            if (sq_dist <= RADIUS) {
            float weight = expf(-sq_dist / DBL_SIGMA_SQ);
            for (size_t j = 0; j < D; ++j) {
            new_position[j] += weight * data[row_n + j];
            }
            tot_weight += weight;
            }
            }
            for (size_t j = 0; j < D; ++j) {
            data_next[row + j] = new_position[j] / tot_weight;
            }
            }

        }
    }
}
extern "C"

  void mean_shift_tiling(const float* data, float* data_next) {
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=data_next offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=local_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=valid_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            // Shared memory allocation
            float local_data[TILE_WIDTH * D];
            float valid_data[TILE_WIDTH];
            // A few convenient variables
            int lid = _tid_x;
            int tid = _bid_x * BLOCK_DIM_X + lid;
            int row = tid * D;
            int local_row = lid * D;
            float new_position[D] = {0.f};
            float tot_weight = 0.f;
            // Load data in shared memory
            for (int t = 0; t < BLOCKS; ++t) {
            int tid_in_tile = t * TILE_WIDTH + lid;
            if (tid_in_tile < N) {
            int row_in_tile = tid_in_tile * D;
            for (int j = 0; j < D; ++j) {
            local_data[local_row + j] = data[row_in_tile + j];
            }
            valid_data[lid] = 1;
            }
            else {
            for (int j = 0; j < D; ++j) {
            local_data[local_row + j] = 0;
            }
            valid_data[lid] = 0;
            }
            for (int i = 0; i < TILE_WIDTH; ++i) {
            int local_row_tile = i * D;
            float valid_radius = RADIUS * valid_data[i];
            float sq_dist = 0.;
            for (int j = 0; j < D; ++j) {
            sq_dist += (data[row + j] - local_data[local_row_tile + j]) *
            (data[row + j] - local_data[local_row_tile + j]);
            }
            if (sq_dist <= valid_radius) {
            float weight = expf(-sq_dist / DBL_SIGMA_SQ);
            for (int j = 0; j < D; ++j) {
            new_position[j] += (weight * local_data[local_row_tile + j]);
            }
            tot_weight += (weight * valid_data[i]);
            }
            }
            }
            if (tid < N) {
            for (int j = 0; j < D; ++j) {
            data_next[row + j] = new_position[j] / tot_weight;
            }
            }

        }
    }
}
