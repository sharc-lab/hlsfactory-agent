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

// --- from kernel.cu ---
#include <cmath>
#include <cfloat>  // FLT_MAX

#define ZERO    0.0f
#define QUARTER 0.25f
#define HALF    0.5f
#define ONE     1.0f
#define TWO     2.0f
#define FOUR    4.0f
#define CNSTNT 45.0f

// Energy evaluation parameters
#define HBTYPE_F 70
#define HBTYPE_E 69
#define HARDNESS 38.0f
#define NPNPDIST  5.5f
#define NPPDIST   1.0f



// --- from main.cu ---
#include <cmath>
#include <memory>
#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>

typedef std::chrono::high_resolution_clock::time_point TimePoint;

struct Params {

  size_t natlig;
  size_t natpro;
  size_t ntypes;
  size_t nposes;

  std::vector<Atom> protein;
  std::vector<Atom> ligand;
  std::vector<FFParams> forcefield;
  std::array<std::vector<float>, 6> poses;

  size_t iterations;

  //  size_t posesPerWI;
  size_t wgSize;
  std::string deckDir;

  friend std::ostream &operator<<(std::ostream &os, const Params &params) {
    os <<
      "natlig:      " << params.natlig << "\n" <<
      "natpro:      " << params.natpro << "\n" <<
      "ntypes:      " << params.ntypes << "\n" <<
      "nposes:      " << params.nposes << "\n" <<
      "iterations:  " << params.iterations << "\n" <<
      "posesPerWI:  " << NUM_TD_PER_THREAD << "\n" <<
      "wgSize:      " << params.wgSize << "\n";
    return os;
  }
};

void fasten_main(
    //    size_t posesPerWI,
    const size_t ntypes,
    const size_t nposes,
    const size_t natlig,
    const size_t natpro,
    const Atom * protein_molecule,
    const Atom * ligand_molecule,
    const float * transforms_0,
    const float * transforms_1,
    const float * transforms_2,
    const float * transforms_3,
    const float * transforms_4,
    const float * transforms_5,
    const FFParams * forcefield,
    float * etotals);



template<typename T>
std::vector<T> readNStruct(const std::string &path) {
  s.ignore(std::numeric_limits<std::streamsize>::max());
  auto len = s.gcount();
  s.clear();
  s.seekg(0, std::ios::beg);
  std::vector<T> xs(len / sizeof(T));
  s.read(reinterpret_cast<char *>(xs.data()), len);
  s.close();
  return xs;
}


std::vector<float> runKernel(Params params) {

  std::vector<float> energies(params.nposes);

  Atom *protein;
  hipMalloc((void**)&protein, params.natpro*sizeof(Atom));
  hipMemcpy(protein, params.protein.data(), params.natpro*sizeof(Atom), hipMemcpyHostToDevice);

  Atom *ligand;
  hipMalloc((void**)&ligand, params.natlig*sizeof(Atom));
  hipMemcpy(ligand, params.ligand.data(), params.natlig*sizeof(Atom), hipMemcpyHostToDevice);

  float *transforms_0;
  hipMalloc((void**)&transforms_0, params.nposes*sizeof(float));
  hipMemcpy(transforms_0, params.poses[0].data(), params.nposes*sizeof(float), hipMemcpyHostToDevice);

  float *transforms_1;
  hipMalloc((void**)&transforms_1, params.nposes*sizeof(float));
  hipMemcpy(transforms_1, params.poses[1].data(), params.nposes*sizeof(float), hipMemcpyHostToDevice);

  float *transforms_2;
  hipMalloc((void**)&transforms_2, params.nposes*sizeof(float));
  hipMemcpy(transforms_2, params.poses[2].data(), params.nposes*sizeof(float), hipMemcpyHostToDevice);

  float *transforms_3;
  hipMalloc((void**)&transforms_3, params.nposes*sizeof(float));
  hipMemcpy(transforms_3, params.poses[3].data(), params.nposes*sizeof(float), hipMemcpyHostToDevice);

  float *transforms_4;
  hipMalloc((void**)&transforms_4, params.nposes*sizeof(float));
  hipMemcpy(transforms_4, params.poses[4].data(), params.nposes*sizeof(float), hipMemcpyHostToDevice);

  float *transforms_5;
  hipMalloc((void**)&transforms_5, params.nposes*sizeof(float));
  hipMemcpy(transforms_5, params.poses[5].data(), params.nposes*sizeof(float), hipMemcpyHostToDevice);

  FFParams *forcefield;
  hipMalloc((void**)&forcefield, params.ntypes*sizeof(FFParams));
  hipMemcpy(forcefield, params.forcefield.data(), params.ntypes*sizeof(FFParams), hipMemcpyHostToDevice);

  float *results;
  hipMalloc((void**)&results, params.nposes*sizeof(float));

  size_t global = ceil((params.nposes) / static_cast<double> (NUM_TD_PER_THREAD));
  global = ceil(static_cast<double> (global) / params.wgSize);

  dim3 grid (global);
  dim3 block (params.wgSize);

  // warmup
  hipLaunchKernelGGL(fasten_main, dim3(grid), dim3(block), params.ntypes * sizeof(FFParams) , 0, 
      params.ntypes,
      params.nposes,
      params.natlig,
      params.natpro,
      protein,
      ligand,
      transforms_0,
      transforms_1,
      transforms_2,
      transforms_3,
      transforms_4,
      transforms_5,
      forcefield,
      results);

  auto kernelStart = std::chrono::high_resolution_clock::now();


  hipDeviceSynchronize();

  auto kernelEnd = std::chrono::high_resolution_clock::now();

  hipMemcpy(energies.data(), results, params.nposes*sizeof(float), hipMemcpyDeviceToHost);

  printTimings(params, elapsedMillis(kernelStart, kernelEnd));

  hipFree(protein);
  hipFree(ligand);
  hipFree(transforms_0);
  hipFree(transforms_1);
  hipFree(transforms_2);
  hipFree(transforms_3);
  hipFree(transforms_4);
  hipFree(transforms_5);
  hipFree(forcefield);
  hipFree(results);

  return energies;
}



// --- from bude.h ---
#pragma once

#include <cstdint>
#include <string>
#include <iomanip>
#include <hip/hip_runtime.h>

#ifndef DEFAULT_PPWI
#define DEFAULT_PPWI 1
#endif
#ifndef DEFAULT_WGSIZE
#define DEFAULT_WGSIZE 4
#endif

#ifndef NUM_TD_PER_THREAD
#define NUM_TD_PER_THREAD DEFAULT_PPWI
#endif

#define DEFAULT_ITERS  8
#define DEFAULT_NPOSES 65536
#define REF_NPOSES     65536

#define DATA_DIR          "../data/bm1"
#define FILE_LIGAND       "/ligand.in"
#define FILE_PROTEIN      "/protein.in"
#define FILE_FORCEFIELD   "/forcefield.in"
#define FILE_POSES        "/poses.in"
#define FILE_REF_ENERGIES "/ref_energies.out"

struct __attribute__((__packed__)) Atom {
  float x, y, z;
  int32_t type;
};

struct __attribute__((__packed__)) FFParams {
  int32_t hbtype;
  float radius;
  float hphb;
  float elsc;
};

