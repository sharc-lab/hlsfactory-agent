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

  Atom *ligand;

  float *transforms_0;

  float *transforms_1;

  float *transforms_2;

  float *transforms_3;

  float *transforms_4;

  float *transforms_5;

  FFParams *forcefield;

  float *results;

  size_t global = ceil((params.nposes) / static_cast<double> (NUM_TD_PER_THREAD));
  global = ceil(static_cast<double> (global) / params.wgSize);

  dim3 grid (global);
  dim3 block (params.wgSize);

  // warmup

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


  auto kernelEnd = std::chrono::high_resolution_clock::now();

  printTimings(params, elapsedMillis(kernelStart, kernelEnd));

  return energies;
}



// --- from bude.h ---
#pragma once

#include <cstdint>
#include <string>
#include <iomanip>

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

