#ifndef orc_proc_h
#define orc_proc_h

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <inttypes.h>

#define AP_INT_MAX_W 4096
#include "ap_int.h"
#include <tapa.h>

// #define __SYNTHESIS__

#define DMUL 16
#define DECOMP_DEPTH 4072
// #define DECOMPOUT_DEPTH 4096


const uint8_t SR = 0;
const uint8_t DIRECT = 1;
const uint8_t PATCHED = 2;
const uint8_t DELTA = 3;

const uint8_t HEADER_ST = 7;
const uint8_t SR_STATE = 0; 
const uint8_t DI_STATE = 1; 
const uint8_t PA_STATE = 2; 
const uint8_t DE_STATE = 3;
const uint8_t ASSIGN_DATA = 4;
const uint8_t PA_META_PROC = 5;
const uint8_t TR_HEADER = 6;
const uint8_t HD_PROC = 8;
const uint8_t waste_cycles = 9;

const uint8_t FOP_LT = 1;
const uint8_t FOP_LTE = 2;
const uint8_t FOP_EQ = 3;
const uint8_t FOP_NE = 4;
const uint8_t FOP_GT = 5;
const uint8_t FOP_GTE = 6;

const uint16_t AXI_WIDTH_2X = 1024;
const uint16_t AXI_WIDTH_4X = 2048;
const uint16_t AXI_WIDTH_5X = 2560;
const uint16_t AXI_WIDTH = 512;
const uint16_t SR_DATAW = 320;
const uint16_t AXI_WIDTH_H = 256;
const uint16_t AXI_WIDTH_HH = 128;

typedef ap_uint<2560> _2560b;
typedef ap_uint<2048> _2048b;
typedef ap_uint<1536> _1536b;
typedef ap_uint<1024> _1024b;
typedef ap_uint<512> _512b;
typedef ap_int<512> _512bi;
typedef ap_uint<320> _320b;
typedef ap_uint<256> _256b;
typedef ap_uint<128> _128b;
typedef ap_uint<72> _72b;
typedef ap_uint<71> _71b;

const uint16_t PEs = 16;


#endif
