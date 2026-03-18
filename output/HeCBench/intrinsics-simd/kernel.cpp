#include "kernel.h"

// --- from main.cu ---
extern "C"
void simd_intrinsics(const int n,
                     const unsigned int* input,
                           unsigned int* output)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _tid_x + _bid_x * BLOCK_DIM_X;
            if (i >= n) return;

            unsigned int a = input[i];
            unsigned int b = a + ((i % 2) ? 1 : -1);
            unsigned int c = a ^ b;
            unsigned int r;

            r  = __vabs2(a);
            r ^= __vabs4(a);

            r ^= __vabsdiffs2(a, b);
            r ^= __vabsdiffs4(a, b);
            r ^= __vabsdiffu2(a, b);
            r ^= __vabsdiffu4(a, b);

            r ^= __vabsss2(a);
            r ^= __vabsss4(a);

            r ^= __vadd2(a, b);
            r ^= __vadd4(a, b);

            r ^= __vaddss2(a, b);
            r ^= __vaddss4(a, b);
            r ^= __vaddus2(a, b);
            r ^= __vaddus4(a, b);

            r ^= __vavgs2(a, b);
            r ^= __vavgs4(a, b);
            r ^= __vavgu2(a, b);
            r ^= __vavgu4(a, b);

            r ^= __vcmpeq2(a, b);
            r ^= __vcmpeq4(a, b);

            r ^= __vcmpges2(a, b);
            r ^= __vcmpges4(a, b);
            r ^= __vcmpgeu2(a, b);
            r ^= __vcmpgeu4(a, b);

            r ^= __vcmpgts2(a, b);
            r ^= __vcmpgts4(a, b);
            r ^= __vcmpgtu2(a, b);
            r ^= __vcmpgtu4(a, b);

            r ^= __vcmples2(a, b);
            r ^= __vcmples4(a, b);
            r ^= __vcmpleu2(a, b);
            r ^= __vcmpleu4(a, b);

            r ^= __vcmplts2(a, b);
            r ^= __vcmplts4(a, b);
            r ^= __vcmpltu2(a, b);
            r ^= __vcmpltu4(a, b);

            r ^= __vcmpne2(a, b);
            r ^= __vcmpne4(a, b);

            r ^= __vhaddu2(a, b);
            r ^= __vhaddu4(a, b);

            r ^= __viaddmax_s16x2(a, b, c);
            r ^= __viaddmax_s16x2_relu(a, b, c);
            r ^= __viaddmax_s32(a, b, c);
            r ^= __viaddmax_s32_relu(a, b, c);
            r ^= __viaddmax_u16x2(a, b, c);
            r ^= __viaddmax_u32(a, b, c);

            r ^= __viaddmin_s16x2(a, b, c);
            r ^= __viaddmin_s16x2_relu(a, b, c);
            r ^= __viaddmin_s32(a, b, c);
            r ^= __viaddmin_s32_relu(a, b, c);
            r ^= __viaddmin_u16x2(a, b, c);
            r ^= __viaddmin_u32(a, b, c);

            bool pred   ;
            bool pred_hi;
            bool pred_lo;

            r ^= __vibmax_s16x2(a, b, &pred_hi, &pred_lo);
            r ^= __vibmax_s32(a, b, &pred);
            r ^= __vibmax_u16x2(a, b, &pred_hi, &pred_lo);
            r ^= __vibmax_u32(a, b, &pred);

            r ^= __vibmin_s16x2(a, b, &pred_hi, &pred_lo);
            r ^= __vibmin_s32(a, b, &pred);
            r ^= __vibmin_u16x2(a, b, &pred_hi, &pred_lo);
            r ^= __vibmin_u32(a, b, &pred);

            r ^= __vimax3_s16x2(a, b, c);
            r ^= __vimax3_s16x2_relu(a, b, c);
            r ^= __vimax3_s32(a, b, c);
            r ^= __vimax3_s32_relu(a, b, c);
            r ^= __vimax3_u16x2(a, b, c);
            r ^= __vimax3_u32(a, b, c);

            r ^= __vimax_s16x2_relu(a, b);
            r ^= __vimax_s32_relu(a, b);

            r ^= __vimin3_s16x2(a, b, c);
            r ^= __vimin3_s16x2_relu(a, b, c);
            r ^= __vimin3_s32(a, b, c);
            r ^= __vimin3_s32_relu(a, b, c);
            r ^= __vimin3_u16x2(a, b, c);
            r ^= __vimin3_u32(a, b, c);

            r ^= __vimin_s16x2_relu(a, b);
            r ^= __vimin_s32_relu(a, b);

            r ^= __vmaxs2(a, b);
            r ^= __vmaxs4(a, b);
            r ^= __vmaxu2(a, b);
            r ^= __vmaxu4(a, b);

            r ^= __vmins2(a, b);
            r ^= __vmins4(a, b);
            r ^= __vminu2(a, b);
            r ^= __vminu4(a, b);

            r ^= __vneg2(a);
            r ^= __vneg4(a);
            r ^= __vnegss2(a);
            r ^= __vnegss4(a);

            r ^= __vsads2(a, b);
            r ^= __vsads4(a, b);
            r ^= __vsadu2(a, b);
            r ^= __vsadu4(a, b);

            r ^= __vseteq2(a, b);
            r ^= __vseteq4(a, b);

            r ^= __vsetges2(a, b);
            r ^= __vsetges4(a, b);
            r ^= __vsetgeu2(a, b);
            r ^= __vsetgeu4(a, b);

            r ^= __vsetgts2(a, b);
            r ^= __vsetgts4(a, b);
            r ^= __vsetgtu2(a, b);
            r ^= __vsetgtu4(a, b);

            r ^= __vsetles2(a, b);
            r ^= __vsetles4(a, b);
            r ^= __vsetleu2(a, b);
            r ^= __vsetleu4(a, b);

            r ^= __vsetlts2(a, b);
            r ^= __vsetlts4(a, b);
            r ^= __vsetltu2(a, b);
            r ^= __vsetltu4(a, b);

            r ^= __vsetne2(a, b);
            r ^= __vsetne4(a, b);

            r ^= __vsub2(a, b);
            r ^= __vsub4(a, b);
            r ^= __vsubss2(a, b);
            r ^= __vsubss4(a, b);
            r ^= __vsubus2(a, b);
            r ^= __vsubus4(a, b);

            output[i] = r;

        }
    }
}
