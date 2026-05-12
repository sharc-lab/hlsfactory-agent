#ifndef HLSLIB_XILINX_FLOW_H
#define HLSLIB_XILINX_FLOW_H

#include <ap_int.h>

#include "../ConstLog2.h"
#include "../op.h"

// Mark synthesis-only branches as active so debug-only host code is skipped.
#ifndef HLSLIB_SYNTHESIS
#define HLSLIB_SYNTHESIS 1
#endif

#ifndef MM_SYNTHESIS
#define MM_SYNTHESIS 1
#endif

// Provide a stand-in half type for non-half builds that still mention it.
struct half {
  float value;

  half(float v = 0.0f) : value(v) {}
  operator float() const { return value; }
};

#endif
