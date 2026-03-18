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

  float* d_psf;
  float* d_residual;
  const size_t psf_size = psf.size();
  const size_t residual_size = residual.size();

  // Find peak of PSF
  Peak psfPeak = findPeak(d_psf, d_peaks, psf_size);

  std::cout << "Found peak of PSF: " << "Maximum = " << psfPeak.val 
    << " at location " << idxToPos(psfPeak.pos, psfWidth).x << ","
    << idxToPos(psfPeak.pos, psfWidth).y << std::endl;
  assert(psfPeak.pos <= psf_size);

  Stopwatch sw;
  sw.start();


  const double time = sw.stop();

  // Report on timings
  std::cout << "    Time " << time << " (s) " << std::endl;
  std::cout << "    Time per cycle " << time / niters * 1000 << " (ms)" << std::endl;
  std::cout << "    Cleaning rate  " << niters / time << " (iterations per second)" << std::endl;
  std::cout << "Done" << std::endl;

  // Copy device arrays back into the host 

  // Free device memory

}


// --- from kernels.h ---
#ifndef HOGBOM_TEST_H
#define HOGBOM_TEST_H

#include <vector>
#include <cstddef>

extern unsigned int niters;
extern const float gain;
extern const float threshold;

class HogbomTest {
  public:
    HogbomTest();
    ~HogbomTest();

    void deconvolve(const std::vector<float>& dirty,
        const size_t dirtyWidth,
        const std::vector<float>& psf,
        const size_t psfWidth,
        std::vector<float>& model,
        std::vector<float>& residual);
  private:
};

#endif


// --- from timer.h ---
#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <sys/times.h>

class Stopwatch {
  public:
    Stopwatch();
    ~Stopwatch();
    void start();
    double stop();

  private:
    clock_t m_start;
};

#endif


// --- from utils.h ---
std::vector<float> readImage(const std::string& filename)
{
  struct stat results;
  if (stat(filename.c_str(), &results) != 0) {
    std::cerr << "Error: Could not stat " << filename << std::endl;
    exit(1);
  }

  std::vector<float> image(results.st_size / sizeof(float));
  std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
  file.read(reinterpret_cast<char *>(&image[0]), results.st_size);
  file.close();
  return image;
}

#ifdef OUTPUT
void writeImage(const std::string& filename, std::vector<float>& image)
{
  std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  file.write(reinterpret_cast<char *>(&image[0]), image.size() * sizeof(float));
  file.close();
}
#endif

size_t checkSquare(std::vector<float>& vec)
{
  const size_t size = vec.size();
  const size_t singleDim = sqrt(size);
  if (singleDim * singleDim != size) {
    std::cerr << "Error: Image is not square" << std::endl;
    exit(1);
  }

  return singleDim;
}

void zeroInit(std::vector<float>& vec)
{
  for (std::vector<float>::size_type i = 0; i < vec.size(); ++i) {
    vec[i] = 0.0;
  }
}

bool compare(const std::vector<float>& expected, const std::vector<float>& actual)
{
  if (expected.size() != actual.size()) {
    std::cout << "Fail (Vector sizes differ)" << std::endl;
    return false;
  }

  const size_t len = expected.size();
  for (size_t i = 0; i < len; ++i) {
    if (fabs(expected[i] - actual[i]) > 1e-3) {
      std::cout << "Fail (Expected " << expected[i] << " got "
        << actual[i] << " at index " << i << ")" << std::endl;
      return false;
    }
  }

  return true;
}

