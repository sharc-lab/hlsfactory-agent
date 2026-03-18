#include "kernel.h"

// --- from main.cu ---
extern "C"
void stencil3d(
    const Real* d_psi, 
          Real* d_npsi, 
    const Real* d_sigmaX, 
    const Real* d_sigmaY, 
    const Real* d_sigmaZ,
    int nx, int ny, int nz)
{
    #pragma HLS INTERFACE m_axi port=d_psi offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_npsi offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_sigmaX offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_sigmaY offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=d_sigmaZ offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=nx
    #pragma HLS INTERFACE s_axilite port=ny
    #pragma HLS INTERFACE s_axilite port=nz
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sm_psi complete dim=1

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1


                        // z is the fastest varying direction
                        Real sm_psi[4][BSIZE][BSIZE];

                        #define V0(y,z) sm_psi[pii][y][z]
                        #define V1(y,z) sm_psi[cii][y][z]
                        #define V2(y,z) sm_psi[nii][y][z]

                        #define sigmaX(x,y,z,dir) d_sigmaX[ z + nz * ( y + ny * ( x + nx * dir ) ) ]
                        #define sigmaY(x,y,z,dir) d_sigmaY[ z + nz * ( y + ny * ( x + nx * dir ) ) ]
                        #define sigmaZ(x,y,z,dir) d_sigmaZ[ z + nz * ( y + ny * ( x + nx * dir ) ) ]

                        #define psi(x,y,z) d_psi[ z + nz * ( (y) + ny * (x) ) ]
                        #define npsi(x,y,z) d_npsi[ z + nz * ( (y) + ny * (x) ) ]

                        const int tjj = _tid_y;
                        const int tkk = _tid_x;

                        // shift for each tile by updating device pointers
                        d_psi = &(psi(XTILE*_bid_x, (BSIZE-2)*_bid_y, (BSIZE-2)*_bid_z));
                        d_npsi = &(npsi(XTILE*_bid_x, (BSIZE-2)*_bid_y, (BSIZE-2)*_bid_z));

                        d_sigmaX = &(sigmaX(XTILE*_bid_x, (BSIZE-2)*_bid_y, (BSIZE-2)*_bid_z, 0));
                        d_sigmaY = &(sigmaY(XTILE*_bid_x, (BSIZE-2)*_bid_y, (BSIZE-2)*_bid_z, 0));
                        d_sigmaZ = &(sigmaZ(XTILE*_bid_x, (BSIZE-2)*_bid_y, (BSIZE-2)*_bid_z, 0));

                        int nLast_x=XTILE+1; int nLast_y=(BSIZE-1); int nLast_z=(BSIZE-1);
                        if (_bid_x == GRID_DIM_X-1) nLast_x = nx-2 - XTILE * _bid_x + 1;
                        if (_bid_y == GRID_DIM_Y-1) nLast_y = ny-2 - (BSIZE-2) * _bid_y + 1;
                        if (_bid_z == GRID_DIM_Z-1) nLast_z = nz-2 - (BSIZE-2) * _bid_z + 1;

                        if(tjj>nLast_y || tkk>nLast_z) return;

                        // previous, current, next, and temp indices
                        int pii,cii,nii,tii;
                        pii=0; cii=1; nii=2;

                        sm_psi[cii][tjj][tkk] = psi(0,tjj,tkk);
                        sm_psi[nii][tjj][tkk] = psi(1,tjj,tkk);
                        Real xcharge,ycharge,zcharge,dV = 0;

                        //initial
                        if ((tkk>0) && (tkk<nLast_z) && (tjj>0) && (tjj<nLast_y))
                        {
                        Real xd=-V1(tjj,tkk) + V2(tjj,tkk);
                        Real yd=(-V1(-1 + tjj,tkk) + V1(1 + tjj,tkk) - V2(-1 + tjj,tkk) + V2(1 + tjj,tkk))/4.;
                        Real zd=(-V1(tjj,-1 + tkk) + V1(tjj,1 + tkk) - V2(tjj,-1 + tkk) + V2(tjj,1 + tkk))/4.;
                        dV -= sigmaX(1,tjj,tkk,0) * xd + sigmaX(1,tjj,tkk,1) * yd + sigmaX(1,tjj,tkk,2) * zd ;
                        }

                        tii=pii; pii=cii; cii=nii; nii=tii;

                        for(int ii=1;ii<nLast_x;ii++)
                        {
                        sm_psi[nii][tjj][tkk] = psi(ii+1,tjj,tkk);

                        // y face current
                        if ((tkk>0) && (tkk<nLast_z) && (tjj<nLast_y))
                        {
                        Real xd=(-V0(tjj,tkk) - V0(1 + tjj,tkk) + V2(tjj,tkk) + V2(1 + tjj,tkk))/4.;
                        Real yd=-V1(tjj,tkk) + V1(1 + tjj,tkk);
                        Real zd=(-V1(tjj,-1 + tkk) + V1(tjj,1 + tkk) - V1(1 + tjj,-1 + tkk) + V1(1 + tjj,1 + tkk))/4.;
                        ycharge = sigmaY(ii,tjj+1,tkk,0) * xd + sigmaY(ii,tjj+1,tkk,1) * yd + sigmaY(ii,tjj+1,tkk,2) * zd ;
                        dV += ycharge;
                        sm_psi[3][tjj][tkk]=ycharge;
                        }

                        if ((tkk>0) && (tkk<nLast_z) && (tjj>0) && (tjj<nLast_y))
                        dV -= sm_psi[3][tjj-1][tkk];  //bring from left

                        // z face current
                        if ((tkk<nLast_z) && (tjj>0) && (tjj<nLast_y))
                        {
                        Real xd=(-V0(tjj,tkk) - V0(tjj,1 + tkk) + V2(tjj,tkk) + V2(tjj,1 + tkk))/4.;
                        Real yd=(-V1(-1 + tjj,tkk) - V1(-1 + tjj,1 + tkk) + V1(1 + tjj,tkk) + V1(1 + tjj,1 + tkk))/4.;
                        Real zd=-V1(tjj,tkk) + V1(tjj,1 + tkk);
                        zcharge = sigmaZ(ii,tjj,tkk+1,0) * xd + sigmaZ(ii,tjj,tkk+1,1) * yd + sigmaZ(ii,tjj,tkk+1,2) * zd ;
                        dV += zcharge;
                        sm_psi[3][tjj][tkk]=zcharge;
                        }

                        if ((tkk>0) && (tkk<nLast_z) && (tjj>0) && (tjj<nLast_y))
                        dV -= sm_psi[3][tjj][tkk-1];

                        // x face current
                        if ((tkk>0) && (tkk<nLast_z) && (tjj>0) && (tjj<nLast_y))
                        {
                        Real xd=-V1(tjj,tkk) + V2(tjj,tkk);
                        Real yd=(-V1(-1 + tjj,tkk) + V1(1 + tjj,tkk) - V2(-1 + tjj,tkk) + V2(1 + tjj,tkk))/4.;
                        Real zd=(-V1(tjj,-1 + tkk) + V1(tjj,1 + tkk) - V2(tjj,-1 + tkk) + V2(tjj,1 + tkk))/4.;
                        xcharge = sigmaX(ii+1,tjj,tkk,0) * xd + sigmaX(ii+1,tjj,tkk,1) * yd + sigmaX(ii+1,tjj,tkk,2) * zd ;
                        dV += xcharge;
                        npsi(ii,tjj,tkk) = dV; //store dV
                        dV = -xcharge; //pass to the next cell in x-dir
                        }
                        tii=pii; pii=cii; cii=nii; nii=tii;
                        }

                    }
                }
            }
        }
    }
}
