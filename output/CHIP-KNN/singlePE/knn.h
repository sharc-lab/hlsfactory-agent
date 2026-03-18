#ifndef KNN_H
#define KNN_H

// Minimal placeholder definitions extracted from knn.cpp
#include <tapa.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <inttypes.h>
#include <stdlib.h>

#define CEIL_DIVISION(X, Y) ( (X-1)/Y + 1 )
#define ROUND_TO_NEXT_MULTIPLE(X, Y) (CEIL_DIVISION(X,Y) * Y)

const int IWIDTH = 512;
#define INTERFACE_WIDTH ap_uint<IWIDTH>
const int INPUT_DIM = 16;
const int TOP = 10;
#define NUM_SP_PTS (1024)
#define DISTANCE_METRIC (1)
#define NUM_PE (1)

#define DATA_TYPE float
#define LOCAL_DIST_SZ (32)
#define LOCAL_DIST_DTYPE ap_uint<LOCAL_DIST_SZ>
#define TRANSFER_TYPE ap_uint<32>
#define INT32 ap_uint<32>

#endif // KNN_H

// Additional macro stubs required for compilation
#ifndef NUM_FEATURES_PER_READ
#define NUM_FEATURES_PER_READ 8
#endif
#ifndef MAX_DATA_TYPE_VAL
#define MAX_DATA_TYPE_VAL 1e9
#endif
#ifndef PARTITION_LEN_IN_I
#define PARTITION_LEN_IN_I 256
#endif
#ifndef QUERY_DATA_RESERVE
#define QUERY_DATA_RESERVE 0
#endif
#ifndef NUM_SEGMENTS
#define NUM_SEGMENTS 2
#endif
#ifndef NUM_PE
#define NUM_PE 1
#endif
#ifndef NUM_SP_PTS_PER_KRNL_PADDED
#define NUM_SP_PTS_PER_KRNL_PADDED 1024
#endif
#ifndef NUM_FEATURES_PER_READ
#define NUM_FEATURES_PER_READ 8
#endif
#ifndef NUM_FEATURES_PER_READ
#define NUM_FEATURES_PER_READ 8
#endif
#ifndef printf
#define printf(...) (void)0
#endif

