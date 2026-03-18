#include "kernel.h"

// --- from main.cu ---
T init_vector(int v){
  return 0;
}

int init_vector(int v){
  return v;
}

int2 init_vector(int v){
  return int2(v, v);
}

int4 init_vector(int v){
  return make_int4(v, v, v, v);
}

int reduce_vector(T v){
  return 0;
}

int reduce_vector(int v){
  return v;
}

int reduce_vector(int2 v){
  return v.x + v.y;
}

int reduce_vector(int4 v){
  return v.x + v.y + v.z + v.w;
}

void add_vector(T &target, const T &v){
}

void add_vector(int &target, const int &v) {
  target += v;
}

void add_vector(int2 &target, const int2 &v) {
  target.x += v.x;
  target.y += v.y;
}

void add_vector(int4 &target, const int4 &v) {
  target.x += v.x;
  target.y += v.y;
  target.z += v.z;
  target.w += v.w;
}
extern "C"

void benchmark_constant(int *output, int repeat)
{
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=repeat
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            T* constant_data_p = (T*)constant_data;
            T sum = init_vector<T>(0);

            for(int i=0; i<4; i++){
            for(int j=0; j<VECTOR_SIZE/(sizeof(T)/sizeof(int)); j+=4){
            add_vector(sum, constant_data_p[j+i]);
            }
            }

            if( _tid_x==0 && _bid_x==0 ) {
            *output = reduce_vector(sum);
            }

        }
    }
}
