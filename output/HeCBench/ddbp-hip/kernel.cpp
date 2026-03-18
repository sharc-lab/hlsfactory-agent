#include "kernel.h"

// --- from main.cu ---
extern "C"
void pad_projections_kernel(
    double* d_img,
    const int nDetXMap,
    const int nDetYMap,
    const int nElem,
    const int np)
{
    #pragma HLS INTERFACE m_axi port=d_img offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=nDetXMap
    #pragma HLS INTERFACE s_axilite port=nDetYMap
    #pragma HLS INTERFACE s_axilite port=nElem
    #pragma HLS INTERFACE s_axilite port=np
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            const int gid = _bid_x * BLOCK_DIM_X + _tid_x;
                            if (gid < nElem)
                            d_img[(np*nDetYMap *nDetXMap) + (gid*nDetYMap)] = 0;

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void map_boudaries_kernel(
    double* d_pBound,
    const int nElem,
    const double valueLeftBound,
    const double sizeElem,
    const double offset)
{
    #pragma HLS INTERFACE m_axi port=d_pBound offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=nElem
    #pragma HLS INTERFACE s_axilite port=valueLeftBound
    #pragma HLS INTERFACE s_axilite port=sizeElem
    #pragma HLS INTERFACE s_axilite port=offset
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            const int gid = _bid_x * BLOCK_DIM_X + _tid_x;
                            if (gid < nElem)
                            d_pBound[gid] = (gid - valueLeftBound) * sizeElem + offset;

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void rot_detector_kernel(
          double*  d_pRdetY,
          double*  d_pRdetZ,
    const double*  d_pYcoord,
    const double*  d_pZcoord,
    const double yOffset,
    const double zOffset,
    const double phi,
    const int nElem)
{
    #pragma HLS INTERFACE m_axi port=d_pRdetY offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_pRdetZ offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_pYcoord offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_pZcoord offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=yOffset
    #pragma HLS INTERFACE s_axilite port=zOffset
    #pragma HLS INTERFACE s_axilite port=phi
    #pragma HLS INTERFACE s_axilite port=nElem
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            const int gid = _bid_x * BLOCK_DIM_X + _tid_x;
                            if (gid < nElem) {
                            // cos and sin are in measured in radians.
                            d_pRdetY[gid] = ((d_pYcoord[gid] - yOffset) * cos(phi) -
                            (d_pZcoord[gid] - zOffset) * sin(phi)) + yOffset;
                            d_pRdetZ[gid] = ((d_pYcoord[gid] - yOffset) * sin(phi) +
                            (d_pZcoord[gid] - zOffset) * cos(phi)) + zOffset;
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void mapDet2Slice_kernel(
           double*  const pXmapp,
           double*  const pYmapp,
    double tubeX,
    double tubeY,
    double tubeZ,
    const double*  const pXcoord,
    const double*  const pYcoord,
    const double*  const pZcoord,
    const double*  const pZSlicecoord,
    const int nDetXMap,
    const int nDetYMap,
    const int nz)
{
    #pragma HLS INTERFACE m_axi port=pXmapp offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pYmapp offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=tubeX
    #pragma HLS INTERFACE s_axilite port=tubeY
    #pragma HLS INTERFACE s_axilite port=tubeZ
    #pragma HLS INTERFACE m_axi port=pXcoord offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=pYcoord offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=pZcoord offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=pZSlicecoord offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=nDetXMap
    #pragma HLS INTERFACE s_axilite port=nDetYMap
    #pragma HLS INTERFACE s_axilite port=nz
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            const int px = _bid_x * BLOCK_DIM_X + _tid_x;
                            const int py = _bid_y * BLOCK_DIM_Y + _tid_y;

                            if (px < nDetYMap && py < nDetXMap) {

                            const int pos = py * nDetYMap + px;

                            pXmapp[pos] = ((pXcoord[py] - tubeX)*(pZSlicecoord[nz] - pZcoord[px]) -
                            (pXcoord[py] * tubeZ) + (pXcoord[py] * pZcoord[px])) / (-tubeZ + pZcoord[px]);

                            if (py == 0)
                            pYmapp[px] = ((pYcoord[px] - tubeY)*(pZSlicecoord[nz] - pZcoord[px]) -
                            (pYcoord[px] * tubeZ) + (pYcoord[px] * pZcoord[px])) / (-tubeZ + pZcoord[px]);
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void img_integration_kernel(
    double* d_img,
    const int nPixX,
    const int nPixY,
    const bool direction,
    const int offsetX,
    const int offsetY,
    const int nSlices)
{
    #pragma HLS INTERFACE m_axi port=d_img offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=nPixX
    #pragma HLS INTERFACE s_axilite port=nPixY
    #pragma HLS INTERFACE s_axilite port=direction
    #pragma HLS INTERFACE s_axilite port=offsetX
    #pragma HLS INTERFACE s_axilite port=offsetY
    #pragma HLS INTERFACE s_axilite port=nSlices
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            /*
                            Integration of 2D slices over the whole volume

                            (S.1.Integration. - Liu et al(2017))

                            Perform an inclusive scan
                            */

                            const int tx = _bid_x * BLOCK_DIM_X + _tid_x;
                            const int ty = _bid_y * BLOCK_DIM_Y + _tid_y;
                            const int px = tx + offsetX;
                            const int py = ty + offsetY;
                            const int pz = _bid_z * BLOCK_DIM_Z + _tid_z;

                            if (px >= nPixY || py >= nPixX || pz >= nSlices) return;

                            if (direction == integrateXcoord) {

                            for (int s = 1; s <= BLOCK_DIM_Y; s *= 2) {

                            int spot = ty - s;

                            double val = 0;

                            if (spot >= 0) {
                            val = d_img[(pz*nPixY*nPixX) + (offsetY + spot) * nPixY + px];
                            }

                            if (spot >= 0) {
                            d_img[(pz*nPixY*nPixX) + (py * nPixY) + px] += val;
                            }
                            }
                            }
                            else
                            {
                            for (int s = 1; s <= BLOCK_DIM_X; s *= 2) {

                            int spot = tx - s;

                            double val = 0;

                            if (spot >= 0) {
                            val = d_img[(pz*nPixY*nPixX) + py * nPixY + spot + offsetX];
                            }

                            if (spot >= 0) {
                            d_img[(pz*nPixY*nPixX) + (py * nPixY) + px] += val;
                            }
                            }
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void bilinear_interpolation_kernel(
          double*  d_sliceI,
    const double*  d_pProj,
    const double*  d_pObjX,
    const double*  d_pObjY,
    const double*  d_pDetmX,
    const double*  d_pDetmY,
    const int nPixXMap,
    const int nPixYMap,
    const int nDetXMap,
    const int nDetYMap,
    const int nDetX,
    const int nDetY,
    const int np) 
{
    #pragma HLS INTERFACE m_axi port=d_sliceI offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_pProj offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_pObjX offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_pObjY offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=d_pDetmX offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=d_pDetmY offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=nPixXMap
    #pragma HLS INTERFACE s_axilite port=nPixYMap
    #pragma HLS INTERFACE s_axilite port=nDetXMap
    #pragma HLS INTERFACE s_axilite port=nDetYMap
    #pragma HLS INTERFACE s_axilite port=nDetX
    #pragma HLS INTERFACE s_axilite port=nDetY
    #pragma HLS INTERFACE s_axilite port=np
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            const int px = _bid_x * BLOCK_DIM_X + _tid_x;
                            const int py = _bid_y * BLOCK_DIM_Y + _tid_y;

                            // Make sure we don't try and access memory outside the detector
                            // by having any threads mapped there return early
                            if (px >= nPixYMap || py >= nPixXMap) return;

                            //  S.2. Interpolation - Liu et al (2017)

                            // Adjust the mapped coordinates to cross the range of (0-nDetX).*duMap
                            // Divide by pixelSize to get a unitary pixel size
                            const double xNormData = nDetX - d_pObjX[py] / d_pDetmX[0];
                            const int    xData = floor(xNormData);
                            const double alpha = xNormData - xData;

                            // Adjust the mapped coordinates to cross the range of (0-nDetY).*dyMap
                            // Divide by pixelSize to get a unitary pixel size
                            const double yNormData = (d_pObjY[px] / d_pDetmX[0]) - (d_pDetmY[0] / d_pDetmX[0]);
                            const int    yData = floor(yNormData);
                            const double beta = yNormData - yData;

                            double d00, d01, d10, d11;
                            if (((xNormData) >= 0) && ((xNormData) <= nDetX) && ((yNormData) >= 0) && ((yNormData) <= nDetY))
                            d00 = d_pProj[(np*nDetYMap*nDetXMap) + (xData*nDetYMap + yData)];
                            else
                            d00 = 0.0;

                            if (((xData + 1) > 0) && ((xData + 1) <= nDetX) && ((yNormData) >= 0) && ((yNormData) <= nDetY))
                            d10 = d_pProj[(np*nDetYMap*nDetXMap) + ((xData + 1)*nDetYMap + yData)];
                            else
                            d10 = 0.0;

                            if (((xNormData) >= 0) && ((xNormData) <= nDetX) && ((yData + 1) > 0) && ((yData + 1) <= nDetY))
                            d01 = d_pProj[(np*nDetYMap*nDetXMap) + (xData*nDetYMap + yData + 1)];
                            else
                            d01 = 0.0;

                            if (((xData + 1) > 0) && ((xData + 1) <= nDetX) && ((yData + 1) > 0) && ((yData + 1) <= nDetY))
                            d11 = d_pProj[(np*nDetYMap*nDetXMap) + ((xData + 1)*nDetYMap + yData + 1)];
                            else
                            d11 = 0.0;

                            double result_temp1 = alpha * d10 + (-d00 * alpha + d00);
                            double result_temp2 = alpha * d11 + (-d01 * alpha + d01);

                            d_sliceI[py * nPixYMap + px] = beta * result_temp2 + (-result_temp1 * beta + result_temp1);

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void differentiation_kernel(
          double*  d_pVolume,
    const double*  d_sliceI,
    double tubeX,
    double rtubeY,
    double rtubeZ,
    const double*  const d_pObjX,
    const double*  const d_pObjY,
    const double*  const d_pObjZ,
    const int nPixX,
    const int nPixY,
    const int nPixXMap,
    const int nPixYMap,
    const double du,
    const double dv,
    const double dx,
    const double dy,
    const double dz,
    const int nz) 
{
    #pragma HLS INTERFACE m_axi port=d_pVolume offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_sliceI offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=tubeX
    #pragma HLS INTERFACE s_axilite port=rtubeY
    #pragma HLS INTERFACE s_axilite port=rtubeZ
    #pragma HLS INTERFACE m_axi port=d_pObjX offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_pObjY offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=d_pObjZ offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=nPixX
    #pragma HLS INTERFACE s_axilite port=nPixY
    #pragma HLS INTERFACE s_axilite port=nPixXMap
    #pragma HLS INTERFACE s_axilite port=nPixYMap
    #pragma HLS INTERFACE s_axilite port=du
    #pragma HLS INTERFACE s_axilite port=dv
    #pragma HLS INTERFACE s_axilite port=dx
    #pragma HLS INTERFACE s_axilite port=dy
    #pragma HLS INTERFACE s_axilite port=dz
    #pragma HLS INTERFACE s_axilite port=nz
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            const int px = _bid_x * BLOCK_DIM_X + _tid_x;
                            const int py = _bid_y * BLOCK_DIM_Y + _tid_y;

                            /*
                            S.3. Differentiation - Eq. 24 - Liu et al (2017)

                            Detector integral projection
                            ___________
                            |_A_|_B_|___|
                            |_C_|_D_|___|
                            |___|___|___|

                            (px,py)
                            ________________
                            |_A_|__B__|_____|
                            |_C_|(0,0)|(0,1)|
                            |___|(1,0)|(1,1)|

                            Threads are lauched from D up to nPixX (py) and nPixY (px)
                            i.e., they are running on the detector image. Thread (0,0) is on D.

                            Coordinates on intergal projection:

                            A = py * nPixYMap + px
                            B = ((py+1) * nPixYMap) + px
                            C = py * nPixYMap + px + 1
                            D = ((py+1) * nPixYMap) + px + 1
                            */

                            if (px < nPixY && py < nPixX) {

                            const int pos = (nPixX*nPixY*nz) + (py * nPixY) + px;

                            int coordA = py * nPixYMap + px;
                            int coordB = ((py + 1) * nPixYMap) + px;
                            int coordC = coordA + 1;
                            int coordD = coordB + 1;

                            // x - ray angle in X coord
                            double gamma = atan((d_pObjX[py] + (dx / 2.0) - tubeX) / (rtubeZ - d_pObjZ[nz]));

                            // x - ray angle in Y coord
                            double alpha = atan((d_pObjY[px] + (dy / 2.0) - rtubeY) / (rtubeZ - d_pObjZ[nz]));

                            double dA, dB, dC, dD;

                            dA = d_sliceI[coordA];
                            dB = d_sliceI[coordB];
                            dC = d_sliceI[coordC];
                            dD = d_sliceI[coordD];

                            // Treat border of interpolated integral detector
                            if (dC == 0 && dD == 0) {
                            dC = dA;
                            dD = dB;
                            }

                            // S.3.Differentiation - Eq. 24 - Liu et al(2017)
                            d_pVolume[pos] += ((dD - dC - dB + dA)*(du*dv*dz / (cos(alpha)*cos(gamma)*dx*dy)));
                            }

                        }
                    }
                }
            }
        }
    }
}
extern "C"

void division_kernel(
    double* d_img,
    const int nPixX,
    const int nPixY,
    const int nSlices,
    const int nProj)
{
    #pragma HLS INTERFACE m_axi port=d_img offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=nPixX
    #pragma HLS INTERFACE s_axilite port=nPixY
    #pragma HLS INTERFACE s_axilite port=nSlices
    #pragma HLS INTERFACE s_axilite port=nProj
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
            for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
                for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                        #pragma HLS PIPELINE II=1

                            const int px = _bid_x * BLOCK_DIM_X + _tid_x;
                            const int py = _bid_y * BLOCK_DIM_Y + _tid_y;
                            const int pz = _bid_z * BLOCK_DIM_Z + _tid_z;

                            if (px < nPixY && py < nPixX && pz < nSlices) {
                            const int pos = (nPixX*nPixY*pz) + (py * nPixY) + px;
                            d_img[pos] /= (double) nProj;
                            }

                        }
                    }
                }
            }
        }
    }
}
