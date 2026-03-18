#include "kernel.h"

// --- from motionsim.cu ---
extern "C"
void Simulation(float* a_particleX,
                float* a_particleY,
		const float* a_randomX,
                const float* a_randomY, 
		size_t * a_map,
                const size_t n_particles,
                unsigned int nIterations,
                int grid_size,
                float radius)
{
    #pragma HLS INTERFACE m_axi port=a_particleX offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=a_particleY offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=a_randomX offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=a_randomY offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=a_map offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=n_particles
    #pragma HLS INTERFACE s_axilite port=nIterations
    #pragma HLS INTERFACE s_axilite port=grid_size
    #pragma HLS INTERFACE s_axilite port=radius
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t ii = BLOCK_DIM_X * _bid_x + _tid_x;
            if (ii >= n_particles) return;
            // Start iterations
            // Each iteration:
            //  1. Updates the position of all water molecules
            //  2. Checks if water molecule is inside a cell or not.
            //  3. Updates counter in cells array
            size_t iter = 0;
            float pX = a_particleX[ii];
            float pY = a_particleY[ii];
            size_t map_base = ii * grid_size * grid_size;
            while (iter < nIterations) {
            // Computes random displacement for each molecule
            // This example shows random distances between
            // -0.05 units and 0.05 units in both X and Y directions
            // Moves each water molecule by a random vector

            float randnumX = a_randomX[iter * n_particles + ii];
            float randnumY = a_randomY[iter * n_particles + ii];

            // Transform the scaled random numbers into small displacements
            float displacementX = randnumX / 1000.0f - 0.0495f;
            float displacementY = randnumY / 1000.0f - 0.0495f;

            // Move particles using random displacements
            pX += displacementX;
            pY += displacementY;

            // Compute distances from particle position to grid point
            float dX = pX - truncf(pX);
            float dY = pY - truncf(pY);

            // Compute grid point indices
            int iX = floorf(pX);
            int iY = floorf(pY);

            // Check if particle is still in computation grid
            if ((pX < grid_size) && (pY < grid_size) && (pX >= 0) && (pY >= 0)) {
            // Check if particle is (or remained) inside cell.
            // Increment cell counter in map array if so
            if ((dX * dX + dY * dY <= radius * radius))
            // The map array is organized as (particle, y, x)
            a_map[map_base + iY * grid_size + iX]++;
            }

            iter++;

            }  // Next iteration

            a_particleX[ii] = pX;
            a_particleY[ii] = pY;

        }
    }
}
