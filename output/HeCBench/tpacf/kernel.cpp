#include "kernel.h"

// --- from ACF_kernel.cu ---
extern "C"
void ACFKernelSymm(cartesian g_idata1, unsigned int* g_odata)
{
    #pragma HLS INTERFACE s_axilite port=g_idata1
    #pragma HLS INTERFACE m_axi port=g_odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sdata complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sdata complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                double3 sdata[4096];
                int tx = (_bid_x<<7) + _tid_x;
                int by = (_bid_y<<7);
                if(_bid_x < _bid_y) {    // All elements computed by block are above the main diagonal
                by <<= (LOG2_GRID_SIZE - 2);
                by += tx;
                #pragma unroll
                for(int i=0; i<128; i+=4) {
                g_odata[by+(i<<(LOG2_GRID_SIZE - 2))] = 2088533116; //  (124<<24) + (124<<16) + (124<<8) + (124);
                }
                }
                else if(_bid_x > _bid_y) {  // All elements computed by block are below the main diagonal
                double temp;
                unsigned int temp2;
                double3 vec1, vec2;

                vec1.x = g_idata1.x[tx];
                vec1.y = g_idata1.y[tx];
                vec1.z = g_idata1.z[tx];
                sdata[_tid_x].x = g_idata1.x[by+_tid_x];
                sdata[_tid_x].y = g_idata1.y[by+_tid_x];
                sdata[_tid_x].z = g_idata1.z[by+_tid_x];

                by <<= (LOG2_GRID_SIZE - 2);
                by += tx;

                #pragma unroll
                for(int i=0; i<128; i+=4) {
                temp2 = 0;
                #pragma unroll
                for(int j=0; j<4; j++) {
                vec2 = sdata[i+j];
                temp = vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z;
                if(temp < binbounds[30]) temp2 += (124<<(j<<3));
                else if(temp < binbounds[29]) temp2 += (120<<(j<<3));
                else if(temp < binbounds[28]) temp2 += (116<<(j<<3));
                else if(temp < binbounds[27]) temp2 += (112<<(j<<3));
                else if(temp < binbounds[26]) temp2 += (108<<(j<<3));
                else if(temp < binbounds[25]) temp2 += (104<<(j<<3));
                else if(temp < binbounds[24]) temp2 += (100<<(j<<3));
                else if(temp < binbounds[23]) temp2 += (96<<(j<<3));
                else if(temp < binbounds[22]) temp2 += (92<<(j<<3));
                else if(temp < binbounds[21]) temp2 += (88<<(j<<3));
                else if(temp < binbounds[20]) temp2 += (84<<(j<<3));
                else if(temp < binbounds[19]) temp2 += (80<<(j<<3));
                else if(temp < binbounds[18]) temp2 += (76<<(j<<3));
                else if(temp < binbounds[17]) temp2 += (72<<(j<<3));
                else if(temp < binbounds[16]) temp2 += (68<<(j<<3));
                else if(temp < binbounds[15]) temp2 += (64<<(j<<3));
                else if(temp < binbounds[14]) temp2 += (60<<(j<<3));
                else if(temp < binbounds[13]) temp2 += (56<<(j<<3));
                else if(temp < binbounds[12]) temp2 += (52<<(j<<3));
                else if(temp < binbounds[11]) temp2 += (48<<(j<<3));
                else if(temp < binbounds[10]) temp2 += (44<<(j<<3));
                else if(temp < binbounds[9]) temp2 += (40<<(j<<3));
                else if(temp < binbounds[8]) temp2 += (36<<(j<<3));
                else if(temp < binbounds[7]) temp2 += (32<<(j<<3));
                else if(temp < binbounds[6]) temp2 += (28<<(j<<3));
                else if(temp < binbounds[5]) temp2 += (24<<(j<<3));
                else if(temp < binbounds[4]) temp2 += (20<<(j<<3));
                else if(temp < binbounds[3]) temp2 += (16<<(j<<3));
                else if(temp < binbounds[2]) temp2 += (12<<(j<<3));
                else if(temp < binbounds[1]) temp2 += (8<<(j<<3));
                else if(temp < binbounds[0]) temp2 += (4<<(j<<3));
                else temp2 += (0<<(j<<3));
                }
                g_odata[by+(i<<(LOG2_GRID_SIZE - 2))] = temp2;
                }
                }
                else {  // _bid_x = _bid_y, so half the block will be ignorable..
                double temp;
                unsigned int temp2;
                double3 vec1, vec2;

                vec1.x = g_idata1.x[tx];
                vec1.y = g_idata1.y[tx];
                vec1.z = g_idata1.z[tx];
                sdata[_tid_x].x = g_idata1.x[by+_tid_x];
                sdata[_tid_x].y = g_idata1.y[by+_tid_x];
                sdata[_tid_x].z = g_idata1.z[by+_tid_x];

                by <<= (LOG2_GRID_SIZE - 2);
                by += tx;

                #pragma unroll
                for(int i=0; i<128; i+=4) {
                temp2 = 0;
                #pragma unroll
                for(int j=0; j<4; j++) {
                if(_tid_x <= i+j) temp2 += (124<<(j<<3));
                else {
                vec2 = sdata[i+j];
                temp = vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z;
                if(temp < binbounds[30]) temp2 += (124<<(j<<3));
                else if(temp < binbounds[29]) temp2 += (120<<(j<<3));
                else if(temp < binbounds[28]) temp2 += (116<<(j<<3));
                else if(temp < binbounds[27]) temp2 += (112<<(j<<3));
                else if(temp < binbounds[26]) temp2 += (108<<(j<<3));
                else if(temp < binbounds[25]) temp2 += (104<<(j<<3));
                else if(temp < binbounds[24]) temp2 += (100<<(j<<3));
                else if(temp < binbounds[23]) temp2 += (96<<(j<<3));
                else if(temp < binbounds[22]) temp2 += (92<<(j<<3));
                else if(temp < binbounds[21]) temp2 += (88<<(j<<3));
                else if(temp < binbounds[20]) temp2 += (84<<(j<<3));
                else if(temp < binbounds[19]) temp2 += (80<<(j<<3));
                else if(temp < binbounds[18]) temp2 += (76<<(j<<3));
                else if(temp < binbounds[17]) temp2 += (72<<(j<<3));
                else if(temp < binbounds[16]) temp2 += (68<<(j<<3));
                else if(temp < binbounds[15]) temp2 += (64<<(j<<3));
                else if(temp < binbounds[14]) temp2 += (60<<(j<<3));
                else if(temp < binbounds[13]) temp2 += (56<<(j<<3));
                else if(temp < binbounds[12]) temp2 += (52<<(j<<3));
                else if(temp < binbounds[11]) temp2 += (48<<(j<<3));
                else if(temp < binbounds[10]) temp2 += (44<<(j<<3));
                else if(temp < binbounds[9]) temp2 += (40<<(j<<3));
                else if(temp < binbounds[8]) temp2 += (36<<(j<<3));
                else if(temp < binbounds[7]) temp2 += (32<<(j<<3));
                else if(temp < binbounds[6]) temp2 += (28<<(j<<3));
                else if(temp < binbounds[5]) temp2 += (24<<(j<<3));
                else if(temp < binbounds[4]) temp2 += (20<<(j<<3));
                else if(temp < binbounds[3]) temp2 += (16<<(j<<3));
                else if(temp < binbounds[2]) temp2 += (12<<(j<<3));
                else if(temp < binbounds[1]) temp2 += (8<<(j<<3));
                else if(temp < binbounds[0]) temp2 += (4<<(j<<3));
                else temp2 += (0<<(j<<3));
                }
                }
                g_odata[by+(i<<(LOG2_GRID_SIZE - 2))] = temp2;
                }
                }

            }
        }
    }
}
extern "C"

void ACFKernel(cartesian g_idata1, cartesian g_idata2, unsigned int* g_odata) 
{
    #pragma HLS INTERFACE s_axilite port=g_idata1
    #pragma HLS INTERFACE s_axilite port=g_idata2
    #pragma HLS INTERFACE m_axi port=g_odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sdata complete dim=1
    #pragma HLS ARRAY_PARTITION variable=sdata complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                // Shared memory used to store vectors from g_idata2
                double3 sdata[4096];
                double temp;
                unsigned int temp2;
                double3 vec1, vec2;
                // tx is the "x position" in the grid
                int tx = (_bid_x<<7) + _tid_x;
                // "y position" depends on i (see below), this is just y block
                int by = (_bid_y<<7);

                // Is coalesced, as cartesians are aligned properly and there are no conflicts.
                vec1.x = g_idata2.x[tx];
                vec1.y = g_idata2.y[tx];
                vec1.z = g_idata2.z[tx];
                // Then reads one unique vector from global to shared per thread, the "shared vectors".
                // Is coalesced for the same reason.
                sdata[_tid_x].x = g_idata1.x[by+_tid_x];
                sdata[_tid_x].y = g_idata1.y[by+_tid_x];
                sdata[_tid_x].z = g_idata1.z[by+_tid_x];
                // Each thread will compute the dot product of its assigned vector with every shared vector.

                // Ensure all reads are finished before using them for any calculations

                // Simplify some notation later on.
                by <<= (LOG2_GRID_SIZE - 2);
                by += tx;

                // Unrolling offers significant speed-up
                #pragma unroll
                for(int i=0; i<128; i+=4) {   // Iterate through 128 vectors in sdata
                temp2 = 0;
                #pragma unroll
                for(int j=0; j<4; j++) {    // 4 vectors per 1 int output
                // sdata broadcasts sdata[i+j] to all threads in a block; so unnecessary bank conflicts are avoided.
                vec2 = sdata[i+j];
                temp = vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z;
                // This follows the form (binNum << (elementNum << 3)).
                // binNum is the bin we are assigning, elementNum is j, and by summing we pack four bin assignments to one int.
                if(temp < binbounds[30]) temp2 += (124<<(j<<3));
                else if(temp < binbounds[29]) temp2 += (120<<(j<<3));
                else if(temp < binbounds[28]) temp2 += (116<<(j<<3));
                else if(temp < binbounds[27]) temp2 += (112<<(j<<3));
                else if(temp < binbounds[26]) temp2 += (108<<(j<<3));
                else if(temp < binbounds[25]) temp2 += (104<<(j<<3));
                else if(temp < binbounds[24]) temp2 += (100<<(j<<3));
                else if(temp < binbounds[23]) temp2 += (96<<(j<<3));
                else if(temp < binbounds[22]) temp2 += (92<<(j<<3));
                else if(temp < binbounds[21]) temp2 += (88<<(j<<3));
                else if(temp < binbounds[20]) temp2 += (84<<(j<<3));
                else if(temp < binbounds[19]) temp2 += (80<<(j<<3));
                else if(temp < binbounds[18]) temp2 += (76<<(j<<3));
                else if(temp < binbounds[17]) temp2 += (72<<(j<<3));
                else if(temp < binbounds[16]) temp2 += (68<<(j<<3));
                else if(temp < binbounds[15]) temp2 += (64<<(j<<3));
                else if(temp < binbounds[14]) temp2 += (60<<(j<<3));
                else if(temp < binbounds[13]) temp2 += (56<<(j<<3));
                else if(temp < binbounds[12]) temp2 += (52<<(j<<3));
                else if(temp < binbounds[11]) temp2 += (48<<(j<<3));
                else if(temp < binbounds[10]) temp2 += (44<<(j<<3));
                else if(temp < binbounds[9]) temp2 += (40<<(j<<3));
                else if(temp < binbounds[8]) temp2 += (36<<(j<<3));
                else if(temp < binbounds[7]) temp2 += (32<<(j<<3));
                else if(temp < binbounds[6]) temp2 += (28<<(j<<3));
                else if(temp < binbounds[5]) temp2 += (24<<(j<<3));
                else if(temp < binbounds[4]) temp2 += (20<<(j<<3));
                else if(temp < binbounds[3]) temp2 += (16<<(j<<3));
                else if(temp < binbounds[2]) temp2 += (12<<(j<<3));
                else if(temp < binbounds[1]) temp2 += (8<<(j<<3));
                else if(temp < binbounds[0]) temp2 += (4<<(j<<3));
                else temp2 += (0<<(j<<3));
                }
                g_odata[by+(i<<(LOG2_GRID_SIZE - 2))] = temp2;
                }

            }
        }
    }
}


// --- from histogram_kernel.cu ---
extern "C"
void mergeKernel(unsigned int* g_iodata, int numBlocks);

// Init and Close are used so that the histogram function can be called many times
// without having to reallocate memory each time.
// Allocate memory
void histoInit(void) {
  cudaMallocHost((void**)&h_odata, HISTOSIZE * MAXBLOCKSEND);

}
extern "C"

void histoKernel(unsigned int* g_odata, unsigned int* g_idata, int size) {
    #pragma HLS INTERFACE m_axi port=g_odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_Hist complete dim=1

    #pragma HLS INTERFACE m_axi port=g_odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_Hist complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                // Map [31:6] bits to [31:6], [5:4] bits to [1:0], and [3:0] bits to [5:2]
                // This ensures there are no bank conflicts when accessing s_Hist below.
                // Basically, the location we write to is (threadPos + data*NUMTHREADS)/4.
                // We take this mod 16 to find the bank we write to
                // data*NUMTHREADS / 4 is congruent to 0 mod 16
                // So we write to bank (threadPos / 4) % 16, or the [5:2] bits of threadPos.
                const int threadPos = (_tid_x & (~63)) | ((_tid_x & 15) << 2) | ((_tid_x & 48) >> 4);

                // Stores all per-thread sub-histograms
                unsigned char s_Hist[MEMPERBLOCK];

                // Zero them out
                for(int pos = _tid_x; pos < (MEMPERBLOCK >> 2); pos += BLOCK_DIM_X)
                ((unsigned int *)s_Hist)[pos] = 0;

                // Location in g_idata in which this block starts reading
                const int gStart = __mul24(_bid_x, DATAPERBLOCK);
                // Amount of data to be processed by this block
                const int blockData = min(size - gStart, DATAPERBLOCK);

                unsigned int dataTemp;
                for(int pos = _tid_x; pos < blockData; pos += BLOCK_DIM_X){
                // Read in integer from global memory, increment appropriate bins in shared memory.
                dataTemp = g_idata[gStart + pos];
                s_Hist[threadPos + __mul24( (dataTemp >>  2) & 63, NUMTHREADS)]++;
                s_Hist[threadPos + __mul24( (dataTemp >> 10) & 63, NUMTHREADS)]++;
                s_Hist[threadPos + __mul24( (dataTemp >> 18) & 63, NUMTHREADS)]++;
                s_Hist[threadPos + __mul24( (dataTemp >> 26) & 63, NUMTHREADS)]++;
                }

                // Use NUMBINS threads to create a per-block sub-histogram from the data in shared memory
                if(_tid_x < NUMBINS){
                unsigned int sum = 0;
                // Each thread calculates the total number of elements in bin tid.
                const int tid = _tid_x;
                // Starting point in the histogram
                const int hStart = __mul24(tid, NUMTHREADS);
                // Another trick to ensure no bank conflicts. See nVidia whitepaper for more details.
                const int accumStart = (_tid_x & 15) * 4;

                // Iterate through thread sub-histograms' tid bins to calculate the sum.
                for(int i = 0, accum = accumStart; i < NUMTHREADS; i++){
                sum += s_Hist[hStart + accum];
                if(++accum == NUMTHREADS) accum = 0;
                }
                // Write to global memory.
                g_odata[_bid_x * NUMBINS + tid] = sum;
                }

            }
        }
    }
thread calculates the total number of elements in bin tid.
                const int tid = _tid_x;
                // Starting point in the histogram
                const int hStart = __mul24(tid, NUMTHREADS);
                // Another trick to ensure no bank conflicts. See nVidia whitepaper for more details.
                const int accumStart = (_tid_x & 15) * 4;

                // Iterate through thread sub-histograms' tid bins to calculate the sum.
                for(int i = 0, accum = accumStart; i < NUMTHREADS; i++){
                sum += s_Hist[hStart + accum];
                if(++accum == NUMTHREADS) accum = 0;
                }
                // Write to global memory.
                g_odata[_bid_x * NUMBINS + tid] = sum;
                }

            }
        }
    }
}

void mergeKernel(unsigned int* d_iodata, int numBlocks) {
    #pragma HLS INTERFACE m_axi port=g_iodata offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=numBlocks
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE m_axi port=g_iodata offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=numBlocks
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                // Total number of histogram bins.
                const int size = numBlocks * NUMBINS;
                // (Starting) Position in global memory for this thread.
                const int gPos = _bid_x * NUMBINS + _tid_x;
                // Number of threads
                const int numThreads = GRID_DIM_X * BLOCK_DIM_X;
                unsigned int sum = 0;

                // Compute bin counts for new sub-histograms
                for(int pos = gPos; pos < size; pos += numThreads)
                sum += d_iodata[pos];

                // Write to memory, overwriting the (now useless) first portion of d_iodata.
                d_iodata[gPos] = sum;

            }
        }
    }
      const int numThreads = GRID_DIM_X * BLOCK_DIM_X;
                unsigned int sum = 0;

                // Compute bin counts for new sub-histograms
                for(int pos = gPos; pos < size; pos += numThreads)
                sum += d_iodata[pos];

                // Write to memory, overwriting the (now useless) first portion of d_iodata.
                d_iodata[gPos] = sum;

            }
        }
    }
}
