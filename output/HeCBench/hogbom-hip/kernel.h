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
#ifndef BLOCK_DIM_Y
#define BLOCK_DIM_Y 1
#endif
#ifndef GRID_DIM_Y
#define GRID_DIM_Y 1
#endif

// --- from kernels.cu ---
#include <vector>
#include <iostream>
#include <cmath>
#include <cassert>
#include <cstddef>
#include <hip/hip_runtime.h>
#include "kernels.h"
#include "timer.h"

// grids and blocks are constant for the findPeak kernel
#define findPeakNBlocks 128
#define findPeakWidth 256

struct Peak {
  size_t pos;
  float val;
};

struct Position {
  int x;
  int y;
};







HogbomTest::HogbomTest()
{
}

HogbomTest::~HogbomTest()
{
}

void HogbomTest::deconvolve(const std::vector<float>& dirty,
    const size_t dirtyWidth,
    const std::vector<float>& psf,
    const size_t psfWidth,
    std::vector<float>& model,
    std::vector<float>& residual)
{
  residual = dirty;

  // Initialise a peaks array on the device. Each thread block will return
  // a peak. Note:  the d_peaks array is not initialized (hence avoiding the
  // memcpy), it is up to the device function to do that
  Peak* d_peaks;
  hipMalloc((void **) &d_peaks, findPeakNBlocks * sizeof(Peak));

  float* d_psf;
  float* d_residual;
  const size_t psf_size = psf.size();
  const size_t residual_size = residual.size();
  hipMalloc((void **) &d_psf, psf_size * sizeof(float));
  hipMalloc((void **) &d_residual, residual_size * sizeof(float));

  hipMemcpy(d_psf, &psf[0], psf_size * sizeof(float), hipMemcpyHostToDevice);
  hipMemcpy(d_residual, &residual[0], residual_size * sizeof(float), hipMemcpyHostToDevice);

  // Find peak of PSF
  Peak psfPeak = findPeak(d_psf, d_peaks, psf_size);

  std::cout << "Found peak of PSF: " << "Maximum = " << psfPeak.val 
    << " at location " << idxToPos(psfPeak.pos, psfWidth).x << ","
    << idxToPos(psfPeak.pos, psfWidth).y << std::endl;
  assert(psfPeak.pos <= psf_size);

  hipDeviceSynchronize();
  Stopwatch sw;
  sw.start();


  hipDeviceSynchronize();
  const double time = sw.stop();

  // Report on timings
  std::cout << "    Time " << time << " (s) " << std::endl;
  std::cout << "    Time per cycle " << time / niters * 1000 << " (ms)" << std::endl;
  std::cout << "    Cleaning rate  " << niters / time << " (iterations per second)" << std::endl;
  std::cout << "Done" << std::endl;

  // Copy device arrays back into the host 
  hipMemcpy(&residual[0], d_residual, residual.size() * sizeof(float), hipMemcpyDeviceToHost);

  // Free device memory
  hipFree(d_peaks);
  hipFree(d_psf);
  hipFree(d_residual);
}
