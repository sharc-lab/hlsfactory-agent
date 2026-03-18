#include "kernel.h"

// --- from main.cu ---
extern "C"
void rotate (const int n, const float angle, const float3 w, float3 *d)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=angle
    #pragma HLS INTERFACE s_axilite port=w
    #pragma HLS INTERFACE m_axi port=d offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i >= n) return;

            float s, c;
            sincosf(angle, &s,&c);

            const float3 p = d[i];
            const float mc = 1.f - c;

            // Rodrigues' formula:
            float m1 = c+(w.x)*(w.x)*(mc);
            float m2 = (w.z)*s+(w.x)*(w.y)*(mc);
            float m3 =-(w.y)*s+(w.x)*(w.z)*(mc);

            float m4 =-(w.z)*s+(w.x)*(w.y)*(mc);
            float m5 = c+(w.y)*(w.y)*(mc);
            float m6 = (w.x)*s+(w.y)*(w.z)*(mc);

            float m7 = (w.y)*s+(w.x)*(w.z)*(mc);
            float m8 =-(w.x)*s+(w.y)*(w.z)*(mc);
            float m9 = c+(w.z)*(w.z)*(mc);

            float ox = p.x*m1 + p.y*m2 + p.z*m3;
            float oy = p.x*m4 + p.y*m5 + p.z*m6;
            float oz = p.x*m7 + p.y*m8 + p.z*m9;
            d[i] = {ox, oy, oz};

        }
    }
}
extern "C"

void rotate2 (const int n, const float angle, const float3 w, float4 *d)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=angle
    #pragma HLS INTERFACE s_axilite port=w
    #pragma HLS INTERFACE m_axi port=d offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i >= n) return;

            float s, c;
            sincosf(angle, &s,&c);

            const float4 p = d[i];
            const float mc = 1.f - c;

            // Rodrigues' formula:
            float m1 = c+(w.x)*(w.x)*(mc);
            float m2 = (w.z)*s+(w.x)*(w.y)*(mc);
            float m3 =-(w.y)*s+(w.x)*(w.z)*(mc);

            float m4 =-(w.z)*s+(w.x)*(w.y)*(mc);
            float m5 = c+(w.y)*(w.y)*(mc);
            float m6 = (w.x)*s+(w.y)*(w.z)*(mc);

            float m7 = (w.y)*s+(w.x)*(w.z)*(mc);
            float m8 =-(w.x)*s+(w.y)*(w.z)*(mc);
            float m9 = c+(w.z)*(w.z)*(mc);

            float ox = p.x*m1 + p.y*m2 + p.z*m3;
            float oy = p.x*m4 + p.y*m5 + p.z*m6;
            float oz = p.x*m7 + p.y*m8 + p.z*m9;
            d[i] = {ox, oy, oz, 0.f};

        }
    }
}
