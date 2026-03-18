#include "kernel.h"

// --- from main.cu ---
extern "C"
void Match1(const float *__restrict d_pts1, 
                       const float *__restrict d_pts2,
                             float *__restrict d_score,
                               int *__restrict d_index)
{
    #pragma HLS INTERFACE m_axi port=d_pts1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_pts2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_score offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_index offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int p1 = _tid_x + M1W*_bid_x;
                float max_score = 0.0f;
                int index = -1;

                for (int p2=0;p2<NPTS;p2++) {
                float score = 0.0f;
                for (int d=0;d<NDIM;d++)
                score += d_pts1[p1*NDIM + d]*d_pts2[p2*NDIM + d];
                if (score>max_score) {
                max_score = score;
                index = p2;
                }
                }

                d_score[p1] = max_score;
                d_index[p1] = index;

            }
        }
    }
}
extern "C"

void Match2(const float *__restrict d_pts1, 
                       const float *__restrict d_pts2,
                             float *__restrict d_score,
                               int *__restrict d_index)
{
    #pragma HLS INTERFACE m_axi port=d_pts1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_pts2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_score offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_index offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                float buffer1[M2W*NDIM];
                float buffer2[M2H*NDIM];
                float scores[M2W*M2H];
                int tx = _tid_x;
                int ty = _tid_y;
                int idx = tx + M2W*ty;
                int bp1 = M2W*_bid_x;
                if (ty<M2W)
                for (int d=tx;d<NDIM;d+=M2W)
                for (int j=ty;j<M2W;j+=M2H)
                buffer1[j*NDIM + d] = d_pts1[(bp1 + j)*NDIM + d];

                float max_score = 0.0f;
                int index = -1;
                for (int bp2=0;bp2<NPTS;bp2+=M2H) {
                for (int d=tx;d<NDIM;d+=M2W)
                buffer2[ty*NDIM + d] = d_pts2[(bp2 + ty)*NDIM + d];

                float score = 0.0f;
                for (int d=0;d<NDIM;d++)
                score += buffer1[tx*NDIM + d]*buffer2[ty*NDIM + d];
                scores[idx] = score;

                if (ty==0) {
                for (int i=0;i<M2H;i++) {
                if (scores[i*M2W + tx]>max_score) {
                max_score = scores[i*M2W + tx];
                index = bp2 + i;
                }
                }
                }
                }

                if (ty==0) {
                d_score[bp1 + tx] = max_score;
                d_index[bp1 + tx] = index;
                }

            }
        }
    }
}
extern "C"

void Match3(const float *__restrict d_pts1, 
                       const float *__restrict d_pts2,
                             float *__restrict d_score,
                               int *__restrict d_index)
{
    #pragma HLS INTERFACE m_axi port=d_pts1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_pts2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_score offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_index offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                float buffer1[M2W*(NDIM + 1)];
                float buffer2[M2H*NDIM];
                float scores[M2W*M2H];
                int tx = _tid_x;
                int ty = _tid_y;
                int idx = tx + M2W*ty;
                int bp1 = M2W*_bid_x;
                if (ty<M2W)
                for (int d=tx;d<NDIM;d+=M2W)
                for (int j=ty;j<M2W;j+=M2H)
                buffer1[j*(NDIM + 1) + d] = d_pts1[(bp1 + j)*NDIM + d];

                float max_score = 0.0f;
                int index = -1;
                for (int bp2=0;bp2<NPTS;bp2+=M2H) {
                for (int d=tx;d<NDIM;d+=M2W)
                buffer2[ty*NDIM + d] = d_pts2[(bp2 + ty)*NDIM + d];

                float score = 0.0f;
                for (int d=0;d<NDIM;d++)
                score += buffer1[tx*(NDIM + 1) + d]*buffer2[ty*NDIM + d];
                scores[idx] = score;

                if (ty==0) {
                for (int i=0;i<M2H;i++) {
                if (scores[i*M2W + tx]>max_score) {
                max_score = scores[i*M2W + tx];
                index = bp2 + i;
                }
                }
                }
                }

                if (ty==0) {
                d_score[bp1 + tx] = max_score;
                d_index[bp1 + tx] = index;
                }

            }
        }
    }
}
extern "C"

void Match4(const float *__restrict d_pts1, 
                       const float *__restrict d_pts2,
                             float *__restrict d_score,
                               int *__restrict d_index)
{
    #pragma HLS INTERFACE m_axi port=d_pts1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_pts2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_score offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_index offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                float4 buffer1[M2W*(NDIM/4 + 1)];
                float4 buffer2[M2H*NDIM/4];
                float scores[M2W*M2H];
                int tx = _tid_x;
                int ty = _tid_y;
                int idx = tx + M2W*ty;
                int bp1 = M2W*_bid_x;
                if (ty<M2W)
                for (int d=tx;d<NDIM/4;d+=M2W)
                for (int j=ty;j<M2W;j+=M2H)
                buffer1[j*(NDIM/4 + 1) + d] = ((float4*)d_pts1)[(bp1 + j)*(NDIM/4) + d];

                float max_score = 0.0f;
                int index = -1;
                for (int bp2=0;bp2<NPTS;bp2+=M2H) {
                for (int d=tx;d<NDIM/4;d+=M2W)
                buffer2[ty*NDIM/4 + d] = ((float4*)d_pts2)[(bp2 + ty)*(NDIM/4) + d];

                float score = 0.0f;
                for (int d=0;d<NDIM/4;d++) {
                float4 v1 = buffer1[tx*(NDIM/4 + 1) + d];
                float4 v2 = buffer2[ty*(NDIM/4) + d];
                score += v1.x*v2.x; score += v1.y*v2.y;
                score += v1.z*v2.z; score += v1.w*v2.w;
                }
                scores[idx] = score;

                if (ty==0) {
                for (int i=0;i<M2H;i++) {
                if (scores[i*M2W + tx]>max_score) {
                max_score = scores[i*M2W + tx];
                index = bp2 + i;
                }
                }
                }
                }

                if (ty==0) {
                d_score[bp1 + tx] = max_score;
                d_index[bp1 + tx] = index;
                }

            }
        }
    }
}
extern "C"

void Match5(const float *__restrict d_pts1, 
                       const float *__restrict d_pts2,
                             float *__restrict d_score,
                               int *__restrict d_index)
{
    #pragma HLS INTERFACE m_axi port=d_pts1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_pts2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_score offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_index offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                float4 buffer1[M5W*(NDIM/4 + 1)];
                float4 buffer2[M5H*NDIM/4];
                float scores[M5W*M5H];
                int tx = _tid_x;
                int ty = _tid_y;
                int bp1 = M5W*_bid_x;
                if (ty<M5W)
                for (int d=tx;d<NDIM/4;d+=M5W)
                for (int j=ty;j<M5W;j+=M5H)
                buffer1[j*(NDIM/4 + 1) + d] = ((float4*)d_pts1)[(bp1 + j)*(NDIM/4) + d];

                float max_score = 0.0f;
                int index = -1;
                for (int bp2=0;bp2<NPTS;bp2+=M5H) {
                for (int d=tx;d<NDIM/4;d+=M5W)
                buffer2[ty*NDIM/4 + d] = ((float4*)d_pts2)[(bp2 + ty)*(NDIM/4) + d];

                if (ty<M5H/M5R) {
                float score[M5R];
                for (int dy=0;dy<M5R;dy++)
                score[dy] = 0.0f;
                for (int d=0;d<NDIM/4;d++) {
                float4 v1 = buffer1[tx*(NDIM/4 + 1) + d];
                for (int dy=0;dy<M5R;dy++) {
                float4 v2 = buffer2[(M5R*ty + dy)*(NDIM/4) + d];
                score[dy] += v1.x*v2.x; score[dy] += v1.y*v2.y;
                score[dy] += v1.z*v2.z; score[dy] += v1.w*v2.w;
                }
                }
                for (int dy=0;dy<M5R;dy++)
                scores[tx + M5W*(M5R*ty + dy)] = score[dy];
                }

                if (ty==0) {
                for (int i=0;i<M5H;i++) {
                if (scores[i*M2W + tx]>max_score) {
                max_score = scores[i*M5W + tx];
                index = bp2 + i;
                }
                }
                }
                }

                if (ty==0) {
                d_score[bp1 + tx] = max_score;
                d_index[bp1 + tx] = index;
                }

            }
        }
    }
}
extern "C"

void Match6(const float *__restrict d_pts1, 
                       const float *__restrict d_pts2,
                             float *__restrict d_score,
                               int *__restrict d_index)
{
    #pragma HLS INTERFACE m_axi port=d_pts1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_pts2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_score offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_index offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                float4 buffer1[M5W*(NDIM/4 + 1)];
                float4 buffer2[M5H*NDIM/4];
                int tx = _tid_x;
                int ty = _tid_y;
                int bp1 = M5W*_bid_x;
                if (ty<M5W)
                for (int d=tx;d<NDIM/4;d+=M5W)
                for (int j=ty;j<M5W;j+=M5H)
                buffer1[j*(NDIM/4 + 1) + d] = ((float4*)d_pts1)[(bp1 + j)*(NDIM/4) + d];

                float max_score = 0.0f;
                int index = -1;
                for (int bp2=0;bp2<NPTS;bp2+=M5H) {
                for (int d=tx;d<NDIM/4;d+=M5W)
                buffer2[ty*NDIM/4 + d] = ((float4*)d_pts2)[(bp2 + ty)*(NDIM/4) + d];

                if (ty<M5H/M5R) {
                float score[M5R];
                for (int dy=0;dy<M5R;dy++)
                score[dy] = 0.0f;
                for (int d=0;d<NDIM/4;d++) {
                float4 v1 = buffer1[tx*(NDIM/4 + 1) + d];
                for (int dy=0;dy<M5R;dy++) {
                float4 v2 = buffer2[(M5R*ty + dy)*(NDIM/4) + d];
                score[dy] += v1.x*v2.x; score[dy] += v1.y*v2.y;
                score[dy] += v1.z*v2.z; score[dy] += v1.w*v2.w;
                }
                }
                for (int dy=0;dy<M5R;dy++) {
                if (score[dy]>max_score) {
                max_score = score[dy];
                index = bp2 + M5R*ty + dy;
                }
                }
                }
                }

                float *scores = (float*)buffer1;
                int *indices = (int*)&scores[M5W*M5H/M5R];
                if (ty<M5H/M5R) {
                scores[ty*M5W + tx] = max_score;
                indices[ty*M5W + tx] = index;
                }

                if (ty==0) {
                max_score = scores[tx];
                index = indices[tx];
                for (int y=0;y<M5H/M5R;y++)
                if (scores[y*M5W + tx]>max_score) {
                max_score = scores[y*M5W + tx];
                index = indices[y*M5W + tx];
                }
                d_score[bp1 + tx] = max_score;
                d_index[bp1 + tx] = index;
                }

            }
        }
    }
}
extern "C"

void Match7(const float *__restrict d_pts1, 
                       const float *__restrict d_pts2,
                             float *__restrict d_score,
                               int *__restrict d_index)
{
    #pragma HLS INTERFACE m_axi port=d_pts1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_pts2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_score offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_index offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                float4 buffer1[M7W*NDIM/4];
                float4 buffer2[M7H*NDIM/4];
                int tx = _tid_x;
                int ty = _tid_y;
                int bp1 = M7W*_bid_x;
                for (int d=tx;d<NDIM/4;d+=M7W)
                for (int j=ty;j<M7W;j+=M7H/M7R)
                buffer1[j*NDIM/4 + (d + j)%(NDIM/4)] = ((float4*)d_pts1)[(bp1 + j)*(NDIM/4) + d];

                float max_score = 0.0f;
                int index = -1;
                for (int bp2=0;bp2<NPTS;bp2+=M7H) {
                for (int d=tx;d<NDIM/4;d+=M7W)
                for (int j=ty;j<M7H;j+=M7H/M7R)
                buffer2[j*NDIM/4 + d] = ((float4*)d_pts2)[(bp2 + j)*(NDIM/4) + d];

                float score[M7R];
                for (int dy=0;dy<M7R;dy++)
                score[dy] = 0.0f;
                for (int d=0;d<NDIM/4;d++) {
                float4 v1 = buffer1[tx*NDIM/4 + (d + tx)%(NDIM/4)];
                for (int dy=0;dy<M7R;dy++) {
                float4 v2 = buffer2[(M7R*ty + dy)*(NDIM/4) + d];
                score[dy] += v1.x*v2.x;
                score[dy] += v1.y*v2.y;
                score[dy] += v1.z*v2.z;
                score[dy] += v1.w*v2.w;
                }
                }
                for (int dy=0;dy<M7R;dy++) {
                if (score[dy]>max_score) {
                max_score = score[dy];
                index = bp2 + M7R*ty + dy;
                }
                }
                }

                float *scores = (float*)buffer1;
                int *indices = (int*)&scores[M7W*M7H/M7R];
                scores[ty*M7W + tx] = max_score;
                indices[ty*M7W + tx] = index;

                if (ty==0) {
                max_score = scores[tx];
                index = indices[tx];
                for (int y=0;y<M7H/M7R;y++)
                if (scores[y*M7W + tx]>max_score) {
                max_score = scores[y*M7W + tx];
                index = indices[y*M7W + tx];
                }
                d_score[bp1 + tx] = max_score;
                d_index[bp1 + tx] = index;
                }

            }
        }
    }
}
extern "C"

void Match8(const float *__restrict d_pts1, 
                       const float *__restrict d_pts2,
                             float *__restrict d_score,
                               int *__restrict d_index)
{
    #pragma HLS INTERFACE m_axi port=d_pts1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_pts2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_score offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_index offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                float4 buffer1[M7W*NDIM/4];
                float4 buffer2[M7H*NDIM/4];
                int tx = _tid_x;
                int ty = _tid_y;
                int bp1 = M7W*_bid_x;
                for (int d=tx;d<NDIM/4;d+=M7W)
                for (int j=ty;j<M7W;j+=M7H/M7R)
                buffer1[j*NDIM/4 + (d + j)%(NDIM/4)] = ((float4*)d_pts1)[(bp1 + j)*(NDIM/4) + d];

                #define NRX 2
                float max_score[NRX];
                int index[NRX];
                for (int i=0;i<NRX;i++) {
                max_score[i] = 0.0f;
                index[i] = -1;
                }
                int idx = ty*M7W + tx;
                int ix = idx%(M7W/NRX);
                int iy = idx/(M7W/NRX);
                for (int bp2=0;bp2<NPTS;bp2+=M7H) {
                for (int d=tx;d<NDIM/4;d+=M7W)
                for (int j=ty;j<M7H;j+=M7H/M7R)
                buffer2[j*NDIM/4 + d] = ((float4*)d_pts2)[(bp2 + j)*(NDIM/4) + d];

                if (idx<M7W*M7H/M7R/NRX) {
                float score[M7R][NRX];
                for (int dy=0;dy<M7R;dy++)
                for (int i=0;i<NRX;i++)
                score[dy][i] = 0.0f;
                for (int d=0;d<NDIM/4;d++) {
                float4 v1[NRX];
                for (int i=0;i<NRX;i++)
                v1[i] = buffer1[((M7W/NRX)*i + ix)*NDIM/4 + (d + (M7W/NRX)*i + ix)%(NDIM/4)];
                for (int dy=0;dy<M7R;dy++) {
                float4 v2 = buffer2[(M7R*iy + dy)*(NDIM/4) + d];
                for (int i=0;i<NRX;i++) {
                score[dy][i] += v1[i].x*v2.x;
                score[dy][i] += v1[i].y*v2.y;
                score[dy][i] += v1[i].z*v2.z;
                score[dy][i] += v1[i].w*v2.w;
                }
                }
                }
                for (int dy=0;dy<M7R;dy++) {
                for (int i=0;i<NRX;i++) {
                if (score[dy][i]>max_score[i]) {
                max_score[i] = score[dy][i];
                index[i] = bp2 + M7R*iy + dy;
                }
                }
                }
                }
                }

                float *scores = (float*)buffer1;
                int *indices = (int*)&scores[M7W*M7H/M7R];
                if (idx<M7W*M7H/M7R/NRX) {
                for (int i=0;i<NRX;i++) {
                scores[iy*M7W + (M7W/NRX)*i + ix] = max_score[i];
                indices[iy*M7W + (M7W/NRX)*i + ix] = index[i];
                }
                }

                if (ty==0) {
                float max_score = scores[tx];
                int index = indices[tx];
                for (int y=0;y<M7H/M7R;y++)
                if (scores[y*M7W + tx]>max_score) {
                max_score = scores[y*M7W + tx];
                index = indices[y*M7W + tx];
                }
                d_score[bp1 + tx] = max_score;
                d_index[bp1 + tx] = index;
                }

            }
        }
    }
}
extern "C"

void Match9(const float *__restrict d_pts1, 
                       const float *__restrict d_pts2,
                             float *__restrict d_score,
                               int *__restrict d_index)
{
    #pragma HLS INTERFACE m_axi port=d_pts1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_pts2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_score offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_index offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                #define NRX 2
                float4 buffer1[M7W*NDIM/4];
                float4 buffer2[M7H*NDIM/4];
                int tx = _tid_x;
                int ty = _tid_y;
                int bp1 = M7W*_bid_x;
                for (int d=tx;d<NDIM/4;d+=M7W)
                for (int j=ty;j<M7W;j+=M7H/M7R/NRX)
                buffer1[j*NDIM/4 + (d + j)%(NDIM/4)] = ((float4*)d_pts1)[(bp1 + j)*(NDIM/4) + d];

                float max_score[NRX];
                int index[NRX];
                for (int i=0;i<NRX;i++) {
                max_score[i] = 0.0f;
                index[i] = -1;
                }
                int idx = ty*M7W + tx;
                int ix = idx%(M7W/NRX);
                int iy = idx/(M7W/NRX);
                for (int bp2=0;bp2<NPTS;bp2+=M7H) {
                for (int d=tx;d<NDIM/4;d+=M7W)
                for (int j=ty;j<M7H;j+=M7H/M7R/NRX)
                buffer2[j*NDIM/4 + d] = ((float4*)d_pts2)[(bp2 + j)*(NDIM/4) + d];

                float score[M7R][NRX];
                for (int dy=0;dy<M7R;dy++)
                for (int i=0;i<NRX;i++)
                score[dy][i] = 0.0f;
                for (int d=0;d<NDIM/4;d++) {
                float4 v1[NRX];
                for (int i=0;i<NRX;i++)
                v1[i] = buffer1[((M7W/NRX)*i + ix)*NDIM/4 + (d + (M7W/NRX)*i + ix)%(NDIM/4)];
                for (int dy=0;dy<M7R;dy++) {
                float4 v2 = buffer2[(M7R*iy + dy)*(NDIM/4) + d];
                for (int i=0;i<NRX;i++) {
                score[dy][i] += v1[i].x*v2.x;
                score[dy][i] += v1[i].y*v2.y;
                score[dy][i] += v1[i].z*v2.z;
                score[dy][i] += v1[i].w*v2.w;
                }
                }
                }
                for (int dy=0;dy<M7R;dy++) {
                for (int i=0;i<NRX;i++) {
                if (score[dy][i]>max_score[i]) {
                max_score[i] = score[dy][i];
                index[i] = bp2 + M7R*iy + dy;
                }
                }
                }
                }

                float *scores = (float*)buffer1;
                int *indices = (int*)&scores[M7W*M7H/M7R];
                if (idx<M7W*M7H/M7R/NRX) {
                for (int i=0;i<NRX;i++) {
                scores[iy*M7W + (M7W/NRX)*i + ix] = max_score[i];
                indices[iy*M7W + (M7W/NRX)*i + ix] = index[i];
                }
                }

                if (ty==0) {
                float max_score = scores[tx];
                int index = indices[tx];
                for (int y=0;y<M7H/M7R;y++)
                if (scores[y*M7W + tx]>max_score) {
                max_score = scores[y*M7W + tx];
                index = indices[y*M7W + tx];
                }
                d_score[bp1 + tx] = max_score;
                d_index[bp1 + tx] = index;
                }

            }
        }
    }
}
extern "C"

void Match10(const float *__restrict d_pts1, 
                        const float *__restrict d_pts2,
                              float *__restrict d_score,
                                int *__restrict d_index)
{
    #pragma HLS INTERFACE m_axi port=d_pts1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_pts2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_score offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_index offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=scores complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=buffer2 complete dim=1

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                #define NRX 2
                #define NUM (NRX*M7R)                       // 32*8 threads
                float4 buffer1[M7W*NDIM/4];    // 32*32
                float4 buffer2[M7H*NUM];       // 32*8
                int tx = _tid_x;
                int ty = _tid_y;
                int bp1 = M7W*_bid_x;
                for (int d=tx;d<NDIM/4;d+=M7W)
                for (int j=ty;j<M7W;j+=M7H/M7R)
                buffer1[j*NDIM/4 + (d + j)%(NDIM/4)] = ((float4*)d_pts1)[(bp1 + j)*(NDIM/4) + d];

                float max_score[NRX];
                int index[NRX];
                for (int i=0;i<NRX;i++) {
                max_score[i] = 0.0f;
                index[i] = -1;
                }
                int idx = ty*M7W + tx;
                int ix = idx%(M7W/NRX);
                int iy = idx/(M7W/NRX);
                for (int bp2=0;bp2<NPTS;bp2+=M7H) {
                float score[M7R][NRX];
                for (int dy=0;dy<M7R;dy++)
                for (int i=0;i<NRX;i++)
                score[dy][i] = 0.0f;

                int d = (idx%NUM);
                int j = (idx/NUM);
                buffer2[j*NUM + d] = ((float4*)d_pts2)[(bp2 + j)*(NDIM/4) + d];
                for (int dp=0;dp<NDIM/4;dp+=NUM) {
                float4 temp;
                if (dp<(NDIM/4-NUM))
                temp = ((float4*)d_pts2)[(bp2 + j)*(NDIM/4) + dp + d + NUM];

                if (idx<M7W*M7H/M7R/NRX) {
                for (int d=0;d<NUM;d++) {
                float4 v1[NRX];
                #pragma unroll
                for (int i=0;i<NRX;i++)
                v1[i] = buffer1[(((M7W/NRX)*i + ix)<<5) + ((dp + d + (M7W/NRX)*i + ix)&31)];
                //v1[i] = buffer1[((M7W/NRX)*i + ix)*NDIM/4 + (dp + d + (M7W/NRX)*i + ix)%(NDIM/4)];
                #pragma unroll
                for (int dy=0;dy<M7R;dy++) {
                float4 v2 = buffer2[(M7R*iy + dy)*NUM + d];
                #pragma unroll
                for (int i=0;i<NRX;i++) {
                score[dy][i] += v1[i].x*v2.x;
                score[dy][i] += v1[i].y*v2.y;
                score[dy][i] += v1[i].z*v2.z;
                score[dy][i] += v1[i].w*v2.w;
                }
                }
                }
                }

                if (dp<(NDIM/4-NUM)) {
                buffer2[j*NUM + d] = temp;
                }
                }
                for (int dy=0;dy<M7R;dy++) {
                for (int i=0;i<NRX;i++) {
                if (score[dy][i]>max_score[i]) {
                max_score[i] = score[dy][i];
                index[i] = bp2 + M7R*iy + dy;
                }
                }
                }
                }

                float *scores = (float*)buffer1;
                int *indices = (int*)&scores[M7W*M7H/M7R];
                if (idx<M7W*M7H/M7R/NRX) {
                for (int i=0;i<NRX;i++) {
                scores[iy*M7W + (M7W/NRX)*i + ix] = max_score[i];
                indices[iy*M7W + (M7W/NRX)*i + ix] = index[i];
                }
                }

                if (ty==0) {
                float max_score = scores[tx];
                int index = indices[tx];
                for (int y=0;y<M7H/M7R;y++)
                if (scores[y*M7W + tx]>max_score) {
                max_score = scores[y*M7W + tx];
                index = indices[y*M7W + tx];
                }
                d_score[bp1 + tx] = max_score;
                d_index[bp1 + tx] = index;
                }

            }
        }
    }
}
