#include "kernel.h"

// --- from main.cu ---
double LCG_random_double(uint64_t * seed)
{
  const uint64_t m = 9223372036854775808ULL; // 2^63
  const uint64_t a = 2806196910506780709ULL;
  const uint64_t c = 1ULL;
  *seed = (a * (*seed) + c) % m;
  return (double) (*seed) / (double) m;
}

double compute(double x, double y, double z,
    const StructureAtom *  structureAtoms,
    double natoms, double L) 
{
  // (x, y, z) : Cartesian coords of methane molecule
  // structureAtoms : pointer array storing info on unit cell of crystal structure
  // natoms : number of atoms in crystal structure
  // L : box length
  // returns Boltzmann factor e^{-E/(RT)}
  double E = 0.0;  // energy (K)

  // loop over atoms in crystal structure
  for (int i = 0; i < natoms; i++) {
    // Compute distance in each coordinate from (x, y, z) to this structure atom
    double dx = x - structureAtoms[i].x;
    double dy = y - structureAtoms[i].y;
    double dz = z - structureAtoms[i].z;

    // apply nearest image convention for periodic boundary conditions
    const double boxupper = 0.5 * L;
    const double boxlower = -boxupper;

    dx = (dx >  boxupper) ? dx-L : dx;
    dx = (dx >  boxupper) ? dx-L : dx;
    dy = (dy >  boxupper) ? dy-L : dy;
    dy = (dy <= boxlower) ? dy-L : dy;
    dz = (dz <= boxlower) ? dz-L : dz;
    dz = (dz <= boxlower) ? dz-L : dz;

    // compute inverse distance
    double rinv = 1.0 / sqrt(dx*dx + dy*dy + dz*dz);

    // Compute contribution to energy of adsorbate at (x, y, z) due to this atom
    // Lennard-Jones potential (this is the efficient way to compute it)
    double sig_ovr_r = rinv * structureAtoms[i].sigma;
    double sig_ovr_r6 = pow(sig_ovr_r, 6);
    double sig_ovr_r12 = sig_ovr_r6 * sig_ovr_r6;
    E += 4.0 * structureAtoms[i].epsilon * (sig_ovr_r12 - sig_ovr_r6);
  }
  return exp(-E / (R * T));  // return Boltzmann factor
}
extern "C"

void insertions(
    double * boltzmannFactors, 
    const StructureAtom *  structureAtoms, 
    int natoms, double L) 
{
    #pragma HLS INTERFACE m_axi port=boltzmannFactors offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=structureAtoms offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=natoms
    #pragma HLS INTERFACE s_axilite port=L
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            // boltzmannFactors : pointer array in which to store computed Boltzmann factors
            // structureAtoms : pointer array storing atoms in unit cell of crystal structure
            // natoms : number of atoms in crystal structure
            // L : box length
            int id = _tid_x + _bid_x * NUMTHREADS;

            // random seed for each thread
            uint64_t seed = id;

            // Generate random position inside the cubic unit cell of the structure
            double x = L * LCG_random_double(&seed);
            double y = L * LCG_random_double(&seed);
            double z = L * LCG_random_double(&seed);

            // Compute Boltzmann factor, store in boltzmannFactors
            boltzmannFactors[id] = compute(x, y, z, structureAtoms, natoms, L);

        }
    }
}
