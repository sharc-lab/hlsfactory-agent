#include "kernel.h"

// --- from kernels.cu ---
void bitonic_sort(int *values, const int N)
{
  unsigned int idx = _tid_x;

  for (int k = 2; k <= N; k <<= 1)
  {
    for (int j = k >> 1; j > 0; j = j >> 1)
    {
      while(idx < N) 
      {
        int ixj = idx^j;
        if (ixj > idx) 
        {
          if ((idx&k) == 0 && values[idx] > values[ixj]) 
          {
            //exchange(idx, ixj);
            int tmp = values[idx];
            values[idx] = values[ixj];
            values[ixj] = tmp;
          }
          if ((idx&k) != 0 && values[idx] < values[ixj]) 
          {
            //exchange(idx, ixj);
            int tmp = values[idx];
            values[idx] = values[ixj];
            values[ixj] = tmp;
          }
        }
        idx += BLOCK_DIM_X;
      }
      idx = _tid_x;
    }
  }
}
