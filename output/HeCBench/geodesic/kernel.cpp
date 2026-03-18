#include "kernel.h"

// --- from main.cu ---
extern "C"
void kernel_distance (const float4 * d_A,
                        float * d_C,
                 const int N)
{
    #pragma HLS INTERFACE m_axi port=d_A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_C offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i >= N) return;

            float  dist, BAZ , C , C2A , CU1 , CU2 , CX , CY , CZ ,
            D , E , FAZ , SA , SU1 , SX  , SY , TU1 , TU2 , X , Y;

            const float rad_lat_1 = d_A[i].x * GDC_DEG_TO_RAD;
            const float rad_lon_1 = d_A[i].y * GDC_DEG_TO_RAD;
            const float rad_lat_2 = d_A[i].z * GDC_DEG_TO_RAD;
            const float rad_lon_2 = d_A[i].w * GDC_DEG_TO_RAD;

            TU1 = GDC_ECCENTRICITY * sinf ( rad_lat_1 ) /
            cosf ( rad_lat_1 );
            TU2 = GDC_ECCENTRICITY * sinf ( rad_lat_2 ) /
            cosf ( rad_lat_2 );

            CU1 = 1.0f / sqrtf ( TU1 * TU1 + 1.0f );
            SU1 = CU1 * TU1;
            CU2 = 1.0f / sqrtf ( TU2 * TU2 + 1.0f );
            dist = CU1 * CU2;
            BAZ = dist * TU2;
            FAZ = BAZ * TU1;
            X = rad_lon_2 - rad_lon_1;

            do {
            SX = sinf ( X );
            CX = cosf ( X );
            TU1 = CU2 * SX;
            TU2 = BAZ - SU1 * CU2 * CX;
            SY = sqrtf ( TU1 * TU1 + TU2 * TU2 );
            CY = dist * CX + FAZ;
            Y = atan2f ( SY, CY );
            SA = dist * SX / SY;
            C2A = - SA * SA + 1.0f;
            CZ = FAZ + FAZ;
            if ( C2A > 0.0f ) CZ = -CZ / C2A + CY;
            E = CZ * CZ * 2.0f - 1.0f;
            C = ( ( -3.0f * C2A + 4.0f ) * GDC_FLATTENING + 4.0f ) * C2A *
            GDC_FLATTENING / 16.0f;
            D = X;
            X = ( ( E * CY * C + CZ ) * SY * C + Y ) * SA;
            X = ( 1.0f - C ) * X * GDC_FLATTENING + rad_lon_2 - rad_lon_1;
            } while ( fabsf ( D - X ) > EPS );

            X = sqrtf ( GDC_ELLIPSOIDAL * C2A + 1.0f ) + 1.0f;
            X = ( X - 2.0f ) / X;
            C = 1.0f - X;
            C = ( X * X / 4.0f + 1.0f ) / C;
            D = ( 0.375f * X * X - 1.0f ) * X;
            X = E * CY;
            dist = 1.0f - E - E;
            dist = ( ( ( ( SY * SY * 4.0f - 3.0f ) * dist * CZ * D / 6.0f -
            X ) * D / 4.0f + CZ ) * SY * D + Y ) * C * GC_SEMI_MINOR;
            d_C[i] = dist;

        }
    }
}
