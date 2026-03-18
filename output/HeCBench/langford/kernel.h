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
// https://github.com/boris-dimitrov/z4_planar_langford_multigpu
//
// Copyright 2017 Boris Dimitrov, Portola Valley, CA 94028.
// Questions? Contact http://www.facebook.com/boris
//
// This program counts all permutations of the sequence 1, 1, 2, 2, 3, 3, ..., n, n
// in which the two occurrences of each m are separated by precisely m other numbers,
// and lines connecting all (m, m) pairs can be drawn on the page without crossing.
//
// See http://www.dialectrix.com/langford.html ("Planar Solutions") or Knuth volume 4a
// page 3.  Todo: Provide better Knuth reference.
//
//
// THE ALGORITHM
//
// This program runs a depth-first-search aka backtracking algorithm which chooses
// to "open" or "close" a pair at each position, starting with position 0, and
// whether that pair would be connected from "below" or "above".  There are 4 choices
// for each of the 2*n positions, making it O(4^(2n)) with maximum stack depth 6*n.
//
// When choosing to "close" at position k, it locates the matching "open" at k',
// and computes the distance m = k - k' + 1.  If this m has already been placed,
// closing at position k is not possible.
//
// When the number of open pairs reaches n, opening new pairs is no longer possible.
// Observing this constraint greatly prunes the search tree.
//
// The matching "open" at k' is very easy to find using two auxiliary stacks of
// currently open pairs, one for "below" and one for "above".
//
//
// DEDUPLICATION
//
// To dedup the Left <-> Right reversal symmetry, (1, 1) is placed in pos <= n.
//
// Many, but not all, top <-> bottom twins are deduped by forcing the pair in
// position 0 to be connected from below.
//
// Remaining duplicates are eliminated by storing all solutions in memory,
// with a final sort and count.  Fortunately, the number of solutions to the
// planar Langford problem is quite small, so this is feasible.
//
//
// IMPLEMENTATION TRICKS
//
// A nice boost in performance is realized through the use of a single 64-bit
// integer to encode the positions of *all* currently open pairs;  in this compact
// representation, we can quickly "pop" the position of the most recently open pair
// by using the operations
//
//     ffsl(x)       if x is non-zero, return one plus the index of the least
//                   significant bit of x;  if x is zero, return zero;
//                   result range 0..64
//
//     x &= (x-1)    clear the least signifficant 1-bit of x
//
//
// EXAMPLE OUTPUT
//
//     1488034642458 Computing PL(2, 19)
//     1488034642458 Will use 2 system GPU(s).
//     1488034642463 GPU 1 init took 0.005 seconds.
//     1488034642463 GPU 0 init took 0.005 seconds.
//     1488034652163 GPU 0 computation took 9.70008 seconds on GPU clock, 9.7 seconds on host clock.
//     1488034652178 GPU 1 computation took 9.71449 seconds on GPU clock, 9.715 seconds on host clock.
//     1488034652229 Result 2384 for n = 19 MATCHES previously published result
//
// The first number on each output line is a unix timestamp, i.e., milliseconds elapsed
// since Jan 1, 1970 GMT.  You may convert it to human-readable datetime using python,
// as follows.
//
// 1) Start "python"
// 2) Type "import time" and press enter
// 3) Type "time.localtime(1488034652163 / 1000.0)" and press enter
//
// The result is a decoding of unix timestamp 1488034652163 in your local time zone:
//
//     time.struct_time(tm_year=2017, tm_mon=2, tm_mday=25, tm_hour=6,
//         tm_min=57, tm_sec=32, tm_wday=5, tm_yday=56, tm_isdst=0)
//
//
// PRINTING ALL SOLUTION SEQUENCES
//
// If you wish all solutions sequences printed, change 'kPrint' below to 'true',
// and recompile.
//
//
// ACHIEVEMENTS
//
// On March 6, 2017 at 11:38am PST this program computed PL(2, 28) after very close to 168 hours
// of work on a pair of NVIDIA Titan X Pascal GPUs in a single workstation.
//
//     1488221331811 Computing PL(2, 28)
//     1488221331811 Will use 2 system GPU(s).
//     1488221331817 GPU 1 init took 0.006 seconds.
//     1488221331817 GPU 0 init took 0.006 seconds.
//     1488823122601 GPU 1 computation took 601769 seconds on GPU clock, 601791 seconds on host clock.
//     1488829105991 GPU 0 computation took 607752 seconds on GPU clock, 607774 seconds on host clock.
//     1488829106231 Result 817717 for n = 28 MATCHES previously published result
//
// The GPU clock ran at ~1835 MHz, with each GPU consuming ~150 watts, at temperature ~62 Celsius,
// during most of that 7 day computation.  The last few hours the chips were less busy while
// finishing up a few of the longest running threads, and consumed correspondingly less power.
//
// SEE ALSO
//
// There is a variant of this program that runs on CPUs.  Performance is comparable between a single
// Titan X Pascal GPU (16nm process, mid 2016) and a 22-core Xeon E5-2699v4 (14nm, early 2016).
//
//     https://github.com/boris-dimitrov/z4_planar_langford

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;

// to avoid integer overflow, n should not exceed this constant
constexpr int kMaxN = 31;

// kLimit = 100 million means we can use up to 3.2GB of RAM on each GPU to store
// results before sorting;  if more memory is needed, we detect and bail
constexpr int64_t kLimit = 100000000;

// set to "true" if you want each solution printed
constexpr bool kPrint = false;

static_assert(sizeof(int64_t) == 8, "int64_t is not 8 bytes");
static_assert(sizeof(int32_t) == 4, "int32_t is not 4 bytes");
static_assert(sizeof(int8_t) == 1, "int64_t is not 1 byte");

constexpr int64_t lsb = 1;
constexpr int32_t lsb32 = 1;


// This type represents a solution by letting pos[m-1] be
// the position of the closing m, for m = 1, 2, ... n.
template <int n>
using Positions = std::array<int8_t, n>;

// All planar sequences (including duplicates) are stored in a vector for sorting.
template <int n>
using Results = std::vector<Positions<n>>;

// 8-byte alignment probably helps with concurrent writes to adjacent instances
template <int n>
using PositionsGPUAligned = int64_t[(n + 7) / 8];

template <int n>
using PositionsGPU = int8_t[div_up(n, 8) * 8];

// at depth k in the search, the bits of availability[k+1] represent the still
// unused m;  thus helping ensure that each (m, m) pair is placed just once
template <int n>
using Availability = int32_t[2 * n + 1];

// at depth k in the search, the bits of open[2*k+2] represent the positions
// of all pairs that are open above;  open[2*k+3] is the same for below;
// k ranges over 0, 1, ..., 2n-1
template <int n>
using Open = int64_t[4 * n + 2];

template <int n>
using Stack = int8_t[24 * n];

template <int n>
void print(const Positions<n>& pos);

// For some reason, 4 is the magic number that gives us best perf in this algorithm
constexpr int kThreadsPerBlock = 4;

// subdivide the search tree for this many logical threads
// due to the naive math below, numbers like 2^r-1 work much better than 2^r
constexpr int kNumLogicalThreads = 16383;

// To do:  Run on CPU (right now only runs on GPU).
template <int n>

template <int n>

// Sort the vector of solution sequences and count the unique ones.
// Optionally print each unique one.
template <int n>

// Return number of milliseconds elapsed since Jan 1, 1970 00:00 GMT.

// Start and manage the computation on GPU device "device"
template <int n>

// Start a CPU thread to manage each GPU device and wait for the computation to end.
template <int n>


template <int n>

