#include "kernel.h"

// --- from kernels.cu ---
extern "C"
void kernel_baseToNumber(char *reads, const long length)
{
    #pragma HLS INTERFACE m_axi port=reads offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=length
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long index = _bid_x * BLOCK_DIM_X + _tid_x;
            while (index < length) {
            switch (reads[index]) {
            case 'A':
            reads[index] = 0;
            break;
            case 'a':
            reads[index] = 0;
            break;
            case 'C':
            reads[index] = 1;
            break;
            case 'c':
            reads[index] = 1;
            break;
            case 'G':
            reads[index] = 2;
            break;
            case 'g':
            reads[index] = 2;
            break;
            case 'T':
            reads[index] = 3;
            break;
            case 't':
            reads[index] = 3;
            break;
            case 'U':
            reads[index] = 3;
            break;
            case 'u':
            reads[index] = 3;
            break;
            default:
            reads[index] = 4;
            break;
            }
            index += 128*128;
            }

        }
    }
}
extern "C"

void kernel_compressData(
    const int *lengths, 
    const long *offsets, 
    const char *reads,
    unsigned int *compressed, 
    int *gaps, 
    const int readsCount)
{
    #pragma HLS INTERFACE m_axi port=lengths offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=offsets offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=reads offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=compressed offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=gaps offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=readsCount
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index >= readsCount) return;  // out of range
            long mark = offsets[index]/16;  // compressed data offset
            int round = 0;  // write when round is 16
            int gapCount = 0;  // gap count
            unsigned int compressedTemp = 0;  // compressed data
            long start = offsets[index];
            long end = start + lengths[index];
            for (long i=start; i<end; i++) {
            unsigned char base = reads[i];  // read a base
            if (base < 4) {
            compressedTemp += base << (15-round)*2;
            round++;
            if (round == 16) {
            compressed[mark] = compressedTemp;
            compressedTemp = 0;
            round = 0;
            mark++;
            }
            } else {  // gap
            gapCount++;
            }
            }
            compressed[mark] = compressedTemp;
            gaps[index] = gapCount;

        }
    }
}
extern "C"

void kernel_createIndex4(
    const char *reads, 
    const int *lengths, 
    const long *offsets,
    unsigned short *indexs, 
    unsigned short *orders, 
    long *words, 
    int *magicBase,
    const int readsCount)
{
    #pragma HLS INTERFACE m_axi port=reads offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=lengths offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=offsets offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=indexs offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=orders offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=words offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=magicBase offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=readsCount
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index >= readsCount) return;  // out of range
            int start = offsets[index];
            int end = start + lengths[index];
            int magic0=0, magic1=0, magic2=0, magic3=0;  // magic base
            char bases[4];
            for(int i=0; i<4; i++) {  // default is gap
            bases[i] = 4;
            }
            int wordCount = 0;
            for (int i=start; i<end; i++) {
            for(int j=0; j<3; j++) {  // copy base to array
            bases[j] = bases[j+1];
            }
            bases[3] = reads[i];
            switch (bases[3]) {  // update magic
            case 0:
            magic0++;
            break;
            case 1:
            magic1++;
            break;
            case 2:
            magic2++;
            break;
            case 3:
            magic3++;
            break;
            }
            unsigned short indexValue = 0;
            int flag = 0;  // if has gap
            for (int j=0; j<4; j++) {
            indexValue += (bases[j]&3)<<(3-j)*2;
            flag += max((int)(bases[j] - 3), 0);
            }
            indexs[i] = flag?65535:indexValue;  // gap value is 65535
            wordCount += flag?0:1;
            }
            words[index] = wordCount;  // index length
            magicBase[index*4+0] = magic0;  // update magicBase
            magicBase[index*4+1] = magic1;
            magicBase[index*4+2] = magic2;
            magicBase[index*4+3] = magic3;

        }
    }
}
extern "C"

void kernel_createIndex5(
    const char *reads, 
    const int *lengths, 
    const long *offsets,
    unsigned short *indexs, 
    unsigned short *orders, 
    long *words, 
    int *magicBase,
    const int readsCount)
{
    #pragma HLS INTERFACE m_axi port=reads offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=lengths offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=offsets offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=indexs offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=orders offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=words offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=magicBase offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=readsCount
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index >= readsCount) return;
            int start = offsets[index];
            int end = start + lengths[index];
            int magic0=0, magic1=0, magic2=0, magic3=0;
            char bases[5];
            for(int i=0; i<5; i++) {
            bases[i] = 4;
            }
            int wordCount = 0;
            for (int i=start; i<end; i++) {
            for(int j=0; j<4; j++) {
            bases[j] = bases[j+1];
            }
            bases[4] = reads[i];
            switch (bases[4]) {
            case 0:
            magic0++;
            break;
            case 1:
            magic1++;
            break;
            case 2:
            magic2++;
            break;
            case 3:
            magic3++;
            break;
            }
            unsigned short indexValue = 0;
            int flag = 0;
            for (int j=0; j<5; j++) {
            indexValue += (bases[j]&3)<<(4-j)*2;
            flag += max((int)(bases[j] - 3), 0);
            }
            indexs[i] = flag?65535:indexValue;
            wordCount += flag?0:1;
            }
            words[index] = wordCount;
            magicBase[index*4+0] = magic0;
            magicBase[index*4+1] = magic1;
            magicBase[index*4+2] = magic2;
            magicBase[index*4+3] = magic3;

        }
    }
}
extern "C"

void kernel_createIndex6(
    const char *reads, 
    const int *lengths, 
    const long *offsets,
    unsigned short *indexs, 
    unsigned short *orders, 
    long *words, 
    int *magicBase,
    const int readsCount)
{
    #pragma HLS INTERFACE m_axi port=reads offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=lengths offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=offsets offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=indexs offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=orders offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=words offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=magicBase offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=readsCount
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index >= readsCount) return;
            int start = offsets[index];
            int end = start + lengths[index];
            int magic0=0, magic1=0, magic2=0, magic3=0;
            char bases[6];
            for(int i=0; i<6; i++) {
            bases[i] = 4;
            }
            int wordCount = 0;
            for (int i=start; i<end; i++) {
            for(int j=0; j<5; j++) {
            bases[j] = bases[j+1];
            }
            bases[5] = reads[i];
            switch (bases[5]) {
            case 0:
            magic0++;
            break;
            case 1:
            magic1++;
            break;
            case 2:
            magic2++;
            break;
            case 3:
            magic3++;
            break;
            }
            unsigned short indexValue = 0;
            int flag = 0;
            for (int j=0; j<6; j++) {
            indexValue += (bases[j]&3)<<(5-j)*2;
            flag += max((int)(bases[j] - 3), 0);
            }
            indexs[i] = flag?65535:indexValue;
            wordCount += flag?0:1;
            }
            words[index] = wordCount;
            magicBase[index*4+0] = magic0;
            magicBase[index*4+1] = magic1;
            magicBase[index*4+2] = magic2;
            magicBase[index*4+3] = magic3;

        }
    }
}
extern "C"

void kernel_createIndex7(
    const char *reads, 
    const int *lengths, 
    const long *offsets,
    unsigned short *indexs, 
    unsigned short *orders, 
    long *words, 
    int *magicBase,
    const int readsCount)
{
    #pragma HLS INTERFACE m_axi port=reads offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=lengths offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=offsets offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=indexs offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=orders offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=words offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=magicBase offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=readsCount
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index >= readsCount) return;
            int start = offsets[index];
            int end = start + lengths[index];
            int magic0=0, magic1=0, magic2=0, magic3=0;
            char bases[7];
            for(int i=0; i<7; i++) {
            bases[i] = 4;
            }
            int wordCount = 0;
            for (int i=start; i<end; i++) {
            for(int j=0; j<6; j++) {
            bases[j] = bases[j+1];
            }
            bases[6] = reads[i];
            switch (bases[6]) {
            case 0:
            magic0++;
            break;
            case 1:
            magic1++;
            break;
            case 2:
            magic2++;
            break;
            case 3:
            magic3++;
            break;
            }
            unsigned short indexValue = 0;
            int flag = 0;
            for (int j=0; j<7; j++) {
            indexValue += (bases[j]&3)<<(6-j)*2;
            flag += max((int)(bases[j] - 3), 0);
            }
            indexs[i] = flag?65535:indexValue;
            wordCount += flag?0:1;
            }
            words[index] = wordCount;
            magicBase[index*4+0] = magic0;
            magicBase[index*4+1] = magic1;
            magicBase[index*4+2] = magic2;
            magicBase[index*4+3] = magic3;

        }
    }
}
extern "C"

void kernel_createCutoff(
    float threshold, 
    int wordLength,
    const int *lengths, 
    long *words, 
    int *wordCutoff, 
    const int readsCount)
{
    #pragma HLS INTERFACE s_axilite port=threshold
    #pragma HLS INTERFACE s_axilite port=wordLength
    #pragma HLS INTERFACE m_axi port=lengths offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=words offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=wordCutoff offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=readsCount
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index >= readsCount) return;  // out of range
            int length = lengths[index];
            int required = length - wordLength + 1;
            int cutoff = ceil((float)length * (1.f - threshold) * (float)wordLength);
            required -= cutoff;
            wordCutoff[index] = required;

        }
    }
}
extern "C"

void kernel_mergeIndex(
    const long *offsets, 
    const unsigned short *indexs,
    unsigned short *orders, 
    const long *words, 
    const int readsCount)
{
    #pragma HLS INTERFACE m_axi port=offsets offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=indexs offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=orders offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=words offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=readsCount
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index >= readsCount) return;  // out of range
            int start = offsets[index];
            int end = start + words[index];
            unsigned short basePrevious = indexs[start];
            unsigned short baseNow;
            int count = 1;
            for (int i=start+1; i<end; i++) {  // merge same index orders is count
            baseNow = indexs[i];
            if (baseNow == basePrevious) {
            count++;
            orders[i-1] = 0;
            } else {
            basePrevious = baseNow;
            orders[i-1] = count;
            count = 1;
            }
            }
            orders[end-1] = count;

        }
    }
}
extern "C"

void kernel_updateRepresentative(
    int *cluster, 
    int *representative, 
    const int readsCount) 
{
    #pragma HLS INTERFACE m_axi port=cluster offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=representative offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=readsCount
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int r = *representative;
            r++;
            while (r < readsCount) {
            if (cluster[r] < 0) {  // is representative
            cluster[r] = r;
            break;
            }
            r++;
            }
            *representative = r;

        }
    }
}
extern "C"

void kernel_makeTable(
    const long *offsets,
    const unsigned short *indexs,
    const unsigned short *orders,
    const long *words,
    unsigned short *table,
    int representative)
{
    #pragma HLS INTERFACE m_axi port=offsets offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=indexs offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=orders offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=words offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=table offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=representative
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            int start = offsets[representative];
            int end = start + words[representative];
            for (int i=index+start; i<end; i+=128*128) {
            unsigned short order = orders[i];
            if (order == 0) continue;
            table[indexs[i]] = order;
            }

        }
    }
}
extern "C"

void kernel_cleanTable(
    const long *offsets, 
    const unsigned short *indexs,
    const unsigned short *orders,  
    const long *words,
    unsigned short *table,
    const int representative)
{
    #pragma HLS INTERFACE m_axi port=offsets offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=indexs offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=orders offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=words offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=table offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=representative
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            int start = offsets[representative];
            int end = start + words[representative];
            for (int i=index+start; i<end; i+=128*128) {
            if (orders[i] == 0) continue;
            table[indexs[i]] = 0;
            }

        }
    }
}
extern "C"

void kernel_magic(float threshold, 
    const int *lengths, 
    const int *magicBase,
    int *cluster, 
    const int representative, 
    const int readsCount)
{
    #pragma HLS INTERFACE s_axilite port=threshold
    #pragma HLS INTERFACE m_axi port=lengths offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=magicBase offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=cluster offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=representative
    #pragma HLS INTERFACE s_axilite port=readsCount
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index >= readsCount) return;  // out of range
            if (cluster[index] >= 0) return;  // is clustered
            int offsetOne = representative*4;  // representative magic offset
            int offsetTwo = index*4;  // query magic offset
            int magic = min(magicBase[offsetOne + 0], magicBase[offsetTwo + 0]) +
            min(magicBase[offsetOne + 1], magicBase[offsetTwo + 1]) +
            min(magicBase[offsetOne + 2], magicBase[offsetTwo + 2]) +
            min(magicBase[offsetOne + 3], magicBase[offsetTwo + 3]);
            int length = lengths[index];
            int minLength = ceil((float)length*threshold);
            if (magic > minLength) {  // pass filter
            cluster[index] = -2;
            }

        }
    }
}
extern "C"

void kernel_filter(
    const float threshold, 
    const int wordLength, 
    const int *lengths,
    const long *offsets, 
    const unsigned short *indexs, 
    const unsigned short *orders, 
    const long *words,
    const int *wordCutoff, 
    int *cluster, 
    const unsigned short *table, 
    const int readsCount)
{
    #pragma HLS INTERFACE s_axilite port=threshold
    #pragma HLS INTERFACE s_axilite port=wordLength
    #pragma HLS INTERFACE m_axi port=lengths offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=offsets offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=indexs offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=orders offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=words offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=wordCutoff offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=cluster offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=table offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=readsCount
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=result complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int result[128];

            int gid = _bid_x;
            int lid = _tid_x;

            if (gid >= readsCount) return;  // out of range
            if (cluster[gid] != -2) return; // out of filter
            result[lid] = 0;             // result in thread
            int start = offsets[gid];
            int end = start + words[gid];
            for (int i = lid + start; i < end; i += 128) {
            result[lid] += min(table[indexs[i]], orders[i]);
            }

            if (lid == 0) {
            for (int i=1; i<128; i++) {
            result[0] += result[i];
            }
            if (result[0] > wordCutoff[gid]) { // pass filter
            cluster[gid] = -3;
            } else {
            cluster[gid] = -1; // not pass filter
            }
            }

        }
    }
}
extern "C"

void kernel_align(
    const float threshold,
    const int *lengths,
    const long *offsets, 
    const unsigned int *compressed,
    const int *gaps,
    const int representative,
    int *cluster,
    const int readsCount)
{
    #pragma HLS INTERFACE s_axilite port=threshold
    #pragma HLS INTERFACE m_axi port=lengths offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=offsets offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=compressed offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=gaps offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=representative
    #pragma HLS INTERFACE m_axi port=cluster offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=readsCount
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            // variables
            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index >= readsCount) return;  // out of range
            if (cluster[index] != -3) return;  // not pass filter
            int target = representative;  // representative read
            int query = index;  // query read
            int minLength = ceil((float)lengths[index] * threshold);
            int targetLength = lengths[target] - gaps[target];  // representative base count
            int queryLength = lengths[query] - gaps[query];  // query base count
            int target32Length = targetLength/16+1;  // compressed target length
            int query32Length  = queryLength/16+1;  // compressed query length
            int targetOffset = offsets[target]/16;  // representative offset
            int queryOffset = offsets[query]/16;  // query offset
            short rowNow[3000] = {0};  // dynamic matrix row
            short rowPrevious[3000] = {0};  // dynamic matrix row
            int columnPrevious[17] = {0};  // dynamic matrix column
            int columnNow[17] = {0};  // dynamic matrix column
            int shift = ceil((float)targetLength - (float)queryLength*threshold);
            shift = ceil((float)shift / 16.f); // shift form diagonal
            // compute
            for (int i = 0; i < query32Length; i++) {  // query is column
            // first big loop
            for (int j=0; j<17; j++) {
            columnPrevious[j] = 0;
            columnNow[j] = 0;
            }
            int targetIndex = 0;  // target position
            unsigned int queryPack = compressed[queryOffset+i];  // get 16 query bases
            int jstart = i-shift;
            jstart = max(jstart, 0);
            int jend = i+shift;
            jend = min(jend, target32Length);
            for (int j=0; j<target32Length; j++) {  // target is row
            columnPrevious[0] = rowPrevious[targetIndex];
            unsigned int targetPack = compressed[targetOffset+j];  // get 16 target bases
            //---16*16core----//
            for (int k=30; k>=0; k-=2) {  // 16 loops get target bases
            // first small loop
            int targetBase = (targetPack>>k)&3;  // get base from target
            int m=0;
            columnNow[m] = rowPrevious[targetIndex+1];
            for (int l=30; l>=0; l-=2) {  // 16 loops get query bases
            m++;
            int queryBase = (queryPack>>l)&3;  // get base from query
            int diffScore = queryBase == targetBase;
            columnNow[m] = columnPrevious[m-1] + diffScore;
            columnNow[m] = max(columnNow[m], columnNow[m - 1]);
            columnNow[m] = max(columnNow[m], columnPrevious[m]);
            }
            targetIndex++;
            rowNow[targetIndex] = columnNow[16];
            if (targetIndex == targetLength) {  // last column of dynamic matirx
            if(i == query32Length-1) {  // complete
            int score = columnNow[queryLength%16];
            if (score >= minLength) {
            cluster[index] = target;
            } else {
            cluster[index] = -1;
            }
            return;
            }
            break;
            }
            // secode small loop exchange columnPrevious and columnNow
            k-=2;
            targetBase = (targetPack>>k)&3;
            m=0;
            columnPrevious[m] = rowPrevious[targetIndex+1];
            for (int l=30; l>=0; l-=2) {
            m++;
            int queryBase = (queryPack>>l)&3;
            int diffScore = queryBase == targetBase;
            columnPrevious[m] = columnNow[m-1] + diffScore;
            columnPrevious[m] =
            max(columnPrevious[m], columnPrevious[m - 1]);
            columnPrevious[m] =
            max(columnPrevious[m], columnNow[m]);
            }
            targetIndex++;
            rowNow[targetIndex] = columnPrevious[16];
            if (targetIndex == targetLength) {
            if(i == query32Length-1) {
            int score = columnPrevious[queryLength%16];
            if (score >= minLength) {
            cluster[index] = target;
            } else {
            cluster[index] = -1;
            }
            return;
            }
            break;
            }
            }
            }
            // secode big loop exchage rowPrevious and rowNow
            i++;
            for (int j=0; j<17; j++) {
            columnPrevious[j] = 0;
            columnNow[j] = 0;
            }
            targetIndex = 0;
            queryPack = compressed[queryOffset+i];
            jstart = i-shift;
            jstart = max(jstart, 0);
            jend = i+shift;
            jend = min(jend, target32Length);
            for (int j=0; j<target32Length; j++) {
            unsigned int targetPack = compressed[targetOffset+j];
            //---16*16 core----//
            for (int k=30; k>=0; k-=2) {
            // first small loop
            int targetBase = (targetPack>>k)&3;
            int m=0;
            columnNow[m] = rowNow[targetIndex+1];
            for (int l=30; l>=0; l-=2) {
            m++;
            int queryBase = (queryPack>>l)&3;
            int diffScore = queryBase == targetBase;
            columnNow[m] = columnPrevious[m-1] + diffScore;
            columnNow[m] = max(columnNow[m], columnNow[m - 1]);
            columnNow[m] = max(columnNow[m], columnPrevious[m]);
            }
            targetIndex++;
            rowPrevious[targetIndex] = columnNow[16];
            if (targetIndex == targetLength) {
            if(i == query32Length-1) {
            int score = columnNow[queryLength%16];
            if (score >= minLength) {
            cluster[index] = target;
            } else {
            cluster[index] = -1;
            }
            return;
            }
            break;
            }
            // second small loop
            k-=2;
            targetBase = (targetPack>>k)&3;
            m=0;
            columnPrevious[m] = rowNow[targetIndex+1];
            for (int l=30; l>=0; l-=2) {
            m++;
            int queryBase = (queryPack>>l)&3;
            int diffScore = queryBase == targetBase;
            columnPrevious[m] = columnNow[m-1] + diffScore;
            columnPrevious[m] =
            max(columnPrevious[m], columnPrevious[m - 1]);
            columnPrevious[m] =
            max(columnPrevious[m], columnNow[m]);
            }
            targetIndex++;
            rowPrevious[targetIndex] = columnPrevious[16];
            if (targetIndex == targetLength) {
            if(i == query32Length-1) {
            int score = columnPrevious[queryLength%16];
            if (score >= minLength) {
            cluster[index] = target;
            } else {
            cluster[index] = -1;
            }
            return;
            }
            break;
            }
            }
            }
            }

        }
    }
}
