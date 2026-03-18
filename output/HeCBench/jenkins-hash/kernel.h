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

// --- from main.cu ---
#include <stdio.h>      /* defines printf for tests */
#include <stdlib.h>     /* defines atol and posix_memalign */
#include <string.h>     /* defines memcpy */
#include <chrono>

#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

/*
   -------------------------------------------------------------------------------
   mix -- mix 3 32-bit values reversibly.

   This is reversible, so any information in (a,b,c) before mix() is
   still in (a,b,c) after mix().

   If four pairs of (a,b,c) inputs are run through mix(), or through
   mix() in reverse, there are at least 32 bits of the output that
   are sometimes the same for one pair and different for another pair.
   This was tested for:
 * pairs that differed by one bit, by two bits, in any combination
 of top bits of (a,b,c), or in any combination of bottom bits of
 (a,b,c).
 * "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
 the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
 is commonly produced by subtraction) look like a single 1-bit
 difference.
 * the base values were pseudorandom, all zero but one bit set, or
 all zero plus a counter that starts at zero.

 Some k values for my "a-=c; a^=rot(c,k); c+=b;" arrangement that
 satisfy this are
 4  6  8 16 19  4
 9 15  3 18 27 15
 14  9  3  7 17  3
 Well, "9 15 3 18 27 15" didn't quite get 32 bits diffing
 for "differ" defined as + with a one-bit base and a two-bit delta.  I
 used http://burtleburtle.net/bob/hash/avalanche.html to choose
 the operations, constants, and arrangements of the variables.

 This does not achieve avalanche.  There are input bits of (a,b,c)

/*
   -------------------------------------------------------------------------------
   final -- final mixing of 3 32-bit values (a,b,c) into c

   Pairs of (a,b,c) values differing in only a few bits will usually
   produce values of c that look totally different.  This was tested for
 * pairs that differed by one bit, by two bits, in any combination




