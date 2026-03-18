#include "kernel.h"

// --- from kernel.cu ---
extern "C"
void fasten_main(
    //    size_t posesPerWI,
    const size_t ntypes,
    const size_t nposes,
    const size_t natlig,
    const size_t natpro,
    const Atom *__restrict protein_molecule,
    const Atom *__restrict ligand_molecule,
    const float *__restrict transforms_0,
    const float *__restrict transforms_1,
    const float *__restrict transforms_2,
    const float *__restrict transforms_3,
    const float *__restrict transforms_4,
    const float *__restrict transforms_5,
    const FFParams *__restrict forcefield,
    float *__restrict etotals) 
{
    #pragma HLS INTERFACE s_axilite port=posesPerWI
    #pragma HLS INTERFACE s_axilite port=ntypes
    #pragma HLS INTERFACE s_axilite port=nposes
    #pragma HLS INTERFACE s_axilite port=natlig
    #pragma HLS INTERFACE s_axilite port=natpro
    #pragma HLS INTERFACE m_axi port=protein_molecule offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=ligand_molecule offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=transforms_0 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=transforms_1 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=transforms_2 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=transforms_3 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=transforms_4 offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=transforms_5 offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=forcefield offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=etotals offset=slave bundle=gmem9
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=local_forcefield complete dim=1

    #pragma HLS INTERFACE s_axilite port=posesPerWI
    #pragma HLS INTERFACE s_axilite port=ntypes
    #pragma HLS INTERFACE s_axilite port=nposes
    #pragma HLS INTERFACE s_axilite port=natlig
    #pragma HLS INTERFACE s_axilite port=natpro
    #pragma HLS INTERFACE m_axi port=protein_molecule offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=ligand_molecule offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=transforms_0 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=transforms_1 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=transforms_2 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=transforms_3 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=transforms_4 offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=transforms_5 offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=forcefield offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=etotals offset=slave bundle=gmem9
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=local_forcefield complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            FFParams local_forcefield[4096];
            const size_t lid = _tid_x;
            const size_t gid = _bid_x;
            const size_t lrange = BLOCK_DIM_X;

            float etot[NUM_TD_PER_THREAD];
            float3 lpos[NUM_TD_PER_THREAD];
            float4 transform[NUM_TD_PER_THREAD][3];

            size_t ix = gid * lrange * NUM_TD_PER_THREAD + lid;
            ix = ix < nposes ? ix : nposes - NUM_TD_PER_THREAD;

            for (int i = lid; i < ntypes; i += lrange) local_forcefield[i] = forcefield[i];

            // Compute transformation matrix to private memory
            for (size_t i = 0; i < NUM_TD_PER_THREAD; i++) {
            size_t index = ix + i * lrange;

            const float sx = sin(transforms_0[index]);
            const float cx = cos(transforms_0[index]);
            const float sy = sin(transforms_1[index]);
            const float cy = cos(transforms_1[index]);
            const float sz = sin(transforms_2[index]);
            const float cz = cos(transforms_2[index]);

            transform[i][0].x = cy * cz;
            transform[i][0].y = sx * sy * cz - cx * sz;
            transform[i][0].z = cx * sy * cz + sx * sz;
            transform[i][0].w = transforms_3[index];
            transform[i][1].x = cy * sz;
            transform[i][1].y = sx * sy * sz + cx * cz;
            transform[i][1].z = cx * sy * sz - sx * cz;
            transform[i][1].w = transforms_4[index];
            transform[i][2].x = -sy;
            transform[i][2].y = sx * cy;
            transform[i][2].z = cx * cy;
            transform[i][2].w = transforms_5[index];

            etot[i] = ZERO;
            }

            // Loop over ligand atoms
            size_t il = 0;
            do {
            // Load ligand atom data
            const Atom l_atom = ligand_molecule[il];
            const FFParams l_params = local_forcefield[l_atom.type];
            const bool lhphb_ltz = l_params.hphb < ZERO;
            const bool lhphb_gtz = l_params.hphb > ZERO;

            const float4 linitpos = float4(l_atom.x, l_atom.y, l_atom.z, ONE);
            for (size_t i = 0; i < NUM_TD_PER_THREAD; i++) {
            // Transform ligand atom
            lpos[i].x = transform[i][0].w +
            linitpos.x * transform[i][0].x +
            linitpos.y * transform[i][0].y +
            linitpos.z * transform[i][0].z;
            lpos[i].y = transform[i][1].w +
            linitpos.x * transform[i][1].x +
            linitpos.y * transform[i][1].y +
            linitpos.z * transform[i][1].z;
            lpos[i].z = transform[i][2].w +
            linitpos.x * transform[i][2].x +
            linitpos.y * transform[i][2].y +
            linitpos.z * transform[i][2].z;
            }

            // Loop over protein atoms
            size_t ip = 0;
            do {
            // Load protein atom data
            const Atom p_atom = protein_molecule[ip];
            const FFParams p_params = local_forcefield[p_atom.type];

            const float radij = p_params.radius + l_params.radius;
            const float r_radij = 1.f / (radij);

            const float elcdst = (p_params.hbtype == HBTYPE_F && l_params.hbtype == HBTYPE_F) ? FOUR : TWO;
            const float elcdst1 = (p_params.hbtype == HBTYPE_F && l_params.hbtype == HBTYPE_F) ? QUARTER : HALF;
            const bool type_E = ((p_params.hbtype == HBTYPE_E || l_params.hbtype == HBTYPE_E));

            const bool phphb_ltz = p_params.hphb < ZERO;
            const bool phphb_gtz = p_params.hphb > ZERO;
            const bool phphb_nz = p_params.hphb != ZERO;
            const float p_hphb = p_params.hphb * (phphb_ltz && lhphb_gtz ? -ONE : ONE);
            const float l_hphb = l_params.hphb * (phphb_gtz && lhphb_ltz ? -ONE : ONE);
            const float distdslv = (phphb_ltz ? (lhphb_ltz ? NPNPDIST : NPPDIST) : (lhphb_ltz ? NPPDIST : -FLT_MAX));
            const float r_distdslv = 1.f / (distdslv);

            const float chrg_init = l_params.elsc * p_params.elsc;
            const float dslv_init = p_hphb + l_hphb;

            for (size_t i = 0; i < NUM_TD_PER_THREAD; i++) {
            // Calculate distance between atoms
            const float x = lpos[i].x - p_atom.x;
            const float y = lpos[i].y - p_atom.y;
            const float z = lpos[i].z - p_atom.z;

            const float distij = sqrt(x * x + y * y + z * z);

            // Calculate the sum of the sphere radii
            const float distbb = distij - radij;
            const bool zone1 = (distbb < ZERO);

            // Calculate steric energy
            etot[i] += (ONE - (distij * r_radij)) * (zone1 ? 2 * HARDNESS : ZERO);

            // Calculate formal and dipole charge interactions
            float chrg_e = chrg_init * ((zone1 ? 1 : (ONE - distbb * elcdst1)) * (distbb < elcdst ? 1 : ZERO));
            const float neg_chrg_e = -fabs(chrg_e);
            chrg_e = type_E ? neg_chrg_e : chrg_e;
            etot[i] += chrg_e * CNSTNT;

            // Calculate the two cases for Nonpolar-Polar repulsive interactions
            const float coeff = (ONE - (distbb * r_distdslv));
            float dslv_e = dslv_init * ((distbb < distdslv && phphb_nz) ? 1 : ZERO);
            dslv_e *= (zone1 ? 1 : coeff);
            etot[i] += dslv_e;
            }
            } while (++ip < natpro); // loop over protein atoms
            } while (++il < natlig); // loop over ligand atoms

            // Write results
            const size_t td_base = gid * lrange * NUM_TD_PER_THREAD + lid;

            if (td_base < nposes) {
            for (size_t i = 0; i < NUM_TD_PER_THREAD; i++) {
            etotals[td_base + i * lrange] = etot[i] * HALF;
            }
            }

        }
    }
cdst ? 1 : ZERO));
            const float neg_chrg_e = -fabs(chrg_e);
            chrg_e = type_E ? neg_chrg_e : chrg_e;
            etot[i] += chrg_e * CNSTNT;

            // Calculate the two cases for Nonpolar-Polar repulsive interactions
            const float coeff = (ONE - (distbb * r_distdslv));
            float dslv_e = dslv_init * ((distbb < distdslv && phphb_nz) ? 1 : ZERO);
            dslv_e *= (zone1 ? 1 : coeff);
            etot[i] += dslv_e;
            }
            } while (++ip < natpro); // loop over protein atoms
            } while (++il < natlig); // loop over ligand atoms

            // Write results
            const size_t td_base = gid * lrange * NUM_TD_PER_THREAD + lid;

            if (td_base < nposes) {
            for (size_t i = 0; i < NUM_TD_PER_THREAD; i++) {
            etotals[td_base + i * lrange] = etot[i] * HALF;
            }
            }

        }
    }
}
