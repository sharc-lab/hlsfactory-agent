#include "kernel.h"

// --- from kernels.cu ---
inline float luminance(float r, float g, float b)
{
  return ( 0.2126f * r ) + ( 0.7152f * g ) + ( 0.0722f * b );
}
extern "C"

void toneMapping(
    const float * const input, 
          float * const output, 
    const float averageLuminance, 
    const float gamma, 
    const float c, 
    const float delta,
    const uint width,
    const uint numChannels,
    const uint height)
{
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=averageLuminance
    #pragma HLS INTERFACE s_axilite port=gamma
    #pragma HLS INTERFACE s_axilite port=c
    #pragma HLS INTERFACE s_axilite port=delta
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=numChannels
    #pragma HLS INTERFACE s_axilite port=height
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    uint x = _bid_x * BLOCK_DIM_X + _tid_x;
                    uint y = _bid_y * BLOCK_DIM_Y + _tid_y;
                    float r, g, b;
                    float cLPattanaik;
                    float yLPattanaik;

                    float r1 = input[width * numChannels * y + (x * numChannels + 0)];
                    float g1 = input[width * numChannels * y + (x * numChannels + 1)];
                    float b1 = input[width * numChannels * y + (x * numChannels + 2)];

                    float yLuminance = luminance(r1, g1, b1);
                    float gcPattanaik = c * averageLuminance;

                    if (x != 0 && y != 0 && x != width-1 && y != height-1)
                    {
                    //Calculating mean
                    float leftUp = 0.0f;
                    float up = 0.0f;
                    float rightUp = 0.0f;
                    float left = 0.0f;
                    float right = 0.0f;
                    float leftDown = 0.0f;
                    float down = 0.0f;
                    float rightDown = 0.0f;

                    r = input[width * numChannels * (y - 1) + ((x - 1) * numChannels) + 0];
                    g = input[width * numChannels * (y - 1) + ((x - 1) * numChannels) + 1];
                    b = input[width * numChannels * (y - 1) + ((x - 1) * numChannels) + 2];

                    leftUp = luminance( r, g, b );

                    r = input[width * numChannels * (y - 1) + ((x) * numChannels) + 0];
                    g = input[width * numChannels * (y - 1) + ((x) * numChannels) + 1];
                    b = input[width * numChannels * (y - 1) + ((x) * numChannels) + 2];

                    up = luminance( r, g, b );

                    r = input[width * numChannels * (y - 1) + ((x + 1) * numChannels) + 0];
                    g = input[width * numChannels * (y - 1) + ((x + 1) * numChannels) + 1];
                    b = input[width * numChannels * (y - 1) + ((x + 1) * numChannels) + 2];

                    rightUp = luminance( r, g, b );

                    r = input[width * numChannels * (y) + ((x - 1) * numChannels) + 0];
                    g = input[width * numChannels * (y) + ((x - 1) * numChannels) + 1];
                    b = input[width * numChannels * (y) + ((x - 1) * numChannels) + 2];

                    left = luminance( r, g, b );

                    r = input[width * numChannels * (y) + ((x + 1) * numChannels) + 0];
                    g = input[width * numChannels * (y) + ((x + 1) * numChannels) + 1];
                    b = input[width * numChannels * (y) + ((x + 1) * numChannels) + 2];

                    right = luminance( r, g, b );

                    r = input[width * numChannels * (y + 1) + ((x - 1) * numChannels) + 0];
                    g = input[width * numChannels * (y + 1) + ((x - 1) * numChannels) + 1];
                    b = input[width * numChannels * (y + 1) + ((x - 1) * numChannels) + 2];

                    leftDown = luminance( r, g, b );

                    r = input[width * numChannels * (y + 1) + ((x) * numChannels) + 0];
                    g = input[width * numChannels * (y + 1) + ((x) * numChannels) + 1];
                    b = input[width * numChannels * (y + 1) + ((x) * numChannels) + 2];

                    down = luminance( r, g, b );

                    r = input[width * numChannels * (y + 1) + ((x + 1) * numChannels) + 0];
                    g = input[width * numChannels * (y + 1) + ((x + 1) * numChannels) + 1];
                    b = input[width * numChannels * (y + 1) + ((x + 1) * numChannels) + 2];

                    rightDown = luminance( r, g, b );

                    //Calculate median
                    yLPattanaik = (leftUp + up + rightUp + left + right + leftDown + down + rightDown) / 8;
                    }
                    else
                    {
                    yLPattanaik = yLuminance;
                    }

                    cLPattanaik =  yLPattanaik * logf(delta + yLPattanaik / yLuminance) + gcPattanaik;

                    float yDPattanaik = yLuminance / (yLuminance + cLPattanaik);

                    r = powf((r1 / yLuminance), gamma) * yDPattanaik;
                    g = powf((g1 / yLuminance), gamma) * yDPattanaik;
                    b = powf((b1 / yLuminance), gamma) * yDPattanaik;

                    output[width * numChannels * y + (x * numChannels + 0)] = r;
                    output[width * numChannels * y + (x * numChannels + 1)] = g;
                    output[width * numChannels * y + (x * numChannels + 2)] = b;
                    output[width * numChannels * y + (x * numChannels + 3)] =
                    input[width * numChannels * y + (x * numChannels + 3)];

                }
            }
        }
    }
}
