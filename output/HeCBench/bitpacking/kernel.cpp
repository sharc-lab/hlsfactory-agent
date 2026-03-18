#include "kernel.h"

// --- from kernels.cu ---
int clamp(size_t const val, int const max)
{
  return static_cast<int>(min(static_cast<size_t>(max), val));
}

void readMinAndMax(
    T const* const inMin,
    T const* const inMax,
    T* const minBuffer,
    T* const maxBuffer,
    int const blockOffset,
    int const blockEnd)
{
  static_assert(
      BLOCK_SIZE <= BLOCK_WIDTH,
      "BLOCK_SIZE must be less than or equal to BLOCK_WIDTH");

  if (_tid_x < blockEnd) {
    T localMin = inMin[blockOffset + _tid_x];
    T localMax = inMax[blockOffset + _tid_x];
    for (int i = _tid_x + BLOCK_SIZE; i < BLOCK_WIDTH && i < blockEnd;
         i += BLOCK_SIZE) {
      int const readIdx = blockOffset + i;
      localMin = min(inMin[readIdx], localMin);
      localMax = max(inMax[readIdx], localMax);
    }
    minBuffer[_tid_x] = localMin;
    maxBuffer[_tid_x] = localMax;
  }
}

void reduceMinAndMax(T* const minBuffer, T* const maxBuffer, int const blockEnd)
{
  // cooperatively compute min and max
  for (int d = BLOCK_SIZE / 2; d > 0; d >>= 1) {
    if (_tid_x < BLOCK_SIZE / 2) {
      int const idx = _tid_x;
      if (idx < d && idx + d < blockEnd) {
        minBuffer[idx] = min(minBuffer[idx], minBuffer[d + idx]);
      }
    } else {
      int const idx = _tid_x - (BLOCK_SIZE / 2);
      if (idx < d && idx + d < blockEnd) {
        maxBuffer[idx] = max(maxBuffer[idx], maxBuffer[d + idx]);
      }
    }
  }
}
extern "C"

void bitPackConfigScanKernel(
    LIMIT* const minValue,
    LIMIT* const maxValue,
    INPUT const* const in,
    const size_t* const numDevice)
{
    #pragma HLS INTERFACE m_axi port=minValue offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=maxValue offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=numDevice offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=minBuffer complete dim=1
    #pragma HLS ARRAY_PARTITION variable=maxBuffer complete dim=1
    #pragma HLS ARRAY_PARTITION variable=minBuffer complete dim=1
    #pragma HLS ARRAY_PARTITION variable=maxBuffer complete dim=1
    #pragma HLS ARRAY_PARTITION variable=inBuffer complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            static_assert(BLOCK_SIZE % 64 == 0, "BLOCK_SIZE must a multiple of 64");

            //assert(BLOCK_SIZE == BLOCK_DIM_X);

            const size_t num = *numDevice;
            const int numBlocks = roundUpDiv(num, BLOCK_SIZE);

            //assert(num > 0);
            //assert(_tid_x < BLOCK_SIZE);

            if (_bid_x < numBlocks) {
            // each block processes it's chunks, updates min/max
            LIMIT minBuffer[BLOCK_SIZE];
            LIMIT maxBuffer[BLOCK_SIZE];

            LIMIT localMin = 0;
            LIMIT localMax = 0;

            int lastThread = 0;
            for (int block = _bid_x; block < numBlocks; block += GRID_DIM_X) {

            int const blockOffset = BLOCK_SIZE * block;
            int const blockEnd = min(static_cast<int>(num) - blockOffset, BLOCK_SIZE);

            lastThread = max(lastThread, blockEnd);

            if (_tid_x < blockEnd) {
            LIMIT const val = in[blockOffset + _tid_x];
            if (block == _bid_x) {
            // first iteration just set values
            localMax = val;
            localMin = val;
            } else {
            localMin = min(val, localMin);
            localMax = max(val, localMax);
            }
            }
            }

            minBuffer[_tid_x] = localMin;
            maxBuffer[_tid_x] = localMax;

            // cooperatively compute min and max
            reduceMinAndMax(minBuffer, maxBuffer, lastThread);

            if (_tid_x == 0) {
            minValue[_bid_x] = minBuffer[0];
            maxValue[_bid_x] = maxBuffer[0];
            }
            }

        }
    }
}
extern "C"

void bitPackConfigFinalizeKernel(
    LIMIT const* const inMin,
    LIMIT const* const inMax,
    unsigned char* const numBitsPtr,
    INPUT* const outMinValPtr,
    const size_t* const numDevice)
{
    #pragma HLS INTERFACE m_axi port=inMin offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=inMax offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=numBitsPtr offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=outMinValPtr offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=numDevice offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=minBuffer complete dim=1
    #pragma HLS ARRAY_PARTITION variable=maxBuffer complete dim=1
    #pragma HLS ARRAY_PARTITION variable=minBuffer complete dim=1
    #pragma HLS ARRAY_PARTITION variable=maxBuffer complete dim=1
    #pragma HLS ARRAY_PARTITION variable=inBuffer complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            static_assert(
            BLOCK_SIZE <= BLOCK_WIDTH,
            "BLOCK_SIZE must be less than or equal to BLOCK_WIDTH");
            static_assert(
            BLOCK_WIDTH % BLOCK_SIZE == 0,
            "BLOCK_WIDTH must be a multiple of BLOCK_SIZE");
            static_assert(BLOCK_SIZE % 64 == 0, "BLOCK_SIZE must a multiple of 64");

            //assert(_bid_x == 0);

            const size_t num = min(
            roundUpDiv(*numDevice, BLOCK_SIZE), static_cast<size_t>(BLOCK_WIDTH));

            //assert(num > 0);

            // each block processes it's chunk, updates min/max, and the calculates
            // the bitwidth based on the last update
            LIMIT minBuffer[BLOCK_SIZE];
            LIMIT maxBuffer[BLOCK_SIZE];

            // load data
            readMinAndMax(inMin, inMax, minBuffer, maxBuffer, 0, num);

            // cooperatively compute min and max
            reduceMinAndMax(minBuffer, maxBuffer, min(BLOCK_SIZE, (int)num));

            if (_tid_x == 0) {
            *outMinValPtr = static_cast<INPUT>(minBuffer[0]);
            // we need to update the number of bits
            if (sizeof(LIMIT) > sizeof(int)) {
            const uint64_t range = static_cast<uint64_t>(maxBuffer[0]) - static_cast<uint64_t>(minBuffer[0]);
            // need 64 bit clz
            *numBitsPtr = sizeof(uint64_t) * 8 - __clzll(range);
            } else {
            const uint32_t range = static_cast<uint32_t>(maxBuffer[0]) - static_cast<uint32_t>(minBuffer[0]);
            // can use 32 bit clz
            *numBitsPtr = sizeof(uint32_t) * 8 - __clz(range);
            }
            }

        }
    }
}
extern "C"

void bitPackKernel(
    unsigned char const* const numBitsPtr,
    INPUT const* const valueOffsetPtr,
    OUTPUT* const outPtr,
    INPUT const* const in,
    const size_t* const numDevice)
{
    #pragma HLS INTERFACE m_axi port=numBitsPtr offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=valueOffsetPtr offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=outPtr offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=numDevice offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=inBuffer complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            using UINPUT = typename std::make_unsigned<INPUT>::type;

            const size_t num = *numDevice;

            const int numBlocks = roundUpDiv(num, BLOCK_SIZE);

            OUTPUT* const out = outPtr;
            int const numBits = *numBitsPtr;
            INPUT const valueOffset = *valueOffsetPtr;

            UINPUT inBuffer[BLOCK_SIZE];

            for (int blockId = _bid_x; blockId < numBlocks; blockId += GRID_DIM_X) {
            // The kernel works by assigning an output index to each thread.
            // The kernel then iterates over chunks of input, filling the bits
            // for each thread.
            // And then writing the stored bits to the output.
            int const outputIdx = _tid_x + blockId * BLOCK_SIZE;
            //assert(outputIdx >= 0);
            //assert(*numBitsPtr <= sizeof(INPUT) * 8U);

            size_t const bitStart = outputIdx * sizeof(*out) * 8U;
            size_t const bitEnd = bitStart + (sizeof(*out) * 8U);

            int const startIdx = clamp(bitStart / static_cast<size_t>(numBits), num);
            int const endIdx = clamp(roundUpDiv(bitEnd, numBits), num);
            //assert(startIdx >= 0);

            size_t const blockStartBit = blockId * BLOCK_SIZE * sizeof(*out) * 8U;
            size_t const blockEndBit = (blockId + 1) * BLOCK_SIZE * sizeof(*out) * 8U;
            //assert(blockStartBit < blockEndBit);

            int const blockStartIdx = clamp(
            roundDownTo(blockStartBit / static_cast<size_t>(numBits), BLOCK_SIZE),
            num);
            int const blockEndIdx
            = clamp(roundUpTo(roundUpDiv(blockEndBit, numBits), BLOCK_SIZE), num);
            //assert(blockStartIdx >= 0);
            //assert(blockStartIdx <= blockEndIdx);

            OUTPUT val = 0;
            for (int bufferStart = blockStartIdx; bufferStart < blockEndIdx;
            bufferStart += BLOCK_SIZE) {

            // fill input buffer
            int const inputIdx = bufferStart + _tid_x;
            if (inputIdx < num) {
            inBuffer[_tid_x] = in[inputIdx] - valueOffset;
            }

            int const currentStartIdx = max(startIdx, bufferStart);
            int const currentEndIdx = min(endIdx, bufferStart + BLOCK_SIZE);

            for (int idx = currentStartIdx; idx < currentEndIdx; ++idx) {
            int const localIdx = idx - bufferStart;

            // keep only bits we're interested in
            OUTPUT bits = static_cast<OUTPUT>(inBuffer[localIdx]);
            int const offset = static_cast<int>(
            static_cast<ssize_t>(idx * numBits)
            - static_cast<ssize_t>(bitStart));
            //assert(std::abs(offset) < sizeof(bits) * 8U);

            if (offset > 0) {
            bits <<= offset;
            } else {
            bits >>= -offset;
            }

            // update b
            val |= bits;
            }
            }

            if (startIdx < num) {
            out[outputIdx] = val;
            }
            }

        }
    }
}
