#include "kernel.h"

// --- from main.cu ---
inline __nv_bfloat162 hmul2(__nv_bfloat162 x, __nv_bfloat162 y) {
  return __hmul2(x, y);
}

inline __nv_bfloat162 hadd2(__nv_bfloat162 x, __nv_bfloat162 y) {
  return __hadd2(x, y);
}

inline __nv_bfloat162 float_to_bfloat2(float val) {
  return __float2bfloat162_rn(val);
}

inline void fp8x4_e4m3_to_bfloat2(__nv_bfloat162* out1, __nv_bfloat162* out2, const __nv_fp8x4_e4m3* in)
{
  const char4 tmp_val = reinterpret_cast<const char4*>(in)[0];
  *out1 = __nv_bfloat162((float)reinterpret_cast<const __nv_fp8_e4m3*>(&tmp_val.x)[0],
                         (float)reinterpret_cast<const __nv_fp8_e4m3*>(&tmp_val.y)[0]);
  *out2 = __nv_bfloat162((float)reinterpret_cast<const __nv_fp8_e4m3*>(&tmp_val.z)[0],
                         (float)reinterpret_cast<const __nv_fp8_e4m3*>(&tmp_val.w)[0]);
}
extern "C"

void FP8TrtAddQKVBiasKernel(FP8TrtAddQKVBiasParam<__nv_fp8_e4m3, __nv_bfloat16> param)
{
    #pragma HLS INTERFACE s_axilite port=FP8TrtAddQKVBiasParam<__nv_fp8_e4m3
    #pragma HLS INTERFACE s_axilite port=param
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // Add bias ([3, head, size]), and then transpose from
                    // [valid_word_num, 3, head, size] -> [valid_word_num, head, 3, size]

                    using T1_4 = __nv_fp8x4_e4m3;
                    using T2_2 = __nv_bfloat162;

                    const T1_4* qkv_src_ptr = (T1_4*)(param.qkv_src + _bid_x * 3 * param.hidden_unit);
                    const T2_2* bias_ptr    = (T2_2*)param.qkv_bias;
                    T1_4*       qkv_tgt_ptr = (T1_4*)(param.qkv_tgt + _bid_x * 3 * param.hidden_unit);

                    const int size_div_4   = param.size_per_head / 4;
                    const int hidden_div_4 = param.hidden_unit / 4;
                    const int src_id       = _tid_z * hidden_div_4 + _tid_y * size_div_4 + _tid_x;

                    T2_2 val1, val2;
                    fp8x4_e4m3_to_bfloat2(&val1, &val2, &qkv_src_ptr[src_id]);
                    T2_2      input_scale_2  = float_to_bfloat2(__ldg(param.input_scale));
                    T2_2      output_scale_2 = float_to_bfloat2(__ldg(param.output_scale));
                    const int bias_id_0      = src_id * 2;
                    val1                     = hmul2(hadd2(hmul2(val1, input_scale_2), bias_ptr[bias_id_0]), output_scale_2);
                    val2                     = hmul2(hadd2(hmul2(val2, input_scale_2), bias_ptr[bias_id_0 + 1]), output_scale_2);

                    // https://docs.nvidia.com/cuda/cuda-math-api/cuda_math_api/struct____nv__fp8x4__e4m3.html
                    qkv_tgt_ptr[(_tid_y * 3 * size_div_4 + _tid_z * size_div_4) + _tid_x] = __nv_fp8x4_e4m3(val1, val2);

                }
            }
        }
    }
}
