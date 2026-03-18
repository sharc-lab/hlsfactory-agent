#include "kernel.h"

// --- from Simulation.cu ---
extern "C"
void lookup (
    const int * num_nucs,
    const double * concs,
    const int * mats,
    const NuclideGridPoint * nuclide_grid,
    int*  verification,
    const double * unionized_energy_array,
    const int * index_grid,
    const int n_lookups,
    const long n_isotopes,
    const long n_gridpoints,
    const int grid_type,
    const int hash_bins,
    const int max_num_nucs ) {
    #pragma HLS INTERFACE m_axi port=num_nucs offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=concs offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=mats offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=nuclide_grid offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=verification offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=unionized_energy_array offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=index_grid offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=n_lookups
    #pragma HLS INTERFACE s_axilite port=n_isotopes
    #pragma HLS INTERFACE s_axilite port=n_gridpoints
    #pragma HLS INTERFACE s_axilite port=grid_type
    #pragma HLS INTERFACE s_axilite port=hash_bins
    #pragma HLS INTERFACE s_axilite port=max_num_nucs
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            // get the index to operate on, first dimemsion
            int i = _tid_x + _bid_x * BLOCK_DIM_X;

            if (i < n_lookups) {

            // Set the initial seed value
            uint64_t seed = STARTING_SEED;

            // Forward seed to lookup index (we need 2 samples per lookup)
            seed = fast_forward_LCG(seed, 2*i);

            // Randomly pick an energy and material for the particle
            double p_energy = LCG_random_double(&seed);
            int mat         = pick_mat(&seed);

            // debugging
            //printf("E = %lf mat = %d\n", p_energy, mat);

            double macro_xs_vector[5] = {0};

            // Perform macroscopic Cross Section Lookup
            calculate_macro_xs(
            p_energy,     // Sampled neutron energy (in lethargy)
            mat,          // Sampled material type index neutron is in
            n_isotopes,   // Total number of isotopes in simulation
            n_gridpoints, // Number of gridpoints per isotope in simulation
            num_nucs,     // 1-D array with number of nuclides per material
            concs,        // Flattened 2-D array with concentration of each nuclide in each material
            unionized_energy_array, // 1-D Unionized energy array
            index_grid,   // Flattened 2-D grid holding indices into nuclide grid for each unionized energy level
            nuclide_grid, // Flattened 2-D grid holding energy levels and XS_data for all nuclides in simulation
            mats,         // Flattened 2-D array with nuclide indices defining composition of each type of material
            macro_xs_vector, // 1-D array with result of the macroscopic cross section (5 different reaction channels)
            grid_type,    // Lookup type (nuclide, hash, or unionized)
            hash_bins,    // Number of hash bins used (if using hash lookup type)
            max_num_nucs  // Maximum number of nuclides present in any material
            );

            // For verification, and to prevent the compiler from optimizing
            // all work out, we interrogate the returned macro_xs_vector array
            // to find its maximum value index, then increment the verification
            // value by that index. In this implementation, we store to a global
            // array that will get tranferred back and reduced on the host.
            double max = -1.0;
            int max_idx = 0;
            for(int j = 0; j < 5; j++ )
            {
            if( macro_xs_vector[j] > max )
            {
            max = macro_xs_vector[j];
            max_idx = j;
            }
            }
            verification[i] = max_idx+1;
            }

        }
    }
}

long grid_search( long n, double quarry, T A)
{
  long lowerLimit = 0;
  long upperLimit = n-1;
  long examinationPoint;
  long length = upperLimit - lowerLimit;

  while( length > 1 )
  {
    examinationPoint = lowerLimit + ( length / 2 );

    if( A[examinationPoint] > quarry )
      upperLimit = examinationPoint;
    else
      lowerLimit = examinationPoint;

    length = upperLimit - lowerLimit;
  }

  return lowerLimit;
}

void calculate_micro_xs(   double p_energy, int nuc, long n_isotopes,
    long n_gridpoints,
    Double_Type  egrid, Int_Type  index_data,
    NGP_Type  nuclide_grids,
    long idx, double *  xs_vector, int grid_type, int hash_bins ){
  // Variables
  double f;
  NuclideGridPoint low, high;
  long low_idx, high_idx;

  // If using only the nuclide grid, we must perform a binary search
  // to find the energy location in this particular nuclide's grid.
  if( grid_type == NUCLIDE )
  {
    // Perform binary search on the Nuclide Grid to find the index
    long offset = nuc * n_gridpoints;
    idx = grid_search_nuclide( n_gridpoints, p_energy, nuclide_grids, offset, offset + n_gridpoints-1);

    // pull ptr from nuclide grid and check to ensure that
    // we're not reading off the end of the nuclide's grid
    if( idx == n_gridpoints - 1 )
      low_idx = idx - 1;
    else
      low_idx = idx;
  }
  else if( grid_type == UNIONIZED) // Unionized Energy Grid - we already know the index, no binary search needed.
  {
    // pull ptr from energy grid and check to ensure that
    // we're not reading off the end of the nuclide's grid
    if( index_data[idx * n_isotopes + nuc] == n_gridpoints - 1 )
      low_idx = nuc*n_gridpoints + index_data[idx * n_isotopes + nuc] - 1;
    else
    {
      low_idx = nuc*n_gridpoints + index_data[idx * n_isotopes + nuc];
    }
  }
  else // Hash grid
  {
    // load lower bounding index
    int u_low = index_data[idx * n_isotopes + nuc];

    // Determine higher bounding index
    int u_high;
    if( idx == hash_bins - 1 )
      u_high = n_gridpoints - 1;
    else
      u_high = index_data[(idx+1)*n_isotopes + nuc] + 1;

    // Check edge cases to make sure energy is actually between these
    // Then, if things look good, search for gridpoint in the nuclide grid
    // within the lower and higher limits we've calculated.
    double e_low  = nuclide_grids[nuc*n_gridpoints + u_low].energy;
    double e_high = nuclide_grids[nuc*n_gridpoints + u_high].energy;
    long lower;
    if( p_energy <= e_low )
      lower = nuc*n_gridpoints;
    else if( p_energy >= e_high )
      lower = nuc*n_gridpoints + n_gridpoints - 1;
    else
    {
      long offset = nuc*n_gridpoints;
      lower = grid_search_nuclide( n_gridpoints, p_energy, nuclide_grids, offset+u_low, offset+u_high);
    }

    if( (lower % n_gridpoints) == n_gridpoints - 1 )
      low_idx = lower - 1;
    else
      low_idx = lower;
  }

  high_idx = low_idx + 1;
  low = nuclide_grids[low_idx];
  high = nuclide_grids[high_idx];

  // calculate the re-useable interpolation factor
  f = (high.energy - p_energy) / (high.energy - low.energy);

  // Total XS
  xs_vector[0] = high.total_xs - f * (high.total_xs - low.total_xs);

  // Elastic XS
  xs_vector[1] = high.elastic_xs - f * (high.elastic_xs - low.elastic_xs);

  // Absorbtion XS
  xs_vector[2] = high.absorbtion_xs - f * (high.absorbtion_xs - low.absorbtion_xs);

  // Fission XS
  xs_vector[3] = high.fission_xs - f * (high.fission_xs - low.fission_xs);

  // Nu Fission XS
  xs_vector[4] = high.nu_fission_xs - f * (high.nu_fission_xs - low.nu_fission_xs);
}

void calculate_macro_xs( double p_energy, int mat, long n_isotopes,
    long n_gridpoints, Int_Type  num_nucs,
    Double_Type  concs,
    E_GRID_TYPE  egrid, INDEX_TYPE  index_data,
    NGP_Type  nuclide_grids,
    Int_Type  mats,
    double * macro_xs_vector, int grid_type, int hash_bins, int max_num_nucs ){
  int p_nuc; // the nuclide we are looking up
  long idx = -1;
  double conc; // the concentration of the nuclide in the material

  // cleans out macro_xs_vector
  for( int k = 0; k < 5; k++ )
    macro_xs_vector[k] = 0;

  // If we are using the unionized energy grid (UEG), we only
  // need to perform 1 binary search per macroscopic lookup.
  // If we are using the nuclide grid search, it will have to be
  // done inside of the "calculate_micro_xs" function for each different
  // nuclide in the material.
  if( grid_type == UNIONIZED )
    idx = grid_search( n_isotopes * n_gridpoints, p_energy, egrid);
  else if( grid_type == HASH )
  {
    double du = 1.0 / hash_bins;
    idx = p_energy / du;
  }

  // Once we find the pointer array on the UEG, we can pull the data
  // from the respective nuclide grids, as well as the nuclide
  // concentration data for the material
  // Each nuclide from the material needs to have its micro-XS array
  // looked up & interpolatied (via calculate_micro_xs). Then, the
  // micro XS is multiplied by the concentration of that nuclide
  // in the material, and added to the total macro XS array.
  // (Independent -- though if parallelizing, must use atomic operations
  //  or otherwise control access to the xs_vector and macro_xs_vector to
  //  avoid simulataneous writing to the same data structure)
  for( int j = 0; j < num_nucs[mat]; j++ )
  {
    double xs_vector[5];
    p_nuc = mats[mat*max_num_nucs + j];
    conc = concs[mat*max_num_nucs + j];
    calculate_micro_xs( p_energy, p_nuc, n_isotopes,
        n_gridpoints, egrid, index_data,
        nuclide_grids, idx, xs_vector, grid_type, hash_bins );
    for( int k = 0; k < 5; k++ )
      macro_xs_vector[k] += xs_vector[k] * conc;
  }
}

int pick_mat( unsigned long * seed )
{
  // I have a nice spreadsheet supporting these numbers. They are
  // the fractions (by volume) of material in the core. Not a
  // *perfect* approximation of where XS lookups are going to occur,
  // but this will do a good job of biasing the system nonetheless.

  // Also could be argued that doing fractions by weight would be
  // a better approximation, but volume does a good enough job for now.

  double dist[12];
  dist[0]  = 0.140;  // fuel
  dist[1]  = 0.052;  // cladding
  dist[2]  = 0.275;  // cold, borated water
  dist[3]  = 0.134;  // hot, borated water
  dist[4]  = 0.154;  // RPV
  dist[5]  = 0.064;  // Lower, radial reflector
  dist[6]  = 0.066;  // Upper reflector / top plate
  dist[7]  = 0.055;  // bottom plate
  dist[8]  = 0.008;  // bottom nozzle
  dist[9]  = 0.015;  // top nozzle
  dist[10] = 0.025;  // top of fuel assemblies
  dist[11] = 0.013;  // bottom of fuel assemblies

  double roll = LCG_random_double(seed);

  // makes a pick based on the distro
  for( int i = 0; i < 12; i++ )
  {
    double running = 0;
    for( int j = i; j > 0; j-- )
      running += dist[j];
    if( roll < running )
      return i;
  }

  return 0;
}

double LCG_random_double(uint64_t * seed)
{
  // LCG parameters
  const uint64_t m = 9223372036854775808ULL; // 2^63
  const uint64_t a = 2806196910506780709ULL;
  const uint64_t c = 1ULL;
  *seed = (a * (*seed) + c) % m;
  return (double) (*seed) / (double) m;
}

uint64_t fast_forward_LCG(uint64_t seed, uint64_t n)
{
  // LCG parameters
  const uint64_t m = 9223372036854775808ULL; // 2^63
  uint64_t a = 2806196910506780709ULL;
  uint64_t c = 1ULL;

  n = n % m;

  uint64_t a_new = 1;
  uint64_t c_new = 0;

  while(n > 0)
  {
    if(n & 1)
    {
      a_new *= a;
      c_new = c_new * a + c;
    }
    c *= (a + 1);
    a *= a;

    n >>= 1;
  }

  return (a_new * seed + c_new) % m;
}
