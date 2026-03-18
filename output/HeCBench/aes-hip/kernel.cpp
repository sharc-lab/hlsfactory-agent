#include "kernel.h"

// --- from kernels.cu ---
uchar galoisMultiplication(uchar a, uchar b)
{
    uchar p = 0; 
    for(unsigned int i=0; i < 8; ++i)
    {
        if((b&1) == 1)
        {
            p^=a;
        }
        uchar hiBitSet = (a & 0x80);
        a <<= 1;
        if(hiBitSet == 0x80)
        {
            a ^= 0x1b;
        }
        b >>= 1;
    }
    return p;
}

inline uchar4 sboxRead(const uchar * SBox, uchar4 block)
{
    return make_uchar4(SBox[block.x], SBox[block.y], SBox[block.z], SBox[block.w]);
}

uchar4 mixColumns(const uchar4 * block, const uchar4 * galiosCoeff, unsigned int j)
{
    unsigned int bw = 4;

    uchar x, y, z, w;

    x = galoisMultiplication(block[0].x, galiosCoeff[(bw-j)%bw].x);
    y = galoisMultiplication(block[0].y, galiosCoeff[(bw-j)%bw].x);
    z = galoisMultiplication(block[0].z, galiosCoeff[(bw-j)%bw].x);
    w = galoisMultiplication(block[0].w, galiosCoeff[(bw-j)%bw].x);
   
    for(unsigned int k=1; k< 4; ++k)
    {
        x ^= galoisMultiplication(block[k].x, galiosCoeff[(k+bw-j)%bw].x);
        y ^= galoisMultiplication(block[k].y, galiosCoeff[(k+bw-j)%bw].x);
        z ^= galoisMultiplication(block[k].z, galiosCoeff[(k+bw-j)%bw].x);
        w ^= galoisMultiplication(block[k].w, galiosCoeff[(k+bw-j)%bw].x);
    }
    
    return make_uchar4(x, y, z, w);
}

uchar4 shiftRows(uchar4 row, unsigned int j)
{
    uchar4 r = row;
    for(uint i=0; i < j; ++i)  
    {
        //r.xyzw() = r.yzwx();
        uchar x = r.x;
        uchar y = r.y;
        uchar z = r.z;
        uchar w = r.w;
        r = make_uchar4(y,z,w,x);
    }
    return r;
}
extern "C"

void AESEncrypt(      uchar4  *__restrict output  ,
                const uchar4  *__restrict input   ,
                const uchar4  *__restrict roundKey,
                const uchar   *__restrict SBox    ,
                const uint     width , 
                const uint     rounds )
                                
{
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=roundKey offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=SBox offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=rounds
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=block0 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block0 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block1 complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    uchar4 block0[4];
                    uchar4 block1[4];

                    unsigned int bx = _bid_x;
                    unsigned int by = _bid_y;

                    //unsigned int localIdx = _tid_x;
                    unsigned int localIdy = _tid_y;

                    unsigned int globalIndex = (((by * width/4) + bx) * 4) + (localIdy);
                    unsigned int localIndex  = localIdy;

                    uchar4 galiosCoeff[4];
                    galiosCoeff[0] = make_uchar4(2, 0, 0, 0);
                    galiosCoeff[1] = make_uchar4(3, 0, 0, 0);
                    galiosCoeff[2] = make_uchar4(1, 0, 0, 0);
                    galiosCoeff[3] = make_uchar4(1, 0, 0, 0);

                    block0[localIndex]  = input[globalIndex];

                    block0[localIndex] ^= roundKey[localIndex];

                    for(unsigned int r=1; r < rounds; ++r)
                    {
                    block0[localIndex] = sboxRead(SBox, block0[localIndex]);

                    block0[localIndex] = shiftRows(block0[localIndex], localIndex);
                    block1[localIndex]  = mixColumns(block0, galiosCoeff, localIndex);
                    block0[localIndex] = block1[localIndex]^roundKey[r*4 + localIndex];
                    }
                    block0[localIndex] = sboxRead(SBox, block0[localIndex]);

                    block0[localIndex] = shiftRows(block0[localIndex], localIndex);

                    output[globalIndex] =  block0[localIndex]^roundKey[(rounds)*4 + localIndex];

                }
            }
        }
    }
}

uchar4 shiftRowsInv(uchar4 row, unsigned int j)
{
    uchar4 r = row;
    for(uint i=0; i < j; ++i)  
    {
        // r = r.wxyz();
        uchar x = r.x;
        uchar y = r.y;
        uchar z = r.z;
        uchar w = r.w;
        r = make_uchar4(w,x,y,z);
    }
    return r;
}
extern "C"

void AESDecrypt(       uchar4  *__restrict output    ,
                const  uchar4  *__restrict input     ,
                const  uchar4  *__restrict roundKey  ,
                const  uchar   *__restrict SBox      ,
                const  uint    width , 
                const  uint    rounds)
                                
{
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=roundKey offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=SBox offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=rounds
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=block0 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block0 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block1 complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    uchar4 block0[4];
                    uchar4 block1[4];

                    unsigned int bx = _bid_x;
                    unsigned int by = _bid_y;

                    //unsigned int localIdx = _tid_x;
                    unsigned int localIdy = _tid_y;

                    unsigned int globalIndex = (((by * width/4) + bx) * 4) + (localIdy);
                    unsigned int localIndex  = localIdy;

                    uchar4 galiosCoeff[4];
                    galiosCoeff[0] = make_uchar4(14, 0, 0, 0);
                    galiosCoeff[1] = make_uchar4(11, 0, 0, 0);
                    galiosCoeff[2] = make_uchar4(13, 0, 0, 0);
                    galiosCoeff[3] = make_uchar4( 9, 0, 0, 0);

                    block0[localIndex]  = input[globalIndex];

                    block0[localIndex] ^= roundKey[4*rounds + localIndex];

                    for(unsigned int r=rounds -1 ; r > 0; --r)
                    {
                    block0[localIndex] = shiftRowsInv(block0[localIndex], localIndex);

                    block0[localIndex] = sboxRead(SBox, block0[localIndex]);
                    block1[localIndex] = block0[localIndex]^roundKey[r*4 + localIndex];
                    block0[localIndex]  = mixColumns(block1, galiosCoeff, localIndex);
                    }

                    block0[localIndex] = shiftRowsInv(block0[localIndex], localIndex);

                    block0[localIndex] = sboxRead(SBox, block0[localIndex]);

                    output[globalIndex] =  block0[localIndex]^roundKey[localIndex];

                }
            }
        }
    }
}


// --- from reference.cu ---
void mixColumns(uchar * state, bool inverse)
{
  uchar column[4];
  for(unsigned int i=0; i < 4; ++i)
  {
    for(unsigned int j=0; j < 4; ++j)
    {
      column[j] = state[j*4 + i];
    }

    if(inverse)
    {
      mixColumnInv(column);
    }
    else
    {
      mixColumn(column);
    }

    for(unsigned int j=0; j < 4; ++j)
    {
      state[j*4 + i] = column[j];
    }
  }
}

void shiftRows(uchar * state, bool inverse)
{
  for(unsigned int i=0; i < 4; ++i)
  {
    if(inverse)
      shiftRowInv(state + i*4, i);
    else
      shiftRow(state + i*4, i);
  }
}
