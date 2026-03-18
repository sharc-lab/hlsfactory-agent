#include "kernel.h"

// --- from main.cu ---
float4 rgbaUintToFloat4(const unsigned int c)
{
  float4 rgba;
  rgba.x = c & 0xff;
  rgba.y = (c >> 8) & 0xff;
  rgba.z = (c >> 16) & 0xff;
  rgba.w = (c >> 24) & 0xff;
  return rgba;
}

unsigned int rgbaFloat4ToUint(const float4 rgba, const float fScale)
{
  unsigned int uiPackedPix = 0U;
  uiPackedPix |= 0x000000FF & (unsigned int)(rgba.x * fScale);
  uiPackedPix |= 0x0000FF00 & (((unsigned int)(rgba.y * fScale)) << 8);
  uiPackedPix |= 0x00FF0000 & (((unsigned int)(rgba.z * fScale)) << 16);
  uiPackedPix |= 0xFF000000 & (((unsigned int)(rgba.w * fScale)) << 24);
  return uiPackedPix;
}
extern "C"

void row_kernel (
    const uchar4*  ucSource,
            uint*  uiDest,
    const int uiWidth,
    const int uiHeight,
    const int iRadius,
    const int iRadiusAligned,
    const float fScale,
    const unsigned int uiNumOutputPix)
{
    #pragma HLS INTERFACE m_axi port=ucSource offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=uiDest offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=uiWidth
    #pragma HLS INTERFACE s_axilite port=uiHeight
    #pragma HLS INTERFACE s_axilite port=iRadius
    #pragma HLS INTERFACE s_axilite port=iRadiusAligned
    #pragma HLS INTERFACE s_axilite port=fScale
    #pragma HLS INTERFACE s_axilite port=uiNumOutputPix
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=uc4LocalData complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                uchar4 uc4LocalData[4096];

                int lid = _tid_x;
                int gidx = _bid_x;
                int gidy = _bid_y;

                int globalPosX = gidx * uiNumOutputPix + lid - iRadiusAligned;
                int globalPosY = gidy;
                int iGlobalOffset = globalPosY * uiWidth + globalPosX;

                // Read global data into LMEM
                if (globalPosX >= 0 && globalPosX < uiWidth)
                {
                uc4LocalData[lid] = ucSource[iGlobalOffset];
                }
                else
                uc4LocalData[lid] = {0, 0, 0, 0};

                if((globalPosX >= 0) && (globalPosX < uiWidth) && (lid >= iRadiusAligned) &&
                (lid < (iRadiusAligned + (int)uiNumOutputPix)))
                {
                // Init summation registers to zero
                float4 f4Sum = {0.0f, 0.0f, 0.0f, 0.0f};

                // Do summation, using inline function to break up uint value from LMEM into independent RGBA values
                int iOffsetX = lid - iRadius;
                int iLimit = iOffsetX + (2 * iRadius) + 1;
                for(; iOffsetX < iLimit; iOffsetX++)
                {
                f4Sum.x += uc4LocalData[iOffsetX].x;
                f4Sum.y += uc4LocalData[iOffsetX].y;
                f4Sum.z += uc4LocalData[iOffsetX].z;
                f4Sum.w += uc4LocalData[iOffsetX].w;
                }

                // Use inline function to scale and convert registers to packed RGBA values in a uchar4,
                // and write back out to GMEM
                uiDest[iGlobalOffset] = rgbaFloat4ToUint(f4Sum, fScale);
                }

            }
        }
    }
}
extern "C"

void col_kernel (
    const uint*  uiSource,
          uint*  uiDest,
    const int uiWidth,
    const int uiHeight,
    const int iRadius,
    const float fScale)
{
    #pragma HLS INTERFACE m_axi port=uiSource offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=uiDest offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=uiWidth
    #pragma HLS INTERFACE s_axilite port=uiHeight
    #pragma HLS INTERFACE s_axilite port=iRadius
    #pragma HLS INTERFACE s_axilite port=fScale
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int globalPosX = _bid_x * BLOCK_DIM_X + _tid_x;
                if (globalPosX >= uiWidth) return;

                const uint* uiInputImage = &uiSource[globalPosX];
                uint* uiOutputImage = &uiDest[globalPosX];

                float4 f4Sum;

                float4 top_color = rgbaUintToFloat4(uiInputImage[0]);
                float4 bot_color = rgbaUintToFloat4(uiInputImage[(uiHeight - 1) * uiWidth]);

                f4Sum = top_color *
                float4((float)iRadius, (float)iRadius, (float)iRadius, (float)iRadius);
                for (int y = 0; y < iRadius + 1; y++)
                {
                if (y < uiHeight)
                f4Sum += rgbaUintToFloat4(uiInputImage[y * uiWidth]);
                }
                for(int y = 1; y < iRadius + 1; y++)
                {
                if (y + iRadius < uiHeight) {
                f4Sum += rgbaUintToFloat4(uiInputImage[(y + iRadius) * uiWidth]);
                f4Sum -= top_color;
                uiOutputImage[y * uiWidth] = rgbaFloat4ToUint(f4Sum, fScale);
                }
                }

                for(int y = iRadius + 1; y < uiHeight - iRadius; y++)
                {
                if (y + iRadius < uiHeight && y - iRadius >= 0) {
                f4Sum += rgbaUintToFloat4(uiInputImage[(y + iRadius) * uiWidth]);
                f4Sum -= rgbaUintToFloat4(uiInputImage[((y - iRadius) * uiWidth) - uiWidth]);
                uiOutputImage[y * uiWidth] = rgbaFloat4ToUint(f4Sum, fScale);
                }
                }

                for (int y = uiHeight - iRadius; y < uiHeight; y++)
                {
                if (y < uiHeight && y - iRadius >= 0) {
                f4Sum += bot_color;
                f4Sum -= rgbaUintToFloat4(uiInputImage[((y - iRadius) * uiWidth) - uiWidth]);
                uiOutputImage[y * uiWidth] = rgbaFloat4ToUint(f4Sum, fScale);
                }
                }

            }
        }
    }
}
