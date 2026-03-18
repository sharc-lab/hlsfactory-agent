#include "kernel.h"

// --- from histogram_compare.cu ---
inline void DecodePixel(float4 &pixel, unsigned int (&bins)[ACTIVE_CHANNELS])
{
    float samples[4];
    samples[0] = pixel.x;
    samples[1] = pixel.y;
    samples[2] = pixel.z;
    samples[3] = pixel.w;

    #pragma unroll
    for (int CHANNEL = 0; CHANNEL < ACTIVE_CHANNELS; ++CHANNEL)
        bins[CHANNEL] = (unsigned int) (samples[CHANNEL] * (float)NUM_BINS);
}

inline void DecodePixel(uchar4 pixel, unsigned int (&bins)[ACTIVE_CHANNELS])
{
    unsigned char samples[4];
    samples[0] = pixel.x;
    samples[1] = pixel.y;
    samples[2] = pixel.z;
    samples[3] = pixel.w;

    #pragma unroll
    for (int CHANNEL = 0; CHANNEL < ACTIVE_CHANNELS; ++CHANNEL)
        bins[CHANNEL] = (unsigned int) (samples[CHANNEL]);
}

inline void DecodePixel(uchar1 pixel, unsigned int (&bins)[ACTIVE_CHANNELS])
{
    bins[0] = (unsigned int) pixel.x;
}
