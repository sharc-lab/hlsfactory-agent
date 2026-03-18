#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
using std::min;
using std::max;

// CUDA thread/block dimension constants (adjust for your workload)
#ifndef BLOCK_DIM_X
#define BLOCK_DIM_X 256
#endif
#ifndef GRID_DIM_X
#define GRID_DIM_X 1
#endif

// --- from MD5Hash.cu ---
#include <cmath>
#include <cstdio>
#include <cfloat>   // FLT_MAX
#include <iostream>
#include <sstream>
#include <chrono>
#include <hip/hip_runtime.h>

using namespace std;

// leftrotate function definition
#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

#define F(x,y,z) ((x & y) | ((~x) & z))
#define G(x,y,z) ((x & z) | ((~z) & y))
#define H(x,y,z) (x ^ y ^ z)
#define I(x,y,z) (y ^ (x | (~z)))

// This version of the round shifts the interpretation of a,b,c,d by one
// and must be called with v/x/y/z in a matching shuffle pattern.
// Every four Rounds, a,b,c,d are back to their original interpretation,
// thogh, so it all works out in the end (we have 64 rounds per block).
#define ROUND_INPLACE_VIA_SHIFT(w, r, k, v, x, y, z, func)       \
{                                                                \
    v += func(x,y,z) + w + k;                                    \
    v = x + LEFTROTATE(v, r);                                    \
}

// This version ignores the mapping of a/b/c/d to v/x/y/z and simply
// uses a temporary variable to keep the interpretation of a/b/c/d
// consistent.  Whether this one or the previous one performs better
// probably depends on the compiler....
#define ROUND_USING_TEMP_VARS(w, r, k, v, x, y, z, func)         \
{                                                                \
    a = a + func(b,c,d) + k + w;                                 \
    unsigned int temp = d;                                       \
    d = c;                                                       \
    c = b;                                                       \
    b = b + LEFTROTATE(a, r);                                    \
    a = temp;                                                    \
}

// Here, we pick which style of ROUND we use.
#define ROUND ROUND_USING_TEMP_VARS
//#define ROUND ROUND_INPLACE_VIA_SHIFT

/// NOTE: this really only allows a length up to 7 bytes, not 8, because
/// we need to start the padding in the first byte following the message,
/// and we only have two words to work with here....
/// It also assumes words[] has all zero bits except the chars of interest.

// ****************************************************************************
// Function:  FindKeyspaceSize
//
// Purpose:
///   Multiply out the byteLength by valsPerByte to find the 
///   total size of the key space, with error checking.
//
// Arguments:
//   byteLength    number of bytes in a key
//   valsPerByte   number of values each byte can take on
//
// Programmer:  Jeremy Meredith
// Creation:    July 23, 2014
//
// Modifications:
// ****************************************************************************

// ****************************************************************************
// Function:  IndexToKey
//
// Purpose:
///   For a given index in the keyspace, find the actual key string
///   which is at that index.
//
// Arguments:
//   index         index in key space
//   byteLength    number of bytes in a key
//   valsPerByte   number of values each byte can take on
//   vals          output key string
//
// Programmer:  Jeremy Meredith
// Creation:    July 23, 2014
//
// Modifications:
// ****************************************************************************

// ****************************************************************************
// Function:  AsHex
//
// Purpose:
///   For a given key string, return the raw hex string for its bytes.
//
// Arguments:
//   vals       key string
//   len        length of key string
//
// Programmer:  Jeremy Meredith
// Creation:    July 23, 2014
//
// Modifications:
// ****************************************************************************

// ****************************************************************************
// Function:  FindKeyWithDigest_CPU
//
// Purpose:
///   On the CPU, search the key space to find a key with the given digest.
//
// Arguments:
//   searchDigest    the digest to search for
//   byteLength      number of bytes in a key
//   valsPerByte     number of values each byte can take on
//   foundIndex      output - the index of the found key (if found)
//   foundKey        output - the string of the found key (if found)
//   foundDigest     output - the digest of the found key (if found)
//
// Programmer:  Jeremy Meredith
// Creation:    July 23, 2014
//
// Modifications:
// ****************************************************************************

// ****************************************************************************
// Function:  FindKeyWithDigest_GPU
//
// Purpose:
///   On the GPU, search the key space to find a key with the given digest.
//
// Arguments:
//   searchDigest    the digest to search for
//   byteLength      number of bytes in a key
//   valsPerByte     number of values each byte can take on
//   foundIndex      output - the index of the found key (if found)
//   foundKey        output - the string of the found key (if found)
//   foundDigest     output - the digest of the found key (if found)
//
// Programmer:  Jeremy Meredith
// Creation:    July 23, 2014
//
// Modifications:
// ****************************************************************************


// ****************************************************************************
// Function: RunBenchmark
//
// Purpose:
//   Executes the MD5 Hash benchmark
//
// Arguments:
//   dev: the opencl device id to use for the benchmark
//   ctx: the opencl context to use for the benchmark
//   queue: the opencl command queue to issue commands to
//   resultDB: results from the benchmark are stored in this db
//   op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Jeremy Meredith
// Creation: July 23, 2014
//
// Modifications:
//
// ****************************************************************************

