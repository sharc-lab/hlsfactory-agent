#include "kernel.h"

// --- from main.cu ---
__inline__ float f(unsigned int x, unsigned int y, unsigned int z)
{
  constexpr float d(2.0f / N);
  float xf((int(x - Nd2)) * d);//[-1, 1)
  float yf((int(z - Nd2)) * d);
  float zf((int(z - Nd2)) * d);
  return 1.f - 16.f * xf * yf * zf - 4.f * (xf * xf + yf * yf + zf * zf);
}

__inline__ float zeroPoint(unsigned int x, float v0, float v1, float isoValue)
{
  return ((x * (v1 - isoValue) + (x + 1) * (isoValue - v0)) / (v1 - v0) - Nd2) * (2.0f / N);
}

__inline__ float transformToCoord(unsigned int x)
{
  return (int(x) - int(Nd2)) * (2.0f / N);
}
extern "C"

void computeMinMaxLv1(float* minMax)
{
    #pragma HLS INTERFACE m_axi port=minMax offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sminMax complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            float sminMax[64];
                            constexpr unsigned int threadNum(voxelXLv1 * voxelYLv1);
                            constexpr unsigned int warpNum(threadNum / 32);
                            unsigned int x(_bid_x * (voxelXLv1 - 1) + _tid_x);
                            unsigned int y(_bid_y * (voxelYLv1 - 1) + _tid_y);
                            unsigned int z(_bid_z * (voxelZLv1 - 1));
                            unsigned int tid(_tid_x + voxelXLv1 * _tid_y);
                            unsigned int laneid = tid % 32;
                            unsigned int blockid(_bid_x + gridXLv1 * (_bid_y + gridYLv1 * _bid_z));
                            unsigned int warpid(tid >> 5);
                            float v(f(x, y, z));
                            float minV(v), maxV(v);
                            for (int c0(1); c0 < voxelZLv1; ++c0)
                            {
                            v = f(x, y, z + c0);
                            if (v < minV)minV = v;
                            if (v > maxV)maxV = v;
                            }
                            #pragma unroll
                            for (int c0(16); c0 > 0; c0 /= 2)
                            {
                            float t0, t1;
                            t0 = 0;
                            t1 = 0;
                            if (t0 < minV)minV = t0;
                            if (t1 > maxV)maxV = t1;
                            }
                            if (laneid == 0)
                            {
                            sminMax[warpid] = minV;
                            sminMax[warpid + warpNum] = maxV;
                            }
                            if (warpid == 0)
                            {
                            minV = sminMax[laneid];
                            maxV = sminMax[laneid + warpNum];
                            #pragma unroll
                            for (int c0(warpNum / 2); c0 > 0; c0 /= 2)
                            {
                            float t0, t1;
                            t0 = 0;
                            t1 = 0;
                            if (t0 < minV)minV = t0;
                            if (t1 > maxV)maxV = t1;
                            }
                            if (laneid == 0)
                            {
                            minMax[blockid * 2] = minV;
                            minMax[blockid * 2 + 1] = maxV;
                            }
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void compactLv1(
  float isoValue, 
  const float* minMax,
  unsigned int* blockIndices,
  unsigned int* countedBlockNum)
{
    #pragma HLS INTERFACE s_axilite port=isoValue
    #pragma HLS INTERFACE m_axi port=minMax offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=blockIndices offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=countedBlockNum offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sums complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sums complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            unsigned int sums[32];
                            constexpr unsigned int warpNum(countingThreadNumLv1 / 32);
                            unsigned int tid(_tid_x);
                            unsigned int laneid = tid % 32;
                            unsigned int bIdx(_bid_x * countingThreadNumLv1 + tid);
                            unsigned int warpid(tid >> 5);
                            unsigned int test;
                            if (minMax[2 * bIdx] <= isoValue && minMax[2 * bIdx + 1] >= isoValue)test = 1;
                            else test = 0;
                            unsigned int testSum(test);
                            #pragma unroll
                            for (int c0(1); c0 < 32; c0 *= 2)
                            {
                            unsigned int tp(__shfl_up_sync(0xffffffffu, testSum, c0));
                            if (laneid >= c0)testSum += tp;
                            }
                            if (laneid == 31)sums[warpid] = testSum;
                            if (warpid == 0)
                            {
                            unsigned int warpSum = sums[laneid];
                            #pragma unroll
                            for (int c0(1); c0 < warpNum; c0 *= 2)
                            {
                            unsigned int tp(__shfl_up_sync(0xffffffffu, warpSum, c0));
                            if (laneid >= c0) warpSum += tp;
                            }
                            sums[laneid] = warpSum;
                            }
                            if (warpid != 0)testSum += sums[warpid - 1];
                            if (tid == countingThreadNumLv1 - 1 && testSum != 0)
                            sums[31] = (*countedBlockNum += testSum);
                            if (test)blockIndices[testSum + sums[31] - 1] = bIdx;

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void computeMinMaxLv2(
  const unsigned int* blockIndicesLv1,
  float* minMax)
{
    #pragma HLS INTERFACE m_axi port=blockIndicesLv1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=minMax offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            unsigned int tid(_tid_x);
                            unsigned int voxelOffset(_tid_y);
                            unsigned int blockIndex(blockIndicesLv1[_bid_x]);
                            unsigned int tp(blockIndex);
                            unsigned int x((blockIndex % gridXLv1) * (voxelXLv1 - 1) + (voxelOffset % 5) * (voxelXLv2 - 1) + (tid & 3));
                            tp /= gridXLv1;
                            unsigned int y((tp % gridYLv1) * (voxelYLv1 - 1) + (voxelOffset / 5) * (voxelYLv2 - 1) + (tid >> 2));
                            tp /= gridYLv1;
                            unsigned int z(tp * (voxelZLv1 - 1));
                            float v(f(x, y, z));
                            float minV(v), maxV(v);
                            unsigned int idx(2 * (voxelOffset + voxelNumLv2 * _bid_x));
                            for (int c0(0); c0 < blockZLv2; ++c0)
                            {
                            for (int c1(1); c1 < voxelZLv2; ++c1)
                            {
                            v = f(x, y, z + c1);
                            if (v < minV)minV = v;
                            if (v > maxV)maxV = v;
                            }
                            z += voxelZLv2 - 1;
                            #pragma unroll
                            for (int c1(8); c1 > 0; c1 /= 2)
                            {
                            float t0, t1;
                            t0 = 0;
                            t1 = 0;
                            if (t0 < minV)minV = t0;
                            if (t1 > maxV)maxV = t1;
                            }
                            if (tid == 0)
                            {
                            minMax[idx] = minV;
                            minMax[idx + 1] = maxV;
                            constexpr unsigned int offsetSize(2 * blockXLv2 * blockYLv2);
                            idx += offsetSize;
                            }
                            minV = v;
                            maxV = v;
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void compactLv2(
  float isoValue,
  const float* minMax,
  const unsigned int* blockIndicesLv1,
  unsigned int* blockIndicesLv2,
  unsigned int counterBlockNumLv1,
  unsigned int* countedBlockNumLv2)
{
    #pragma HLS INTERFACE s_axilite port=isoValue
    #pragma HLS INTERFACE m_axi port=minMax offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=blockIndicesLv1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=blockIndicesLv2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=counterBlockNumLv1
    #pragma HLS INTERFACE m_axi port=countedBlockNumLv2 offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sums complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sums complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            unsigned int sums[32];
                            constexpr unsigned int warpNum(countingThreadNumLv2 / 32);
                            unsigned int tid(_tid_x);
                            unsigned int laneid = tid % 32;
                            unsigned int warpid(tid >> 5);
                            unsigned int id0(tid + _bid_x * countingThreadNumLv2);
                            unsigned int id1(id0 / voxelNumLv2);
                            unsigned int test;
                            if (id1 < counterBlockNumLv1)
                            {
                            if (minMax[2 * id0] <= isoValue && minMax[2 * id0 + 1] >= isoValue)
                            test = 1;
                            else
                            test = 0;
                            }
                            else test = 0;
                            unsigned int testSum(test);
                            #pragma unroll
                            for (int c0(1); c0 < 32; c0 *= 2)
                            {
                            unsigned int tp(__shfl_up_sync(0xffffffffu, testSum, c0));
                            if (laneid >= c0)testSum += tp;
                            }
                            if (laneid == 31)sums[warpid] = testSum;
                            if (warpid == 0)
                            {
                            unsigned warpSum = sums[laneid];
                            #pragma unroll
                            for (int c0(1); c0 < warpNum; c0 *= 2)
                            {
                            unsigned int tp(__shfl_up_sync(0xffffffffu, warpSum, c0));
                            if (laneid >= c0)warpSum += tp;
                            }
                            sums[laneid] = warpSum;
                            }
                            if (warpid != 0)testSum += sums[warpid - 1];
                            if (tid == countingThreadNumLv2 - 1)
                            sums[31] = (*countedBlockNumLv2 += testSum);

                            if (test)
                            {
                            unsigned int bIdx1(blockIndicesLv1[id1]);
                            unsigned int bIdx2;
                            unsigned int x1, y1, z1;
                            unsigned int x2, y2, z2;
                            unsigned int tp1(bIdx1);
                            unsigned int tp2((tid + _bid_x * countingThreadNumLv2) % voxelNumLv2);
                            x1 = tp1 % gridXLv1;
                            x2 = tp2 % blockXLv2;
                            tp1 /= gridXLv1;
                            tp2 /= blockXLv2;
                            y1 = tp1 % gridYLv1;
                            y2 = tp2 % blockYLv2;
                            z1 = tp1 / gridYLv1;
                            z2 = tp2 / blockYLv2;
                            bIdx2 = x2 + blockXLv2 * (x1 + gridXLv1 * (y2 + blockYLv2 * (y1 + gridYLv1 * (z1 * blockZLv2 + z2))));
                            blockIndicesLv2[testSum + sums[31] - 1] = bIdx2;
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void generatingTriangles(
  float isoValue, 
  const unsigned int* blockIndicesLv2,
  const unsigned short * distinctEdgesTable,
  const int * triTable,
  const uchar4 * edgeIDTable,
  unsigned int* countedVerticesNum,
  unsigned int* countedTrianglesNum,
  unsigned long long* triangles,
  float* coordX,
  float* coordY,
  float* coordZ,
  float* coordZP)
{
    #pragma HLS INTERFACE s_axilite port=isoValue
    #pragma HLS INTERFACE m_axi port=blockIndicesLv2 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=distinctEdgesTable offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=triTable offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=edgeIDTable offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=countedVerticesNum offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=countedTrianglesNum offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=triangles offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=coordX offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=coordY offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=coordZ offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=coordZP offset=slave bundle=gmem10
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sums complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sums complete dim=1
    #pragma HLS ARRAY_PARTITION variable=vertexIndices complete dim=1
    #pragma HLS ARRAY_PARTITION variable=value complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sumsVertices complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sumsTriangles complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            unsigned short vertexIndices[voxelZLv2][voxelYLv2][voxelXLv2];
                            float value[voxelZLv2 + 1][voxelYLv2 + 1][voxelXLv2 + 1];
                            unsigned int sumsVertices[32];
                            unsigned int sumsTriangles[32];

                            unsigned int blockId(blockIndicesLv2[_bid_x]);
                            unsigned int tp(blockId);
                            unsigned int x((tp % gridXLv2) * (voxelXLv2 - 1) + _tid_x);
                            tp /= gridXLv2;
                            unsigned int y((tp % gridYLv2) * (voxelYLv2 - 1) + _tid_y);
                            unsigned int z((tp / gridYLv2) * (voxelZLv2 - 1) + _tid_z);
                            unsigned int eds(7);
                            float v(value[_tid_z][_tid_y][_tid_x] = f(x, y, z));
                            if (_tid_x == voxelXLv2 - 1)
                            {
                            eds &= 6;
                            value[_tid_z][_tid_y][voxelXLv2] = f(x + 1, y, z);
                            if (_tid_y == voxelYLv2 - 1)
                            value[_tid_z][voxelYLv2][voxelXLv2] = f(x + 1, y + 1, z);
                            }
                            if (_tid_y == voxelYLv2 - 1)
                            {
                            eds &= 5;
                            value[_tid_z][voxelYLv2][_tid_x] = f(x, y + 1, z);
                            if (_tid_z == voxelZLv2 - 1)
                            value[voxelZLv2][voxelYLv2][_tid_x] = f(x, y + 1, z + 1);
                            }
                            if (_tid_z == voxelZLv2 - 1)
                            {
                            eds &= 3;
                            value[voxelZLv2][_tid_y][_tid_x] = f(x, y, z + 1);
                            if (_tid_x == voxelXLv2 - 1)
                            value[voxelZLv2][_tid_y][voxelXLv2] = f(x + 1, y, z + 1);
                            }
                            eds <<= 13;
                            unsigned int cubeCase(0);
                            if (value[_tid_z][_tid_y][_tid_x] < isoValue) cubeCase |= 1;
                            if (value[_tid_z][_tid_y][_tid_x + 1] < isoValue) cubeCase |= 2;
                            if (value[_tid_z][_tid_y + 1][_tid_x + 1] < isoValue) cubeCase |= 4;
                            if (value[_tid_z][_tid_y + 1][_tid_x] < isoValue) cubeCase |= 8;
                            if (value[_tid_z + 1][_tid_y][_tid_x] < isoValue) cubeCase |= 16;
                            if (value[_tid_z + 1][_tid_y][_tid_x + 1] < isoValue) cubeCase |= 32;
                            if (value[_tid_z + 1][_tid_y + 1][_tid_x + 1] < isoValue) cubeCase |= 64;
                            if (value[_tid_z + 1][_tid_y + 1][_tid_x] < isoValue) cubeCase |= 128;

                            unsigned int distinctEdges(eds ? distinctEdgesTable[cubeCase] : 0);
                            unsigned int numTriangles(eds != 0xe000 ? 0 : distinctEdges & 7);
                            unsigned int numVertices(__builtin_popcount(distinctEdges &= eds));
                            unsigned int laneid = (_tid_x + voxelXLv2 * (_tid_y + voxelYLv2 * _tid_z)) % 32;
                            unsigned warpid((_tid_x + voxelXLv2 * (_tid_y + voxelYLv2 * _tid_z)) >> 5);
                            constexpr unsigned int threadNum(voxelXLv2 * voxelYLv2 * voxelZLv2);
                            constexpr unsigned int warpNum(threadNum / 32);
                            unsigned int sumVertices(numVertices);
                            unsigned int sumTriangles(numTriangles);

                            #pragma unroll
                            for (int c0(1); c0 < 32; c0 *= 2)
                            {
                            unsigned int tp0(__shfl_up_sync(0xffffffffu, sumVertices, c0));
                            unsigned int tp1(__shfl_up_sync(0xffffffffu, sumTriangles, c0));
                            if (laneid >= c0)
                            {
                            sumVertices += tp0;
                            sumTriangles += tp1;
                            }
                            }
                            if (laneid == 31)
                            {
                            sumsVertices[warpid] = sumVertices;
                            sumsTriangles[warpid] = sumTriangles;
                            }
                            if (warpid == 0)
                            {
                            unsigned warpSumVertices = sumsVertices[laneid];
                            unsigned warpSumTriangles = sumsTriangles[laneid];
                            #pragma unroll
                            for (int c0(1); c0 < warpNum; c0 *= 2)
                            {
                            unsigned int tp0(__shfl_up_sync(0xffffffffu, warpSumVertices, c0));
                            unsigned int tp1(__shfl_up_sync(0xffffffffu, warpSumTriangles, c0));
                            if (laneid >= c0)
                            {
                            warpSumVertices += tp0;
                            warpSumTriangles += tp1;
                            }
                            }
                            sumsVertices[laneid] = warpSumVertices;
                            sumsTriangles[laneid] = warpSumTriangles;
                            }
                            if (warpid != 0)
                            {
                            sumVertices += sumsVertices[warpid - 1];
                            sumTriangles += sumsTriangles[warpid - 1];
                            }
                            if (eds == 0)
                            {
                            sumsVertices[31] = (*countedVerticesNum += sumVertices);
                            sumsTriangles[31] = (*countedTrianglesNum += sumTriangles);
                            }

                            unsigned int interOffsetVertices(sumVertices - numVertices);
                            sumVertices = interOffsetVertices + sumsVertices[31];//exclusive offset
                            sumTriangles = sumTriangles + sumsTriangles[31] - numTriangles;//exclusive offset
                            vertexIndices[_tid_z][_tid_y][_tid_x] = interOffsetVertices | distinctEdges;

                            for (unsigned int c0(0); c0 < numTriangles; ++c0)
                            {
                            #pragma unroll
                            for (unsigned int c1(0); c1 < 3; ++c1)
                            {
                            int edgeID(triTable[16 * cubeCase + 3 * c0 + c1]);
                            uchar4 edgePos(edgeIDTable[edgeID]);
                            unsigned short vertexIndex(vertexIndices[_tid_z + edgePos.z][_tid_y + edgePos.y][_tid_x + edgePos.x]);
                            unsigned int tp(__builtin_popcount(vertexIndex >> (16 - edgePos.w)) + (vertexIndex & 0x1fff));
                            (*triangles += (unsigned long long)(sumsVertices[31] + tp));
                            }
                            }

                            // sumVertices may be too large for a GPU memory
                            float zp = 0.f, cx = 0.f, cy = 0.f, cz = 0.f;

                            if (distinctEdges & (1 << 15))
                            {
                            zp = zeroPoint(x, v, value[_tid_z][_tid_y][_tid_x + 1], isoValue);
                            cy = transformToCoord(y);
                            cz = transformToCoord(z);
                            }
                            if (distinctEdges & (1 << 14))
                            {
                            cx = transformToCoord(x);
                            zp += zeroPoint(y, v, value[_tid_z][_tid_y + 1][_tid_x], isoValue);
                            cz += transformToCoord(z);
                            }
                            if (distinctEdges & (1 << 13))
                            {
                            cx += transformToCoord(x);
                            cy += transformToCoord(y);
                            zp += zeroPoint(z, v, value[_tid_z + 1][_tid_y][_tid_x], isoValue);
                            }
                            (*coordX += cx);
                            (*coordY += cy);
                            (*coordZ += cz);
                            (*coordZP += zp);

                        }
                    }
                }
            }
        }
    }
}
