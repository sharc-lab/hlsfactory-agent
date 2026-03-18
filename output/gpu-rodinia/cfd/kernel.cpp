#include "kernel.h"

// --- from euler3d.cu ---
extern "C"
void cuda_initialize_variables(int nelr, float* variables)
{
    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = (BLOCK_DIM_X*_bid_x + _tid_x);
            for(int j = 0; j < NVAR; j++)
            variables[i + j*nelr] = ff_variable[j];

        }
    }
{
        #pragma HLS PIPELINE II=1

            const int i = (BLOCK_DIM_X*_bid_x + _tid_x);
            for(int j = 0; j < NVAR; j++)
            variables[i + j*nelr] = ff_variable[j];

        }
    }
{
        #pragma HLS PIPELINE II=1

            const int i = (BLOCK_DIM_X*_bid_x + _tid_x);
            for(int j = 0; j < NVAR; j++)
            variables[i + j*nelr] = ff_variable[j];

        }
    }
{
        #pragma HLS PIPELINE II=1

            const int i = (BLOCK_DIM_X*_bid_x + _tid_x);
            for(int j = 0; j < NVAR; j++)
            variables[i + j*nelr] = ff_variable[j];

        }
    }
}

inline void compute_flux_contribution(float& density, float3& momentum, float& density_energy, float& pressure, float3& velocity, float3& fc_momentum_x, float3& fc_momentum_y, float3& fc_momentum_z, float3& fc_density_energy)
{
	fc_momentum_x.x = velocity.x*momentum.x + pressure;
	fc_momentum_x.y = velocity.x*momentum.y;
	fc_momentum_x.z = velocity.x*momentum.z;
	
	
	fc_momentum_y.x = fc_momentum_x.y;
	fc_momentum_y.y = velocity.y*momentum.y + pressure;
	fc_momentum_y.z = velocity.y*momentum.z;

	fc_momentum_z.x = fc_momentum_x.z;
	fc_momentum_z.y = fc_momentum_y.z;
	fc_momentum_z.z = velocity.z*momentum.z + pressure;

	float de_p = density_energy+pressure;
	fc_density_energy.x = velocity.x*de_p;
	fc_density_energy.y = velocity.y*de_p;
	fc_density_energy.z = velocity.z*de_p;
}

inline void compute_velocity(float& density, float3& momentum, float3& velocity)
{
	velocity.x = momentum.x / density;
	velocity.y = momentum.y / density;
	velocity.z = momentum.z / density;
}

inline float compute_speed_sqd(float3& velocity)
{
	return velocity.x*velocity.x + velocity.y*velocity.y + velocity.z*velocity.z;
}

inline float compute_pressure(float& density, float& density_energy, float& speed_sqd)
{
	return (float(GAMMA)-float(1.0f))*(density_energy - float(0.5f)*density*speed_sqd);
}

inline float compute_speed_of_sound(float& density, float& pressure)
{
	return sqrtf(float(GAMMA)*pressure/density);
}
extern "C"

void cuda_compute_step_factor(int nelr, float* variables, float* areas, float* step_factors)
{
    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=areas offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=step_factors offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=areas offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=step_factors offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=areas offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=step_factors offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=areas offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=step_factors offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = (BLOCK_DIM_X*_bid_x + _tid_x);

            float density = variables[i + VAR_DENSITY*nelr];
            float3 momentum;
            momentum.x = variables[i + (VAR_MOMENTUM+0)*nelr];
            momentum.y = variables[i + (VAR_MOMENTUM+1)*nelr];
            momentum.z = variables[i + (VAR_MOMENTUM+2)*nelr];

            float density_energy = variables[i + VAR_DENSITY_ENERGY*nelr];

            float3 velocity;       compute_velocity(density, momentum, velocity);
            float speed_sqd      = compute_speed_sqd(velocity);
            float pressure       = compute_pressure(density, density_energy, speed_sqd);
            float speed_of_sound = compute_speed_of_sound(density, pressure);

            // dt = float(0.5f) * sqrtf(areas[i]) /  (||v|| + c).... but when we do time stepping, this later would need to be divided by the area, so we just do it all at once
            step_factors[i] = float(0.5f) / (sqrtf(areas[i]) * (sqrtf(speed_sqd) + speed_of_sound));

        }
    }
nsity, pressure);

            // dt = float(0.5f) * sqrtf(areas[i]) /  (||v|| + c).... but when we do time stepping, this later would need to be divided by the area, so we just do it all at once
            step_factors[i] = float(0.5f) / (sqrtf(areas[i]) * (sqrtf(speed_sqd) + speed_of_sound));

        }
    }
nsity, pressure);

            // dt = float(0.5f) * sqrtf(areas[i]) /  (||v|| + c).... but when we do time stepping, this later would need to be divided by the area, so we just do it all at once
            step_factors[i] = float(0.5f) / (sqrtf(areas[i]) * (sqrtf(speed_sqd) + speed_of_sound));

        }
    }
nsity, pressure);

            // dt = float(0.5f) * sqrtf(areas[i]) /  (||v|| + c).... but when we do time stepping, this later would need to be divided by the area, so we just do it all at once
            step_factors[i] = float(0.5f) / (sqrtf(areas[i]) * (sqrtf(speed_sqd) + speed_of_sound));

        }
    }
}
extern "C"

void cuda_compute_flux(int nelr, int* elements_surrounding_elements, float* normals, float* variables, float* fluxes)
{
    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=elements_surrounding_elements offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=normals offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fluxes offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=elements_surrounding_elements offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=normals offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fluxes offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=elements_surrounding_elements offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=normals offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fluxes offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=elements_surrounding_elements offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=normals offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fluxes offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const float smoothing_coefficient = float(0.2f);
            const int i = (BLOCK_DIM_X*_bid_x + _tid_x);

            int j, nb;
            float3 normal; float normal_len;
            float factor;

            float density_i = variables[i + VAR_DENSITY*nelr];
            float3 momentum_i;
            momentum_i.x = variables[i + (VAR_MOMENTUM+0)*nelr];
            momentum_i.y = variables[i + (VAR_MOMENTUM+1)*nelr];
            momentum_i.z = variables[i + (VAR_MOMENTUM+2)*nelr];

            float density_energy_i = variables[i + VAR_DENSITY_ENERGY*nelr];

            float3 velocity_i;             				compute_velocity(density_i, momentum_i, velocity_i);
            float speed_sqd_i                          = compute_speed_sqd(velocity_i);
            float speed_i                              = sqrtf(speed_sqd_i);
            float pressure_i                           = compute_pressure(density_i, density_energy_i, speed_sqd_i);
            float speed_of_sound_i                     = compute_speed_of_sound(density_i, pressure_i);
            float3 flux_contribution_i_momentum_x, flux_contribution_i_momentum_y, flux_contribution_i_momentum_z;
            float3 flux_contribution_i_density_energy;
            compute_flux_contribution(density_i, momentum_i, density_energy_i, pressure_i, velocity_i, flux_contribution_i_momentum_x, flux_contribution_i_momentum_y, flux_contribution_i_momentum_z, flux_contribution_i_density_energy);

            float flux_i_density = float(0.0f);
            float3 flux_i_momentum;
            flux_i_momentum.x = float(0.0f);
            flux_i_momentum.y = float(0.0f);
            flux_i_momentum.z = float(0.0f);
            float flux_i_density_energy = float(0.0f);

            float3 velocity_nb;
            float density_nb, density_energy_nb;
            float3 momentum_nb;
            float3 flux_contribution_nb_momentum_x, flux_contribution_nb_momentum_y, flux_contribution_nb_momentum_z;
            float3 flux_contribution_nb_density_energy;
            float speed_sqd_nb, speed_of_sound_nb, pressure_nb;

            #pragma unroll
            for(j = 0; j < NNB; j++)
            {
            nb = elements_surrounding_elements[i + j*nelr];
            normal.x = normals[i + (j + 0*NNB)*nelr];
            normal.y = normals[i + (j + 1*NNB)*nelr];
            normal.z = normals[i + (j + 2*NNB)*nelr];
            normal_len = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);

            if(nb >= 0) 	// a legitimate neighbor
            {
            density_nb = variables[nb + VAR_DENSITY*nelr];
            momentum_nb.x = variables[nb + (VAR_MOMENTUM+0)*nelr];
            momentum_nb.y = variables[nb + (VAR_MOMENTUM+1)*nelr];
            momentum_nb.z = variables[nb + (VAR_MOMENTUM+2)*nelr];
            density_energy_nb = variables[nb + VAR_DENSITY_ENERGY*nelr];
            compute_velocity(density_nb, momentum_nb, velocity_nb);
            speed_sqd_nb                      = compute_speed_sqd(velocity_nb);
            pressure_nb                       = compute_pressure(density_nb, density_energy_nb, speed_sqd_nb);
            speed_of_sound_nb                 = compute_speed_of_sound(density_nb, pressure_nb);
            compute_flux_contribution(density_nb, momentum_nb, density_energy_nb, pressure_nb, velocity_nb, flux_contribution_nb_momentum_x, flux_contribution_nb_momentum_y, flux_contribution_nb_momentum_z, flux_contribution_nb_density_energy);

            // artificial viscosity
            factor = -normal_len*smoothing_coefficient*float(0.5f)*(speed_i + sqrtf(speed_sqd_nb) + speed_of_sound_i + speed_of_sound_nb);
            flux_i_density += factor*(density_i-density_nb);
            flux_i_density_energy += factor*(density_energy_i-density_energy_nb);
            flux_i_momentum.x += factor*(momentum_i.x-momentum_nb.x);
            flux_i_momentum.y += factor*(momentum_i.y-momentum_nb.y);
            flux_i_momentum.z += factor*(momentum_i.z-momentum_nb.z);

            // accumulate cell-centered fluxes
            factor = float(0.5f)*normal.x;
            flux_i_density += factor*(momentum_nb.x+momentum_i.x);
            flux_i_density_energy += factor*(flux_contribution_nb_density_energy.x+flux_contribution_i_density_energy.x);
            flux_i_momentum.x += factor*(flux_contribution_nb_momentum_x.x+flux_contribution_i_momentum_x.x);
            flux_i_momentum.y += factor*(flux_contribution_nb_momentum_y.x+flux_contribution_i_momentum_y.x);
            flux_i_momentum.z += factor*(flux_contribution_nb_momentum_z.x+flux_contribution_i_momentum_z.x);

            factor = float(0.5f)*normal.y;
            flux_i_density += factor*(momentum_nb.y+momentum_i.y);
            flux_i_density_energy += factor*(flux_contribution_nb_density_energy.y+flux_contribution_i_density_energy.y);
            flux_i_momentum.x += factor*(flux_contribution_nb_momentum_x.y+flux_contribution_i_momentum_x.y);
            flux_i_momentum.y += factor*(flux_contribution_nb_momentum_y.y+flux_contribution_i_momentum_y.y);
            flux_i_momentum.z += factor*(flux_contribution_nb_momentum_z.y+flux_contribution_i_momentum_z.y);

            factor = float(0.5f)*normal.z;
            flux_i_density += factor*(momentum_nb.z+momentum_i.z);
            flux_i_density_energy += factor*(flux_contribution_nb_density_energy.z+flux_contribution_i_density_energy.z);
            flux_i_momentum.x += factor*(flux_contribution_nb_momentum_x.z+flux_contribution_i_momentum_x.z);
            flux_i_momentum.y += factor*(flux_contribution_nb_momentum_y.z+flux_contribution_i_momentum_y.z);
            flux_i_momentum.z += factor*(flux_contribution_nb_momentum_z.z+flux_contribution_i_momentum_z.z);
            }
            else if(nb == -1)	// a wing boundary
            {
            flux_i_momentum.x += normal.x*pressure_i;
            flux_i_momentum.y += normal.y*pressure_i;
            flux_i_momentum.z += normal.z*pressure_i;
            }
            else if(nb == -2) // a far field boundary
            {
            factor = float(0.5f)*normal.x;
            flux_i_density += factor*(ff_variable[VAR_MOMENTUM+0]+momentum_i.x);
            flux_i_density_energy += factor*(ff_flux_contribution_density_energy[0].x+flux_contribution_i_density_energy.x);
            flux_i_momentum.x += factor*(ff_flux_contribution_momentum_x[0].x + flux_contribution_i_momentum_x.x);
            flux_i_momentum.y += factor*(ff_flux_contribution_momentum_y[0].x + flux_contribution_i_momentum_y.x);
            flux_i_momentum.z += factor*(ff_flux_contribution_momentum_z[0].x + flux_contribution_i_momentum_z.x);

            factor = float(0.5f)*normal.y;
            flux_i_density += factor*(ff_variable[VAR_MOMENTUM+1]+momentum_i.y);
            flux_i_density_energy += factor*(ff_flux_contribution_density_energy[0].y+flux_contribution_i_density_energy.y);
            flux_i_momentum.x += factor*(ff_flux_contribution_momentum_x[0].y + flux_contribution_i_momentum_x.y);
            flux_i_momentum.y += factor*(ff_flux_contribution_momentum_y[0].y + flux_contribution_i_momentum_y.y);
            flux_i_momentum.z += factor*(ff_flux_contribution_momentum_z[0].y + flux_contribution_i_momentum_z.y);

            factor = float(0.5f)*normal.z;
            flux_i_density += factor*(ff_variable[VAR_MOMENTUM+2]+momentum_i.z);
            flux_i_density_energy += factor*(ff_flux_contribution_density_energy[0].z+flux_contribution_i_density_energy.z);
            flux_i_momentum.x += factor*(ff_flux_contribution_momentum_x[0].z + flux_contribution_i_momentum_x.z);
            flux_i_momentum.y += factor*(ff_flux_contribution_momentum_y[0].z + flux_contribution_i_momentum_y.z);
            flux_i_momentum.z += factor*(ff_flux_contribution_momentum_z[0].z + flux_contribution_i_momentum_z.z);

            }
            }

            fluxes[i + VAR_DENSITY*nelr] = flux_i_density;
            fluxes[i + (VAR_MOMENTUM+0)*nelr] = flux_i_momentum.x;
            fluxes[i + (VAR_MOMENTUM+1)*nelr] = flux_i_momentum.y;
            fluxes[i + (VAR_MOMENTUM+2)*nelr] = flux_i_momentum.z;
            fluxes[i + VAR_DENSITY_ENERGY*nelr] = flux_i_density_energy;

        }
    }
      flux_i_momentum.y += factor*(ff_flux_contribution_momentum_y[0].y + flux_contribution_i_momentum_y.y);
            flux_i_momentum.z += factor*(ff_flux_contribution_momentum_z[0].y + flux_contribution_i_momentum_z.y);

            factor = float(0.5f)*normal.z;
            flux_i_density += factor*(ff_variable[VAR_MOMENTUM+2]+momentum_i.z);
            flux_i_density_energy += factor*(ff_flux_contribution_density_energy[0].z+flux_contribution_i_density_energy.z);
            flux_i_momentum.x += factor*(ff_flux_contribution_momentum_x[0].z + flux_contribution_i_momentum_x.z);
            flux_i_momentum.y += factor*(ff_flux_contribution_momentum_y[0].z + flux_contribution_i_momentum_y.z);
            flux_i_momentum.z += factor*(ff_flux_contribution_momentum_z[0].z + flux_contribution_i_momentum_z.z);

            }
            }

            fluxes[i + VAR_DENSITY*nelr] = flux_i_density;
            fluxes[i + (VAR_MOMENTUM+0)*nelr] = flux_i_momentum.x;
            fluxes[i + (VAR_MOMENTUM+1)*nelr] = flux_i_momentum.y;
            fluxes[i + (VAR_MOMENTUM+2)*nelr] = flux_i_momentum.z;
            fluxes[i + VAR_DENSITY_ENERGY*nelr] = flux_i_density_energy;

        }
    }
      flux_i_momentum.y += factor*(ff_flux_contribution_momentum_y[0].y + flux_contribution_i_momentum_y.y);
            flux_i_momentum.z += factor*(ff_flux_contribution_momentum_z[0].y + flux_contribution_i_momentum_z.y);

            factor = float(0.5f)*normal.z;
            flux_i_density += factor*(ff_variable[VAR_MOMENTUM+2]+momentum_i.z);
            flux_i_density_energy += factor*(ff_flux_contribution_density_energy[0].z+flux_contribution_i_density_energy.z);
            flux_i_momentum.x += factor*(ff_flux_contribution_momentum_x[0].z + flux_contribution_i_momentum_x.z);
            flux_i_momentum.y += factor*(ff_flux_contribution_momentum_y[0].z + flux_contribution_i_momentum_y.z);
            flux_i_momentum.z += factor*(ff_flux_contribution_momentum_z[0].z + flux_contribution_i_momentum_z.z);

            }
            }

            fluxes[i + VAR_DENSITY*nelr] = flux_i_density;
            fluxes[i + (VAR_MOMENTUM+0)*nelr] = flux_i_momentum.x;
            fluxes[i + (VAR_MOMENTUM+1)*nelr] = flux_i_momentum.y;
            fluxes[i + (VAR_MOMENTUM+2)*nelr] = flux_i_momentum.z;
            fluxes[i + VAR_DENSITY_ENERGY*nelr] = flux_i_density_energy;

        }
    }
      flux_i_momentum.y += factor*(ff_flux_contribution_momentum_y[0].y + flux_contribution_i_momentum_y.y);
            flux_i_momentum.z += factor*(ff_flux_contribution_momentum_z[0].y + flux_contribution_i_momentum_z.y);

            factor = float(0.5f)*normal.z;
            flux_i_density += factor*(ff_variable[VAR_MOMENTUM+2]+momentum_i.z);
            flux_i_density_energy += factor*(ff_flux_contribution_density_energy[0].z+flux_contribution_i_density_energy.z);
            flux_i_momentum.x += factor*(ff_flux_contribution_momentum_x[0].z + flux_contribution_i_momentum_x.z);
            flux_i_momentum.y += factor*(ff_flux_contribution_momentum_y[0].z + flux_contribution_i_momentum_y.z);
            flux_i_momentum.z += factor*(ff_flux_contribution_momentum_z[0].z + flux_contribution_i_momentum_z.z);

            }
            }

            fluxes[i + VAR_DENSITY*nelr] = flux_i_density;
            fluxes[i + (VAR_MOMENTUM+0)*nelr] = flux_i_momentum.x;
            fluxes[i + (VAR_MOMENTUM+1)*nelr] = flux_i_momentum.y;
            fluxes[i + (VAR_MOMENTUM+2)*nelr] = flux_i_momentum.z;
            fluxes[i + VAR_DENSITY_ENERGY*nelr] = flux_i_density_energy;

        }
    }
}
extern "C"

void cuda_time_step(int j, int nelr, float* old_variables, float* variables, float* step_factors, float* fluxes)
{
    #pragma HLS INTERFACE s_axilite port=j
    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=old_variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=step_factors offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fluxes offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=j
    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=old_variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=step_factors offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fluxes offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=j
    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=old_variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=step_factors offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fluxes offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=j
    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=old_variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=step_factors offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fluxes offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = (BLOCK_DIM_X*_bid_x + _tid_x);

            float factor = step_factors[i]/float(RK+1-j);

            variables[i + VAR_DENSITY*nelr] = old_variables[i + VAR_DENSITY*nelr] + factor*fluxes[i + VAR_DENSITY*nelr];
            variables[i + VAR_DENSITY_ENERGY*nelr] = old_variables[i + VAR_DENSITY_ENERGY*nelr] + factor*fluxes[i + VAR_DENSITY_ENERGY*nelr];
            variables[i + (VAR_MOMENTUM+0)*nelr] = old_variables[i + (VAR_MOMENTUM+0)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+0)*nelr];
            variables[i + (VAR_MOMENTUM+1)*nelr] = old_variables[i + (VAR_MOMENTUM+1)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+1)*nelr];
            variables[i + (VAR_MOMENTUM+2)*nelr] = old_variables[i + (VAR_MOMENTUM+2)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+2)*nelr];

        }
    }
)*nelr] = old_variables[i + (VAR_MOMENTUM+1)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+1)*nelr];
            variables[i + (VAR_MOMENTUM+2)*nelr] = old_variables[i + (VAR_MOMENTUM+2)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+2)*nelr];

        }
    }
)*nelr] = old_variables[i + (VAR_MOMENTUM+1)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+1)*nelr];
            variables[i + (VAR_MOMENTUM+2)*nelr] = old_variables[i + (VAR_MOMENTUM+2)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+2)*nelr];

        }
    }
)*nelr] = old_variables[i + (VAR_MOMENTUM+1)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+1)*nelr];
            variables[i + (VAR_MOMENTUM+2)*nelr] = old_variables[i + (VAR_MOMENTUM+2)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+2)*nelr];

        }
    }
}


// --- from euler3d_double.cu ---
void cuda_initialize_variables(int nelr, double* variables)
{
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);
	for(int j = 0; j < NVAR; j++)
		variables[i + j*nelr] = ff_variable[j];
}

inline void compute_flux_contribution(double& density, double3& momentum, double& density_energy, double& pressure, double3& velocity, double3& fc_momentum_x, double3& fc_momentum_y, double3& fc_momentum_z, double3& fc_density_energy)
{
	fc_momentum_x.x = velocity.x*momentum.x + pressure;
	fc_momentum_x.y = velocity.x*momentum.y;
	fc_momentum_x.z = velocity.x*momentum.z;
	
	
	fc_momentum_y.x = fc_momentum_x.y;
	fc_momentum_y.y = velocity.y*momentum.y + pressure;
	fc_momentum_y.z = velocity.y*momentum.z;

	fc_momentum_z.x = fc_momentum_x.z;
	fc_momentum_z.y = fc_momentum_y.z;
	fc_momentum_z.z = velocity.z*momentum.z + pressure;

	double de_p = density_energy+pressure;
	fc_density_energy.x = velocity.x*de_p;
	fc_density_energy.y = velocity.y*de_p;
	fc_density_energy.z = velocity.z*de_p;
}

inline void compute_velocity(double& density, double3& momentum, double3& velocity)
{
	velocity.x = momentum.x / density;
	velocity.y = momentum.y / density;
	velocity.z = momentum.z / density;
}

inline double compute_speed_sqd(double3& velocity)
{
	return velocity.x*velocity.x + velocity.y*velocity.y + velocity.z*velocity.z;
}

inline double compute_pressure(double& density, double& density_energy, double& speed_sqd)
{
	return (double(GAMMA)-double(1.0))*(density_energy - double(0.5)*density*speed_sqd);
}

inline double compute_speed_of_sound(double& density, double& pressure)
{
	return sqrt(double(GAMMA)*pressure/density);
}

void cuda_compute_step_factor(int nelr, double* variables, double* areas, double* step_factors)
{
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);

	double density = variables[i + VAR_DENSITY*nelr];
	double3 momentum;
	momentum.x = variables[i + (VAR_MOMENTUM+0)*nelr];
	momentum.y = variables[i + (VAR_MOMENTUM+1)*nelr];
	momentum.z = variables[i + (VAR_MOMENTUM+2)*nelr];
	
	double density_energy = variables[i + VAR_DENSITY_ENERGY*nelr];
	
	double3 velocity;       compute_velocity(density, momentum, velocity);
	double speed_sqd      = compute_speed_sqd(velocity);
	double pressure       = compute_pressure(density, density_energy, speed_sqd);
	double speed_of_sound = compute_speed_of_sound(density, pressure);

	// dt = double(0.5) * sqrt(areas[i]) /  (||v|| + c).... but when we do time stepping, this later would need to be divided by the area, so we just do it all at once
	step_factors[i] = double(0.5) / (sqrt(areas[i]) * (sqrt(speed_sqd) + speed_of_sound));
}

void cuda_compute_flux(int nelr, int* elements_surrounding_elements, double* normals, double* variables, double* fluxes)
{
	const double smoothing_coefficient = double(0.2f);
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);
	
	int j, nb;
	double3 normal; double normal_len;
	double factor;
	
	double density_i = variables[i + VAR_DENSITY*nelr];
	double3 momentum_i;
	momentum_i.x = variables[i + (VAR_MOMENTUM+0)*nelr];
	momentum_i.y = variables[i + (VAR_MOMENTUM+1)*nelr];
	momentum_i.z = variables[i + (VAR_MOMENTUM+2)*nelr];

	double density_energy_i = variables[i + VAR_DENSITY_ENERGY*nelr];

	double3 velocity_i;             				compute_velocity(density_i, momentum_i, velocity_i);
	double speed_sqd_i                          = compute_speed_sqd(velocity_i);
	double speed_i                              = sqrt(speed_sqd_i);
	double pressure_i                           = compute_pressure(density_i, density_energy_i, speed_sqd_i);
	double speed_of_sound_i                     = compute_speed_of_sound(density_i, pressure_i);
	double3 flux_contribution_i_momentum_x, flux_contribution_i_momentum_y, flux_contribution_i_momentum_z;
	double3 flux_contribution_i_density_energy;	
	compute_flux_contribution(density_i, momentum_i, density_energy_i, pressure_i, velocity_i, flux_contribution_i_momentum_x, flux_contribution_i_momentum_y, flux_contribution_i_momentum_z, flux_contribution_i_density_energy);
	
	double flux_i_density = double(0.0);
	double3 flux_i_momentum;
	flux_i_momentum.x = double(0.0);
	flux_i_momentum.y = double(0.0);
	flux_i_momentum.z = double(0.0);
	double flux_i_density_energy = double(0.0);
		
	double3 velocity_nb;
	double density_nb, density_energy_nb;
	double3 momentum_nb;
	double3 flux_contribution_nb_momentum_x, flux_contribution_nb_momentum_y, flux_contribution_nb_momentum_z;
	double3 flux_contribution_nb_density_energy;	
	double speed_sqd_nb, speed_of_sound_nb, pressure_nb;
	
	#pragma unroll
	for(j = 0; j < NNB; j++)
	{
		nb = elements_surrounding_elements[i + j*nelr];
		normal.x = normals[i + (j + 0*NNB)*nelr];
		normal.y = normals[i + (j + 1*NNB)*nelr];
		normal.z = normals[i + (j + 2*NNB)*nelr];
		normal_len = sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
		
		if(nb >= 0) 	// a legitimate neighbor
		{
			density_nb = variables[nb + VAR_DENSITY*nelr];
			momentum_nb.x = variables[nb + (VAR_MOMENTUM+0)*nelr];
			momentum_nb.y = variables[nb + (VAR_MOMENTUM+1)*nelr];
			momentum_nb.z = variables[nb + (VAR_MOMENTUM+2)*nelr];
			density_energy_nb = variables[nb + VAR_DENSITY_ENERGY*nelr];
												compute_velocity(density_nb, momentum_nb, velocity_nb);
			speed_sqd_nb                      = compute_speed_sqd(velocity_nb);
			pressure_nb                       = compute_pressure(density_nb, density_energy_nb, speed_sqd_nb);
			speed_of_sound_nb                 = compute_speed_of_sound(density_nb, pressure_nb);
			                                    compute_flux_contribution(density_nb, momentum_nb, density_energy_nb, pressure_nb, velocity_nb, flux_contribution_nb_momentum_x, flux_contribution_nb_momentum_y, flux_contribution_nb_momentum_z, flux_contribution_nb_density_energy);
			
			// artificial viscosity
			factor = -normal_len*smoothing_coefficient*double(0.5)*(speed_i + sqrt(speed_sqd_nb) + speed_of_sound_i + speed_of_sound_nb);
			flux_i_density += factor*(density_i-density_nb);
			flux_i_density_energy += factor*(density_energy_i-density_energy_nb);
			flux_i_momentum.x += factor*(momentum_i.x-momentum_nb.x);
			flux_i_momentum.y += factor*(momentum_i.y-momentum_nb.y);
			flux_i_momentum.z += factor*(momentum_i.z-momentum_nb.z);

			// accumulate cell-centered fluxes
			factor = double(0.5)*normal.x;
			flux_i_density += factor*(momentum_nb.x+momentum_i.x);
			flux_i_density_energy += factor*(flux_contribution_nb_density_energy.x+flux_contribution_i_density_energy.x);
			flux_i_momentum.x += factor*(flux_contribution_nb_momentum_x.x+flux_contribution_i_momentum_x.x);
			flux_i_momentum.y += factor*(flux_contribution_nb_momentum_y.x+flux_contribution_i_momentum_y.x);
			flux_i_momentum.z += factor*(flux_contribution_nb_momentum_z.x+flux_contribution_i_momentum_z.x);
			
			factor = double(0.5)*normal.y;
			flux_i_density += factor*(momentum_nb.y+momentum_i.y);
			flux_i_density_energy += factor*(flux_contribution_nb_density_energy.y+flux_contribution_i_density_energy.y);
			flux_i_momentum.x += factor*(flux_contribution_nb_momentum_x.y+flux_contribution_i_momentum_x.y);
			flux_i_momentum.y += factor*(flux_contribution_nb_momentum_y.y+flux_contribution_i_momentum_y.y);
			flux_i_momentum.z += factor*(flux_contribution_nb_momentum_z.y+flux_contribution_i_momentum_z.y);
			
			factor = double(0.5)*normal.z;
			flux_i_density += factor*(momentum_nb.z+momentum_i.z);
			flux_i_density_energy += factor*(flux_contribution_nb_density_energy.z+flux_contribution_i_density_energy.z);
			flux_i_momentum.x += factor*(flux_contribution_nb_momentum_x.z+flux_contribution_i_momentum_x.z);
			flux_i_momentum.y += factor*(flux_contribution_nb_momentum_y.z+flux_contribution_i_momentum_y.z);
			flux_i_momentum.z += factor*(flux_contribution_nb_momentum_z.z+flux_contribution_i_momentum_z.z);
		}
		else if(nb == -1)	// a wing boundary
		{
			flux_i_momentum.x += normal.x*pressure_i;
			flux_i_momentum.y += normal.y*pressure_i;
			flux_i_momentum.z += normal.z*pressure_i;
		}
		else if(nb == -2) // a far field boundary
		{
			factor = double(0.5)*normal.x;
			flux_i_density += factor*(ff_variable[VAR_MOMENTUM+0]+momentum_i.x);
			flux_i_density_energy += factor*(ff_flux_contribution_density_energy[0].x+flux_contribution_i_density_energy.x);
			flux_i_momentum.x += factor*(ff_flux_contribution_momentum_x[0].x + flux_contribution_i_momentum_x.x);
			flux_i_momentum.y += factor*(ff_flux_contribution_momentum_y[0].x + flux_contribution_i_momentum_y.x);
			flux_i_momentum.z += factor*(ff_flux_contribution_momentum_z[0].x + flux_contribution_i_momentum_z.x);
			
			factor = double(0.5)*normal.y;
			flux_i_density += factor*(ff_variable[VAR_MOMENTUM+1]+momentum_i.y);
			flux_i_density_energy += factor*(ff_flux_contribution_density_energy[0].y+flux_contribution_i_density_energy.y);
			flux_i_momentum.x += factor*(ff_flux_contribution_momentum_x[0].y + flux_contribution_i_momentum_x.y);
			flux_i_momentum.y += factor*(ff_flux_contribution_momentum_y[0].y + flux_contribution_i_momentum_y.y);
			flux_i_momentum.z += factor*(ff_flux_contribution_momentum_z[0].y + flux_contribution_i_momentum_z.y);

			factor = double(0.5)*normal.z;
			flux_i_density += factor*(ff_variable[VAR_MOMENTUM+2]+momentum_i.z);
			flux_i_density_energy += factor*(ff_flux_contribution_density_energy[0].z+flux_contribution_i_density_energy.z);
			flux_i_momentum.x += factor*(ff_flux_contribution_momentum_x[0].z + flux_contribution_i_momentum_x.z);
			flux_i_momentum.y += factor*(ff_flux_contribution_momentum_y[0].z + flux_contribution_i_momentum_y.z);
			flux_i_momentum.z += factor*(ff_flux_contribution_momentum_z[0].z + flux_contribution_i_momentum_z.z);

		}
	}

	fluxes[i + VAR_DENSITY*nelr] = flux_i_density;
	fluxes[i + (VAR_MOMENTUM+0)*nelr] = flux_i_momentum.x;
	fluxes[i + (VAR_MOMENTUM+1)*nelr] = flux_i_momentum.y;
	fluxes[i + (VAR_MOMENTUM+2)*nelr] = flux_i_momentum.z;
	fluxes[i + VAR_DENSITY_ENERGY*nelr] = flux_i_density_energy;
}

void cuda_time_step(int j, int nelr, double* old_variables, double* variables, double* step_factors, double* fluxes)
{
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);

	double factor = step_factors[i]/double(RK+1-j);

	variables[i + VAR_DENSITY*nelr] = old_variables[i + VAR_DENSITY*nelr] + factor*fluxes[i + VAR_DENSITY*nelr];
	variables[i + VAR_DENSITY_ENERGY*nelr] = old_variables[i + VAR_DENSITY_ENERGY*nelr] + factor*fluxes[i + VAR_DENSITY_ENERGY*nelr];
	variables[i + (VAR_MOMENTUM+0)*nelr] = old_variables[i + (VAR_MOMENTUM+0)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+0)*nelr];
	variables[i + (VAR_MOMENTUM+1)*nelr] = old_variables[i + (VAR_MOMENTUM+1)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+1)*nelr];	
	variables[i + (VAR_MOMENTUM+2)*nelr] = old_variables[i + (VAR_MOMENTUM+2)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+2)*nelr];	
}


// --- from pre_euler3d.cu ---
void cuda_initialize_variables(int nelr, float* variables)
{
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);
	for(int j = 0; j < NVAR; j++)
		variables[i + j*nelr] = ff_variable[j];
}

inline void compute_flux_contribution(float& density, float3& momentum, float& density_energy, float& pressure, float3& velocity, float3& fc_momentum_x, float3& fc_momentum_y, float3& fc_momentum_z, float3& fc_density_energy)
{
	fc_momentum_x.x = velocity.x*momentum.x + pressure;
	fc_momentum_x.y = velocity.x*momentum.y;
	fc_momentum_x.z = velocity.x*momentum.z;
	
	
	fc_momentum_y.x = fc_momentum_x.y;
	fc_momentum_y.y = velocity.y*momentum.y + pressure;
	fc_momentum_y.z = velocity.y*momentum.z;

	fc_momentum_z.x = fc_momentum_x.z;
	fc_momentum_z.y = fc_momentum_y.z;
	fc_momentum_z.z = velocity.z*momentum.z + pressure;

	float de_p = density_energy+pressure;
	fc_density_energy.x = velocity.x*de_p;
	fc_density_energy.y = velocity.y*de_p;
	fc_density_energy.z = velocity.z*de_p;
}

inline void compute_velocity(float& density, float3& momentum, float3& velocity)
{
	velocity.x = momentum.x / density;
	velocity.y = momentum.y / density;
	velocity.z = momentum.z / density;
}

inline float compute_speed_sqd(float3& velocity)
{
	return velocity.x*velocity.x + velocity.y*velocity.y + velocity.z*velocity.z;
}

inline float compute_pressure(float& density, float& density_energy, float& speed_sqd)
{
	return (float(GAMMA)-float(1.0f))*(density_energy - float(0.5f)*density*speed_sqd);
}

inline float compute_speed_of_sound(float& density, float& pressure)
{
	return sqrtf(float(GAMMA)*pressure/density);
}

void cuda_compute_step_factor(int nelr, float* variables, float* areas, float* step_factors)
{
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);

	float density = variables[i + VAR_DENSITY*nelr];
	float3 momentum;
	momentum.x = variables[i + (VAR_MOMENTUM+0)*nelr];
	momentum.y = variables[i + (VAR_MOMENTUM+1)*nelr];
	momentum.z = variables[i + (VAR_MOMENTUM+2)*nelr];
	
	float density_energy = variables[i + VAR_DENSITY_ENERGY*nelr];
	
	float3 velocity;       compute_velocity(density, momentum, velocity);
	float speed_sqd      = compute_speed_sqd(velocity);
	float pressure       = compute_pressure(density, density_energy, speed_sqd);
	float speed_of_sound = compute_speed_of_sound(density, pressure);

	// dt = float(0.5f) * sqrtf(areas[i]) /  (||v|| + c).... but when we do time stepping, this later would need to be divided by the area, so we just do it all at once
	step_factors[i] = float(0.5f) / (sqrtf(areas[i]) * (sqrtf(speed_sqd) + speed_of_sound));
}
extern "C"

void cuda_compute_flux_contributions(int nelr, float* variables, float* fc_momentum_x, float* fc_momentum_y, float* fc_momentum_z, float* fc_density_energy)
{
    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fc_momentum_x offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=fc_momentum_y offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fc_momentum_z offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=fc_density_energy offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=nelr
    #pragma HLS INTERFACE m_axi port=variables offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fc_momentum_x offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=fc_momentum_y offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fc_momentum_z offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=fc_density_energy offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = (BLOCK_DIM_X*_bid_x + _tid_x);

            float density_i = variables[i + VAR_DENSITY*nelr];
            float3 momentum_i;
            momentum_i.x = variables[i + (VAR_MOMENTUM+0)*nelr];
            momentum_i.y = variables[i + (VAR_MOMENTUM+1)*nelr];
            momentum_i.z = variables[i + (VAR_MOMENTUM+2)*nelr];
            float density_energy_i = variables[i + VAR_DENSITY_ENERGY*nelr];

            float3 velocity_i;             				compute_velocity(density_i, momentum_i, velocity_i);
            float speed_sqd_i                          = compute_speed_sqd(velocity_i);
            float speed_i                              = sqrtf(speed_sqd_i);
            float pressure_i                           = compute_pressure(density_i, density_energy_i, speed_sqd_i);
            float speed_of_sound_i                     = compute_speed_of_sound(density_i, pressure_i);
            float3 fc_i_momentum_x, fc_i_momentum_y, fc_i_momentum_z;
            float3 fc_i_density_energy;
            compute_flux_contribution(density_i, momentum_i, density_energy_i, pressure_i, velocity_i, fc_i_momentum_x, fc_i_momentum_y, fc_i_momentum_z, fc_i_density_energy);

            fc_momentum_x[i + 0*nelr] = fc_i_momentum_x.x;
            fc_momentum_x[i + 1*nelr] = fc_i_momentum_x.y;
            fc_momentum_x[i + 2*nelr] = fc_i_momentum_x.z;

            fc_momentum_y[i + 0*nelr] = fc_i_momentum_y.x;
            fc_momentum_y[i + 1*nelr] = fc_i_momentum_y.y;
            fc_momentum_y[i + 2*nelr] = fc_i_momentum_y.z;

            fc_momentum_z[i + 0*nelr] = fc_i_momentum_z.x;
            fc_momentum_z[i + 1*nelr] = fc_i_momentum_z.y;
            fc_momentum_z[i + 2*nelr] = fc_i_momentum_z.z;

            fc_density_energy[i + 0*nelr] = fc_i_density_energy.x;
            fc_density_energy[i + 1*nelr] = fc_i_density_energy.y;
            fc_density_energy[i + 2*nelr] = fc_i_density_energy.z;


        }
    }
mentum_y.y;
            fc_momentum_y[i + 2*nelr] = fc_i_momentum_y.z;

            fc_momentum_z[i + 0*nelr] = fc_i_momentum_z.x;
            fc_momentum_z[i + 1*nelr] = fc_i_momentum_z.y;
            fc_momentum_z[i + 2*nelr] = fc_i_momentum_z.z;

            fc_density_energy[i + 0*nelr] = fc_i_density_energy.x;
            fc_density_energy[i + 1*nelr] = fc_i_density_energy.y;
            fc_density_energy[i + 2*nelr] = fc_i_density_energy.z;


        }
    }
}

void cuda_compute_flux(int nelr, int* elements_surrounding_elements, float* normals, float* variables, float* fc_momentum_x, float* fc_momentum_y, float* fc_momentum_z, float* fc_density_energy, float* fluxes)
{
	const float smoothing_coefficient = float(0.2f);
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);
	
	int j, nb;
	float3 normal; float normal_len;
	float factor;
	
	float density_i = variables[i + VAR_DENSITY*nelr];
	float3 momentum_i;
	momentum_i.x = variables[i + (VAR_MOMENTUM+0)*nelr];
	momentum_i.y = variables[i + (VAR_MOMENTUM+1)*nelr];
	momentum_i.z = variables[i + (VAR_MOMENTUM+2)*nelr];

	float density_energy_i = variables[i + VAR_DENSITY_ENERGY*nelr];

	float3 velocity_i;             	           compute_velocity(density_i, momentum_i, velocity_i);
	float speed_sqd_i                          = compute_speed_sqd(velocity_i);
	float speed_i                              = sqrtf(speed_sqd_i);
	float pressure_i                           = compute_pressure(density_i, density_energy_i, speed_sqd_i);
	float speed_of_sound_i                     = compute_speed_of_sound(density_i, pressure_i);
	float3 fc_i_momentum_x, fc_i_momentum_y, fc_i_momentum_z;
	float3 fc_i_density_energy;	

	fc_i_momentum_x.x = fc_momentum_x[i + 0*nelr];
	fc_i_momentum_x.y = fc_momentum_x[i + 1*nelr];
	fc_i_momentum_x.z = fc_momentum_x[i + 2*nelr];

	fc_i_momentum_y.x = fc_momentum_y[i + 0*nelr];
	fc_i_momentum_y.y = fc_momentum_y[i + 1*nelr];
	fc_i_momentum_y.z = fc_momentum_y[i + 2*nelr];

	fc_i_momentum_z.x = fc_momentum_z[i + 0*nelr];
	fc_i_momentum_z.y = fc_momentum_z[i + 1*nelr];
	fc_i_momentum_z.z = fc_momentum_z[i + 2*nelr];

	fc_i_density_energy.x = fc_density_energy[i + 0*nelr];
	fc_i_density_energy.y = fc_density_energy[i + 1*nelr];
	fc_i_density_energy.z = fc_density_energy[i + 2*nelr];
	
	float flux_i_density = float(0.0f);
	float3 flux_i_momentum;
	flux_i_momentum.x = float(0.0f);
	flux_i_momentum.y = float(0.0f);
	flux_i_momentum.z = float(0.0f);
	float flux_i_density_energy = float(0.0f);
		
	float3 velocity_nb;
	float density_nb, density_energy_nb;
	float3 momentum_nb;
	float3 fc_nb_momentum_x, fc_nb_momentum_y, fc_nb_momentum_z;
	float3 fc_nb_density_energy;	
	float speed_sqd_nb, speed_of_sound_nb, pressure_nb;
	
	#pragma unroll
	for(j = 0; j < NNB; j++)
	{
		nb = elements_surrounding_elements[i + j*nelr];
		normal.x = normals[i + (j + 0*NNB)*nelr];
		normal.y = normals[i + (j + 1*NNB)*nelr];
		normal.z = normals[i + (j + 2*NNB)*nelr];
		normal_len = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
		
		if(nb >= 0) 	// a legitimate neighbor
		{
			density_nb = variables[nb + VAR_DENSITY*nelr];
			momentum_nb.x = variables[nb + (VAR_MOMENTUM+0)*nelr];
			momentum_nb.y = variables[nb + (VAR_MOMENTUM+1)*nelr];
			momentum_nb.z = variables[nb + (VAR_MOMENTUM+2)*nelr];
			density_energy_nb = variables[nb + VAR_DENSITY_ENERGY*nelr];
								  compute_velocity(density_nb, momentum_nb, velocity_nb);
			speed_sqd_nb                      = compute_speed_sqd(velocity_nb);
			pressure_nb                       = compute_pressure(density_nb, density_energy_nb, speed_sqd_nb);
			speed_of_sound_nb                 = compute_speed_of_sound(density_nb, pressure_nb);
			                                   
			fc_nb_momentum_x.x = fc_momentum_x[nb + 0*nelr];
			fc_nb_momentum_x.y = fc_momentum_x[nb + 1*nelr];
			fc_nb_momentum_x.z = fc_momentum_x[nb + 2*nelr];

			fc_nb_momentum_y.x = fc_momentum_y[nb + 0*nelr];
			fc_nb_momentum_y.y = fc_momentum_y[nb + 1*nelr];
			fc_nb_momentum_y.z = fc_momentum_y[nb + 2*nelr];

			fc_nb_momentum_z.x = fc_momentum_z[nb + 0*nelr];
			fc_nb_momentum_z.y = fc_momentum_z[nb + 1*nelr];
			fc_nb_momentum_z.z = fc_momentum_z[nb + 2*nelr];

			fc_nb_density_energy.x = fc_density_energy[nb + 0*nelr];
			fc_nb_density_energy.y = fc_density_energy[nb + 1*nelr];
			fc_nb_density_energy.z = fc_density_energy[nb + 2*nelr];
			

			// artificial viscosity
			factor = -normal_len*smoothing_coefficient*float(0.5f)*(speed_i + sqrtf(speed_sqd_nb) + speed_of_sound_i + speed_of_sound_nb);
			flux_i_density += factor*(density_i-density_nb);
			flux_i_density_energy += factor*(density_energy_i-density_energy_nb);
			flux_i_momentum.x += factor*(momentum_i.x-momentum_nb.x);
			flux_i_momentum.y += factor*(momentum_i.y-momentum_nb.y);
			flux_i_momentum.z += factor*(momentum_i.z-momentum_nb.z);

			// accumulate cell-centered fluxes
			factor = float(0.5f)*normal.x;
			flux_i_density += factor*(momentum_nb.x+momentum_i.x);
			flux_i_density_energy += factor*(fc_nb_density_energy.x+fc_i_density_energy.x);
			flux_i_momentum.x += factor*(fc_nb_momentum_x.x+fc_i_momentum_x.x);
			flux_i_momentum.y += factor*(fc_nb_momentum_y.x+fc_i_momentum_y.x);
			flux_i_momentum.z += factor*(fc_nb_momentum_z.x+fc_i_momentum_z.x);
			
			factor = float(0.5f)*normal.y;
			flux_i_density += factor*(momentum_nb.y+momentum_i.y);
			flux_i_density_energy += factor*(fc_nb_density_energy.y+fc_i_density_energy.y);
			flux_i_momentum.x += factor*(fc_nb_momentum_x.y+fc_i_momentum_x.y);
			flux_i_momentum.y += factor*(fc_nb_momentum_y.y+fc_i_momentum_y.y);
			flux_i_momentum.z += factor*(fc_nb_momentum_z.y+fc_i_momentum_z.y);
			
			factor = float(0.5f)*normal.z;
			flux_i_density += factor*(momentum_nb.z+momentum_i.z);
			flux_i_density_energy += factor*(fc_nb_density_energy.z+fc_i_density_energy.z);
			flux_i_momentum.x += factor*(fc_nb_momentum_x.z+fc_i_momentum_x.z);
			flux_i_momentum.y += factor*(fc_nb_momentum_y.z+fc_i_momentum_y.z);
			flux_i_momentum.z += factor*(fc_nb_momentum_z.z+fc_i_momentum_z.z);
		}
		else if(nb == -1)	// a wing boundary
		{
			flux_i_momentum.x += normal.x*pressure_i;
			flux_i_momentum.y += normal.y*pressure_i;
			flux_i_momentum.z += normal.z*pressure_i;
		}
		else if(nb == -2) // a far field boundary
		{
			factor = float(0.5f)*normal.x;
			flux_i_density += factor*(ff_variable[VAR_MOMENTUM+0]+momentum_i.x);
			flux_i_density_energy += factor*(ff_fc_density_energy[0].x+fc_i_density_energy.x);
			flux_i_momentum.x += factor*(ff_fc_momentum_x[0].x + fc_i_momentum_x.x);
			flux_i_momentum.y += factor*(ff_fc_momentum_y[0].x + fc_i_momentum_y.x);
			flux_i_momentum.z += factor*(ff_fc_momentum_z[0].x + fc_i_momentum_z.x);
			
			factor = float(0.5f)*normal.y;
			flux_i_density += factor*(ff_variable[VAR_MOMENTUM+1]+momentum_i.y);
			flux_i_density_energy += factor*(ff_fc_density_energy[0].y+fc_i_density_energy.y);
			flux_i_momentum.x += factor*(ff_fc_momentum_x[0].y + fc_i_momentum_x.y);
			flux_i_momentum.y += factor*(ff_fc_momentum_y[0].y + fc_i_momentum_y.y);
			flux_i_momentum.z += factor*(ff_fc_momentum_z[0].y + fc_i_momentum_z.y);

			factor = float(0.5f)*normal.z;
			flux_i_density += factor*(ff_variable[VAR_MOMENTUM+2]+momentum_i.z);
			flux_i_density_energy += factor*(ff_fc_density_energy[0].z+fc_i_density_energy.z);
			flux_i_momentum.x += factor*(ff_fc_momentum_x[0].z + fc_i_momentum_x.z);
			flux_i_momentum.y += factor*(ff_fc_momentum_y[0].z + fc_i_momentum_y.z);
			flux_i_momentum.z += factor*(ff_fc_momentum_z[0].z + fc_i_momentum_z.z);

		}
	}

	fluxes[i + VAR_DENSITY*nelr] = flux_i_density;
	fluxes[i + (VAR_MOMENTUM+0)*nelr] = flux_i_momentum.x;
	fluxes[i + (VAR_MOMENTUM+1)*nelr] = flux_i_momentum.y;
	fluxes[i + (VAR_MOMENTUM+2)*nelr] = flux_i_momentum.z;
	fluxes[i + VAR_DENSITY_ENERGY*nelr] = flux_i_density_energy;
}

void cuda_time_step(int j, int nelr, float* old_variables, float* variables, float* step_factors, float* fluxes)
{
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);

	float factor = step_factors[i]/float(RK+1-j);

	variables[i + VAR_DENSITY*nelr] = old_variables[i + VAR_DENSITY*nelr] + factor*fluxes[i + VAR_DENSITY*nelr];
	variables[i + VAR_DENSITY_ENERGY*nelr] = old_variables[i + VAR_DENSITY_ENERGY*nelr] + factor*fluxes[i + VAR_DENSITY_ENERGY*nelr];
	variables[i + (VAR_MOMENTUM+0)*nelr] = old_variables[i + (VAR_MOMENTUM+0)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+0)*nelr];
	variables[i + (VAR_MOMENTUM+1)*nelr] = old_variables[i + (VAR_MOMENTUM+1)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+1)*nelr];	
	variables[i + (VAR_MOMENTUM+2)*nelr] = old_variables[i + (VAR_MOMENTUM+2)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+2)*nelr];	
}


// --- from pre_euler3d_double.cu ---
void cuda_initialize_variables(int nelr, double* variables)
{
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);
	for(int j = 0; j < NVAR; j++)
		variables[i + j*nelr] = ff_variable[j];
}

inline void compute_flux_contribution(double& density, double3& momentum, double& density_energy, double& pressure, double3& velocity, double3& fc_momentum_x, double3& fc_momentum_y, double3& fc_momentum_z, double3& fc_density_energy)
{
	fc_momentum_x.x = velocity.x*momentum.x + pressure;
	fc_momentum_x.y = velocity.x*momentum.y;
	fc_momentum_x.z = velocity.x*momentum.z;
	
	
	fc_momentum_y.x = fc_momentum_x.y;
	fc_momentum_y.y = velocity.y*momentum.y + pressure;
	fc_momentum_y.z = velocity.y*momentum.z;

	fc_momentum_z.x = fc_momentum_x.z;
	fc_momentum_z.y = fc_momentum_y.z;
	fc_momentum_z.z = velocity.z*momentum.z + pressure;

	double de_p = density_energy+pressure;
	fc_density_energy.x = velocity.x*de_p;
	fc_density_energy.y = velocity.y*de_p;
	fc_density_energy.z = velocity.z*de_p;
}

inline void compute_velocity(double& density, double3& momentum, double3& velocity)
{
	velocity.x = momentum.x / density;
	velocity.y = momentum.y / density;
	velocity.z = momentum.z / density;
}

inline double compute_speed_sqd(double3& velocity)
{
	return velocity.x*velocity.x + velocity.y*velocity.y + velocity.z*velocity.z;
}

inline double compute_pressure(double& density, double& density_energy, double& speed_sqd)
{
	return (double(GAMMA)-double(1.0))*(density_energy - double(0.5)*density*speed_sqd);
}

inline double compute_speed_of_sound(double& density, double& pressure)
{
	return sqrt(double(GAMMA)*pressure/density);
}

void cuda_compute_step_factor(int nelr, double* variables, double* areas, double* step_factors)
{
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);

	double density = variables[i + VAR_DENSITY*nelr];
	double3 momentum;
	momentum.x = variables[i + (VAR_MOMENTUM+0)*nelr];
	momentum.y = variables[i + (VAR_MOMENTUM+1)*nelr];
	momentum.z = variables[i + (VAR_MOMENTUM+2)*nelr];
	
	double density_energy = variables[i + VAR_DENSITY_ENERGY*nelr];
	
	double3 velocity;       compute_velocity(density, momentum, velocity);
	double speed_sqd      = compute_speed_sqd(velocity);
	double pressure       = compute_pressure(density, density_energy, speed_sqd);
	double speed_of_sound = compute_speed_of_sound(density, pressure);

	// dt = double(0.5) * sqrt(areas[i]) /  (||v|| + c).... but when we do time stepping, this later would need to be divided by the area, so we just do it all at once
	step_factors[i] = double(0.5) / (sqrt(areas[i]) * (sqrt(speed_sqd) + speed_of_sound));
}

void cuda_compute_flux_contributions(int nelr, double* variables, double* fc_momentum_x, double* fc_momentum_y, double* fc_momentum_z, double* fc_density_energy)
{
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);

	double density_i = variables[i + VAR_DENSITY*nelr];
	double3 momentum_i;
	momentum_i.x = variables[i + (VAR_MOMENTUM+0)*nelr];
	momentum_i.y = variables[i + (VAR_MOMENTUM+1)*nelr];
	momentum_i.z = variables[i + (VAR_MOMENTUM+2)*nelr];
	double density_energy_i = variables[i + VAR_DENSITY_ENERGY*nelr];

	double3 velocity_i;             				compute_velocity(density_i, momentum_i, velocity_i);
	double speed_sqd_i                          = compute_speed_sqd(velocity_i);
	double speed_i                              = sqrtf(speed_sqd_i);
	double pressure_i                           = compute_pressure(density_i, density_energy_i, speed_sqd_i);
	double speed_of_sound_i                     = compute_speed_of_sound(density_i, pressure_i);
	double3 fc_i_momentum_x, fc_i_momentum_y, fc_i_momentum_z;
	double3 fc_i_density_energy;	
	compute_flux_contribution(density_i, momentum_i, density_energy_i, pressure_i, velocity_i, fc_i_momentum_x, fc_i_momentum_y, fc_i_momentum_z, fc_i_density_energy);

	fc_momentum_x[i + 0*nelr] = fc_i_momentum_x.x;
	fc_momentum_x[i + 1*nelr] = fc_i_momentum_x.y;
	fc_momentum_x[i + 2*nelr] = fc_i_momentum_x.z;

	fc_momentum_y[i + 0*nelr] = fc_i_momentum_y.x;
	fc_momentum_y[i + 1*nelr] = fc_i_momentum_y.y;
	fc_momentum_y[i + 2*nelr] = fc_i_momentum_y.z;

	fc_momentum_z[i + 0*nelr] = fc_i_momentum_z.x;
	fc_momentum_z[i + 1*nelr] = fc_i_momentum_z.y;
	fc_momentum_z[i + 2*nelr] = fc_i_momentum_z.z;

	fc_density_energy[i + 0*nelr] = fc_i_density_energy.x;
	fc_density_energy[i + 1*nelr] = fc_i_density_energy.y;
	fc_density_energy[i + 2*nelr] = fc_i_density_energy.z;

}

void cuda_compute_flux(int nelr, int* elements_surrounding_elements, double* normals, double* variables, double* fc_momentum_x, double* fc_momentum_y, double* fc_momentum_z, double* fc_density_energy, double* fluxes)
{
	const double smoothing_coefficient = double(0.2f);
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);
	
	int j, nb;
	double3 normal; double normal_len;
	double factor;
	
	double density_i = variables[i + VAR_DENSITY*nelr];
	double3 momentum_i;
	momentum_i.x = variables[i + (VAR_MOMENTUM+0)*nelr];
	momentum_i.y = variables[i + (VAR_MOMENTUM+1)*nelr];
	momentum_i.z = variables[i + (VAR_MOMENTUM+2)*nelr];

	double density_energy_i = variables[i + VAR_DENSITY_ENERGY*nelr];

	double3 velocity_i;             				compute_velocity(density_i, momentum_i, velocity_i);
	double speed_sqd_i                          = compute_speed_sqd(velocity_i);
	double speed_i                              = sqrt(speed_sqd_i);
	double pressure_i                           = compute_pressure(density_i, density_energy_i, speed_sqd_i);
	double speed_of_sound_i                     = compute_speed_of_sound(density_i, pressure_i);
	double3 fc_i_momentum_x, fc_i_momentum_y, fc_i_momentum_z;
	double3 fc_i_density_energy;	

	fc_i_momentum_x.x = fc_momentum_x[i + 0*nelr];
	fc_i_momentum_x.y = fc_momentum_x[i + 1*nelr];
	fc_i_momentum_x.z = fc_momentum_x[i + 2*nelr];

	fc_i_momentum_y.x = fc_momentum_y[i + 0*nelr];
	fc_i_momentum_y.y = fc_momentum_y[i + 1*nelr];
	fc_i_momentum_y.z = fc_momentum_y[i + 2*nelr];

	fc_i_momentum_z.x = fc_momentum_z[i + 0*nelr];
	fc_i_momentum_z.y = fc_momentum_z[i + 1*nelr];
	fc_i_momentum_z.z = fc_momentum_z[i + 2*nelr];

	fc_i_density_energy.x = fc_density_energy[i + 0*nelr];
	fc_i_density_energy.y = fc_density_energy[i + 1*nelr];
	fc_i_density_energy.z = fc_density_energy[i + 2*nelr];

	double flux_i_density = double(0.0);
	double3 flux_i_momentum;
	flux_i_momentum.x = double(0.0);
	flux_i_momentum.y = double(0.0);
	flux_i_momentum.z = double(0.0);
	double flux_i_density_energy = double(0.0);
		
	double3 velocity_nb;
	double density_nb, density_energy_nb;
	double3 momentum_nb;
	double3 fc_nb_momentum_x, fc_nb_momentum_y, fc_nb_momentum_z;
	double3 fc_nb_density_energy;	
	double speed_sqd_nb, speed_of_sound_nb, pressure_nb;
	
	#pragma unroll
	for(j = 0; j < NNB; j++)
	{
		nb = elements_surrounding_elements[i + j*nelr];
		normal.x = normals[i + (j + 0*NNB)*nelr];
		normal.y = normals[i + (j + 1*NNB)*nelr];
		normal.z = normals[i + (j + 2*NNB)*nelr];
		normal_len = sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
		
		if(nb >= 0) 	// a legitimate neighbor
		{
			density_nb = variables[nb + VAR_DENSITY*nelr];
			momentum_nb.x = variables[nb + (VAR_MOMENTUM+0)*nelr];
			momentum_nb.y = variables[nb + (VAR_MOMENTUM+1)*nelr];
			momentum_nb.z = variables[nb + (VAR_MOMENTUM+2)*nelr];
			density_energy_nb = variables[nb + VAR_DENSITY_ENERGY*nelr];
												compute_velocity(density_nb, momentum_nb, velocity_nb);
			speed_sqd_nb                      = compute_speed_sqd(velocity_nb);
			pressure_nb                       = compute_pressure(density_nb, density_energy_nb, speed_sqd_nb);
			speed_of_sound_nb                 = compute_speed_of_sound(density_nb, pressure_nb);

			fc_nb_momentum_x.x = fc_momentum_x[nb + 0*nelr];
			fc_nb_momentum_x.y = fc_momentum_x[nb + 1*nelr];
			fc_nb_momentum_x.z = fc_momentum_x[nb + 2*nelr];

			fc_nb_momentum_y.x = fc_momentum_y[nb + 0*nelr];
			fc_nb_momentum_y.y = fc_momentum_y[nb + 1*nelr];
			fc_nb_momentum_y.z = fc_momentum_y[nb + 2*nelr];

			fc_nb_momentum_z.x = fc_momentum_z[nb + 0*nelr];
			fc_nb_momentum_z.y = fc_momentum_z[nb + 1*nelr];
			fc_nb_momentum_z.z = fc_momentum_z[nb + 2*nelr];

			fc_nb_density_energy.x = fc_density_energy[nb + 0*nelr];
			fc_nb_density_energy.y = fc_density_energy[nb + 1*nelr];
			fc_nb_density_energy.z = fc_density_energy[nb + 2*nelr];
			
			// artificial viscosity
			factor = -normal_len*smoothing_coefficient*double(0.5)*(speed_i + sqrt(speed_sqd_nb) + speed_of_sound_i + speed_of_sound_nb);
			flux_i_density += factor*(density_i-density_nb);
			flux_i_density_energy += factor*(density_energy_i-density_energy_nb);
			flux_i_momentum.x += factor*(momentum_i.x-momentum_nb.x);
			flux_i_momentum.y += factor*(momentum_i.y-momentum_nb.y);
			flux_i_momentum.z += factor*(momentum_i.z-momentum_nb.z);

			// accumulate cell-centered fluxes
			factor = double(0.5)*normal.x;
			flux_i_density += factor*(momentum_nb.x+momentum_i.x);
			flux_i_density_energy += factor*(fc_nb_density_energy.x+fc_i_density_energy.x);
			flux_i_momentum.x += factor*(fc_nb_momentum_x.x+fc_i_momentum_x.x);
			flux_i_momentum.y += factor*(fc_nb_momentum_y.x+fc_i_momentum_y.x);
			flux_i_momentum.z += factor*(fc_nb_momentum_z.x+fc_i_momentum_z.x);
			
			factor = double(0.5)*normal.y;
			flux_i_density += factor*(momentum_nb.y+momentum_i.y);
			flux_i_density_energy += factor*(fc_nb_density_energy.y+fc_i_density_energy.y);
			flux_i_momentum.x += factor*(fc_nb_momentum_x.y+fc_i_momentum_x.y);
			flux_i_momentum.y += factor*(fc_nb_momentum_y.y+fc_i_momentum_y.y);
			flux_i_momentum.z += factor*(fc_nb_momentum_z.y+fc_i_momentum_z.y);
			
			factor = double(0.5)*normal.z;
			flux_i_density += factor*(momentum_nb.z+momentum_i.z);
			flux_i_density_energy += factor*(fc_nb_density_energy.z+fc_i_density_energy.z);
			flux_i_momentum.x += factor*(fc_nb_momentum_x.z+fc_i_momentum_x.z);
			flux_i_momentum.y += factor*(fc_nb_momentum_y.z+fc_i_momentum_y.z);
			flux_i_momentum.z += factor*(fc_nb_momentum_z.z+fc_i_momentum_z.z);
		}
		else if(nb == -1)	// a wing boundary
		{
			flux_i_momentum.x += normal.x*pressure_i;
			flux_i_momentum.y += normal.y*pressure_i;
			flux_i_momentum.z += normal.z*pressure_i;
		}
		else if(nb == -2) // a far field boundary
		{
			factor = double(0.5)*normal.x;
			flux_i_density += factor*(ff_variable[VAR_MOMENTUM+0]+momentum_i.x);
			flux_i_density_energy += factor*(ff_fc_density_energy[0].x+fc_i_density_energy.x);
			flux_i_momentum.x += factor*(ff_fc_momentum_x[0].x + fc_i_momentum_x.x);
			flux_i_momentum.y += factor*(ff_fc_momentum_y[0].x + fc_i_momentum_y.x);
			flux_i_momentum.z += factor*(ff_fc_momentum_z[0].x + fc_i_momentum_z.x);
			
			factor = double(0.5)*normal.y;
			flux_i_density += factor*(ff_variable[VAR_MOMENTUM+1]+momentum_i.y);
			flux_i_density_energy += factor*(ff_fc_density_energy[0].y+fc_i_density_energy.y);
			flux_i_momentum.x += factor*(ff_fc_momentum_x[0].y + fc_i_momentum_x.y);
			flux_i_momentum.y += factor*(ff_fc_momentum_y[0].y + fc_i_momentum_y.y);
			flux_i_momentum.z += factor*(ff_fc_momentum_z[0].y + fc_i_momentum_z.y);

			factor = double(0.5)*normal.z;
			flux_i_density += factor*(ff_variable[VAR_MOMENTUM+2]+momentum_i.z);
			flux_i_density_energy += factor*(ff_fc_density_energy[0].z+fc_i_density_energy.z);
			flux_i_momentum.x += factor*(ff_fc_momentum_x[0].z + fc_i_momentum_x.z);
			flux_i_momentum.y += factor*(ff_fc_momentum_y[0].z + fc_i_momentum_y.z);
			flux_i_momentum.z += factor*(ff_fc_momentum_z[0].z + fc_i_momentum_z.z);

		}
	}

	fluxes[i + VAR_DENSITY*nelr] = flux_i_density;
	fluxes[i + (VAR_MOMENTUM+0)*nelr] = flux_i_momentum.x;
	fluxes[i + (VAR_MOMENTUM+1)*nelr] = flux_i_momentum.y;
	fluxes[i + (VAR_MOMENTUM+2)*nelr] = flux_i_momentum.z;
	fluxes[i + VAR_DENSITY_ENERGY*nelr] = flux_i_density_energy;
}

void cuda_time_step(int j, int nelr, double* old_variables, double* variables, double* step_factors, double* fluxes)
{
	const int i = (BLOCK_DIM_X*_bid_x + _tid_x);

	double factor = step_factors[i]/double(RK+1-j);

	variables[i + VAR_DENSITY*nelr] = old_variables[i + VAR_DENSITY*nelr] + factor*fluxes[i + VAR_DENSITY*nelr];
	variables[i + VAR_DENSITY_ENERGY*nelr] = old_variables[i + VAR_DENSITY_ENERGY*nelr] + factor*fluxes[i + VAR_DENSITY_ENERGY*nelr];
	variables[i + (VAR_MOMENTUM+0)*nelr] = old_variables[i + (VAR_MOMENTUM+0)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+0)*nelr];
	variables[i + (VAR_MOMENTUM+1)*nelr] = old_variables[i + (VAR_MOMENTUM+1)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+1)*nelr];	
	variables[i + (VAR_MOMENTUM+2)*nelr] = old_variables[i + (VAR_MOMENTUM+2)*nelr] + factor*fluxes[i + (VAR_MOMENTUM+2)*nelr];	
}
