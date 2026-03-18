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
#ifndef BLOCK_DIM_Z
#define BLOCK_DIM_Z 1
#endif
#ifndef GRID_DIM_Z
#define GRID_DIM_Z 1
#endif

// --- from main.cu ---
/*
This function reconstruct the 3D volume from projections, based on
the Distance-Driven principle. It works by calculating the overlap
in X and Y axis of the volume and the detector boundaries.
The geometry is for DBT with half cone-beam. All parameters are set
in "ParameterSettings" code.

Reference:
- Branchless Distance Driven Projection and Backprojection,
Samit Basu and Bruno De Man (2006)
- GPU Acceleration of Branchless Distance Driven Projection and
Backprojection, Liu et al (2016)
- GPU-Based Branchless Distance-Driven Projection and Backprojection,
Liu et al (2017)
- A GPU Implementation of Distance-Driven Computed Tomography,
Ryan D. Wagner (2017)
---------------------------------------------------------------------
Copyright (C) <2019>  <Rodrigo de Barros Vimieiro>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Original author: Rodrigo de Barros Vimieiro
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <hip/hip_runtime.h>

// thread block size
#define BLOCK_SIZE 256

// integration direction
#define integrateXcoord 1
#define integrateYcoord 0









// Branchless distance-driven backprojection 
void backprojectionDDb(
          double* const h_pVolume,
    const double* const h_pProj,
    const double* const h_pTubeAngle,
    const double* const h_pDetAngle,
    const int idXProj,
    const int nProj,
    const int nPixX,
    const int nPixY,
    const int nSlices,
    const int nDetX,
    const int nDetY,
    const double dx,
    const double dy,
    const double dz,
    const double du,
    const double dv,
    const double DSD,
    const double DDR,
    const double DAG)
{
  // Number of mapped detectors
  const int nDetXMap = nDetX + 1;
  const int nDetYMap = nDetY + 1;

  // Number of mapped pixels
  const int nPixXMap = nPixX + 1;
  const int nPixYMap = nPixY + 1;

  double *d_pProj, *d_sliceI, *d_pVolume;

  hipMalloc((void **)&d_pProj, nDetXMap*nDetYMap*nProj * sizeof(double)); 
  hipMalloc((void **)&d_sliceI, nPixXMap*nPixYMap * sizeof(double));
  hipMalloc((void **)&d_pVolume, nPixX*nPixY*nSlices * sizeof(double));

  // device memory for projections coordinates
  double *d_pDetX, *d_pDetY, *d_pDetZ, *d_pObjX, *d_pObjY, *d_pObjZ;

  hipMalloc((void **)&d_pDetX, nDetXMap * sizeof(double));
  hipMalloc((void **)&d_pDetY, nDetYMap * sizeof(double));
  hipMalloc((void **)&d_pDetZ, nDetYMap * sizeof(double));
  hipMalloc((void **)&d_pObjX, nPixXMap * sizeof(double));
  hipMalloc((void **)&d_pObjY, nPixYMap * sizeof(double));
  hipMalloc((void **)&d_pObjZ, nSlices * sizeof(double));

  // device memory for mapped coordinates
  double *d_pDetmY, *d_pDetmX;

  hipMalloc((void **)&d_pDetmY, nDetYMap * sizeof(double));
  hipMalloc((void **)&d_pDetmX, nDetYMap * nDetXMap * sizeof(double));

  // device memory for rotated detector coords
  double *d_pRdetY, *d_pRdetZ;

  hipMalloc((void **)&d_pRdetY, nDetYMap * sizeof(double));
  hipMalloc((void **)&d_pRdetZ, nDetYMap * sizeof(double));

  auto start = std::chrono::steady_clock::now();

  // Will reuse grid configurations
  dim3 threadsPerBlock (1,1,1);
  dim3 blockSize (1,1,1);

  const int maxThreadsPerBlock = BLOCK_SIZE;

  // Copy projection data padding with zeros for image integation

  // Initialize first column and row with zeros
  const double* h_pProj_tmp;
  double* d_pProj_tmp;

  threadsPerBlock.x = maxThreadsPerBlock;
  blockSize.x = (nDetXMap / maxThreadsPerBlock) + 1;


  // Copy projections data from host to device

  // Generate detector and object boudaries

  threadsPerBlock.x = maxThreadsPerBlock;

  blockSize.x = (nDetX / maxThreadsPerBlock) + 1;

  blockSize.x = (nDetY / maxThreadsPerBlock) + 1;

  blockSize.x = (nPixX / maxThreadsPerBlock) + 1;

  blockSize.x = (nPixY / maxThreadsPerBlock) + 1;

  blockSize.x = (nSlices / maxThreadsPerBlock) + 1;

  // Initiate variables value with 0
  hipMemset(d_pDetZ, 0, nDetYMap * sizeof(double));
  hipMemset(d_pVolume, 0, nPixX * nPixY * nSlices * sizeof(double));

  // X - ray tube initial position
  double tubeX = 0;
  double tubeY = 0;
  double tubeZ = DSD;

  // Iso - center position
  double isoY = 0;
  double isoZ = DDR;

  // Integration of 2D projection over the whole projections
  // (S.1.Integration. - Liu et al(2017))

  // Naive integration o the X coord
  threadsPerBlock.x = 8;
  threadsPerBlock.y = 4;
  threadsPerBlock.z = 8;

  blockSize.x = (int)ceilf((float)nDetYMap / (threadsPerBlock.x - 1));
  blockSize.y = 1;
  blockSize.z = (int)ceilf((float)nProj / threadsPerBlock.z);

  int Xk = (int)ceilf((float)nDetXMap / (threadsPerBlock.x - 1));

  // Naive integration o the Y coord
  threadsPerBlock.x = 4;
  threadsPerBlock.y = 8;
  threadsPerBlock.z = 8;

  blockSize.x = 1;
  blockSize.y = (int)ceilf((float)nDetXMap / (threadsPerBlock.y - 1));
  blockSize.z = (int)ceilf((float)nProj / threadsPerBlock.z);

  int Yk = (int)ceilf((float)nDetYMap / (threadsPerBlock.y - 1));

  double* d_pDetmX_tmp = d_pDetmX + (nDetYMap * (nDetXMap-2));

  int projIni, projEnd, nProj2Run;
  else {
    projIni = idXProj;
    projEnd = idXProj + 1;
    nProj2Run = 1;
  }

  // For each projection

  // Normalize volume dividing by the number of projections
  threadsPerBlock.x = 8;
  threadsPerBlock.y = 8;
  threadsPerBlock.z = 4;

  blockSize.x = (nPixY / threadsPerBlock.x) + 1;
  blockSize.y = (nPixX / threadsPerBlock.y) + 1;
  blockSize.z = (nSlices / threadsPerBlock.z) + 1;

  hipDeviceSynchronize();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Total kernel execution %f (s)\n", time * 1e-9f);

  hipMemcpy(h_pVolume, d_pVolume, nSlices* nPixX * nPixY * sizeof(double), hipMemcpyDeviceToHost);

  hipFree(d_pProj);
  hipFree(d_sliceI);
  hipFree(d_pVolume);
  hipFree(d_pDetX);
  hipFree(d_pDetY);
  hipFree(d_pDetZ);
  hipFree(d_pObjX);
  hipFree(d_pObjY);
  hipFree(d_pObjZ);
  hipFree(d_pDetmY);
  hipFree(d_pDetmX);
  hipFree(d_pRdetY);
                            // image voxel density
  const int nPixX = 1996;   // number of voxels
  const int nPixY = 2457;   // number of voxels
  const int nSlices = 78;  

                            // detector panel pixel density
  const int nDetX = 1664;   // number of pixels
  const int nDetY = 2048;   // number of pixels

  const int nProj = 15;     // number of projections
  const int idXProj = -1;   // loop over all projections

  const double dx = 0.112;  // single voxel size (mm)
  const double dy = 0.112;
  const double dz = 1.0;

  const double du = 0.14;   // single detector size (mm)
  const double dv = 0.14;

  const double DSD = 700;   // distance from source to detector (mm)
  const double DDR = 0.0;   // distance from detector to pivot (mm)
  const double DAG = 25.0;  // distance of air gap (mm)

  const size_t pixVol = nPixX * nPixY * nSlices;
  const size_t detVol = nDetX * nDetY * nProj;
  double *h_pVolume = (double*) malloc (pixVol * sizeof(double));
  double *h_pProj = (double*) malloc (detVol * sizeof(double));

  double *h_pTubeAngle = (double*) malloc (nProj * sizeof(double));
  double *h_pDetAngle = (double*) malloc (nProj * sizeof(double));
  
  // tube angles in degrees
  for (int i = 0; i < nProj; i++) 
    h_pTubeAngle[i] = -7.5 + i * 15.0/nProj;

  // detector angles in degrees
  for (int i = 0; i < nProj; i++) 
    h_pDetAngle[i] = -2.1 + i * 4.2/nProj;

  // random values
  srand(123);
  for (size_t i = 0; i < pixVol; i++) 
    h_pVolume[i] = (double)rand() / (double)RAND_MAX;

  for (size_t i = 0; i < detVol; i++) 
    h_pProj[i] = (double)rand() / (double)RAND_MAX;

  backprojectionDDb(
    h_pVolume,
    h_pProj,
    h_pTubeAngle,
    h_pDetAngle,
    idXProj,
    nProj,
    nPixX, nPixY,
    nSlices,
    nDetX, nDetY,
    dx, dy, dz,
    du, dv,
    DSD, DDR, DAG);

  double checkSum = 0;
  for (size_t i = 0; i < pixVol; i++)
    checkSum += h_pVolume[i];
  printf("checksum = %lf\n", checkSum);

  free(h_pVolume);
  free(h_pTubeAngle);
  free(h_pDetAngle);
  free(h_pProj);
  return 0;
}
