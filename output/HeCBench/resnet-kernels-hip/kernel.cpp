#include "kernel.h"

// --- from Kernel128_one.cu ---
extern "C"
void kernel_512_one_128(
  const float * A,
  const float * B,
  const float * bnBias,
  const float * bnScale,
        float * C) 
{
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bnBias offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=bnScale offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int tile = _bid_x, in_channel = _tid_x, line = _tid_y;
                        int ind = line*128 + in_channel;

                        float shared_[4096];
                        float * weights = shared_ + 512*4,
                        * output = weights + 128*64,
                        * input = shared_;
                        float * bias = output + 4*128,
                        * scale = bias + 128;

                        for (int i = 0; i < 4; i++)
                        input[ind + i*512] = A[tile*2048 + i*512 + ind];
                        bias[in_channel] = bnBias[in_channel];
                        scale[in_channel] = bnScale[in_channel];
                        output[ind] = 0.0f;

                        for (int k = 0; k < 512; k += 64) {
                        const float *B_start = B + k*128;
                        for (int i = 0; i < 16; i++)
                        weights[ind + i*512] = B_start[i*512 + ind];

                        const float *A_start = input + k;
                        for (int p = 0; p < 64; p++) {
                        output[ind] += A_start[line*512 + p] * weights[in_channel + p*128];
                        }
                        }

                        float *C_start = C + tile*512, res = scale[in_channel] * output[ind] + bias[in_channel];
                        C_start[ind] = res > 0 ? res : 0;

                    }
                }
            }
        }
    }
}
extern "C"

void kernel_128_one_512(
  const float * A,
  const float * B,
  const float * bnBias,
  const float * bnScale,
        float * C) 
{
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bnBias offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=bnScale offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int tile = _bid_x, part = _bid_y, in_channel = _tid_x, line = _tid_y;
                        int ind = line*128 + in_channel;

                        float shared_[4096];
                        float *weights = shared_ + 128*4, *output = weights + 128*64, *input = shared_;
                        float *bias = output + 4*128, *scale = bias + 128;

                        input[ind] = A[tile * 512 + ind];
                        bias[in_channel] = bnBias[part*128 + in_channel];
                        scale[in_channel] = bnScale[part*128+ in_channel];
                        output[ind] = 0.0f;

                        for (int k = 0; k < 128; k += 64) {
                        for (int i = 0; i < 16; i++)
                        weights[ind + 512*i] = B[(k + i*4 + line)*512 + part*128 + in_channel];

                        float *A_start = input + k;
                        for (int p = 0; p < 64; p++) {
                        output[ind] += A_start[line*128 + p] * weights[in_channel + p*128];
                        }
                        }

                        float *C_start = C + tile*2048 + part*128;
                        float res = scale[in_channel] * output[ind] + bias[in_channel];
                        C_start[line * 512 + in_channel] = res;

                    }
                }
            }
        }
    }
}


// --- from Kernel128_winograd.cu ---
extern "C"
void kernel_128_winograd_BtdB(
  const float * pInputs,
        float * pOutputs)
{
    #pragma HLS INTERFACE m_axi port=pInputs offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pOutputs offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int Inx = _bid_x<<2, Iny0 = _bid_y<<2, Iny1 = _tid_y, Inz = _tid_x;
                        int Iny = Iny0+Iny1, stride_r = 2048, stride_c = 128; // 2048 = 16*128
                        int c_glb_start = Inx*stride_r + Iny*stride_c + Inz, c_input = Iny1*stride_c + Inz;

                        float input[4096];

                        int tmp[6] = {0, 768, 1536, 2304, 3072, 3840}; // 768 = 6*128
                        for (int i = 0; i < 6; i++) {
                        input[c_input + tmp[i]] = pInputs[c_glb_start + i*stride_r];
                        }

                        float BTd[6];
                        switch(Iny1) {
                        case 0:
                        for (int j = 0; j < 6; j++) {
                        BTd[j] = d(input, 0, j, Inz)*4 - d(input, 2, j, Inz)*5 + d(input, 4, j, Inz);
                        }
                        break;
                        case 1:
                        for (int j = 0; j < 6; j++) {
                        BTd[j] = -d(input, 1, j, Inz)*4 - d(input, 2, j, Inz)*4 + d(input, 3, j, Inz) + d(input, 4, j, Inz);
                        }
                        break;
                        case 2:
                        for (int j = 0; j < 6; j++) {
                        BTd[j] = d(input, 1, j, Inz)*4 - d(input, 2, j, Inz)*4 - d(input, 3, j, Inz) + d(input, 4, j, Inz);
                        }
                        break;
                        case 3:
                        for (int j = 0; j < 6; j++) {
                        BTd[j] = -d(input, 1, j, Inz)*2 - d(input, 2, j, Inz) + d(input, 3, j, Inz)*2 + d(input, 4, j, Inz);
                        }
                        break;
                        case 4:
                        for (int j = 0; j < 6; j++) {
                        BTd[j] = d(input, 1, j, Inz)*2 - d(input, 2, j, Inz) - d(input, 3, j, Inz)*2 + d(input, 4, j, Inz);
                        }
                        break;
                        case 5:
                        for (int j = 0; j < 6; j++) {
                        BTd[j] = d(input, 1, j, Inz)*4 - d(input, 3, j, Inz)*5 + d(input, 5, j, Inz);
                        }
                        break;
                        }

                        int tmp_offset = Iny1*768+Inz;
                        for (int i = 0; i < 6; i++) {
                        input[tmp_offset + i*stride_c] = BTd[i];
                        }

                        float BTdB[6];
                        switch(Iny1) {
                        case 0:
                        for (int i = 0; i < 6; i++) {
                        BTdB[i] = 4*d(input, i, 0, Inz) - 5*d(input, i, 2, Inz) + d(input, i, 4, Inz);
                        }
                        break;
                        case 1:
                        for (int i = 0; i < 6; i++) {
                        BTdB[i] = -4*d(input, i, 1, Inz) - 4*d(input, i, 2, Inz) + d(input, i, 3, Inz) + d(input, i, 4, Inz);
                        }
                        break;
                        case 2:
                        for (int i = 0; i < 6; i++) {
                        BTdB[i] = 4*d(input, i, 1, Inz) - 4*d(input, i, 2, Inz) - d(input, i, 3, Inz) + d(input, i, 4, Inz);
                        }
                        break;
                        case 3:
                        for (int i = 0; i < 6; i++) {
                        BTdB[i] = -2*d(input, i, 1, Inz) - d(input, i, 2, Inz) + 2*d(input, i, 3, Inz) + d(input, i, 4, Inz);
                        }
                        break;
                        case 4:
                        for (int i = 0; i < 6; i++) {
                        BTdB[i] = 2*d(input, i, 1, Inz) - d(input, i, 2, Inz) - 2*d(input, i, 3, Inz) + d(input, i, 4, Inz);
                        }
                        break;
                        case 5:
                        for (int i = 0; i < 6; i++) {
                        BTdB[i] = 4*d(input, i, 1, Inz) - 5*d(input, i, 3, Inz) + d(input, i, 5, Inz);
                        }
                        break;
                        }

                        for (int i = 0; i < 6; i++) {
                        pOutputs[(Iny1 + i*6)*2048 + (_bid_x*4+_bid_y)*128 + Inz] = BTdB[i];
                        }

                    }
                }
            }
        }
    }
}
extern "C"

void kernel_128_winograd_AtIA(
  const float * pInputs,
  const float * pBiases,
  const float * pScales,
        float * pOutputs)
{
    #pragma HLS INTERFACE m_axi port=pInputs offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pBiases offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=pScales offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=pOutputs offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int Tilex = _bid_x, Tiley = _bid_y, Iny = _tid_y, kz = _bid_z, Inx = _tid_x;
                        int c_input = Inx*6 + Iny;

                        float bias, scale;
                        float input[4096];

                        input[c_input] = pInputs[c_input*16*128 + (Tilex*4+Tiley)*128 + kz];
                        bias = pBiases[kz];
                        scale = pScales[kz];

                        float tmp = 0;
                        switch(Inx) {
                        case 0:
                        tmp = input[Iny] + input[6+Iny] + input[12+Iny] + input[18+Iny] + input[24+Iny];
                        break;
                        case 1:
                        tmp = input[6+Iny] - input[12+Iny] + 2*input[18+Iny] - 2*input[24+Iny];
                        break;
                        case 2:
                        tmp = input[6+Iny] + input[12+Iny] + 4*input[18+Iny] + 4*input[24+Iny];
                        break;
                        case 3:
                        tmp = input[6+Iny] - input[12+Iny] + 8*input[18+Iny] - 8*input[24+Iny] + input[30+Iny];
                        break;
                        }

                        input[c_input] = tmp;

                        if (Inx > 3 || (Tilex == 3 && Inx > 1)) return;

                        int x;
                        float o;
                        switch(Iny) {
                        case 0:
                        x = Inx*6;
                        o = scale*(input[x]+input[x+1]+input[x+2]+input[x+3]+input[x+4])+ bias;
                        pOutputs[(((Tilex<<2)+1+Inx)*16 + (Tiley<<2)+1)*128 + kz] = o > 0 ? o : 0;
                        break;
                        case 1:
                        x = Inx*6;
                        o = scale*(input[x+1] - input[x+2] + 2*input[x+3] - 2*input[x+4]) + bias;
                        pOutputs[(((Tilex<<2)+1+Inx)*16 + (Tiley<<2)+2)*128 + kz] = o > 0 ? o : 0;
                        break;
                        case 2:
                        if (Tiley == 3) break;
                        x = Inx*6;
                        o = scale*(input[x+1] + input[x+2] + 4*input[x+3] + 4*input[x+4]) + bias;
                        pOutputs[(((Tilex<<2)+1+Inx)*16 + (Tiley<<2)+3)*128 + kz] = o > 0 ? o : 0;
                        break;
                        case 3:
                        if (Tiley == 3) break;
                        x = Inx*6;
                        o = scale*(input[x+1] - input[x+2] + 8*input[x+3] - 8*input[x+4] + input[x+5]) + bias;
                        pOutputs[(((Tilex<<2)+1+Inx)*16 + (Tiley<<2)+4)*128 + kz] = o > 0 ? o : 0;
                        break;
                        }

                    }
                }
            }
        }
    }
}
extern "C"

void kernel_128_OuterProduct_128(
  const float * A,
  const float * B,
        float * C)
{
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int Tile = _bid_x, Part = _bid_y, tX = _tid_x, tY = _tid_y;
                        int c_input = tY*128 + tX, c_kernel = c_input;
                        int T_offset = (Tile<<11) + (Part<<10) + c_input;
                        int B_offset = (Tile<<14) + c_kernel;

                        float input[4096];
                        float *kernel = input + 1024, *out = kernel + 8192;
                        int B_stride[32] = {0, 128, 256, 384, 512, 640, 768, 896,
                        1024, 1152, 1280, 1408, 1536, 1664, 1792, 1920,
                        2048, 2176, 2304, 2432, 2560, 2688, 2816, 2944,
                        3072, 3200, 3328, 3456, 3584, 3712, 3840, 3968};
                        out[c_input] = 0.0f;

                        input[c_input] = A[T_offset];

                        for (int k = 0; k < 4; k++) {
                        int B_start = B_offset + (k<<12); // 32*64
                        kernel[c_kernel] = B[B_start], kernel[c_kernel+1024] = B[B_start+1024];
                        kernel[c_kernel+2048] = B[B_start+2048], kernel[c_kernel+3072] = B[B_start+3072];

                        float sum = 0;
                        int y_tmp = (tY<<7)+(k<<5);
                        for (int j = 0; j < 32; j++) {
                        sum += input[y_tmp + j] * kernel[tX + B_stride[j]];
                        }
                        out[tY*128 + tX] += sum;
                        }

                        C[T_offset] = out[c_input];

                    }
                }
            }
        }
    }
}


// --- from Kernel256_one.cu ---
extern "C"
void kernel_1024_one_256(
  const float * A,
  const float * B,
  const float * bnBias,
  const float * bnScale,
        float * C) 
{
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bnBias offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=bnScale offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int tile = _bid_x, in_channel = _tid_x, line = _tid_y;
                        int ind = line*256 + in_channel;

                        float shared_[4096];
                        float * weights = shared_ + 1024*4,
                        * output = weights + 256*16,
                        * input = shared_;
                        float * bias = output + 4*256,
                        * scale = bias + 256;

                        for (int i = 0; i < 4; i++)
                        input[ind + i*1024] = A[tile*4096 + i*1024 + ind];
                        bias[in_channel] = bnBias[in_channel];
                        scale[in_channel] = bnScale[in_channel];
                        output[ind] = 0.0f;

                        for (int k = 0; k < 1024; k += 16) {
                        const float *B_start = B + k*256;
                        for (int i = 0; i < 4; i++)
                        weights[ind + i*1024] = B_start[i*1024 + ind];

                        const float *A_start = input + k;
                        for (int p = 0; p < 16; p++) {
                        output[ind] += A_start[line*1024 + p] * weights[in_channel + p*256];
                        }
                        }

                        float *C_start = C + tile*1024, res = scale[in_channel] * output[ind] + bias[in_channel];
                        C_start[ind] = res > 0 ? res : 0;

                    }
                }
            }
        }
    }
}
extern "C"

void kernel_256_one_1024(
  const float * A,
  const float * B,
  const float * bnBias,
  const float * bnScale,
        float * C) 
{
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bnBias offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=bnScale offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_ complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int tile = _bid_x, part = _bid_y, in_channel = _tid_x, line = _tid_y;
                        int ind = line*256 + in_channel;

                        float shared_[4096];
                        float *weights = shared_ + 256*4, *output = weights + 256*32, *input = shared_;
                        float *bias = output + 4*256, *scale = bias + 256;

                        input[ind] = A[tile * 1024 + ind];
                        bias[in_channel] = bnBias[part*256 + in_channel];
                        scale[in_channel] = bnScale[part*256+ in_channel];
                        output[ind] = 0.0f;

                        for (int k = 0; k < 256; k += 32) {
                        for (int i = 0; i < 8; i++)
                        weights[ind + 1024*i] = B[(k + i*4 + line)*1024 + part*256 + in_channel];

                        float *A_start = input + k;
                        for (int p = 0; p < 32; p++) {
                        output[ind] += A_start[line*256 + p] * weights[in_channel + p*256];
                        }
                        }

                        float *C_start = C + tile*4096 + part*256;
                        C_start[line * 1024 + in_channel] = scale[in_channel] * output[ind] + bias[in_channel];

                    }
                }
            }
        }
    }
}


// --- from Kernel256_winograd.cu ---
extern "C"
void kernel_256_winograd_BtdB(
  const float * pInputs,
        float * pOutputs)
{
    #pragma HLS INTERFACE m_axi port=pInputs offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pOutputs offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int Inx = _bid_x<<2, Iny0 = _bid_y<<2, Part = _bid_z,
                        Iny1 = _tid_y, Inz = _tid_x;
                        int Iny = Iny0+Iny1, stride_r = 4096, stride_c = 256; // 4096 = 16*256
                        int c_glb_start = Inx*stride_r + Iny*stride_c + Inz + (Part<<7), c_input = Iny1*128 + Inz;

                        float input[4096];

                        int stride_768[6] = {0, 768, 1536, 2304, 3072, 3840}; // 768 = 6*128
                        for (int i = 0; i < 6; i++) {
                        input[c_input + stride_768[i]] = pInputs[c_glb_start + i*stride_r];
                        }

                        float BTd[6];
                        switch(Iny1) {
                        case 0:
                        for (int j = 0; j < 6; j++) {
                        BTd[j] = d(input, 0, j, Inz)*4 - d(input, 2, j, Inz)*5 + d(input, 4, j, Inz);
                        }
                        break;
                        case 1:
                        for (int j = 0; j < 6; j++) {
                        BTd[j] = -d(input, 1, j, Inz)*4 - d(input, 2, j, Inz)*4 + d(input, 3, j, Inz) + d(input, 4, j, Inz);
                        }
                        break;
                        case 2:
                        for (int j = 0; j < 6; j++) {
                        BTd[j] = d(input, 1, j, Inz)*4 - d(input, 2, j, Inz)*4 - d(input, 3, j, Inz) + d(input, 4, j, Inz);
                        }
                        break;
                        case 3:
                        for (int j = 0; j < 6; j++) {
                        BTd[j] = -d(input, 1, j, Inz)*2 - d(input, 2, j, Inz) + d(input, 3, j, Inz)*2 + d(input, 4, j, Inz);
                        }
                        break;
                        case 4:
                        for (int j = 0; j < 6; j++) {
                        BTd[j] = d(input, 1, j, Inz)*2 - d(input, 2, j, Inz) - d(input, 3, j, Inz)*2 + d(input, 4, j, Inz);
                        }
                        break;
                        case 5:
                        for (int j = 0; j < 6; j++) {
                        BTd[j] = d(input, 1, j, Inz)*4 - d(input, 3, j, Inz)*5 + d(input, 5, j, Inz);
                        }
                        break;
                        }

                        int tmp_offset = Iny1*768+Inz;
                        for (int i = 0; i < 6; i++) {
                        input[tmp_offset + i*128] = BTd[i];
                        }

                        float BTdB[6];
                        switch(Iny1) {
                        case 0:
                        for (int i = 0; i < 6; i++) {
                        BTdB[i] = 4*d(input, i, 0, Inz) - 5*d(input, i, 2, Inz) + d(input, i, 4, Inz);
                        }
                        break;
                        case 1:
                        for (int i = 0; i < 6; i++) {
                        BTdB[i] = -4*d(input, i, 1, Inz) - 4*d(input, i, 2, Inz) + d(input, i, 3, Inz) + d(input, i, 4, Inz);
                        }
                        break;
                        case 2:
                        for (int i = 0; i < 6; i++) {
                        BTdB[i] = 4*d(input, i, 1, Inz) - 4*d(input, i, 2, Inz) - d(input, i, 3, Inz) + d(input, i, 4, Inz);
                        }
                        break;
                        case 3:
                        for (int i = 0; i < 6; i++) {
                        BTdB[i] = -2*d(input, i, 1, Inz) - d(input, i, 2, Inz) + 2*d(input, i, 3, Inz) + d(input, i, 4, Inz);
                        }
                        break;
                        case 4:
                        for (int i = 0; i < 6; i++) {
                        BTdB[i] = 2*d(input, i, 1, Inz) - d(input, i, 2, Inz) - 2*d(input, i, 3, Inz) + d(input, i, 4, Inz);
                        }
                        break;
                        case 5:
                        for (int i = 0; i < 6; i++) {
                        BTdB[i] = 4*d(input, i, 1, Inz) - 5*d(input, i, 3, Inz) + d(input, i, 5, Inz);
                        }
                        break;
                        }

                        for (int i = 0; i < 6; i++) {
                        pOutputs[(Iny1 + i*6)*4096 + (_bid_x*4+_bid_y)*256 + Inz + (Part<<7)] = BTdB[i];
                        }

                    }
                }
            }
        }
    }
}
extern "C"

void kernel_256_winograd_AtIA(
  const float * pInputs,
  const float * pBiases,
  const float * pScales,
        float * pOutputs)
{
    #pragma HLS INTERFACE m_axi port=pInputs offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pBiases offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=pScales offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=pOutputs offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int Tilex = _bid_x, Inx = _tid_x;
                        int Tiley = _bid_y, Iny = _tid_y;
                        int kz = _bid_z;
                        int c_input = Inx*6 + Iny;

                        float bias, scale;
                        float input[4096];

                        input[c_input] = pInputs[c_input*16*256 + (Tilex*4+Tiley)*256 + kz];
                        bias = pBiases[kz];
                        scale = pScales[kz];

                        float tmp = 0;
                        switch(Inx) {
                        case 0:
                        tmp = input[Iny] + input[6+Iny] + input[12+Iny] + input[18+Iny] + input[24+Iny];
                        break;
                        case 1:
                        tmp = input[6+Iny] - input[12+Iny] + 2*input[18+Iny] - 2*input[24+Iny];
                        break;
                        case 2:
                        tmp = input[6+Iny] + input[12+Iny] + 4*input[18+Iny] + 4*input[24+Iny];
                        break;
                        case 3:
                        tmp = input[6+Iny] - input[12+Iny] + 8*input[18+Iny] - 8*input[24+Iny] + input[30+Iny];
                        break;
                        }

                        input[c_input] = tmp;

                        if (Inx > 3 || (Tilex == 3 && Inx > 1)) return;

                        int x;
                        float o;
                        switch(Iny) {
                        case 0:
                        x = Inx*6;
                        o = scale*(input[x]+input[x+1]+input[x+2]+input[x+3]+input[x+4]) + bias;
                        pOutputs[(((Tilex<<2)+1+Inx)*16 + (Tiley<<2)+1)*256 + kz] = o > 0 ? o : 0;
                        break;
                        case 1:
                        x = Inx*6;
                        o = scale*(input[x+1] - input[x+2] + 2*input[x+3] - 2*input[x+4]) + bias;
                        pOutputs[(((Tilex<<2)+1+Inx)*16 + (Tiley<<2)+2)*256 + kz] = o > 0 ? o : 0;
                        break;
                        case 2:
                        if (Tiley == 3) break;
                        x = Inx*6;
                        o = scale*(input[x+1] + input[x+2] + 4*input[x+3] + 4*input[x+4]) + bias;
                        pOutputs[(((Tilex<<2)+1+Inx)*16 + (Tiley<<2)+3)*256 + kz] = o > 0 ? o : 0;
                        break;
                        case 3:
                        if (Tiley == 3) break;
                        x = Inx*6;
                        o = scale*(input[x+1] - input[x+2] + 8*input[x+3] - 8*input[x+4] + input[x+5]) + bias;
                        pOutputs[(((Tilex<<2)+1+Inx)*16 + (Tiley<<2)+4)*256 + kz] = o > 0 ? o : 0;
                        break;
                        }

                    }
                }
            }
        }
    }
}
extern "C"

void kernel_256_OuterProduct_256(
  const float * A,
  const float * B,
        float * C)
{
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1
    #pragma HLS ARRAY_PARTITION variable=input complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int Tile = _bid_x, Part = _bid_y,
                        tX = _tid_x, tY = _tid_y;
                        int c_input = tY*256 + tX,
                        c_kernel = c_input,
                        T_offset = (Tile<<12) + (Part<<11) + c_input, B_offset = (Tile<<16) + c_kernel;

                        float input[4096];
                        float *kernel = input + 2048, *out = kernel + 8192;
                        int B_stride[32] = {0, 256, 512, 768, 1024, 1280, 1536, 1792,
                        2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840,
                        4096, 4352, 4608, 4864, 5120, 5376, 5632, 5888,
                        6144, 6400, 6656, 6912, 7168, 7424, 7680, 7936};
                        out[c_input] = 0.0f;
                        out[c_input+1024] = 0;

                        input[c_input] = A[T_offset];
                        input[c_input+1024] = A[T_offset+1024];

                        for (int k = 0; k < 8; k++) {
                        int B_start = B_offset + (k<<13); // 32*64
                        kernel[c_kernel] = B[B_start], kernel[c_kernel+1024] = B[B_start+1024];
                        kernel[c_kernel+2048] = B[B_start+2048], kernel[c_kernel+3072] = B[B_start+3072];
                        kernel[c_kernel+4096] = B[B_start+4096], kernel[c_kernel+5120] = B[B_start+5120];
                        kernel[c_kernel+6144] = B[B_start+6144], kernel[c_kernel+7168] = B[B_start+7168];

                        float sum = 0, sum1 = 0;
                        int y_tmp = (tY<<8)+(k<<5), y_tmp1 = y_tmp+1024;
                        for (int j = 0; j < 32; j++) {
                        sum += input[y_tmp + j] * kernel[tX + B_stride[j]];
                        sum1 += input[y_tmp1 + j] * kernel[tX + B_stride[j]];
                        }
                        out[c_input] += sum;
                        out[c_input+1024] += sum1;
                        }

                        C[T_offset] = out[c_input];
                        C[T_offset+1024] = out[c_input+1024];

                    }
                }
            }
        }
    }
}
