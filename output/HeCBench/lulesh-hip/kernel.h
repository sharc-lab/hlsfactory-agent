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

// --- from lulesh-init.cu ---
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <cstdlib>

/////////////////////////////////////////////////////////////////////
Domain::Domain(Int_t numRanks, Index_t colLoc,
               Index_t rowLoc, Index_t planeLoc,
               Index_t nx, int tp, int nr, int balance, Int_t cost)
   :

////////////////////////////////////////////////////////////////////////////////
void
Domain::BuildMesh(Int_t nx, Int_t edgeNodes, Int_t edgeElems)
{
  Index_t meshEdgeElems = m_tp*nx ;

  // initialize nodal coordinates 
  Index_t nidx = 0 ;
  Real_t tz = Real_t(1.125)*Real_t(m_planeLoc*nx)/Real_t(meshEdgeElems) ;

  // embed hexehedral elements in nodal point lattice 
  Index_t zidx = 0 ;
  nidx = 0 ;
}

////////////////////////////////////////////////////////////////////////////////
void
Domain::SetupThreadSupportStructures()
{
   Index_t numthreads = NT;

  else {
    // These arrays are not used if we're not threaded
    m_nodeElemStart = NULL;
    m_nodeElemCornerList = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////
void
Domain::SetupCommBuffers(Int_t edgeNodes)
{
  // allocate a buffer large enough for nodal ghost data 
  Index_t maxEdgeSize = MAX(this->sizeX(), MAX(this->sizeY(), this->sizeZ()))+1 ;
  m_maxPlaneSize = CACHE_ALIGN_REAL(maxEdgeSize*maxEdgeSize) ;
  m_maxEdgeSize = CACHE_ALIGN_REAL(maxEdgeSize) ;

  // assume communication to 6 neighbors by default 
  m_rowMin = (m_rowLoc == 0)        ? 0 : 1;
  m_rowMax = (m_rowLoc == m_tp-1)     ? 0 : 1;
  m_colMin = (m_colLoc == 0)        ? 0 : 1;
  m_colMax = (m_colLoc == m_tp-1)     ? 0 : 1;
  m_planeMin = (m_planeLoc == 0)    ? 0 : 1;
  m_planeMax = (m_planeLoc == m_tp-1) ? 0 : 1;

  // Boundary nodesets
  if (m_colLoc == 0)
    m_symmX.resize(edgeNodes*edgeNodes);
  if (m_rowLoc == 0)
    m_symmY.resize(edgeNodes*edgeNodes);
  if (m_planeLoc == 0)
    m_symmZ.resize(edgeNodes*edgeNodes);
}

////////////////////////////////////////////////////////////////////////////////
void
Domain::CreateRegionIndexSets(Int_t nr, Int_t balance)
{
   srand(0);
   Index_t myRank = 0;
   this->numReg() = nr;
   m_regElemSize = new Index_t[numReg()];
   m_regElemlist = new Index_t*[numReg()];
   Index_t nextIndex = 0;
   //if we only have one region just fill it
   // Fill out the regNumList with material numbers, which are always
   // the region index plus one 
   //If we have more than one region distribute the elements.
   else {
      Int_t regionNum;
      Int_t regionVar;
      Int_t lastReg = -1;
      Int_t binSize;
      Index_t elements;
      Index_t runto = 0;
      Int_t costDenominator = 0;
      Int_t* regBinEnd = new Int_t[numReg()];
      //Determine the relative weights of all the regions.  This is based off the -b flag.  Balance is the value passed into b.  
      //Until all elements are assigned
   }
   // Convert regNumList to region index sets
   // First, count size of each region 
   // Second, allocate each region index set
   // Third, fill index sets
   
}

/////////////////////////////////////////////////////////////
void 
Domain::SetupSymmetryPlanes(Int_t edgeNodes)
{
  Index_t nidx = 0 ;
}

/////////////////////////////////////////////////////////////
void
Domain::SetupElementConnectivities(Int_t edgeElems)
{

}

/////////////////////////////////////////////////////////////
void
Domain::SetupBoundaryConditions(Int_t edgeElems) 
{
  Index_t ghostIdx[6] ;  // offsets to ghost locations

  // set up boundary condition information


  Int_t pidx = numElem() ;






  // symmetry plane or free surface BCs 
}

///////////////////////////////////////////////////////////////////////////



// --- from lulesh-util.cu ---
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/* Helper function for converting strings to ints, with error checking */




/////////////////////////////////////////////////////////////////////



// --- from lulesh-viz.cu ---
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef VIZ_MESH

#ifdef __cplusplus
  extern "C" {
#endif
#include "silo.h"
#ifdef __cplusplus
  }
#endif

// Function prototypes
static void DumpDomainToVisit(DBfile *db, Domain& domain, int myRank);
static


/**********************************************************************/


/**********************************************************************/


#else
   
#endif



// --- from lulesh.cu ---
/*
   Copyright (c) 2010-2013.
   Lawrence Livermore National Security, LLC.
   Produced at the Lawrence Livermore National Laboratory.
   LLNL-CODE-461231
   All rights reserved.

   This file is part of LULESH, Version 2.0.
   Please also read this link -- http://www.opensource.org/licenses/index.php

//////////////
DIFFERENCES BETWEEN THIS VERSION (2.x) AND EARLIER VERSIONS:
 * Addition of regions to make work more representative of multi-material codes
 * Default size of each domain is 30^3 (27000 elem) instead of 45^3. This is
 more representative of our actual working set sizes
 * Single source distribution supports pure serial, pure OpenMP, MPI-only, 
 and MPI+OpenMP
 * Addition of ability to visualize the mesh using VisIt 
https://wci.llnl.gov/codes/visit/download.html
 * Various command line options (see ./lulesh2.0 -h)
 -q              : quiet mode - suppress stdout
 -i <iterations> : number of cycles to run
 -s <size>       : length of cube mesh along side
 -r <numregions> : Number of distinct regions (def: 11)
 -b <balance>    : Load balance between regions of a domain (def: 1)
 -c <cost>       : Extra cost of more expensive regions (def: 1)
 -f <filepieces> : Number of file parts for viz output (def: np/9)
 -p              : Print out progress
 -v              : Output viz file (requires compiling with -DVIZ_MESH
 -h              : This message

 printf("Usage: %s [opts]\n", execname);
 printf(" where [opts] is one or more of:\n");
 printf(" -q              : quiet mode - suppress all stdout\n");
 printf(" -i <iterations> : number of cycles to run\n");
 printf(" -s <size>       : length of cube mesh along side\n");
 printf(" -r <numregions> : Number of distinct regions (def: 11)\n");
 printf(" -b <balance>    : Load balance between regions of a domain (def: 1)\n");
 printf(" -c <cost>       : Extra cost of more expensive regions (def: 1)\n");
 printf(" -f <numfiles>   : Number of files to split viz dump into (def: (np+10)/9)\n");
 printf(" -p              : Print out progress\n");
 printf(" -v              : Output viz file (requires compiling with -DVIZ_MESH\n");
 printf(" -h              : This message\n");
 printf("\n\n");

 *Notable changes in LULESH 2.0

 * Split functionality into different files
 lulesh.cc - where most (all?) of the timed functionality lies
 lulesh-comm.cc - MPI functionality
 lulesh-init.cc - Setup code
 lulesh-viz.cc  - Support for visualization option
 lulesh-util.cc - Non-timed functions
 *
 * The concept of "regions" was added, although every region is the same ideal
 *    gas material, and the same sedov blast wave problem is still the only
 *    problem its hardcoded to solve.
 * Regions allow two things important to making this proxy app more representative:
 *   Four of the LULESH routines are now performed on a region-by-region basis,
 *     making the memory access patterns non-unit stride
 *   Artificial load imbalances can be easily introduced that could impact
 *     parallelization strategies.  
 * The load balance flag changes region assignment.  Region number is raised to
 *   the power entered for assignment probability.  Most likely regions changes
 *   with MPI process id.
 * The cost flag raises the cost of ~45% of the regions to evaluate EOS by the
 *   entered multiple. The cost of 5% is 10x the entered multiple.
 * MPI and OpenMP were added, and coalesced into a single version of the source
 *   that can support serial builds, MPI-only, OpenMP-only, and MPI+OpenMP
* Added support to write plot files using "poor mans parallel I/O" when linked
*   with the silo library, which in turn can be read by VisIt.
* Enabled variable timestep calculation by default (courant condition), which
*   results in an additional reduction.
* Default domain (mesh) size reduced from 45^3 to 30^3
* Command line options to allow numerous test cases without needing to recompile
* Performance optimizations and code cleanup beyond LULESH 1.0
* Added a "Figure of Merit" calculation (elements solved per microsecond) and
*   output in support of using LULESH 2.0 for the 2017 CORAL procurement
  *
* Possible Differences in Final Release (other changes possible)
  *
  * High Level mesh structure to allow data structure transformations
  * Different default parameters
  * Minor code performance changes and cleanup

  TODO in future versions
  * Add reader for (truly) unstructured meshes, probably serial only
  * CMake based build system

  //////////////

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the disclaimer below.

  * Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the disclaimer (as noted below)
  in the documentation and/or other materials provided with the
  distribution.

  * Neither the name of the LLNS/LLNL nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY, LLC,
  THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
      BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
      NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Additional BSD Notice

  1. This notice is required to be provided under our contract with the U.S.
  Department of Energy (DOE). This work was produced at Lawrence Livermore
  National Laboratory under Contract No. DE-AC52-07NA27344 with the DOE.

  2. Neither the United States Government nor Lawrence Livermore National
  Security, LLC nor any of their employees, makes any warranty, express
  or implied, or assumes any liability or responsibility for the accuracy,
  completeness, or usefulness of any information, apparatus, product, or
  process disclosed, or represents that its use would not infringe
  privately-owned rights.

  3. Also, reference herein to any specific commercial products, process, or
  services by trade name, trademark, manufacturer or otherwise does not
  necessarily constitute or imply its endorsement, recommendation, or
  favoring by the United States Government or Lawrence Livermore National
  Security, LLC. The views and opinions of authors expressed herein do not
  necessarily state or reflect those of the United States Government or
  Lawrence Livermore National Security, LLC, and shall not be used for
  advertising or product endorsement purposes.

  */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <climits>
#include <iostream>
#include <sstream>
#include <limits>
#include <fstream>
#include <string>
#ifdef VERIFY
#include <cassert>
#include <random>
#endif

#define EPSILON 1e-7

#define THREADS 256

#define ZERO  Real_t(0)
#define HALF  Real_t(0.5)
#define ONE   Real_t(1.0)
#define THREE Real_t(3.0)
#define FOUR  Real_t(4.0)
#define C1    Real_t(.1111111e-36)
#define C2    Real_t(.3333333e-18)
#define SEVEN Real_t(7.0)
#define EIGHT Real_t(8.0)
#define C1S   Real_t(2.0/3.0)
#define SIXTH Real_t(1.0/6.0)

#define PTINY Real_t(1e-36)

static inline

/******************************************/
static inline
/******************************************/
static inline
//#pragma omp end declare target
/******************************************/
static inline
//#pragma omp end declare target
/******************************************/

/******************************************/
static inline
//#pragma omp end declare target

/******************************************/
static inline

/******************************************/
static inline

//inline

static inline
//#pragma omp end declare target
/******************************************/
//#pragma omp declare target
#define max(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
static inline
//#pragma omp end declare target
/******************************************/
static inline





void fb (
    const Real_t * dvdx,
    const Real_t * dvdy,
    const Real_t * dvdz,
    const Real_t * x8n,
    const Real_t * y8n,
    const Real_t * z8n,
    const Real_t * determ,
    const Real_t * xd,
    const Real_t * yd,
    const Real_t * zd,
    const Real_t * ss,
    const Real_t * elemMass,
    const Index_t * nodelist,
    const Real_t * gamma,
    Real_t * fx_elem,
    Real_t * fy_elem,
    Real_t * fz_elem,
    Real_t hgcoef,
    const Index_t numElem )
{
  Index_t i2 = BLOCK_DIM_X*_bid_x+_tid_x;
  if (i2 >= numElem) return;

  Index_t i3 = 8*i2;

  const Index_t* elemToNode = nodelist + i3;

  Real_t hgfx[8], hgfy[8], hgfz[8] ;

  Real_t coefficient;

  Real_t hourgam[8][4];
  Real_t xd1[8], yd1[8], zd1[8] ;

  Real_t volinv = ONE/determ[i2];
  Real_t ss1, mass1, volume13 ;


  /* compute forces */
  /* store forces into h arrays (force arrays) */

  ss1 = ss[i2];
  mass1 = elemMass[i2];
  volume13 = cbrt(determ[i2]);

  Index_t n0si2 = elemToNode[0];
  Index_t n1si2 = elemToNode[1];
  Index_t n2si2 = elemToNode[2];
  Index_t n3si2 = elemToNode[3];
  Index_t n4si2 = elemToNode[4];
  Index_t n5si2 = elemToNode[5];
  Index_t n6si2 = elemToNode[6];
  Index_t n7si2 = elemToNode[7];

  xd1[0] = xd[n0si2];
  xd1[1] = xd[n1si2];
  xd1[2] = xd[n2si2];
  xd1[3] = xd[n3si2];
  xd1[4] = xd[n4si2];
  xd1[5] = xd[n5si2];
  xd1[6] = xd[n6si2];
  xd1[7] = xd[n7si2];

  yd1[0] = yd[n0si2];
  yd1[1] = yd[n1si2];
  yd1[2] = yd[n2si2];
  yd1[3] = yd[n3si2];
  yd1[4] = yd[n4si2];
  yd1[5] = yd[n5si2];
  yd1[6] = yd[n6si2];
  yd1[7] = yd[n7si2];

  zd1[0] = zd[n0si2];
  zd1[1] = zd[n1si2];
  zd1[2] = zd[n2si2];
  zd1[3] = zd[n3si2];
  zd1[4] = zd[n4si2];
  zd1[5] = zd[n5si2];
  zd1[6] = zd[n6si2];
  zd1[7] = zd[n7si2];

  coefficient = hgcoef * Real_t(-0.01) * ss1 * mass1 / volume13;

  Real_t hxx[4], hyy[4], hzz[4];


  // With the threaded version, we write into local arrays per elem
  // so we don't have to worry about race conditions

  Real_t *fx_local = fx_elem + i3 ;
  fx_local[0] = hgfx[0];
  fx_local[1] = hgfx[1];
  fx_local[2] = hgfx[2];
  fx_local[3] = hgfx[3];
  fx_local[4] = hgfx[4];
  fx_local[5] = hgfx[5];
  fx_local[6] = hgfx[6];
  fx_local[7] = hgfx[7];

  Real_t *fy_local = fy_elem + i3 ;
  fy_local[0] = hgfy[0];
  fy_local[1] = hgfy[1];
  fy_local[2] = hgfy[2];
  fy_local[3] = hgfy[3];
  fy_local[4] = hgfy[4];
  fy_local[5] = hgfy[5];
  fy_local[6] = hgfy[6];
  fy_local[7] = hgfy[7];

  Real_t *fz_local = fz_elem + i3 ;
  fz_local[0] = hgfz[0];
  fz_local[1] = hgfz[1];
  fz_local[2] = hgfz[2];
  fz_local[3] = hgfz[3];
  fz_local[4] = hgfz[4];
  fz_local[5] = hgfz[5];
  fz_local[6] = hgfz[6];
  fz_local[7] = hgfz[7];
}









void calcMonotonicQForElems (
    const Index_t * elemBC,
    const Real_t * elemMass,
    Real_t * ql,
    Real_t * qq,
    const Real_t * vdov,
    const Real_t * volo,
    const Real_t * delv_eta,
    const Real_t * delx_eta,
    const Real_t * delv_zeta,
    const Real_t * delx_zeta,
    const Real_t * delv_xi,
    const Real_t * delx_xi,
    const Index_t * lxim,
    const Index_t * lxip,
    const Index_t * lzetam,
    const Index_t * lzetap,
    const Index_t * letap,
    const Index_t * letam,
    const Real_t * vnew,
    const Real_t monoq_limiter_mult,
    const Real_t monoq_max_slope,
    const Real_t qlc_monoq,
    const Real_t qqc_monoq,
    const Index_t numElem )
{
  Index_t i = BLOCK_DIM_X*_bid_x+_tid_x;
  if (i >= numElem) return;

  Real_t qlin, qquad ;
  Real_t phixi, phieta, phizeta ;
  Int_t bcMask = elemBC[i] ;
  Real_t delvm = 0.0, delvp =0.0;

  /*  phixi     */
  Real_t norm = Real_t(1.) / (delv_xi[i]+ PTINY ) ;


  delvm = delvm * norm ;
  delvp = delvp * norm ;

  phixi = Real_t(.5) * ( delvm + delvp ) ;

  delvm *= monoq_limiter_mult ;
  delvp *= monoq_limiter_mult ;

  if ( delvm < phixi ) phixi = delvm ;
  if ( delvp < phixi ) phixi = delvp ;
  if ( phixi < Real_t(0.)) phixi = Real_t(0.) ;
  if ( phixi > monoq_max_slope) phixi = monoq_max_slope;

  /*  phieta     */
  norm = Real_t(1.) / ( delv_eta[i] + PTINY ) ;


  delvm = delvm * norm ;
  delvp = delvp * norm ;

  phieta = Real_t(.5) * ( delvm + delvp ) ;

  delvm *= monoq_limiter_mult ;
  delvp *= monoq_limiter_mult ;

  if ( delvm  < phieta ) phieta = delvm ;
  if ( delvp  < phieta ) phieta = delvp ;
  if ( phieta < Real_t(0.)) phieta = Real_t(0.) ;
  if ( phieta > monoq_max_slope)  phieta = monoq_max_slope;

  /*  phizeta     */
  norm = Real_t(1.) / ( delv_zeta[i] + PTINY ) ;


  delvm = delvm * norm ;
  delvp = delvp * norm ;

  phizeta = Real_t(.5) * ( delvm + delvp ) ;

  delvm *= monoq_limiter_mult ;
  delvp *= monoq_limiter_mult ;

  if ( delvm   < phizeta ) phizeta = delvm ;
  if ( delvp   < phizeta ) phizeta = delvp ;
  if ( phizeta < Real_t(0.)) phizeta = Real_t(0.);
  else {
    Real_t delvxxi   = delv_xi[i]   * delx_xi[i]   ;
    Real_t delvxeta  = delv_eta[i]  * delx_eta[i]  ;
    Real_t delvxzeta = delv_zeta[i] * delx_zeta[i] ;

    if ( delvxxi   > Real_t(0.) ) delvxxi   = Real_t(0.) ;
    if ( delvxeta  > Real_t(0.) ) delvxeta  = Real_t(0.) ;
    if ( delvxzeta > Real_t(0.) ) delvxzeta = Real_t(0.) ;

    Real_t rho = elemMass[i] / (volo[i] * vnew[i]) ;

    qlin = -qlc_monoq * rho *
      (  delvxxi   * (Real_t(1.) - phixi) +
         delvxeta  * (Real_t(1.) - phieta) +
         delvxzeta * (Real_t(1.) - phizeta)  ) ;

    qquad = qqc_monoq * rho *
      (  delvxxi*delvxxi     * (Real_t(1.) - phixi*phixi) +
         delvxeta*delvxeta   * (Real_t(1.) - phieta*phieta) +
         delvxzeta*delvxzeta * (Real_t(1.) - phizeta*phizeta)  ) ;
  }

  qq[i] = qquad ;
  ql[i] = qlin  ;
}

void applyMaterialPropertiesForElems(
    const Real_t * ql,
    const Real_t * qq,
    const Real_t * delv,
    const Index_t * elemRep,
    const Index_t * elemElem,
    Real_t * q,
    Real_t * p,
    Real_t * e,
    Real_t * ss,
    Real_t * v,
    Real_t * vnewc,
    const Real_t  e_cut,
    const Real_t  p_cut,
    const Real_t  ss4o3,
    const Real_t  q_cut,
    const Real_t  v_cut,

    const Real_t eosvmax,
    const Real_t eosvmin,
    const Real_t pmin,
    const Real_t emin,
    const Real_t rho0,
    const Index_t numElem )
{
  Index_t elem = BLOCK_DIM_X*_bid_x+_tid_x;
  if (elem >= numElem) return;
  Index_t rep = elemRep[elem];
  Real_t e_old, delvc, p_old, q_old, qq_old, ql_old;
  Real_t p_new, q_new, e_new;
  Real_t work, compression, compHalfStep, bvc, pbvc, pHalfStep;
  Real_t vchalf ;
  Real_t vhalf ;
  Real_t ssc ;
  Real_t q_tilde ;
  Real_t ssTmp ;



  // This check may not make perfect sense in LULESH, but
  // it's representative of something in the full code -
  // just leave it in, please
  Real_t vc = v[elem] ;

  Real_t vnewc_t = vnewc[elem];

  Real_t e_temp    =    e[elem];
  Real_t delv_temp = delv[elem];
  Real_t p_temp    =    p[elem];
  Real_t q_temp    =    q[elem];
  Real_t qq_temp   =   qq[elem];
  Real_t ql_temp   =   ql[elem];

  p[elem] = p_new ;
  e[elem] = e_new ;
  q[elem] = q_new ;

  ssTmp = (pbvc * e_new + vnewc_t * vnewc_t * bvc * p_new) / rho0;
  ss[elem] = ssTmp ;

  if ( fabs(vnewc_t - ONE) < v_cut )
    vnewc_t = ONE ;

  v[elem] = vnewc_t ;
}
/*********************************/
/* Data structure implementation */
/*********************************/

/* might want to add access methods so that memory can be */
/* better managed, as in luleshFT */

template <typename T>
T *Allocate(size_t size)
{
  return static_cast<T *>(malloc(sizeof(T)*size)) ;
}

template <typename T>

/******************************************/

/* Work Routines */

static inline

/******************************************/

/******************************************/

static inline

/******************************************/

static inline

/******************************************/

static inline
/******************************************/

/******************************************/



// --- from lulesh.h ---
#ifndef __LULESH
#define __LULESH

#include <vector>
#include <hip/hip_runtime.h>

// Emulate the number of CPU threads
#define NT 2

//**************************************************
// Allow flexibility for arithmetic representations 
//**************************************************

#define MAX(a, b) ( ((a) > (b)) ? (a) : (b))

// Precision specification
typedef float        real4 ;
typedef double       real8 ;
typedef long double  real10 ;  // 10 bytes on x86

typedef int    Index_t ; // array subscript and loop index
typedef real8  Real_t ;  // floating point representation
typedef int    Int_t ;   // integer representation

enum { VolumeError = -1, QStopError = -2 } ;

inline real4  SQRT(real4  arg) { return sqrtf(arg) ; }
inline real8  SQRT(real8  arg) { return sqrt(arg) ; }
inline real10 SQRT(real10 arg) { return sqrtl(arg) ; }

inline real4  CBRT(real4  arg) { return cbrtf(arg) ; }
inline real8  CBRT(real8  arg) { return cbrt(arg) ; }
inline real10 CBRT(real10 arg) { return cbrtl(arg) ; }

inline real4  FABS(real4  arg) { return fabsf(arg) ; }
inline real8  FABS(real8  arg) { return fabs(arg) ; }
inline real10 FABS(real10 arg) { return fabsl(arg) ; }

// Stuff needed for boundary conditions
// 2 BCs on each of 6 hexahedral faces (12 bits)
#define XI_M        0x00007
#define XI_M_SYMM   0x00001
#define XI_M_FREE   0x00002
#define XI_M_COMM   0x00004

#define XI_P        0x00038
#define XI_P_SYMM   0x00008
#define XI_P_FREE   0x00010
#define XI_P_COMM   0x00020

#define ETA_M       0x001c0
#define ETA_M_SYMM  0x00040
#define ETA_M_FREE  0x00080
#define ETA_M_COMM  0x00100

#define ETA_P       0x00e00
#define ETA_P_SYMM  0x00200
#define ETA_P_FREE  0x00400
#define ETA_P_COMM  0x00800

#define ZETA_M      0x07000
#define ZETA_M_SYMM 0x01000
#define ZETA_M_FREE 0x02000
#define ZETA_M_COMM 0x04000

#define ZETA_P      0x38000
#define ZETA_P_SYMM 0x08000
#define ZETA_P_FREE 0x10000
#define ZETA_P_COMM 0x20000

// Assume 128 byte coherence
// Assume Real_t is an "integral power of 2" bytes wide
#define CACHE_COHERENCE_PAD_REAL (128 / sizeof(Real_t))

#define CACHE_ALIGN_REAL(n) \
   (((n) + (CACHE_COHERENCE_PAD_REAL - 1)) & ~(CACHE_COHERENCE_PAD_REAL-1))

//////////////////////////////////////////////////////
// Primary data structure
//////////////////////////////////////////////////////

/*
 * The implementation of the data abstraction used for lulesh
 * resides entirely in the Domain class below.  You can change
 * grouping and interleaving of fields here to maximize data layout
 * efficiency for your underlying architecture or compiler.
 *
 * For example, fields can be implemented as STL objects or
 * raw array pointers.  As another example, individual fields
 * m_x, m_y, m_z could be grouped into
 *
 *    struct { Real_t x, y, z ; } *m_coord ;
 *
 * allowing accessor functions such as
 *
 *  "Real_t &x(Index_t idx) { return m_coord[idx].x ; }"
 *  "Real_t &y(Index_t idx) { return m_coord[idx].y ; }"
 *  "Real_t &z(Index_t idx) { return m_coord[idx].z ; }"
 */
class Domain {

   public:

   // Constructor
   Domain(Int_t numRanks, Index_t colLoc,
          Index_t rowLoc, Index_t planeLoc,
          Index_t nx, Int_t tp, Int_t nr, Int_t balance, Int_t cost);

   //
   // ALLOCATION
   //

   void AllocateNodePersistent(Int_t numNode) // Node-centered
   {
      m_x.resize(numNode);  // coordinates
      m_y.resize(numNode);
      m_z.resize(numNode);

      m_xd.resize(numNode); // velocities
      m_yd.resize(numNode);
      m_zd.resize(numNode);

      m_xdd.resize(numNode); // accelerations
      m_ydd.resize(numNode);
      m_zdd.resize(numNode);

      m_fx.resize(numNode);  // forces
      m_fy.resize(numNode);
      m_fz.resize(numNode);

      m_nodalMass.resize(numNode);  // mass
   }

   void AllocateElemPersistent(Int_t numElem) // Elem-centered
   {
      m_nodelist.resize(8*numElem);

      // elem connectivities through face
      m_lxim.resize(numElem);
      m_lxip.resize(numElem);
      m_letam.resize(numElem);
      m_letap.resize(numElem);
      m_lzetam.resize(numElem);
      m_lzetap.resize(numElem);

      m_elemBC.resize(numElem);

      m_e.resize(numElem);
      m_p.resize(numElem);

      m_q.resize(numElem);
      m_ql.resize(numElem);
      m_qq.resize(numElem);

      m_v.resize(numElem);

      m_volo.resize(numElem);
      m_delv.resize(numElem);
      m_vdov.resize(numElem);

      m_arealg.resize(numElem);

      m_ss.resize(numElem);

      m_elemMass.resize(numElem);

      m_elemRep.resize(numElem);

      m_elemElem.resize(numElem);
   }

   void AllocateGradients(Int_t numElem, Int_t allElem)
   {
      // Position gradients
      m_delx_xi.resize(numElem) ;
      m_delx_eta.resize(numElem) ;
      m_delx_zeta.resize(numElem) ;

      // Velocity gradients
      m_delv_xi.resize(allElem) ;
      m_delv_eta.resize(allElem);
      m_delv_zeta.resize(allElem) ;
   }

   void DeallocateGradients()
   {
      m_delx_zeta.clear() ;
      m_delx_eta.clear() ;
      m_delx_xi.clear() ;

      m_delv_zeta.clear() ;
      m_delv_eta.clear() ;
      m_delv_xi.clear() ;
   }

   void AllocateStrains(Int_t numElem)
   {
      m_dxx.resize(numElem) ;
      m_dyy.resize(numElem) ;
      m_dzz.resize(numElem) ;
   }

   void DeallocateStrains()
   {
      m_dzz.clear() ;
      m_dyy.clear() ;
      m_dxx.clear() ;
   }
   
   //
   // ACCESSORS
   //

   // Node-centered

   // Nodal coordinates
   Real_t& x(Index_t idx)    { return m_x[idx] ; }
   Real_t& y(Index_t idx)    { return m_y[idx] ; }
   Real_t& z(Index_t idx)    { return m_z[idx] ; }

   // Nodal velocities
   Real_t& xd(Index_t idx)   { return m_xd[idx] ; }
   Real_t& yd(Index_t idx)   { return m_yd[idx] ; }
   Real_t& zd(Index_t idx)   { return m_zd[idx] ; }

   // Nodal accelerations
   Real_t& xdd(Index_t idx)  { return m_xdd[idx] ; }
   Real_t& ydd(Index_t idx)  { return m_ydd[idx] ; }
   Real_t& zdd(Index_t idx)  { return m_zdd[idx] ; }

   // Nodal forces
   Real_t& fx(Index_t idx)   { return m_fx[idx] ; }
   Real_t& fy(Index_t idx)   { return m_fy[idx] ; }
   Real_t& fz(Index_t idx)   { return m_fz[idx] ; }

   // Nodal mass
   Real_t& nodalMass(Index_t idx) { return m_nodalMass[idx] ; }

   // Nodes on symmertry planes
   Index_t symmX(Index_t idx) { return m_symmX[idx] ; }
   Index_t symmY(Index_t idx) { return m_symmY[idx] ; }
   Index_t symmZ(Index_t idx) { return m_symmZ[idx] ; }
   bool symmXempty()          { return m_symmX.empty(); }
   bool symmYempty()          { return m_symmY.empty(); }
   bool symmZempty()          { return m_symmZ.empty(); }

   //
   // Element-centered
   //
   Index_t&  regElemSize(Index_t idx) { return m_regElemSize[idx] ; }
   Index_t&  regNumList(Index_t idx) { return m_regNumList[idx] ; }
   Index_t*  regNumList()            { return &m_regNumList[0] ; }
   Index_t*  regElemlist(Int_t r)    { return m_regElemlist[r] ; }
   Index_t&  regElemlist(Int_t r, Index_t idx) { return m_regElemlist[r][idx] ; }

   Index_t*  nodelist(Index_t idx)    { return &m_nodelist[Index_t(8)*idx] ; }

   // elem connectivities through face
   Index_t&  lxim(Index_t idx) { return m_lxim[idx] ; }
   Index_t&  lxip(Index_t idx) { return m_lxip[idx] ; }
   Index_t&  letam(Index_t idx) { return m_letam[idx] ; }
   Index_t&  letap(Index_t idx) { return m_letap[idx] ; }
   Index_t&  lzetam(Index_t idx) { return m_lzetam[idx] ; }
   Index_t&  lzetap(Index_t idx) { return m_lzetap[idx] ; }

   // elem face symm/free-surface flag
   Int_t&  elemBC(Index_t idx) { return m_elemBC[idx] ; }

   // Principal strains - temporary
   Real_t& dxx(Index_t idx)  { return m_dxx[idx] ; }
   Real_t& dyy(Index_t idx)  { return m_dyy[idx] ; }
   Real_t& dzz(Index_t idx)  { return m_dzz[idx] ; }

   // Velocity gradient - temporary
   Real_t& delv_xi(Index_t idx)    { return m_delv_xi[idx] ; }
   Real_t& delv_eta(Index_t idx)   { return m_delv_eta[idx] ; }
   Real_t& delv_zeta(Index_t idx)  { return m_delv_zeta[idx] ; }

   // Position gradient - temporary
   Real_t& delx_xi(Index_t idx)    { return m_delx_xi[idx] ; }
   Real_t& delx_eta(Index_t idx)   { return m_delx_eta[idx] ; }
   Real_t& delx_zeta(Index_t idx)  { return m_delx_zeta[idx] ; }

   // Energy
   Real_t& e(Index_t idx)          { return m_e[idx] ; }

   // Pressure
   Real_t& p(Index_t idx)          { return m_p[idx] ; }

   // Artificial viscosity
   Real_t& q(Index_t idx)          { return m_q[idx] ; }

   // Linear term for q
   Real_t& ql(Index_t idx)         { return m_ql[idx] ; }
   // Quadratic term for q
   Real_t& qq(Index_t idx)         { return m_qq[idx] ; }

   // Relative volume
   Real_t& v(Index_t idx)          { return m_v[idx] ; }
   Real_t& delv(Index_t idx)       { return m_delv[idx] ; }

   // Reference volume
   Real_t& volo(Index_t idx)       { return m_volo[idx] ; }

   // volume derivative over volume
   Real_t& vdov(Index_t idx)       { return m_vdov[idx] ; }

   // Element characteristic length
   Real_t& arealg(Index_t idx)     { return m_arealg[idx] ; }

   // Sound speed
   Real_t& ss(Index_t idx)         { return m_ss[idx] ; }

   // Element mass
   Real_t& elemMass(Index_t idx)  { return m_elemMass[idx] ; }

   // Element mass
   Index_t& elemRep(Index_t idx)  { return m_elemRep[idx] ; }

   // Element mass
   Index_t& elemElem(Index_t idx)  { return m_elemElem[idx] ; }

   Index_t nodeElemCount(Index_t idx)
   { return m_nodeElemStart[idx+1] - m_nodeElemStart[idx] ; }

   Index_t *nodeElemCornerList(Index_t idx)
   { return &m_nodeElemCornerList[m_nodeElemStart[idx]] ; }

   // Parameters 

   // Cutoffs
   Real_t u_cut() const               { return m_u_cut ; }
   Real_t e_cut() const               { return m_e_cut ; }
   Real_t p_cut() const               { return m_p_cut ; }
   Real_t q_cut() const               { return m_q_cut ; }
   Real_t v_cut() const               { return m_v_cut ; }

   // Other constants (usually are settable via input file in real codes)
   Real_t hgcoef() const              { return m_hgcoef ; }
   Real_t qstop() const               { return m_qstop ; }
   Real_t monoq_max_slope() const     { return m_monoq_max_slope ; }
   Real_t monoq_limiter_mult() const  { return m_monoq_limiter_mult ; }
   Real_t ss4o3() const               { return m_ss4o3 ; }
   Real_t qlc_monoq() const           { return m_qlc_monoq ; }
   Real_t qqc_monoq() const           { return m_qqc_monoq ; }
   Real_t qqc() const                 { return m_qqc ; }

   Real_t eosvmax() const             { return m_eosvmax ; }
   Real_t eosvmin() const             { return m_eosvmin ; }
   Real_t pmin() const                { return m_pmin ; }
   Real_t emin() const                { return m_emin ; }
   Real_t dvovmax() const             { return m_dvovmax ; }
   Real_t refdens() const             { return m_refdens ; }

   // Timestep controls, etc...
   Real_t& time()                 { return m_time ; }
   Real_t& deltatime()            { return m_deltatime ; }
   Real_t& deltatimemultlb()      { return m_deltatimemultlb ; }
   Real_t& deltatimemultub()      { return m_deltatimemultub ; }
   Real_t& stoptime()             { return m_stoptime ; }
   Real_t& dtcourant()            { return m_dtcourant ; }
   Real_t& dthydro()              { return m_dthydro ; }
   Real_t& dtmax()                { return m_dtmax ; }
   Real_t& dtfixed()              { return m_dtfixed ; }

   Int_t&  cycle()                { return m_cycle ; }
   Index_t&  numRanks()           { return m_numRanks ; }

   Index_t&  colLoc()             { return m_colLoc ; }
   Index_t&  rowLoc()             { return m_rowLoc ; }
   Index_t&  planeLoc()           { return m_planeLoc ; }
   Index_t&  tp()                 { return m_tp ; }

   Index_t&  sizeX()              { return m_sizeX ; }
   Index_t&  sizeY()              { return m_sizeY ; }
   Index_t&  sizeZ()              { return m_sizeZ ; }
   Index_t&  numReg()             { return m_numReg ; }
   Int_t&  cost()             { return m_cost ; }
   Index_t&  numElem()            { return m_numElem ; }
   Index_t&  numNode()            { return m_numNode ; }
   
   Index_t&  maxPlaneSize()       { return m_maxPlaneSize ; }
   Index_t&  maxEdgeSize()        { return m_maxEdgeSize ; }
   
  // private:

   void BuildMesh(Int_t nx, Int_t edgeNodes, Int_t edgeElems);
   void SetupThreadSupportStructures();
   void CreateRegionIndexSets(Int_t nreg, Int_t balance);
   void SetupCommBuffers(Int_t edgeNodes);
   void SetupSymmetryPlanes(Int_t edgeNodes);
   void SetupElementConnectivities(Int_t edgeElems);
   void SetupBoundaryConditions(Int_t edgeElems);

   //
   // IMPLEMENTATION
   //

   /* Node-centered */
   std::vector<Real_t> m_x ;  /* coordinates */
   std::vector<Real_t> m_y ;
   std::vector<Real_t> m_z ;

   std::vector<Real_t> m_xd ; /* velocities */
   std::vector<Real_t> m_yd ;
   std::vector<Real_t> m_zd ;

   std::vector<Real_t> m_xdd ; /* accelerations */
   std::vector<Real_t> m_ydd ;
   std::vector<Real_t> m_zdd ;

   std::vector<Real_t> m_fx ;  /* forces */
   std::vector<Real_t> m_fy ;
   std::vector<Real_t> m_fz ;

   std::vector<Real_t> m_nodalMass ;  /* mass */

   std::vector<Index_t> m_symmX ;  /* symmetry plane nodesets */
   std::vector<Index_t> m_symmY ;
   std::vector<Index_t> m_symmZ ;

   // Element-centered

   // Region information
   Int_t    m_numReg ;
   Int_t    m_cost; //imbalance cost
   Index_t *m_regElemSize ;   // Size of region sets
   Index_t *m_regNumList ;    // Region number per domain element
   Index_t **m_regElemlist ;  // region indexset 

   std::vector<Index_t>  m_nodelist ;     /* elemToNode connectivity */

   std::vector<Index_t>  m_lxim ;  /* element connectivity across each face */
   std::vector<Index_t>  m_lxip ;
   std::vector<Index_t>  m_letam ;
   std::vector<Index_t>  m_letap ;
   std::vector<Index_t>  m_lzetam ;
   std::vector<Index_t>  m_lzetap ;

   std::vector<Int_t>    m_elemBC ;  /* symmetry/free-surface flags for each elem face */

   std::vector<Real_t> m_dxx ;  /* principal strains -- temporary */
   std::vector<Real_t> m_dyy ;
   std::vector<Real_t> m_dzz ;

   std::vector<Real_t> m_delv_xi ;    /* velocity gradient -- temporary */
   std::vector<Real_t> m_delv_eta ;
   std::vector<Real_t> m_delv_zeta ;

   std::vector<Real_t> m_delx_xi ;    /* coordinate gradient -- temporary */
   std::vector<Real_t> m_delx_eta ;
   std::vector<Real_t> m_delx_zeta ;
   
   std::vector<Real_t> m_e ;   /* energy */

   std::vector<Real_t> m_p ;   /* pressure */
   std::vector<Real_t> m_q ;   /* q */
   std::vector<Real_t> m_ql ;  /* linear term for q */
   std::vector<Real_t> m_qq ;  /* quadratic term for q */

   std::vector<Real_t> m_v ;     /* relative volume */
   std::vector<Real_t> m_volo ;  /* reference volume */
   std::vector<Real_t> m_vnew ;  /* new relative volume -- temporary */
   std::vector<Real_t> m_delv ;  /* m_vnew - m_v */
   std::vector<Real_t> m_vdov ;  /* volume derivative over volume */

   std::vector<Real_t> m_arealg ;  /* characteristic length of an element */
   
   std::vector<Real_t> m_ss ;      /* "sound speed" */

   std::vector<Real_t> m_elemMass ;  /* mass */

   std::vector<Index_t> m_elemRep ;  /* reps */

   std::vector<Index_t> m_elemElem ;  /* elems */

   // Cutoffs (treat as constants)
   const Real_t  m_e_cut ;             // energy tolerance 
   const Real_t  m_p_cut ;             // pressure tolerance 
   const Real_t  m_q_cut ;             // q tolerance 
   const Real_t  m_v_cut ;             // relative volume tolerance 
   const Real_t  m_u_cut ;             // velocity tolerance 

   // Other constants (usually setable, but hardcoded in this proxy app)

   const Real_t  m_hgcoef ;            // hourglass control 
   const Real_t  m_ss4o3 ;
   const Real_t  m_qstop ;             // excessive q indicator 
   const Real_t  m_monoq_max_slope ;
   const Real_t  m_monoq_limiter_mult ;
   const Real_t  m_qlc_monoq ;         // linear term coef for q 
   const Real_t  m_qqc_monoq ;         // quadratic term coef for q 
   const Real_t  m_qqc ;
   const Real_t  m_eosvmax ;
   const Real_t  m_eosvmin ;
   const Real_t  m_pmin ;              // pressure floor 
   const Real_t  m_emin ;              // energy floor 
   const Real_t  m_dvovmax ;           // maximum allowable volume change 
   const Real_t  m_refdens ;           // reference density 

   // Variables to keep track of timestep, simulation time, and cycle
   Real_t  m_dtcourant ;         // courant constraint 
   Real_t  m_dthydro ;           // volume change constraint 
   Int_t   m_cycle ;             // iteration count for simulation 
   Real_t  m_dtfixed ;           // fixed time increment 
   Real_t  m_time ;              // current time 
   Real_t  m_deltatime ;         // variable time increment 
   Real_t  m_deltatimemultlb ;
   Real_t  m_deltatimemultub ;
   Real_t  m_dtmax ;             // maximum allowable time increment 
   Real_t  m_stoptime ;          // end time for simulation 

   Int_t   m_numRanks ;

   Index_t m_colLoc ;
   Index_t m_rowLoc ;
   Index_t m_planeLoc ;
   Index_t m_tp ;

   Index_t m_sizeX ;
   Index_t m_sizeY ;
   Index_t m_sizeZ ;
   Index_t m_numElem ;
   Index_t m_numNode ;

   Index_t m_maxPlaneSize ;
   Index_t m_maxEdgeSize ;

   // OMP hack 
   Index_t *m_nodeElemStart ;
   Index_t *m_nodeElemCornerList ;

   // Used in setup
   Index_t m_rowMin, m_rowMax;
   Index_t m_colMin, m_colMax;
   Index_t m_planeMin, m_planeMax ;
} ;

typedef Real_t &(Domain::* Domain_member )(Index_t) ;

struct cmdLineOpts {
   Int_t its; // -i 
   Int_t nx;  // -s 
   Int_t numReg; // -r 
   Int_t numFiles; // -f
   Int_t showProg; // -p
   Int_t quiet; // -q
   Int_t viz; // -v 
   Int_t cost; // -c
   Int_t balance; // -b
   Int_t iteration_cap; // -z
};

// Function Prototypes
Real_t CalcElemVolume( const Real_t x[8], const Real_t y[8], const Real_t z[8] );

// lulesh-util
void ParseCommandLineOptions(int argc, char *argv[],
                             Int_t myRank, struct cmdLineOpts *opts);
void VerifyAndWriteFinalOutput(Real_t elapsed_time,
                               Domain& locDom,
                               Int_t nx,
                               Int_t numRanks);

// lulesh-viz
void DumpToVisit(Domain& domain, int numFiles, int myRank, int numRanks);

// lulesh-init
void InitMeshDecomp(Int_t numRanks, Int_t myRank,
                    Int_t *col, Int_t *row, Int_t *plane, Int_t *side);

#endif
