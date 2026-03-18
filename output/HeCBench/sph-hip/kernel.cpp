#include "kernel.h"

// --- from fluid.cu ---
double W(double3 p_pos, double3 q_pos, double h)
{
    double r = sqrt((p_pos.x-q_pos.x)*(p_pos.x-q_pos.x)
                  + (p_pos.y-q_pos.y)*(p_pos.y-q_pos.y)
                  + (p_pos.z-q_pos.z)*(p_pos.z-q_pos.z));
    double C = 1.0/(M_PI*h*h*h);
    double u = r/h;
    double val = 0.0;
    if(u >= 2.0)
        return val;
    else if(u < 1.0 )
        val = 1.0 - (3.0/2.0)*u*u + (3.0/4.0)*u*u*u;
    else if(u >= 1.0 && u < 2.0)
        val = (1.0/4.0) * pow(2.0-u,3.0);

    val *= C;
    return val;
}

double del_W(double3 p_pos, double3 q_pos, double h)
{
    double r = sqrt((p_pos.x-q_pos.x)*(p_pos.x-q_pos.x)
                  + (p_pos.y-q_pos.y)*(p_pos.y-q_pos.y)
                  + (p_pos.z-q_pos.z)*(p_pos.z-q_pos.z));
    double C = 1.0/(M_PI * h*h*h);
    double u = r/h;
    double val = 0.0;
    if(u >= 2.0)
        return val;
    else if(u < 1.0 )
        val = -1.0/(h*h) * (3.0 - 9.0/4.0*u);
    else if(u >= 1.0 && u < 2.0)
        val = -3.0/(4.0*h*r) * pow(2.0-u,2.0);

    val *= C;
    return val;
}

double boundaryGamma(double3 p_pos, double3 k_pos, double3 k_n, double h, double speed_sound)
{
    // Radial distance between p,q
    double r = sqrt((p_pos.x-k_pos.x)*(p_pos.x-k_pos.x)
                  + (p_pos.y-k_pos.y)*(p_pos.y-k_pos.y)
                  + (p_pos.z-k_pos.z)*(p_pos.z-k_pos.z));
    // Distance to p normal to surface particle
    double y = sqrt((p_pos.x-k_pos.x)*(p_pos.x-k_pos.x)*(k_n.x*k_n.x)
                  + (p_pos.y-k_pos.y)*(p_pos.y-k_pos.y)*(k_n.y*k_n.y)
                  + (p_pos.z-k_pos.z)*(p_pos.z-k_pos.z)*(k_n.z*k_n.z));
    // Tangential distance
    double x = r-y;

    double u = y/h;
    double xi = (1-x/h)?x<h:0.0;
    double C = xi*2.0*0.02 * speed_sound * speed_sound / y;
    double val = 0.0;

    if(u > 0.0 && u < 2.0/3.0)
        val = 2.0/3.0;
    else if(u < 1.0 && u > 2.0/3.0 )
        val = (2*u - 3.0/2.0*u*u);
    else if (u < 2.0 && u > 1.0)
        val = 0.5*(2.0-u)*(2.0-u);
    else
        val = 0.0;

    val *= C;

    return val;
}

double computeDensity(double3 p_pos, double3 p_v, double3 q_pos, double3 q_v,
                      const param *params)
{
    double v_x = (p_v.x - q_v.x);
    double v_y = (p_v.y - q_v.y);
    double v_z = (p_v.z - q_v.z);

    double density = params->mass_particle * del_W(p_pos,q_pos,
                                                   params->smoothing_radius);
    double density_x = density * v_x * (p_pos.x - q_pos.x);
    double density_y = density * v_y * (p_pos.y - q_pos.y);
    double density_z = density * v_z * (p_pos.z - q_pos.z);

    density = (density_x + density_y + density_z)*params->time_step;

    return density;
}

double computePressure(double p_density, const param *params)
{
    double gam = 7.0;
    double B = params->rest_density * params->speed_sound*params->speed_sound / gam;
    double pressure =  B * (pow((p_density/params->rest_density),gam) - 1.0);

    return pressure;
}
extern "C"

void updatePressures(fluid_particle * fluid_particles,
                     const param * params)
{
    #pragma HLS INTERFACE m_axi port=fluid_particles offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=params offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int num_fluid_particles = params->number_fluid_particles;
            int i = BLOCK_DIM_X * _bid_x + _tid_x;
            if (i >= num_fluid_particles) return;
            double3 p_pos = fluid_particles[i].pos;
            double3 p_v   = fluid_particles[i].v;
            double density = fluid_particles[i].density;

            for(int j=0; j< num_fluid_particles; j++) {
            double3 q_pos = fluid_particles[j].pos;
            double3 q_v   = fluid_particles[j].v;
            density += computeDensity(p_pos,p_v,q_pos,q_v, params);
            }
            fluid_particles[i].density = density;
            fluid_particles[i].pressure = computePressure(density, params);

        }
    }
}

double3 computeBoundaryAcceleration(double3 p_pos, double3 k_pos, double3 k_n,
                                    double h, double speed_sound)
{
    double3 p_a;
    double bGamma = boundaryGamma(p_pos,k_pos,k_n,h,speed_sound);
    p_a.x = bGamma * k_n.x;
    p_a.y = bGamma * k_n.y;
    p_a.z = bGamma * k_n.z;

    return p_a;
}

double3 computeAcceleration(double3 p_pos, double3 p_v, double p_density,
                            double p_pressure, double3 q_pos, double3 q_v,
                            double q_density, double q_pressure, const param *const params)
{
    double3 a;
    double accel;
    double h = params->smoothing_radius;
    double alpha = params->alpha;
    double speed_sound = params->speed_sound;
    double mass_particle = params->mass_particle;
    double surface_tension = params->surface_tension;

    // Pressure force
    accel = (p_pressure/(p_density*p_density) + q_pressure/(q_density*q_density))
            * mass_particle * del_W(p_pos,q_pos,h);
    a.x = -accel * (p_pos.x - q_pos.x);
    a.y = -accel * (p_pos.y - q_pos.y);
    a.z = -accel * (p_pos.z - q_pos.z);

    // Viscosity force
    double VdotR = (p_v.x-q_v.x)*(p_pos.x-q_pos.x)
                 + (p_v.y-q_v.y)*(p_pos.y-q_pos.y)
                 + (p_v.z-q_v.z)*(p_pos.z-q_pos.z);
    if(VdotR < 0.0)
    {
        double nu = 2.0 * alpha * h * speed_sound / (p_density + q_density);
        double r2 = (p_pos.x-q_pos.x)*(p_pos.x-q_pos.x)
                  + (p_pos.y-q_pos.y)*(p_pos.y-q_pos.y)
                  + (p_pos.z-q_pos.z)*(p_pos.z-q_pos.z);
        double eps = h/10.0;
        double stress = nu * VdotR / (r2 + eps*h*h);
        accel = mass_particle * stress * del_W(p_pos, q_pos, h);
        a.x += accel * (p_pos.x - q_pos.x);
        a.y += accel * (p_pos.y - q_pos.y);
        a.z += accel * (p_pos.z - q_pos.z);
    }

    //Surface tension
    // BT 07 http://cg.informatik.uni-freiburg.de/publications/2011_GRAPP_airBubbles.pdf
    accel = surface_tension * W(p_pos,q_pos,h);
    a.x += accel * (p_pos.x - q_pos.x);
    a.y += accel * (p_pos.y - q_pos.y);
    a.z += accel * (p_pos.z - q_pos.z);

    return a;
}
extern "C"

void updateAccelerationsFP(fluid_particle * fluid_particles,
                           const param * params)
{
    #pragma HLS INTERFACE m_axi port=fluid_particles offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=params offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int num_fluid_particles = params->number_fluid_particles;

            int i = BLOCK_DIM_X * _bid_x + _tid_x;
            if (i >= num_fluid_particles) return;

            double ax = 0.0;
            double ay = 0.0;
            double az = -9.8;

            double3 p_pos = fluid_particles[i].pos;
            double3 p_v   = fluid_particles[i].v;
            double p_density = fluid_particles[i].density;
            double p_pressure = fluid_particles[i].pressure;

            for(int j=0; j<num_fluid_particles; j++) {
            if (i!=j) {
            double3 q_pos = fluid_particles[j].pos;
            double3 q_v   = fluid_particles[j].v;
            double q_density = fluid_particles[j].density;
            double q_pressure = fluid_particles[j].pressure;
            double3 tmp_a = computeAcceleration(p_pos, p_v, p_density,
            p_pressure, q_pos, q_v,
            q_density, q_pressure, params);

            ax += tmp_a.x;
            ay += tmp_a.y;
            az += tmp_a.z;
            }
            }

            fluid_particles[i].a.x = ax;
            fluid_particles[i].a.y = ay;
            fluid_particles[i].a.z = az;

        }
    }
}
extern "C"

void updateAccelerationsBP(fluid_particle * fluid_particles,
                           const boundary_particle * boundary_particles, 
                           const param * params)
{
    #pragma HLS INTERFACE m_axi port=fluid_particles offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=boundary_particles offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=params offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int num_fluid_particles = params->number_fluid_particles;
            int num_boundary_particles = params->number_boundary_particles;
            int i = BLOCK_DIM_X * _bid_x + _tid_x;
            if (i >= num_fluid_particles) return;

            double ax = fluid_particles[i].a.x;
            double ay = fluid_particles[i].a.y;
            double az = fluid_particles[i].a.z;
            double3 p_pos = fluid_particles[i].pos;

            for (int j=0; j<num_boundary_particles; j++) {
            double3 k_pos = boundary_particles[j].pos;
            double3 k_n   = boundary_particles[j].n;
            double3 tmp_a = computeBoundaryAcceleration(p_pos,k_pos,k_n,
            params->smoothing_radius,
            params->speed_sound);
            ax += tmp_a.x;
            ay += tmp_a.y;
            az += tmp_a.z;
            }

            fluid_particles[i].a.x = ax;
            fluid_particles[i].a.y = ay;
            fluid_particles[i].a.z = az;

        }
    }
}
extern "C"

void updatePositions(fluid_particle * fluid_particles,
                     const param * params)
{
    #pragma HLS INTERFACE m_axi port=fluid_particles offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=params offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            double dt = params->time_step;

            int num_fluid_particles = params->number_fluid_particles;
            int i = BLOCK_DIM_X * _bid_x + _tid_x;
            if (i >= num_fluid_particles) return;

            // Velocity at t + dt/2
            double3 v_half = fluid_particles[i].v_half;
            double3 v      = fluid_particles[i].v;
            double3 pos    = fluid_particles[i].pos;
            double3 a      = fluid_particles[i].a;

            v_half.x = v_half.x + dt * a.x;
            v_half.y = v_half.y + dt * a.y;
            v_half.z = v_half.z + dt * a.z;

            // Velocity at t + dt, must estimate for foce calc
            v.x = v_half.x + a.x * (dt / 2.0);
            v.y = v_half.y + a.y * (dt / 2.0);
            v.z = v_half.z + a.z * (dt / 2.0);

            // Position at time t + dt
            pos.x = pos.x + dt * v_half.x;
            pos.y = pos.y + dt * v_half.y;
            pos.z = pos.z + dt * v_half.z;

            fluid_particles[i].v_half = v_half;
            fluid_particles[i].v      = v;
            fluid_particles[i].pos    = pos;

        }
    }
}
