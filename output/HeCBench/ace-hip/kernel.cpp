#include "kernel.h"

// --- from main.cu ---
double dFphi(double phi, double u, double lambda)
{
  return (-phi*(1.0-phi*phi)+lambda*u*(1.0-phi*phi)*(1.0-phi*phi));
}

double GradientX(double phi[][DATAYSIZE][DATAZSIZE],
                 double dx, double dy, double dz, int x, int y, int z)
{
  return (phi[x+1][y][z] - phi[x-1][y][z]) / (2.0*dx);
}

double GradientY(double phi[][DATAYSIZE][DATAZSIZE],
                 double dx, double dy, double dz, int x, int y, int z)
{
  return (phi[x][y+1][z] - phi[x][y-1][z]) / (2.0*dy);
}

double GradientZ(double phi[][DATAYSIZE][DATAZSIZE],
                 double dx, double dy, double dz, int x, int y, int z)
{
  return (phi[x][y][z+1] - phi[x][y][z-1]) / (2.0*dz);
}

double Divergence(double phix[][DATAYSIZE][DATAZSIZE],
                  double phiy[][DATAYSIZE][DATAZSIZE],
                  double phiz[][DATAYSIZE][DATAZSIZE],
                  double dx, double dy, double dz, int x, int y, int z)
{
  return GradientX(phix,dx,dy,dz,x,y,z) +
         GradientY(phiy,dx,dy,dz,x,y,z) +
         GradientZ(phiz,dx,dy,dz,x,y,z);
}

double Laplacian(double phi[][DATAYSIZE][DATAZSIZE],
                 double dx, double dy, double dz, int x, int y, int z)
{
  double phixx = (phi[x+1][y][z] + phi[x-1][y][z] - 2.0 * phi[x][y][z]) / SQ(dx);
  double phiyy = (phi[x][y+1][z] + phi[x][y-1][z] - 2.0 * phi[x][y][z]) / SQ(dy);
  double phizz = (phi[x][y][z+1] + phi[x][y][z-1] - 2.0 * phi[x][y][z]) / SQ(dz);
  return phixx + phiyy + phizz;
}

double An(double phix, double phiy, double phiz, double epsilon)
{
  if (phix != 0.0 || phiy != 0.0 || phiz != 0.0){
    return ((1.0 - 3.0 * epsilon) * (1.0 + (((4.0 * epsilon) / (1.0-3.0*epsilon))*
           ((SQ(phix)*SQ(phix)+SQ(phiy)*SQ(phiy)+SQ(phiz)*SQ(phiz)) /
           ((SQ(phix)+SQ(phiy)+SQ(phiz))*(SQ(phix)+SQ(phiy)+SQ(phiz)))))));
  }
  else
  {
    return (1.0-((5.0/3.0)*epsilon));
  }
}

double Wn(double phix, double phiy, double phiz, double epsilon, double W0)
{
  return (W0*An(phix,phiy,phiz,epsilon));
}

double taun(double phix, double phiy, double phiz, double epsilon, double tau0)
{
  return tau0 * SQ(An(phix,phiy,phiz,epsilon));
}

double dFunc(double l, double m, double n)
{
  if (l != 0.0 || m != 0.0 || n != 0.0){
    return (((l*l*l*(SQ(m)+SQ(n)))-(l*(SQ(m)*SQ(m)+SQ(n)*SQ(n)))) /
            ((SQ(l)+SQ(m)+SQ(n))*(SQ(l)+SQ(m)+SQ(n))));
  }
  else
  {
    return 0.0;
  }
}
extern "C"

void calculateForce(double phi[][DATAYSIZE][DATAZSIZE],
                    double Fx[][DATAYSIZE][DATAZSIZE],
                    double Fy[][DATAYSIZE][DATAZSIZE],
                    double Fz[][DATAYSIZE][DATAZSIZE],
                    double dx, double dy, double dz,
                    double epsilon, double W0, double tau0)
{
    #pragma HLS INTERFACE s_axilite port=phi[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=Fx[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=Fy[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=Fz[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=dx
    #pragma HLS INTERFACE s_axilite port=dy
    #pragma HLS INTERFACE s_axilite port=dz
    #pragma HLS INTERFACE s_axilite port=epsilon
    #pragma HLS INTERFACE s_axilite port=W0
    #pragma HLS INTERFACE s_axilite port=tau0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1


                            unsigned iz = _bid_x*BLOCK_DIM_X + _tid_x;
                            unsigned iy = _bid_y*BLOCK_DIM_Y + _tid_y;
                            unsigned ix = _bid_z*BLOCK_DIM_Z + _tid_z;

                            if ((ix < (DATAXSIZE-1)) && (iy < (DATAYSIZE-1)) &&
                            (iz < (DATAZSIZE-1)) && (ix > (0)) &&
                            (iy > (0)) && (iz > (0))) {

                            double phix = GradientX(phi,dx,dy,dz,ix,iy,iz);
                            double phiy = GradientY(phi,dx,dy,dz,ix,iy,iz);
                            double phiz = GradientZ(phi,dx,dy,dz,ix,iy,iz);
                            double sqGphi = SQ(phix) + SQ(phiy) + SQ(phiz);
                            double c = 16.0 * W0 * epsilon;
                            double w = Wn(phix,phiy,phiz,epsilon,W0);
                            double w2 = SQ(w);

                            Fx[ix][iy][iz] = w2 * phix + sqGphi * w * c * dFunc(phix,phiy,phiz);
                            Fy[ix][iy][iz] = w2 * phiy + sqGphi * w * c * dFunc(phiy,phiz,phix);
                            Fz[ix][iy][iz] = w2 * phiz + sqGphi * w * c * dFunc(phiz,phix,phiy);
                            }
                            else
                            {
                            Fx[ix][iy][iz] = 0.0;
                            Fy[ix][iy][iz] = 0.0;
                            Fz[ix][iy][iz] = 0.0;
                            }


                        }
                    }
                }
            }
        }
    }
}
extern "C"

void allenCahn(double phinew[][DATAYSIZE][DATAZSIZE],
               double phiold[][DATAYSIZE][DATAZSIZE],
               double uold[][DATAYSIZE][DATAZSIZE],
               double Fx[][DATAYSIZE][DATAZSIZE],
               double Fy[][DATAYSIZE][DATAZSIZE],
               double Fz[][DATAYSIZE][DATAZSIZE],
               double epsilon, double W0, double tau0, double lambda,
               double dt, double dx, double dy, double dz)
{
    #pragma HLS INTERFACE s_axilite port=phinew[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=phiold[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=uold[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=Fx[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=Fy[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=Fz[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=epsilon
    #pragma HLS INTERFACE s_axilite port=W0
    #pragma HLS INTERFACE s_axilite port=tau0
    #pragma HLS INTERFACE s_axilite port=lambda
    #pragma HLS INTERFACE s_axilite port=dt
    #pragma HLS INTERFACE s_axilite port=dx
    #pragma HLS INTERFACE s_axilite port=dy
    #pragma HLS INTERFACE s_axilite port=dz
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            unsigned iz = _bid_x*BLOCK_DIM_X + _tid_x;
                            unsigned iy = _bid_y*BLOCK_DIM_Y + _tid_y;
                            unsigned ix = _bid_z*BLOCK_DIM_Z + _tid_z;

                            if ((ix < (DATAXSIZE-1)) && (iy < (DATAYSIZE-1)) &&
                            (iz < (DATAZSIZE-1)) && (ix > (0)) &&
                            (iy > (0)) && (iz > (0))) {

                            double phix = GradientX(phiold,dx,dy,dz,ix,iy,iz);
                            double phiy = GradientY(phiold,dx,dy,dz,ix,iy,iz);
                            double phiz = GradientZ(phiold,dx,dy,dz,ix,iy,iz);

                            phinew[ix][iy][iz] = phiold[ix][iy][iz] +
                            (dt / taun(phix,phiy,phiz,epsilon,tau0)) *
                            (Divergence(Fx,Fy,Fz,dx,dy,dz,ix,iy,iz) -
                            dFphi(phiold[ix][iy][iz], uold[ix][iy][iz],lambda));
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void boundaryConditionsPhi(double phinew[][DATAYSIZE][DATAZSIZE])
{
    #pragma HLS INTERFACE s_axilite port=phinew[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            unsigned iz = _bid_x*BLOCK_DIM_X + _tid_x;
                            unsigned iy = _bid_y*BLOCK_DIM_Y + _tid_y;
                            unsigned ix = _bid_z*BLOCK_DIM_Z + _tid_z;
                            if (iz >= DATAZSIZE || iy >= DATAYSIZE || ix >= DATAXSIZE) return;

                            if (ix == 0){
                            phinew[ix][iy][iz] = -1.0;
                            }
                            else if (ix == DATAXSIZE-1){
                            phinew[ix][iy][iz] = -1.0;
                            }
                            else if (iy == 0){
                            phinew[ix][iy][iz] = -1.0;
                            }
                            else if (iy == DATAYSIZE-1){
                            phinew[ix][iy][iz] = -1.0;
                            }
                            else if (iz == 0){
                            phinew[ix][iy][iz] = -1.0;
                            }
                            else if (iz == DATAZSIZE-1){
                            phinew[ix][iy][iz] = -1.0;
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void thermalEquation(double unew[][DATAYSIZE][DATAZSIZE],
                     double uold[][DATAYSIZE][DATAZSIZE],
                     double phinew[][DATAYSIZE][DATAZSIZE],
                     double phiold[][DATAYSIZE][DATAZSIZE],
                     double D, double dt, double dx, double dy, double dz)
{
    #pragma HLS INTERFACE s_axilite port=unew[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=uold[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=phinew[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=phiold[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=D
    #pragma HLS INTERFACE s_axilite port=dt
    #pragma HLS INTERFACE s_axilite port=dx
    #pragma HLS INTERFACE s_axilite port=dy
    #pragma HLS INTERFACE s_axilite port=dz
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            unsigned iz = _bid_x*BLOCK_DIM_X + _tid_x;
                            unsigned iy = _bid_y*BLOCK_DIM_Y + _tid_y;
                            unsigned ix = _bid_z*BLOCK_DIM_Z + _tid_z;

                            if ((ix < (DATAXSIZE-1)) && (iy < (DATAYSIZE-1)) &&
                            (iz < (DATAZSIZE-1)) && (ix > (0)) &&
                            (iy > (0)) && (iz > (0))){
                            unew[ix][iy][iz] = uold[ix][iy][iz] +
                            0.5*(phinew[ix][iy][iz]- phiold[ix][iy][iz]) +
                            dt * D * Laplacian(uold,dx,dy,dz,ix,iy,iz);
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void boundaryConditionsU(double unew[][DATAYSIZE][DATAZSIZE], double delta)
{
    #pragma HLS INTERFACE s_axilite port=unew[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=delta
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            unsigned iz = _bid_x*BLOCK_DIM_X + _tid_x;
                            unsigned iy = _bid_y*BLOCK_DIM_Y + _tid_y;
                            unsigned ix = _bid_z*BLOCK_DIM_Z + _tid_z;
                            if (iz >= DATAZSIZE || iy >= DATAYSIZE || ix >= DATAXSIZE) return;

                            if (ix == 0){
                            unew[ix][iy][iz] =  -delta;
                            }
                            else if (ix == DATAXSIZE-1){
                            unew[ix][iy][iz] =  -delta;
                            }
                            else if (iy == 0){
                            unew[ix][iy][iz] =  -delta;
                            }
                            else if (iy == DATAYSIZE-1){
                            unew[ix][iy][iz] =  -delta;
                            }
                            else if (iz == 0){
                            unew[ix][iy][iz] =  -delta;
                            }
                            else if (iz == DATAZSIZE-1){
                            unew[ix][iy][iz] =  -delta;
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void swapGrid(double cnew[][DATAYSIZE][DATAZSIZE],
              double cold[][DATAYSIZE][DATAZSIZE])
{
    #pragma HLS INTERFACE s_axilite port=cnew[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=cold[][DATAYSIZE][DATAZSIZE]
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            unsigned iz = _bid_x*BLOCK_DIM_X + _tid_x;
                            unsigned iy = _bid_y*BLOCK_DIM_Y + _tid_y;
                            unsigned ix = _bid_z*BLOCK_DIM_Z + _tid_z;
                            if (iz >= DATAZSIZE || iy >= DATAYSIZE || ix >= DATAXSIZE) return;

                            double tmp = cnew[ix][iy][iz];
                            cnew[ix][iy][iz] = cold[ix][iy][iz];
                            cold[ix][iy][iz] = tmp;

                        }
                    }
                }
            }
        }
    }
}
