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

// --- from cuda_rys_dp.cu ---
/*****************************************************************************
 This file is part of the XLQC program.                                      
 Copyright (C) 2015 Xin Li <lixin.reco@gmail.com>                            
                                                                           
 Filename:  cuda_rys_dp.cu                                                      
 License:   BSD 3-Clause License

 * The implementation of Rys quadrature routines in C is taken from the
 * PyQuante quantum chemistry program, Copyright (c) 2004, Richard P. Muller.
 * PyQuante version 1.2 and later is covered by the modified BSD license. 
 * Please see int_lib/LICENSE.
 
 This software is provided by the copyright holders and contributors "as is"
 and any express or implied warranties, including, but not limited to, the
 implied warranties of merchantability and fitness for a particular purpose are
 disclaimed. In no event shall the copyright holder or contributors be liable
 for any direct, indirect, incidental, special, exemplary, or consequential
 damages (including, but not limited to, procurement of substitute goods or
 services; loss of use, data, or profits; or business interruption) however
 caused and on any theory of liability, whether in contract, strict liability,






// calculate ERI over 4 primitive basis functions

// calculate J matrix using 1-thread-1-primitive-integral scheme

// calculate K matrix using 1-thread-1-primitive-integral scheme


// --- from cuda_rys_sp.cu ---
/*****************************************************************************
  This file is part of the XLQC program.                                      
  Copyright (C) 2015 Xin Li <lixin.reco@gmail.com>                            

Filename:  cuda_rys_sp.cu                                                      
License:   BSD 3-Clause License

 * The implementation of Rys quadrature routines in C is taken from the
 * PyQuante quantum chemistry program, Copyright (c) 2004, Richard P. Muller.
 * PyQuante version 1.2 and later is covered by the modified BSD license. 
 * Please see int_lib/LICENSE.

 This software is provided by the copyright holders and contributors "as is"
 and any express or implied warranties, including, but not limited to, the
 implied warranties of merchantability and fitness for a particular purpose are
 disclaimed. In no event shall the copyright holder or contributors be liable
 for any direct, indirect, incidental, special, exemplary, or consequential
 damages (including, but not limited to, procurement of substitute goods or
 services; loss of use, data, or profits; or business interruption) however
 caused and on any theory of liability, whether in contract, strict liability,










// calculate ERI over 4 primitive basis functions

// calculate J matrix using 1-thread-1-primitive-integral scheme

// calculate K matrix using 1-thread-1-primitive-integral scheme


// --- from main.cu ---
/*****************************************************************************
 This file is part of the XLQC program.                                      
 Copyright (C) 2015 Xin Li <lixin.reco@gmail.com>                            
                                                                           
 Filename:  main.cu                                                      
 License:   BSD 3-Clause License

 This software is provided by the copyright holders and contributors "as is"
 and any express or implied warranties, including, but not limited to, the
 implied warranties of merchantability and fitness for a particular purpose are
 disclaimed. In no event shall the copyright holder or contributors be liable
 for any direct, indirect, incidental, special, exemplary, or consequential
 damages (including, but not limited to, procurement of substitute goods or
 services; loss of use, data, or profits; or business interruption) however
 caused and on any theory of liability, whether in contract, strict liability,
 or tort (including negligence or otherwise) arising in any way out of the use
 of this software, even if advised of the possibility of such damage.
 *****************************************************************************/

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <string>
#include <iostream>

#include <gsl/gsl_math.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>

#include "int_lib/cints.h"
#include "int_lib/crys.h"




// --- from basis.h ---
/*****************************************************************************
 This file is part of the XLQC program.                                      
 Copyright (C) 2015 Xin Li <lixin.reco@gmail.com>                            
                                                                           
 Filename:  basis.h                                                      
 License:   BSD 3-Clause License

 This software is provided by the copyright holders and contributors "as is"
 and any express or implied warranties, including, but not limited to, the
 implied warranties of merchantability and fitness for a particular purpose are
 disclaimed. In no event shall the copyright holder or contributors be liable
 for any direct, indirect, incidental, special, exemplary, or consequential
 damages (including, but not limited to, procurement of substitute goods or
 services; loss of use, data, or profits; or business interruption) however
 caused and on any theory of liability, whether in contract, strict liability,
 or tort (including negligence or otherwise) arising in any way out of the use
 of this software, even if advised of the possibility of such damage.
 *****************************************************************************/

// allocate memory with failure checking
void* my_malloc(size_t bytes);

void* my_malloc_2(size_t bytes, std::string word);

// combination index
int ij2intindex(int i, int j);

// get nuclear charge of an element
int get_nuc_chg(char *element);

// get number of atoms 
int get_natoms(void);

// read geometry
void read_geom(Atom *p_atom);

// calculate nuclear repulsion energy
double calc_ene_nucl(Atom *p_atom);

// parse basis set; get number of basis functions
void parse_basis(Atom *p_atom, Basis *p_basis, int use_5d);

// read the full basis set created by parse_basis
void read_basis(Atom *p_atom, Basis *p_basis, int use_5d);

// print the basis set
void print_basis(Basis *p_basis);

// calculate one-electron integrals
double calc_int_overlap(Basis *p_basis, int a, int b);
double calc_int_kinetic(Basis *p_basis, int a, int b);
double calc_int_nuc_attr(Basis *p_basis, int a, int b, Atom *p_atom);

// calculate two-electron integrals
double calc_int_eri_rys(Basis *p_basis, int a, int b, int c, int d);


// --- from cuda_rys_dp.h ---
/*****************************************************************************
 This file is part of the XLQC program.                                      
 Copyright (C) 2015 Xin Li <lixin.reco@gmail.com>                            
                                                                           
 Filename:  cuda_rys_dp.h                                                      
 License:   BSD 3-Clause License

 * The implementation of Rys quadrature routines in C is taken from the
 * PyQuante quantum chemistry program, Copyright (c) 2004, Richard P. Muller.
 * PyQuante version 1.2 and later is covered by the modified BSD license. 
 * Please see int_lib/LICENSE.
 
 This software is provided by the copyright holders and contributors "as is"
 and any express or implied warranties, including, but not limited to, the
 implied warranties of merchantability and fitness for a particular purpose are
 disclaimed. In no event shall the copyright holder or contributors be liable
 for any direct, indirect, incidental, special, exemplary, or consequential
 damages (including, but not limited to, procurement of substitute goods or
 services; loss of use, data, or profits; or business interruption) however
 caused and on any theory of liability, whether in contract, strict liability,
 or tort (including negligence or otherwise) arising in any way out of the use
 of this software, even if advised of the possibility of such damage.
 *****************************************************************************/

#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAXROOTS 7

void cuda_Roots_dp(int n, double X, double roots[], double weights[]);
void cuda_Root123_dp(int n, double X, double roots[], double weights[]);
void cuda_Root4_dp(double X, double roots[], double weights[]);
void cuda_Root5_dp(double X, double roots[], double weights[]);
void cuda_Root6_dp(int n,double X, double roots[], double weights[]);
double cuda_Int1d_dp(int i, int j,int k, int l,
                                double xi, double xj, double xk, double xl,
                                double alpha_ij_A, double alpha_kl_B, double sqrt_AB,
                                double A, double B, double Px, double Qx,
                                double inv_t1, double B00, double B1, double B1p, 
                                double G[][MAXROOTS]);
double cuda_rys_coulomb_repulsion_dp(double xa,double ya,double za,double norma,
                                                int la,int ma,int na,double alphaa,
                                                double xb,double yb,double zb,double normb,
                                                int lb,int mb,int nb,double alphab,
                                                double xc,double yc,double zc,double normc,
                                                int lc,int mc,int nc,double alphac,
                                                double xd,double yd,double zd,double normd,
                                                int ld,int md,int nd,double alphad);

double cuda_rys_pbf_dp(const double *ptr_i, const double *ptr_j, 
                                  const double *ptr_k, const double *ptr_l);

void cuda_mat_J_PI_dp(const double *pbf_xlec, const int *pbf_to_cbf, int n_pbf,
                                 const double *mat_D, double *mat_J_PI, const double *mat_Q);

void cuda_mat_K_PI_dp(const double *pbf_xlec, const int *pbf_to_cbf, int n_pbf,
                                 const double *mat_D, double *mat_K_PI, const double *mat_Q);


// --- from cuda_rys_sp.h ---
/*****************************************************************************
 This file is part of the XLQC program.                                      
 Copyright (C) 2015 Xin Li <lixin.reco@gmail.com>                            
                                                                           
 Filename:  cuda_rys_sp.h                                                      
 License:   BSD 3-Clause License

 * The implementation of Rys quadrature routines in C is taken from the
 * PyQuante quantum chemistry program, Copyright (c) 2004, Richard P. Muller.
 * PyQuante version 1.2 and later is covered by the modified BSD license. 
 * Please see int_lib/LICENSE.
 
 This software is provided by the copyright holders and contributors "as is"
 and any express or implied warranties, including, but not limited to, the
 implied warranties of merchantability and fitness for a particular purpose are
 disclaimed. In no event shall the copyright holder or contributors be liable
 for any direct, indirect, incidental, special, exemplary, or consequential
 damages (including, but not limited to, procurement of substitute goods or
 services; loss of use, data, or profits; or business interruption) however
 caused and on any theory of liability, whether in contract, strict liability,
 or tort (including negligence or otherwise) arising in any way out of the use
 of this software, even if advised of the possibility of such damage.
 *****************************************************************************/

#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAXROOTS 7

void my_cuda_safe(cudaError_t err, std::string word);

int cuda_ij2intindex(int i, int j);
int cuda_fact(int n);
int cuda_binomial(int a, int b);
void cuda_Roots(int n, float X, float roots[], float weights[]);
void cuda_Root123(int n, float X, float roots[], float weights[]);
void cuda_Root4(float X, float roots[], float weights[]);
void cuda_Root5(float X, float roots[], float weights[]);
void cuda_Root6(int n,float X, float roots[], float weights[]);
float cuda_Int1d(int i, int j,int k, int l,
                            float xi, float xj, float xk, float xl,
                            float alpha_ij_A, float alpha_kl_B, float sqrt_AB,
                            float A, float B, float Px, float Qx,
                            float inv_t1, float B00, float B1, float B1p, 
                            float G[][MAXROOTS]);

float cuda_rys_pbf(const double *ptr_i, const double *ptr_j, 
                              const double *ptr_k, const double *ptr_l);

void cuda_mat_J_PI(const double *pbf_xlec, const int *pbf_to_cbf, int n_pbf,
                              const double *mat_D, double *mat_J_PI, const double *mat_Q);

void cuda_mat_K_PI(const double *pbf_xlec, const int *pbf_to_cbf, int n_pbf,
                              const double *mat_D, double *mat_K_PI, const double *mat_Q);


// --- from scf.h ---
/*****************************************************************************
 This file is part of the XLQC program.                                      
 Copyright (C) 2015 Xin Li <lixin.reco@gmail.com>                            
                                                                           
 Filename:  scf.h                                                      
 License:   BSD 3-Clause License

 This software is provided by the copyright holders and contributors "as is"
 and any express or implied warranties, including, but not limited to, the
 implied warranties of merchantability and fitness for a particular purpose are
 disclaimed. In no event shall the copyright holder or contributors be liable
 for any direct, indirect, incidental, special, exemplary, or consequential
 damages (including, but not limited to, procurement of substitute goods or
 services; loss of use, data, or profits; or business interruption) however
 caused and on any theory of liability, whether in contract, strict liability,
 or tort (including negligence or otherwise) arising in any way out of the use
 of this software, even if advised of the possibility of such damage.
 *****************************************************************************/

// matrix inner product
double mat_inn_prod(int DIM, double **A, double **B);

// GSL eigen solver for real symmetric matrix
void my_eigen_symmv(gsl_matrix* data, int DIM,
                    gsl_vector* eval, gsl_matrix* evec);

// print gsl matrix
void my_print_matrix(gsl_matrix* A);

// print gsl vector
void my_print_vector(gsl_vector* x);

// get core Hamiltonian
void sum_H_core(int nbasis, gsl_matrix *H_core, gsl_matrix *T, gsl_matrix *V);

// diagonalize overlap matrix
void diag_overlap(int nbasis, gsl_matrix *S, gsl_matrix *S_invsqrt);

// from Fock matrix to MO coeffcients
void Fock_to_Coef(int nbasis, gsl_matrix *Fock, gsl_matrix *S_invsqrt, 
                  gsl_matrix *Coef, gsl_vector *emo);

// from MO coeffcients to density matrix
void Coef_to_Dens(int nbasis, int n_occ, gsl_matrix *Coef, gsl_matrix *D);

// compute the initial SCF energy
double get_elec_ene(int nbasis, gsl_matrix *D, gsl_matrix *H_core, 
                    gsl_matrix *Fock);

// form Fock matrix
void form_Fock(int nbasis, gsl_matrix *H_core, gsl_matrix *J, gsl_matrix *K, gsl_matrix *Fock);

// Generalized Wolfsberg-Helmholtz initial guess
void init_guess_GWH(Basis *p_basis, gsl_matrix *H_core, gsl_matrix *S, gsl_matrix *Fock);

// DIIS
void update_Fock_DIIS(int *p_diis_dim, int *p_diis_index, double *p_delta_DIIS, 
                      gsl_matrix *Fock, gsl_matrix *D_prev, gsl_matrix *S, Basis *p_basis,
                      double ***diis_err, double ***diis_Fock);


// --- from typedef.h ---
/*****************************************************************************
 This file is part of the XLQC program.                                      
 Copyright (C) 2015 Xin Li <lixin.reco@gmail.com>                            
                                                                           
 Filename:  typedef.h                                                      
 License:   BSD 3-Clause License

 This software is provided by the copyright holders and contributors "as is"
 and any express or implied warranties, including, but not limited to, the
 implied warranties of merchantability and fitness for a particular purpose are
 disclaimed. In no event shall the copyright holder or contributors be liable
 for any direct, indirect, incidental, special, exemplary, or consequential
 damages (including, but not limited to, procurement of substitute goods or
 services; loss of use, data, or profits; or business interruption) however
 caused and on any theory of liability, whether in contract, strict liability,
 or tort (including negligence or otherwise) arising in any way out of the use
 of this software, even if advised of the possibility of such damage.
 *****************************************************************************/

// Cartesian dimension
#define CART_DIM 3

#define BLOCKSIZE 8
#define SCREEN_THR 1.0e-16

// number of basis functions
#define N_S   1
#define N_SP  4
#define N_P   3
#define N_D   5
#define N_D_CART   6

// maximal number of DIIS error matrices
#define MAX_DIIS_DIM 6

// maximal length of string
#define MAX_STR_LEN 256

// PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// CODATA 2014: 
// 1 Hartree = 27.21138602 eV
// 1 Bohr = 0.52917721067 Angstrom
//#define HARTREE2EV 27.21138602
//#define BOHR2ANGSTROM 0.52917721067

// CODATA 2006
#define BOHR2ANGSTROM 0.5291772086
#define HARTREE2EV   27.2113839

// vector
typedef struct {
    double x, y, z;
} Vec_R;

// atomic name, position and nuclear charge
typedef struct {
    int num;
    char **name;
    double **pos;
    int *nuc_chg;
} Atom;

// basis set
typedef struct {
    int num;
    double **expon, **coef, **norm;
    double *xbas, *ybas, *zbas;
    int *nprims;
    int **lx, **ly, **lz;
} Basis;

