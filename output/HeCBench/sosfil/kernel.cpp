#include "kernel.h"

// --- from main.cu ---
extern "C"
void sosfilt(
    const int n_signals,
    const int n_samples,
    const int n_sections,
    const int zi_width,
    const T * sos,
    const T * zi,
          T * x_in)
{
    #pragma HLS INTERFACE s_axilite port=n_signals
    #pragma HLS INTERFACE s_axilite port=n_samples
    #pragma HLS INTERFACE s_axilite port=n_sections
    #pragma HLS INTERFACE s_axilite port=zi_width
    #pragma HLS INTERFACE m_axi port=sos offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=zi offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=x_in offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=smem complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            char smem[4096];
            T *s_out = reinterpret_cast<T *>( smem );
            T *s_zi = reinterpret_cast<T *>( &s_out[n_sections] ) ;
            T *s_sos = reinterpret_cast<T *>( &s_zi[n_sections * zi_width] ) ;

            const int tx = _tid_x;
            const int ty = _bid_x;

            // Reset shared memory
            s_out[tx] = 0;

            // Load zi
            for ( int i = 0; i < zi_width; i++ ) {
            s_zi[tx * zi_width + i] = zi[ty * n_sections * zi_width + tx * zi_width + i];
            }

            // Load SOS
            #pragma unroll
            for ( int i = 0; i < sos_width; i++ ) {
            s_sos[tx * sos_width + i] = sos[tx * sos_width + i];
            }

            const int load_size = n_sections - 1 ;
            const int unload_size = n_samples - load_size ;

            T temp;
            T x_n;

            if ( ty < n_signals ) {
            // Loading phase
            for ( int n = 0; n < load_size; n++ ) {
            if ( tx == 0 ) {
            x_n = x_in[ty * n_samples + n];
            } else {
            x_n = s_out[tx - 1];
            }

            // Use direct II transposed structure
            temp = s_sos[tx * sos_width + 0] * x_n + s_zi[tx * zi_width + 0];

            s_zi[tx * zi_width + 0] =
            s_sos[tx * sos_width + 1] * x_n - s_sos[tx * sos_width + 4] * temp + s_zi[tx * zi_width + 1];

            s_zi[tx * zi_width + 1] = s_sos[tx * sos_width + 2] * x_n - s_sos[tx * sos_width + 5] * temp;

            s_out[tx] = temp;
            }

            // Processing phase
            for ( int n = load_size; n < n_samples; n++ ) {
            if ( tx == 0 ) {
            x_n = x_in[ty * n_samples + n];
            } else {
            x_n = s_out[tx - 1];
            }

            // Use direct II transposed structure
            temp = s_sos[tx * sos_width + 0] * x_n + s_zi[tx * zi_width + 0];

            s_zi[tx * zi_width + 0] =
            s_sos[tx * sos_width + 1] * x_n - s_sos[tx * sos_width + 4] * temp + s_zi[tx * zi_width + 1];

            s_zi[tx * zi_width + 1] = s_sos[tx * sos_width + 2] * x_n - s_sos[tx * sos_width + 5] * temp;

            if ( tx < load_size ) {
            s_out[tx] = temp;
            } else {
            x_in[ty * n_samples + ( n - load_size )] = temp;
            }
            }

            // Unloading phase
            for ( int n = 0; n < n_sections; n++ ) {
            // retire threads that are less than n
            if ( tx > n ) {
            x_n = s_out[tx - 1];

            // Use direct II transposed structure
            temp = s_sos[tx * sos_width + 0] * x_n + s_zi[tx * zi_width + 0];

            s_zi[tx * zi_width + 0] =
            s_sos[tx * sos_width + 1] * x_n - s_sos[tx * sos_width + 4] * temp + s_zi[tx * zi_width + 1];

            s_zi[tx * zi_width + 1] = s_sos[tx * sos_width + 2] * x_n - s_sos[tx * sos_width + 5] * temp;

            if ( tx < load_size ) {
            s_out[tx] = temp;
            } else {
            x_in[ty * n_samples + ( n + unload_size )] = temp;
            }
            }
            }
            }

        }
    }
}
