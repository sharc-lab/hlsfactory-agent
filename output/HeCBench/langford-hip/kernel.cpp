#include "kernel.h"

// --- from main.cu ---
void dfs(int64_t* p_result,
    Availability<n> &availability,
    Open<n> &open,
    Stack<n> &stack,
    PositionsGPUAligned<n>& pgpualigned,
    const int32_t logical_thread_index)
{
  constexpr int two_n = 2 * n;
  constexpr int64_t msb = lsb << (int64_t)(n - 1);
  constexpr int64_t nn1 = lsb << (2 * n - 1);
  PositionsGPU<n> &pos = *((PositionsGPU<n>*)(&pgpualigned[0]));
  // initially none of the numbers 1, 2, ..., n have been placed;
  // this is represented by setting bits 0..n-1 to 1 in avail
  availability[0] = msb | (msb - 1);
  open[0] = 0;
  open[1] = 0;
  int top = 0;
  int8_t k, m, d, num_open;
  // The following "push" and "pop" should be lambdas, but unfortunately Cuda C++ does not
  // yet support reference capture in lambdas that can run on both CPU and GPU.
  // Hoping for a compiler fix soon.
#define push(k, m, d, num_open) do { \
  stack[top++] = k; \
  stack[top++] = m; \
  stack[top++] = d; \
  stack[top++] = num_open; \
} while (0)
#define pop(k, m, d, num_open) do { \
  num_open = stack[--top]; \
  d = stack[--top]; \
  m = stack[--top]; \
  k = stack[--top]; \
} while (0)
  // every solution starts out by opening a below-pair at position 0
  push(0, -1, 0, 0);
  while (top) {
    pop(k, m, d, num_open);
    int64_t* openings = open + 2 * k + 2;
    openings[0] = openings[-2];
    openings[1] = openings[-1];
    int32_t avail = availability[k];
    // On CPU, this macro trick improves perf over 10% by letting the compiler
    // take advantage of the fact that d can only be 0 or 1.
    // Makes no difference on GPU.
#define place_macro(d) do { \
  if (m>=0) { \
    pos[m] = k; \
    avail ^= (lsb32 << m); \
    openings[d] &= (openings[d] - 1); \
  } else { \
    openings[d] |= (nn1 >> k); \
    ++num_open; \
  } \
} while (0)
    if (d) {
      place_macro(1);
    } else {
      place_macro(0);
    }
++k;
availability[k] = avail;
if (k == two_n) {
  // this is equivalent to results.push_back(pos);
  // p_results[0] is a counter;  after it follow the data
  // atomic increment of counter in device memory (i.e., RAM DIMMs on the GPU board)
  int64_t cnt = atomicAdd((unsigned long long*)p_result, (unsigned long long)1);
  if (cnt < kLimit) {
    constexpr int kAlignedCnt = (n + 7) / 8;
    int64_t* dst = p_result + 1 + (kAlignedCnt * cnt);
#pragma unroll
    for (int i=0; i<kAlignedCnt; ++i) {
      dst[i] = pgpualigned[i];
    }
  }
  // if cnt reaches or exceeds kLimit, that will be detected and the program will fail
} else {
  // A super-naive way to divide the work across threads.  A hash of the current state at k_limit
  // determines whether the current thread should be pursuing a completion from this state or not.
  // The depth k_limit is chosen empirically to be both shallow enough so it's quick to reach and
  // deep enough to allow plenty of concurrency. This seems to work remarkably well in practice.
  constexpr int8_t k_limit = (n > 19 ? (8 + (n / 3)) : (n - 5));
  if (kNumLogicalThreads > 1 &&
      k == k_limit &&
      // multiply by a nice Mersenne prime to divide the work evenly across the threads... it works well...
      uint64_t(131071 * (openings[1] - openings[0]) + avail) % kNumLogicalThreads != logical_thread_index) {
    // some other thread will work on this
    continue;
  }
  // Now push on the stack the the children of the current node in the search tree.
  int8_t offset = k - two_n - 2;
  for (d=0; d<2; ++d) {
    if (openings[d]) { // if there is an opening, try closing it
      m = offset + __ffsll((long long int)openings[d]);
      // m could be -1, for example if the decision at pos k-1 was to open;
      // only m from 0 .. n - 1 are useful
      if (((unsigned)m < n) && ((avail >> m) & 1)) {
        if (m || k <= n) { // this dedups L <==> R reversal twins
          push(k, m, d, num_open);
        }
      }
    }
  }
  if (num_open < n) {
    push(k, -1, 1, num_open);
    push(k, -1, 0, num_open);
  }
}
}
}
extern "C"

void dfs_gpu(int64_t* p_result) {
    #pragma HLS INTERFACE m_axi port=p_result offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            Availability<n> availability[kThreadsPerBlock];
            // PositionsGPU<n> pos;
            Open<n> open[kThreadsPerBlock];
            // there are 2*n positions with 3 decisions per position and 4 bytes per decision on the stack
            Stack<n> stack[kThreadsPerBlock];
            PositionsGPUAligned<n> pgpualigned[kThreadsPerBlock];
            // the size of the arrays above add up to ~2KB for n=32
            // this bodes well for fitting tousands of threads inside on-chip memory
            // assume 1D grid of 1D blocks of threads
            const int32_t result_index = _bid_x * kThreadsPerBlock + _tid_x;
            dfs<n>(p_result,
            availability[_tid_x],
            open[_tid_x],
            stack[_tid_x],
            pgpualigned[_tid_x],
            result_index);

        }
    }
}
