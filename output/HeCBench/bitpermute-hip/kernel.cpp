#include "kernel.h"

// --- from main.cu ---
static void bit_rev(fr_t* d_out, const fr_t* d_inp, uint32_t lg_domain_size)
{
  size_t domain_size = (size_t)1 << lg_domain_size;
  const uint32_t Z_COUNT = 256 / sizeof(fr_t); // 4: 64, 8: 32
  const uint32_t bsize = Z_COUNT>WARP_SZ ? Z_COUNT : WARP_SZ;

  if (domain_size <= 1024)

      (d_out, d_inp, lg_domain_size);

  else if (domain_size < bsize * Z_COUNT)

      (d_out, d_inp, lg_domain_size);

  else if (Z_COUNT > WARP_SZ || lg_domain_size <= 32)

        (d_out, d_inp, lg_domain_size);
  else {
    int numProcs;
    hipDeviceGetAttribute(&numProcs, hipDeviceAttributeMultiprocessorCount, 0);

        (d_out, d_inp, lg_domain_size);
  }
}
