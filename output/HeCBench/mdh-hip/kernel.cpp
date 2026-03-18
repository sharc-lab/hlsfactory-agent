#include "kernel.h"

// --- from main.cu ---
inline float4 sqrtf(float4 v)
{
    return float4(sqrtf(v.x), sqrtf(v.y), sqrtf(v.z), sqrtf(v.w));
}

inline float4 fast_sqrtf(float4 v)
{
    return float4(__fsqrt_rn(v.x), __fsqrt_rn(v.y), __fsqrt_rn(v.z), __fsqrt_rn(v.w));
}

inline float4 expf(float4 v)
{
    return float4(expf(v.x), expf(v.y), expf(v.z), expf(v.w));
}

inline float4 fast_expf(float4 v)
{
    return float4(__expf(v.x), __expf(v.y), __expf(v.z), __expf(v.w));
}
extern "C"

void mdh (
    const float * ax, 
    const float * ay,
    const float * az,
    const float * gx, 
    const float * gy, 
    const float * gz,
    const float * charge, 
    const float * size, 
          float * val,
    const float pre1,
    const float xkappa, 
    const int natom)
{
    #pragma HLS INTERFACE m_axi port=ax offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=ay offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=az offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=gx offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=gy offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=gz offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=charge offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=size offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=val offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=pre1
    #pragma HLS INTERFACE s_axilite port=xkappa
    #pragma HLS INTERFACE s_axilite port=natom
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            float shared[4096];

            int lid = _tid_x;
            int lsize = BLOCK_DIM_X;
            int igrid = _bid_x * lsize + lid;
            float4 v = float4(0.f, 0.f, 0.f, 0.f);
            float4 lgx = reinterpret_cast<const float4*>(gx)[igrid];
            float4 lgy = reinterpret_cast<const float4*>(gy)[igrid];
            float4 lgz = reinterpret_cast<const float4*>(gz)[igrid];

            for(int jatom = 0; jatom < natom; jatom+=lsize )
            {
            if((jatom+lsize) > natom) lsize = natom - jatom;

            if((jatom + lid) < natom) {
            shared[lid * 5    ] = ax[jatom + lid];
            shared[lid * 5 + 1] = ay[jatom + lid];
            shared[lid * 5 + 2] = az[jatom + lid];
            shared[lid * 5 + 3] = charge[jatom + lid];
            shared[lid * 5 + 4] = size[jatom + lid];
            }

            for(int i=0; i<lsize; i++) {
            float4 dx = lgx - shared[i * 5    ];
            float4 dy = lgy - shared[i * 5 + 1];
            float4 dz = lgz - shared[i * 5 + 2];
            float4 dist = sqrtf( dx * dx + dy * dy + dz * dz );
            v += pre1 * (shared[i * 5 + 3] / dist)  *
            expf( -xkappa * (dist - shared[i * 5 + 4])) /
            (1.0f + xkappa * shared[i * 5 + 4]);
            }
            }
            reinterpret_cast<float4*>(val)[ igrid ] = v;

        }
    }
}
extern "C"

void mdh2 (
    const float * ax, 
    const float * ay,
    const float * az,
    const float * gx, 
    const float * gy, 
    const float * gz,
    const float * charge, 
    const float * size, 
          float * val,
    const float pre1, 
    const float xkappa, 
    const int natom)
{
    #pragma HLS INTERFACE m_axi port=ax offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=ay offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=az offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=gx offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=gy offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=gz offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=charge offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=size offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=val offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=pre1
    #pragma HLS INTERFACE s_axilite port=xkappa
    #pragma HLS INTERFACE s_axilite port=natom
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            float shared[4096];

            int lid = _tid_x;
            int lsize = BLOCK_DIM_X;
            int igrid = _bid_x * lsize + lid;
            float4 v = float4(0.f, 0.f, 0.f, 0.f);
            float4 lgx = reinterpret_cast<const float4*>(gx)[igrid];
            float4 lgy = reinterpret_cast<const float4*>(gy)[igrid];
            float4 lgz = reinterpret_cast<const float4*>(gz)[igrid];

            for(int jatom = 0; jatom < natom; jatom+=lsize )
            {
            if((jatom+lsize) > natom) lsize = natom - jatom;

            if((jatom + lid) < natom) {
            shared[lid          ] = ax[jatom + lid];
            shared[lid +   lsize] = ay[jatom + lid];
            shared[lid + 2*lsize] = az[jatom + lid];
            shared[lid + 3*lsize] = charge[jatom + lid];
            shared[lid + 4*lsize] = size[jatom + lid];
            }

            for(int i=0; i<lsize; i++) {
            float4 dx = lgx - shared[i          ];
            float4 dy = lgy - shared[i +   lsize];
            float4 dz = lgz - shared[i + 2*lsize];
            float4 dist = sqrtf( dx * dx + dy * dy + dz * dz );
            v += pre1 * ( shared[i + 3*lsize] / dist )  *
            expf( -xkappa * (dist - shared[i + 4*lsize])) /
            (1.0f + xkappa * shared[i + 4*lsize]);
            }
            }
            reinterpret_cast<float4*>(val)[ igrid ] = v;

        }
    }
}
extern "C"

void mdh3 (
    const float * ax, 
    const float * ay,
    const float * az,
    const float * gx, 
    const float * gy, 
    const float * gz,
    const float * charge, 
    const float * size, 
          float * val,
    const float pre1, 
    const float xkappa, 
    const int natom)
{
    #pragma HLS INTERFACE m_axi port=ax offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=ay offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=az offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=gx offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=gy offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=gz offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=charge offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=size offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=val offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=pre1
    #pragma HLS INTERFACE s_axilite port=xkappa
    #pragma HLS INTERFACE s_axilite port=natom
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            float shared[4096];

            int lid = _tid_x;
            int lsize = BLOCK_DIM_X;
            int igrid = _bid_x * lsize + lid;
            float4 v = float4(0.f, 0.f, 0.f, 0.f);
            float4 lgx = reinterpret_cast<const float4*>(gx)[igrid];
            float4 lgy = reinterpret_cast<const float4*>(gy)[igrid];
            float4 lgz = reinterpret_cast<const float4*>(gz)[igrid];

            for(int jatom = 0; jatom < natom; jatom+=lsize )
            {
            if((jatom+lsize) > natom) lsize = natom - jatom;

            if((jatom + lid) < natom) {
            shared[lid          ] = ax[jatom + lid];
            shared[lid +   lsize] = ay[jatom + lid];
            shared[lid + 2*lsize] = az[jatom + lid];
            shared[lid + 3*lsize] = charge[jatom + lid];
            shared[lid + 4*lsize] = size[jatom + lid];
            }

            for(int i=0; i<lsize; i++) {
            float4 dx = lgx - shared[i          ];
            float4 dy = lgy - shared[i +   lsize];
            float4 dz = lgz - shared[i + 2*lsize];
            float4 dist = fast_sqrtf( dx * dx + dy * dy + dz * dz );
            v += pre1 * ( shared[i + 3*lsize] / dist )  *
            fast_expf( -xkappa * (dist - shared[i + 4*lsize])) /
            (1.0f + xkappa * shared[i + 4*lsize]);
            }
            }
            reinterpret_cast<float4*>(val)[ igrid ] = v;

        }
    }
}
