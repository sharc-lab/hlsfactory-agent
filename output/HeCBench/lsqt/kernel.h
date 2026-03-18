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

// --- from anderson.cu ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/


void Anderson::add_disorder(int N, std::mt19937& generator, real* potential)
{
  real W2 = disorder_strength * 0.5;
  std::uniform_real_distribution<real> on_site_potential(-W2, W2);
}


// --- from charge.cu ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <fstream>
#include <iostream>
#include <limits.h>

void Charge::add_impurities(
  std::mt19937& generator,
  int number_of_atoms,
  real box_length[3],
  int pbc[3],
  std::vector<real>& x,
  std::vector<real>& y,
  std::vector<real>& z,
  real* potential)
{
  rc = 5.0 * xi; // exp(-25/2) is of the order of 1.0e-6
  rc2 = rc * rc;
  impurity_indices.resize(Ni);
  impurity_strength.resize(Ni);
  find_impurity_indices(generator, number_of_atoms);
  find_impurity_strength(generator);
  find_cell_numbers(pbc, box_length);
  find_cell_contents(number_of_atoms, pbc, box_length, x, y, z);
  int* permuted_numbers = new int[max_value];
  std::uniform_int_distribution<int> rand_int(0, INT_MAX);
  delete[] permuted_numbers;
}

void Charge::find_impurity_strength(std::mt19937& generator)
{
  real W2 = W * 0.5;
  std::uniform_real_distribution<real> strength(-W2, W2);
}

void Charge::find_cell_numbers(int pbc[3], real box_length[3])
{
  if (pbc[0])
    Nx = floor(box_length[0] / rc);
  else
    Nx = 1;
  if (pbc[1])
    Ny = floor(box_length[1] / rc);
  else
    Ny = 1;
  if (pbc[2])
    Nz = floor(box_length[2] / rc);
  else
    Nz = 1;
  Nxyz = Nx * Ny * Nz;
}

void Charge::find_cell_contents(
  int N,
  int pbc[3],
  real box_length[3],
  std::vector<real>& x,
  std::vector<real>& y,
  std::vector<real>& z)
{
  cell_count.assign(Nxyz, 0);
  cell_count_sum.assign(Nxyz, 0);
  cell_count.assign(Nxyz, 0);
  cell_contents.assign(N, 0);
}


void Charge::find_potentials(
  int number_of_atoms,
  real box_length[3],
  int pbc[3],
  std::vector<real>& x,
  std::vector<real>& y,
  std::vector<real>& z,
  real* potential)
{
  real xi_factor = -0.5 / (xi * xi);
  real box_length_half[3];
  for (int d = 0; d < 3; ++d)
    box_length_half[d] = box_length[d] * 0.5;
  for (int n = 0; n < number_of_atoms; ++n)
    potential[n] = 0.0;
}

int Charge::find_cell_id(real x, real y, real z, real rc)
{
  int nx = floor(x / rc);
  int ny = floor(y / rc);
  int nz = floor(z / rc);
  while (nx < 0)
    nx += Nx;
  while (nx >= Nx)
    nx -= Nx;
  while (ny < 0)
    ny += Ny;
  while (ny >= Ny)
    ny -= Ny;
  while (nz < 0)
    nz += Nz;
  while (nz >= Nz)
    nz -= Nz;
  int nxyz = nx + Nx * ny + Nx * Ny * nz;
  return nxyz;
}

void Charge::find_cell_id(real x, real y, real z, real rc, int& nx, int& ny, int& nz, int& nxyz)
{
  nx = floor(x / rc);
  ny = floor(y / rc);
  nz = floor(z / rc);
  while (nx < 0)
    nx += Nx;
  while (nx >= Nx)
    nx -= Nx;
  while (ny < 0)
    ny += Ny;
  while (ny >= Ny)
    ny -= Ny;
  while (nz < 0)
    nz += Nz;
  while (nz >= Nz)
    nz -= Nz;
  nxyz = nx + Nx * ny + Nx * Ny * nz;
}

int Charge::find_neighbor_cell(int nx, int ny, int nz, int nxyz, int i, int j, int k)
{
  int neighbor = nxyz + k * Nx * Ny + j * Nx + i;
  if (nx + i < 0)
    neighbor += Nx;
  if (nx + i >= Nx)
    neighbor -= Nx;
  if (ny + j < 0)
    neighbor += Ny * Nx;
  if (ny + j >= Ny)
    neighbor -= Ny * Nx;
  if (nz + k < 0)
    neighbor += Nxyz;
  if (nz + k >= Nz)
    neighbor -= Nxyz;
  return neighbor;
}


// --- from hamiltonian.cu ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>    // memcpy
#define BLOCK_SIZE 256

#ifndef CPU_ONLY
void Hamiltonian::initialize_gpu(Model& model)
{
  n = model.number_of_atoms;
  max_neighbor = model.max_neighbor;
  energy_max = model.energy_max;
  grid_size = (model.number_of_atoms - 1) / BLOCK_SIZE + 1;

  CHECK(

  delete[] model.neighbor_number;

  delete[] model.potential;

  int* neighbor_list_new = new int[model.number_of_pairs];
  delete[] model.neighbor_list;

    neighbor_list, neighbor_list_new, sizeof(int) * model.number_of_pairs, cudaMemcpyHostToDevice));
  delete[] neighbor_list_new;

  real* hopping_real_new = new real[model.number_of_pairs];
  delete[] model.hopping_real;

    hopping_real, hopping_real_new, sizeof(real) * model.number_of_pairs, cudaMemcpyHostToDevice));
  delete[] hopping_real_new;

  real* hopping_imag_new = new real[model.number_of_pairs];
  delete[] model.hopping_imag;

    hopping_imag, hopping_imag_new, sizeof(real) * model.number_of_pairs, cudaMemcpyHostToDevice));
  delete[] hopping_imag_new;

  real* xx_new = new real[model.number_of_pairs];
  delete[] model.xx;

  delete[] xx_new;
}
#else
void Hamiltonian::initialize_cpu(Model& model)
{
  n = model.number_of_atoms;
  max_neighbor = model.max_neighbor;
  energy_max = model.energy_max;
  int number_of_pairs = model.number_of_pairs;

  neighbor_number = new int[n];
  memcpy(neighbor_number, model.neighbor_number, sizeof(int) * n);
  delete[] model.neighbor_number;

  neighbor_list = new int[number_of_pairs];
  memcpy(neighbor_list, model.neighbor_list, sizeof(int) * number_of_pairs);
  delete[] model.neighbor_list;

  potential = new real[n];
  memcpy(potential, model.potential, sizeof(real) * n);
  delete[] model.potential;

  hopping_real = new real[number_of_pairs];
  memcpy(hopping_real, model.hopping_real, sizeof(real) * number_of_pairs);
  delete[] model.hopping_real;

  hopping_imag = new real[number_of_pairs];
  memcpy(hopping_imag, model.hopping_imag, sizeof(real) * number_of_pairs);
  delete[] model.hopping_imag;

  xx = new real[number_of_pairs];
  memcpy(xx, model.xx, sizeof(real) * number_of_pairs);
  delete[] model.xx;
}
#endif

Hamiltonian::Hamiltonian(Model& model)
{
#ifndef CPU_ONLY
  initialize_gpu(model);
#else
#ifndef CPU_ONLY

#else
  delete[] neighbor_number;
  delete[] neighbor_list;
  delete[] potential;
  delete[] hopping_real;
  delete[] hopping_imag;
  delete[] xx;
#endif
}

#ifndef CPU_ONLY
#else
#endif

// |output> = H |input>
void Hamiltonian::apply(Vector& input, Vector& output)
{
#ifndef CPU_ONLY

    n, energy_max, neighbor_number, neighbor_list, potential, hopping_real, hopping_imag,
    input.real_part, input.imag_part, output.real_part, output.imag_part);

#else
  cpu_apply_hamiltonian(
    n, max_neighbor, energy_max, neighbor_number, neighbor_list, potential, hopping_real,
    hopping_imag, input.real_part, input.imag_part, output.real_part, output.imag_part);
#endif
}

#ifndef CPU_ONLY
#else
#endif

// |output> = [X, H] |input>
void Hamiltonian::apply_commutator(Vector& input, Vector& output)
{
#ifndef CPU_ONLY

    n, energy_max, neighbor_number, neighbor_list, hopping_real, hopping_imag, xx, input.real_part,
    input.imag_part, output.real_part, output.imag_part);

#else
  cpu_apply_commutator(
    n, max_neighbor, energy_max, neighbor_number, neighbor_list, hopping_real, hopping_imag, xx,
    input.real_part, input.imag_part, output.real_part, output.imag_part);
#endif
}

#ifndef CPU_ONLY
#else
#endif

// |output> = V |input>
void Hamiltonian::apply_current(Vector& input, Vector& output)
{
#ifndef CPU_ONLY

    n, neighbor_number, neighbor_list, hopping_real, hopping_imag, xx, input.real_part,
    input.imag_part, output.real_part, output.imag_part);

#else
  cpu_apply_current(
    n, max_neighbor, neighbor_number, neighbor_list, hopping_real, hopping_imag, xx,
    input.real_part, input.imag_part, output.real_part, output.imag_part);
#endif
}

// Kernel which calculates the two first terms of time evolution as described by
// Eq. (36) in [Comput. Phys. Commun.185, 28 (2014)].
#ifndef CPU_ONLY
#else
#endif

// Wrapper for the kernel above
void Hamiltonian::chebyshev_01(
  Vector& state_0, Vector& state_1, Vector& state, real bessel_0, real bessel_1, int direction)
{
#ifndef CPU_ONLY

    n, state_0.real_part, state_0.imag_part, state_1.real_part, state_1.imag_part, state.real_part,
    state.imag_part, bessel_0, bessel_1, direction);

#else
  cpu_chebyshev_01(
    n, state_0.real_part, state_0.imag_part, state_1.real_part, state_1.imag_part, state.real_part,
    state.imag_part, bessel_0, bessel_1, direction);
#endif
}

// Kernel for calculating further terms of Eq. (36)
// in [Comput. Phys. Commun.185, 28 (2014)].
#ifndef CPU_ONLY
#else
#endif

// Wrapper for the kernel above
void Hamiltonian::chebyshev_2(
  Vector& state_0, Vector& state_1, Vector& state_2, Vector& state, real bessel_m, int label)
{
#ifndef CPU_ONLY

    n, energy_max, neighbor_number, neighbor_list, potential, hopping_real, hopping_imag,
    state_0.real_part, state_0.imag_part, state_1.real_part, state_1.imag_part, state_2.real_part,
    state_2.imag_part, state.real_part, state.imag_part, bessel_m, label);

#else
  cpu_chebyshev_2(
    n, max_neighbor, energy_max, neighbor_number, neighbor_list, potential, hopping_real,
    hopping_imag, state_0.real_part, state_0.imag_part, state_1.real_part, state_1.imag_part,
    state_2.real_part, state_2.imag_part, state.real_part, state.imag_part, bessel_m, label);
#endif
}

// Kernel which calculates the two first terms of commutator [X, U(dt)]
// Corresponds to Eq. (37) in [Comput. Phys. Commun.185, 28 (2014)].
#ifndef CPU_ONLY
#else
#endif

// Wrapper for kernel above
void Hamiltonian::chebyshev_1x(Vector& input, Vector& output, real bessel_1)
{
#ifndef CPU_ONLY

    n, input.real_part, input.imag_part, output.real_part, output.imag_part, bessel_1);

#else
  cpu_chebyshev_1x(
    n, input.real_part, input.imag_part, output.real_part, output.imag_part, bessel_1);
#endif
}

// Kernel which calculates the further terms of [X, U(dt)]
#ifndef CPU_ONLY
void gpu_chebyshev_2x(
  const int number_of_atoms,
  const real energy_max,
  const  int*  g_neighbor_number,
  const  int*  g_neighbor_list,
  const real*  g_potential,
  const real*  g_hopping_real,
  const real*  g_hopping_imag,
  const real*  g_xx,
  const real*  g_state_0_real,
  const real*  g_state_0_imag,
  const real*  g_state_0x_real,
  const real*  g_state_0x_imag,
  const real*  g_state_1_real,
  const real*  g_state_1_imag,
  const real*  g_state_1x_real,
  const real*  g_state_1x_imag,
        real*  g_state_2_real,
        real*  g_state_2_imag,
        real*  g_state_2x_real,
        real*  g_state_2x_imag,
        real*  g_state_real,
        real*  g_state_imag,
  const real g_bessel_m,
  const int g_label)
{
  int n = _bid_x * BLOCK_DIM_X + _tid_x;
}
#else
void cpu_chebyshev_2x(
  int number_of_atoms,
  int max_neighbor,
  real energy_max,
  int* g_neighbor_number,
  int* g_neighbor_list,
  real* g_potential,
  real* g_hopping_real,
  real* g_hopping_imag,
  real* g_xx,
  real* g_state_0_real,
  real* g_state_0_imag,
  real* g_state_0x_real,
  real* g_state_0x_imag,
  real* g_state_1_real,
  real* g_state_1_imag,
  real* g_state_1x_real,
  real* g_state_1x_imag,
  real* g_state_2_real,
  real* g_state_2_imag,
  real* g_state_2x_real,
  real* g_state_2x_imag,
  real* g_state_real,
  real* g_state_imag,
  real g_bessel_m,
  int g_label)
{
}
#endif

// Wrapper for the kernel above
void Hamiltonian::chebyshev_2x(
  Vector& state_0,
  Vector& state_0x,
  Vector& state_1,
  Vector& state_1x,
  Vector& state_2,
  Vector& state_2x,
  Vector& state,
  real bessel_m,
  int label)
{
#ifndef CPU_ONLY

    n, energy_max, neighbor_number, neighbor_list, potential, hopping_real, hopping_imag, xx,
    state_0.real_part, state_0.imag_part, state_0x.real_part, state_0x.imag_part, state_1.real_part,
    state_1.imag_part, state_1x.real_part, state_1x.imag_part, state_2.real_part, state_2.imag_part,
    state_2x.real_part, state_2x.imag_part, state.real_part, state.imag_part, bessel_m, label);

#else
  cpu_chebyshev_2x(
    n, max_neighbor, energy_max, neighbor_number, neighbor_list, potential, hopping_real,
    hopping_imag, xx, state_0.real_part, state_0.imag_part, state_0x.real_part, state_0x.imag_part,
    state_1.real_part, state_1.imag_part, state_1x.real_part, state_1x.imag_part, state_2.real_part,
    state_2.imag_part, state_2x.real_part, state_2x.imag_part, state.real_part, state.imag_part,
    bessel_m, label);
#endif
}

// Kernel for doing the Chebyshev iteration phi_2 = 2 * H * phi_1 - phi_0.
#ifndef CPU_ONLY
#else
#endif

// Wrapper for the Chebyshev iteration
void Hamiltonian::kernel_polynomial(Vector& state_0, Vector& state_1, Vector& state_2)
{
#ifndef CPU_ONLY

    n, energy_max, neighbor_number, neighbor_list, potential, hopping_real, hopping_imag,
    state_0.real_part, state_0.imag_part, state_1.real_part, state_1.imag_part, state_2.real_part,
    state_2.imag_part);

#else
  cpu_kernel_polynomial(
    n, max_neighbor, energy_max, neighbor_number, neighbor_list, potential, hopping_real,
    hopping_imag, state_0.real_part, state_0.imag_part, state_1.real_part, state_1.imag_part,
    state_2.real_part, state_2.imag_part);
#endif
}


// --- from lsqt.cu ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

/*----------------------------------------------------------------------------80
    The driver function of LSQT
------------------------------------------------------------------------------*/

#include <iostream>













// --- from main.cu ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

/*----------------------------------------------------------------------------80
    The main function of the LSQT code
------------------------------------------------------------------------------*/

#include <fstream>
#include <iostream>

static void print_welcome();
static void check_argc(int);
static void print_start(std::string);






// --- from model.cu ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>

#define PI 3.141592653589793

Model::Model(std::string input_dir)
{
#ifdef DEBUG
  // use the same seed for different runs
  generator = std::mt19937(12345678);
#else
  // use different seeds for different runs
  generator = std::mt19937(std::chrono::system_clock::now().time_since_epoch().count());
#endif

  // determine the input directory
  this->input_dir = input_dir;

  // read in para.in
  {
    initialize_model_general();
  }

  // always need to read in energies
  initialize_energy();

  // only read in time steps when needed
  if (requires_time)
    initialize_time();

  // only read in local orbitals when needed
  if (calculate_ldos)
  // other memory will be freed when constructing the Hamiltonian
  delete[] energy;
  if (requires_time)
    delete[] time_step;
}

// This function is called by the lsqt function in the lsqt.cu file
// It initializes a random vector
void Model::initialize_state(Vector& random_state, int orbital)
{
  std::uniform_real_distribution<real> phase(0, 2 * PI);
  real* random_state_real = new real[number_of_atoms];
  real* random_state_imag = new real[number_of_atoms];

  {
  } else // normalize to N to keep spin degeneracy
  {
  }
  random_state.copy_from_host(random_state_real, random_state_imag);
  delete[] random_state_real;
  delete[] random_state_imag;
}

void Model::print_started_reading(std::string filename)
{
  std::cout << std::endl;
  std::cout << "===========================================================";
  std::cout << std::endl;
  std::cout << "Started reading " + filename << std::endl;
  std::cout << std::endl;
}

void Model::print_finished_reading(std::string filename)
{
  std::cout << std::endl;
  std::cout << "Finished reading " + filename << std::endl;
  std::cout << "===========================================================";
  std::cout << std::endl << std::endl;
}

void Model::verify_parameters()
{
  // determine whether or not we need to read in time steps
  if (calculate_vac || calculate_msd || calculate_spin)
    requires_time = true;

  // Verify the used parameters (make a seperate function later)

  std::cout << "- DOS will be calculated" << std::endl;

  if (calculate_vac0)
    std::cout << "- VAC0 will be calculated" << std::endl;
  else
    std::cout << "- VAC0 will not be calculated" << std::endl;

  if (calculate_vac)
    std::cout << "- VAC will be calculated" << std::endl;
  else
    std::cout << "- VAC will not be calculated" << std::endl;

  if (calculate_msd)
    std::cout << "- MSD will be calculated" << std::endl;
  else
    std::cout << "- MSD will not be calculated" << std::endl;

  if (calculate_spin)
    std::cout << "- spin polarization will be calculated" << std::endl;
  else
    std::cout << "- spin polarization will not be calculated" << std::endl;




  std::cout << "- Number of random vectors is " << number_of_random_vectors << std::endl;

  std::cout << "- Number of moments is " << number_of_moments << std::endl;

  std::cout << "- Energy maximum is " << energy_max << std::endl;
}

void Model::initialize_parameters()
{
  std::string filename = input_dir + "/para.in";
  print_started_reading(filename);

  std::string line;
  input.close();
  verify_parameters();
  std::string filename = input_dir + "/energy.in";

  print_started_reading(filename);

  input >> number_of_energy_points;
  std::cout << "- number of energy points = " << number_of_energy_points << std::endl;
  energy = new real[number_of_energy_points];


  input.close();

  std::string filename = input_dir + "/local_orbitals.in";
  print_started_reading(filename);

  input >> number_of_local_orbitals;
  std::cout << "- number of local orbitals = " << number_of_local_orbitals << std::endl;
  local_orbitals.resize(number_of_local_orbitals);


  input.close();
  std::string filename = input_dir + "/time_step.in";
  print_started_reading(filename);

  input >> number_of_steps_correlation;
  std::cout << "- number of time steps = " << number_of_steps_correlation << std::endl;
  time_step = new real[number_of_steps_correlation];


  input.close();
  print_finished_reading(filename);
}


// --- from model_general.cu ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <fstream>
#include <iostream>

void Model::initialize_model_general()
{
  initialize_neighbor();
  initialize_positions();
  initialize_potential();
  std::string filename = input_dir + "/neighbor.in";
  print_started_reading(filename);

  input >> number_of_atoms >> max_neighbor;
  number_of_pairs = number_of_atoms * max_neighbor;

  neighbor_number = new int[number_of_atoms];
  neighbor_list = new int[number_of_pairs];


  input.close();

  std::cout << "- Number of orbitals is " << number_of_atoms << std::endl;
  std::cout << "- Maximum neighbor number is " << max_neighbor << std::endl;
  if (d > box / 2.0)
    return d - box;
  if (d < -box / 2.0)
    return d + box;
  else
    return d;
}

void Model::initialize_positions()
{
  std::string filename = input_dir + "/position.in";
  print_started_reading(filename);

  real box;
  input >> box >> volume;
  real* x = new real[number_of_atoms];

  for (int i = 0; i < number_of_atoms; ++i)
    input >> x[i];
  input.close();

  std::cout << "- Box length along the transport direction is " << box << std::endl;
  std::cout << "- System volume is " << volume << std::endl;

  xx = new real[number_of_pairs];

  delete[] x;
  std::string filename = input_dir + "/potential.in";
  print_started_reading(filename);

  std::ifstream input(filename);
  bool nonzero_potential = true;

  potential = new real[number_of_atoms];


  input.close();

  std::string filename = input_dir + "/hopping.in";
  print_started_reading(filename);
  std::ifstream input(filename);

  // type == 1 : complex hoppings
  // type == 2 : real hoppings
  // type == 3 : uniform hoppings (hoppings.in is not read)
  int type = 0;


  hopping_real = new real[number_of_pairs];
  hopping_imag = new real[number_of_pairs];
  input.close();

  print_finished_reading(filename);
}


// --- from model_lattice.cu ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <fstream>
#include <iostream>
#include <limits.h>
#include <random>

void Model::create_random_numbers(int max_value, int total_number, int* random_numbers)
{
  int* permuted_numbers = new int[max_value];
  std::uniform_int_distribution<int> rand_int(0, INT_MAX);
  delete[] permuted_numbers;
}

void Model::specify_vacancies(int* is_vacancy, int number_of_atoms_pristine)
{
  int* vacancy_indices = new int[number_of_vacancies];
  delete[] vacancy_indices;
}

void Model::find_new_atom_index(int* is_vacancy, int* new_atom_index, int number_of_atoms_pristine)
{
  int count = 0;
}

void Model::add_vacancies()
{
  // copy some data
  int* neighbor_number_pristine = new int[number_of_atoms];
  int* neighbor_list_pristine = new int[number_of_pairs];
  real* hopping_real_pristine = new real[number_of_pairs];
  real* hopping_imag_pristine = new real[number_of_pairs];
  real* xx_pristine = new real[number_of_pairs];


  // change parameters
  int number_of_atoms_pristine = number_of_atoms;
  number_of_atoms = number_of_atoms_pristine - number_of_vacancies;
  number_of_pairs = number_of_atoms * max_neighbor;

  // delete old memory
  delete[] neighbor_number;
  delete[] neighbor_list;
  delete[] hopping_real;
  delete[] hopping_imag;
  delete[] xx;

  // allocate new memory
  neighbor_number = new int[number_of_atoms];
  neighbor_list = new int[number_of_pairs];
  hopping_real = new real[number_of_pairs];
  hopping_imag = new real[number_of_pairs];
  xx = new real[number_of_pairs];

  // specify the distribution of the vacancies
  int* is_vacancy = new int[number_of_atoms_pristine];
  specify_vacancies(is_vacancy, number_of_atoms_pristine);

  // find the new indices of the atoms
  int* new_atom_index = new int[number_of_atoms_pristine];
  find_new_atom_index(is_vacancy, new_atom_index, number_of_atoms_pristine);

  // get the new neighbor structure and related data
  int count_atom = 0;

  // free memory
  delete[] neighbor_number_pristine;
  delete[] neighbor_list_pristine;
  delete[] hopping_real_pristine;
  delete[] hopping_imag_pristine;
  delete[] xx_pristine;
  delete[] is_vacancy;
  delete[] new_atom_index;
}


void Model::initialize_lattice_model()
{
  std::string filename = input_dir + "/lattice.in";
  print_started_reading(filename);

  int N_orbital;
  int transport_direction;
  int N_cell[3];
  real lattice_constant[3];

  input >> N_cell[0] >> N_cell[1] >> N_cell[2];
  std::cout << "- Number of cells in the x direction = " << N_cell[0] << std::endl;
  std::cout << "- Number of cells in the y direction = " << N_cell[1] << std::endl;
  std::cout << "- Number of cells in the z direction = " << N_cell[2] << std::endl;

  input >> pbc[0] >> pbc[1] >> pbc[2] >> transport_direction;






  input >> lattice_constant[0] >> lattice_constant[1] >> lattice_constant[2];

  std::cout << "- lattice constant in x direction = " << lattice_constant[0] << std::endl;

  std::cout << "- lattice constant in y direction = " << lattice_constant[1] << std::endl;

  std::cout << "- lattice constant in z direction = " << lattice_constant[2] << std::endl;

  for (int d = 0; d < 3; ++d)
    box_length[d] = lattice_constant[d] * N_cell[d];
  volume = box_length[0] * box_length[1] * box_length[2];

  input >> N_orbital >> max_neighbor;
  std::cout << "- number of orbitals per cell = " << N_orbital << std::endl;

  std::cout << "- maximum number of hoppings per orbital = " << max_neighbor << std::endl;

  number_of_atoms = N_orbital * N_cell[0] * N_cell[1] * N_cell[2];
  std::cout << "- total number of orbitals = " << number_of_atoms << std::endl;

  number_of_pairs = number_of_atoms * max_neighbor;
  neighbor_number = new int[number_of_atoms];
  neighbor_list = new int[number_of_pairs];
  hopping_real = new real[number_of_pairs];
  hopping_imag = new real[number_of_pairs];
  xx = new real[number_of_pairs];
  potential = new real[number_of_atoms];

  // currently, I only need the positions in this case

  std::vector<real> x_cell, y_cell, z_cell;
  x_cell.resize(N_orbital);
  y_cell.resize(N_orbital);
  z_cell.resize(N_orbital);
  int number_of_hoppings_per_cell = N_orbital * max_neighbor;
  std::vector<std::vector<int>> hopping_index;
  hopping_index.assign(4, std::vector<int>(number_of_hoppings_per_cell, 0));
  std::vector<std::vector<real>> hopping_data;
  hopping_data.assign(2, std::vector<real>(number_of_hoppings_per_cell, 0));

  std::cout << std::endl << "\torbital\tx\ty\tz" << std::endl;

  std::vector<int> number_of_hoppings;
  number_of_hoppings.resize(N_orbital);





  print_finished_reading(filename);
}


// --- from sigma.cu ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <fstream>
#include <iostream>
#define BLOCK_SIZE 256
#define PI 3.141592653589793
//#define LORENTZ // Lorentz damping is not as good as Jackson damping

// Find the Chebyshev moments defined in Eqs. (32-34)
// in [Comput. Phys. Commun.185, 28 (2014)].
// See Algorithm 5 in [Comput. Phys. Commun.185, 28 (2014)].

// Jackson damping in Eq. (35) of [Comput. Phys. Commun.185, 28 (2014)].
#ifdef LORENTZ
#else
#endif

// Do the summation in Eqs. (29-31) in [Comput. Phys. Commun.185, 28 (2014)]

// Calculate:
// U(+t) |state> when direction = +1;
// U(-t) |state> when direction = -1.
// See Eq. (36) and Algorithm 6 in [Comput. Phys. Commun.185, 28 (2014)].

// Calculate:
// [X, U(+t)] |state> when direction = +1;
// [U(-t), X] |state> when direction = -1.
// See Eq. (37) and Algorithm 7 in [Comput. Phys. Commun.185, 28 (2014)].

// calculate the DOS as a function of Fermi energy
// See Algorithm 1 in [Comput. Phys. Commun.185, 28 (2014)].

// calculate the group velocity, which is sqrt{VAC(t=0)}
// as a function of Fermi energy

// calculate the VAC as a function of correlation time and Fermi energy
// See Algorithm 2 in [Comput. Phys. Commun.185, 28 (2014)].

// calculate the MSD as a function of correlation time and Fermi energy
// See Algorithm 3 in [Comput. Phys. Commun.185, 28 (2014)].

// calculate the spin polarization as a function of correlation time and
// Fermi energy. See Eq. (6) in [Phys. Rev. B 95, 041401(R) (2017)].


// --- from vector.cu ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>    // memcpy
#define BLOCK_SIZE 256

#ifndef CPU_ONLY
#else
#endif

#ifndef CPU_ONLY
void Vector::initialize_gpu(int n)
{
  this->n = n;
  array_size = n * sizeof(real);

}
#else
void Vector::initialize_cpu(int n)
{
  this->n = n;
  array_size = n * sizeof(real);
  real_part = new real[n];
  imag_part = new real[n];
}
#endif

Vector::Vector(int n)
{
#ifndef CPU_ONLY
  initialize_gpu(n);

#else
  initialize_cpu(n);
  int n = _bid_x * BLOCK_DIM_X + _tid_x;
}
#else
#endif

Vector::Vector(Vector& original)
{
  // Just teach myself: one can access private members of another instance
  // of the class from within the class
#ifndef CPU_ONLY
  initialize_gpu(original.n);

    n, original.real_part, original.imag_part, real_part, imag_part);

#else
  initialize_cpu(original.n);
#ifndef CPU_ONLY

#else
  delete[] real_part;
  delete[] imag_part;
#endif
}

#ifndef CPU_ONLY
#else
#endif

void Vector::add(Vector& other)
{
#ifndef CPU_ONLY

    n, other.real_part, other.imag_part, real_part, imag_part);

#else
#ifndef CPU_ONLY

    n, other.real_part, other.imag_part, real_part, imag_part);

#else
  int i = _bid_x * BLOCK_DIM_X + _tid_x;
}
#else
#endif

void Vector::apply_sz(Vector& other)
{
#ifndef CPU_ONLY

    n, other.real_part, other.imag_part, real_part, imag_part);

#else
#ifndef CPU_ONLY

#else
  memcpy(real_part, other_real, array_size);
#ifndef CPU_ONLY

#else
  memcpy(target_real, real_part, array_size);
  real* tmp_real = real_part;
  real* tmp_imag = imag_part;
  real_part = other.real_part, imag_part = other.imag_part;
  other.real_part = tmp_real;
  other.imag_part = tmp_imag;
}

#ifndef CPU_ONLY
#endif

#ifndef CPU_ONLY
#else
#endif

void Vector::inner_product_1(int number_of_atoms, Vector& other, Vector& target, int offset)
{
  int grid_size = (number_of_atoms - 1) / BLOCK_SIZE + 1;
#ifndef CPU_ONLY

    number_of_atoms, real_part, imag_part, other.real_part, other.imag_part, target.real_part,
    target.imag_part, offset);

#else
  cpu_find_inner_product_1(
    grid_size, number_of_atoms, real_part, imag_part, other.real_part, other.imag_part,
    target.real_part, target.imag_part, offset);
#endif
}

#ifndef CPU_ONLY
#else
#endif

void Vector::inner_product_2(int number_of_atoms, int number_of_moments, Vector& target)
{
#ifndef CPU_ONLY

    number_of_atoms, real_part, imag_part, target.real_part, target.imag_part);

#else
  int grid_size = (number_of_atoms - 1) / BLOCK_SIZE + 1;
  cpu_find_inner_product_2(
    number_of_moments, grid_size, real_part, imag_part, target.real_part, target.imag_part);
#endif
}


// --- from anderson.h ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <random>

class Anderson
{
public:
  void add_disorder(int N, std::mt19937& generator, real* potential);
  bool has_disorder = false;
  real disorder_strength;
};


// --- from charge.h ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <random>

class Charge
{
public:
  void add_impurities(
    std::mt19937&,
    int,
    real*,
    int*,
    std::vector<real>&,
    std::vector<real>&,
    std::vector<real>&,
    real*);
  bool has = false;
  int Ni;  // number of impurities
  real W;  // impurity strength
  real xi; // impurity range
private:
  int Nx, Ny, Nz, Nxyz; // number of cells
  real rc;              // cutoff distance for impurity potential
  real rc2;             // cutoff square
  std::vector<int> cell_count;
  std::vector<int> cell_count_sum;
  std::vector<int> cell_contents;
  std::vector<int> impurity_indices;
  std::vector<real> impurity_strength;
  void find_impurity_indices(std::mt19937&, int);
  void find_impurity_strength(std::mt19937&);
  void find_potentials(
    int, real*, int*, std::vector<real>&, std::vector<real>&, std::vector<real>&, real*);
  int find_cell_id(real, real, real, real);
  void find_cell_id(real, real, real, real, int&, int&, int&, int&);
  void find_cell_numbers(int*, real*);
  void
  find_cell_contents(int, int*, real*, std::vector<real>&, std::vector<real>&, std::vector<real>&);
  int find_neighbor_cell(int, int, int, int, int, int, int);
};


// --- from common.h ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifdef USE_SP
typedef float real; // single precision
#else
typedef double real; // double precision
#endif

#ifndef CPU_ONLY
#include <stdio.h>

#define CHECK(call)                                                                                \
  do {                                                                                             \
    const cudaError_t error_code = call;                                                           \
    if (error_code != cudaSuccess) {                                                               \
      printf("CUDA Error:\n");                                                                     \
      printf("    File:       %s\n", __FILE__);                                                    \
      printf("    Line:       %d\n", __LINE__);                                                    \
      printf("    Error code: %d\n", error_code);                                                  \
      printf("    Error text: %s\n", cudaGetErrorString(error_code));                              \
      exit(1);                                                                                     \
    }                                                                                              \
  } while (0)

#endif // #ifndef CPU_ONLY


// --- from hamiltonian.h ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
class Vector;
class Model;

class Hamiltonian
{
public:
  Hamiltonian(Model&);
  ~Hamiltonian();

  void apply(Vector&, Vector&);
  void apply_commutator(Vector&, Vector&);
  void apply_current(Vector&, Vector&);
  void kernel_polynomial(Vector&, Vector&, Vector&);
  void chebyshev_01(Vector&, Vector&, Vector&, real, real, int);
  void chebyshev_2(Vector&, Vector&, Vector&, Vector&, real, int);
  void chebyshev_1x(Vector&, Vector&, real);
  void chebyshev_2x(Vector&, Vector&, Vector&, Vector&, Vector&, Vector&, Vector&, real, int);

private:
  void initialize_gpu(Model&);
  void initialize_cpu(Model&);

  int* neighbor_number;
  int* neighbor_list;
  real* potential;
  real* hopping_real;
  real* hopping_imag;
  real* xx;
  int grid_size;
  int n;
  int max_neighbor;
  real energy_max;
};


// --- from lsqt.h ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <chrono>
#include <string>

void lsqt(std::string input_directory);


// --- from model.h ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <random>
class Vector;

class Model
{
public:
  Model(std::string input_dir);
  ~Model();
  void initialize_state(Vector& random_state, int orbital);

  bool calculate_vac0 = false;
  bool calculate_vac = false;
  bool calculate_msd = false;
  bool calculate_spin = false;
  bool calculate_ldos = false;

  int number_of_random_vectors = 1;
  int number_of_atoms = 0;
  int max_neighbor = 0;
  int number_of_pairs = 0;
  int number_of_energy_points = 0;
  int number_of_moments = 1000;
  int number_of_steps_correlation = 0;
  int number_of_local_orbitals = 0;
  std::string input_dir;
  real energy_max = 10;

  real* energy;
  real* time_step;
  std::vector<int> local_orbitals;

  int* neighbor_number;
  int* neighbor_list;
  real* xx;
  real* potential;
  real* hopping_real;
  real* hopping_imag;

  real volume;

private:
  void print_started_reading(std::string filename);
  void print_finished_reading(std::string filename);

  // for both lattice and general models
  void initialize_parameters();
  void verify_parameters();
  void initialize_energy();
  void initialize_time();
  void initialize_local_orbitals();

  // only for general model
  void initialize_neighbor();
  void initialize_positions();
  void initialize_potential();
  void initialize_hopping();
  void initialize_model_general();

  // only for lattice model
  void initialize_lattice_model();
  void add_vacancies();
  void create_random_numbers(int, int, int*);
  void specify_vacancies(int*, int);
  void find_new_atom_index(int*, int*, int);

  bool requires_time = false;
  bool use_lattice_model = false;

  // disorder
  Anderson anderson;
  Charge charge;

  bool has_vacancy_disorder = false;
  int number_of_vacancies;

  int pbc[3];
  real box_length[3];
  std::vector<real> x, y, z;

  std::mt19937 generator;
};


// --- from sigma.h ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
class Model;
class Hamiltonian;
class Vector;

void find_dos(Model&, Hamiltonian&, Vector&, int);
void find_vac0(Model&, Hamiltonian&, Vector&);
void find_vac(Model&, Hamiltonian&, Vector&);
void find_msd(Model&, Hamiltonian&, Vector&);
void find_spin_polarization(Model&, Hamiltonian&, Vector&);


// --- from vector.h ---
/*
    Copyright 2017 Zheyong Fan, Ville Vierimaa, and Ari Harju

    This file is part of GPUQT.

    GPUQT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GPUQT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GPUQT.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

class Vector
{
public:
  Vector(int n);
  Vector(Vector& original);
  ~Vector();

  void add(Vector& other);
  void copy(Vector& other);
  void apply_sz(Vector& other);
  void copy_from_host(real* other_real, real* other_imag);
  void copy_to_host(real* target_real, real* target_imag);
  void swap(Vector& other);
  void inner_product_1(int, Vector& other, Vector& target, int offset);
  void inner_product_2(int, int, Vector& target);

  real* real_part;
  real* imag_part;

private:
  void initialize_gpu(int n);
  void initialize_cpu(int n);
  int n;
  int array_size;
};
