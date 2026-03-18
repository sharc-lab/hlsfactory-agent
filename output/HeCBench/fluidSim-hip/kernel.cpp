#include "kernel.h"

// --- from kernels.cu ---
inline double dot (double4 &a, double4 &b)
{
  return (a.x * b.x + a.y * b.y) + (a.z * b.z + a.w * b.w);
}

double ced(double rho, double weight, const double2 dir, const double2 u)
{
  double u2 = (u.x * u.x) + (u.y * u.y);
  double eu = (dir.x * u.x) + (dir.y * u.y);
  return rho * weight * (1.0 + 3.0 * eu + 4.5 * eu * eu - 1.5 * u2);
}

inline int8 newPos (const int p, const double8 &dir)
{
  int8 np;
  np.s0 = p + (int)dir.s0;
  np.s1 = p + (int)dir.s1;
  np.s2 = p + (int)dir.s2;
  np.s3 = p + (int)dir.s3;
  np.s4 = p + (int)dir.s4;
  np.s5 = p + (int)dir.s5;
  np.s6 = p + (int)dir.s6;
  np.s7 = p + (int)dir.s7;
  return np;
}

inline int8 fma8 (const uint &a, const int8 &b, const int8 &c)
{
  int8 r;
  r.s0 = a * b.s0 + c.s0;
  r.s1 = a * b.s1 + c.s1;
  r.s2 = a * b.s2 + c.s2;
  r.s3 = a * b.s3 + c.s3;
  r.s4 = a * b.s4 + c.s4;
  r.s5 = a * b.s5 + c.s5;
  r.s6 = a * b.s6 + c.s6;
  r.s7 = a * b.s7 + c.s7;
  return r;
}
extern "C"

void lbm (
    const double * if0,
          double * of0, 
    const double4 * if1234,
          double4 * of1234,
    const double4 * if5678,
          double4 * of5678,
    const bool * type,
    const double8 dirX,
    const double8 dirY,
    const double * weight,
    double omega)
{
    #pragma HLS INTERFACE m_axi port=if0 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=of0 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=if1234 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=of1234 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=if5678 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=of5678 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=type offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=dirX
    #pragma HLS INTERFACE s_axilite port=dirY
    #pragma HLS INTERFACE m_axi port=weight offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=omega
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    uint idx = BLOCK_DIM_X * _bid_x + _tid_x;
                    uint idy = BLOCK_DIM_Y * _bid_y + _tid_y;
                    uint width = GRID_DIM_X * BLOCK_DIM_X;
                    uint height = GRID_DIM_Y * BLOCK_DIM_Y;
                    uint pos = idx + width * idy;

                    // Read input distributions
                    double f0 = if0[pos];
                    double4 f1234 = if1234[pos];
                    double4 f5678 = if5678[pos];

                    // intermediate results
                    double e0;
                    double4 e1234;
                    double4 e5678;

                    double rho; // Density
                    double2 u;  // Velocity

                    // Collide
                    if(type[pos]) // Boundary
                    {
                    e0 = f0;
                    // Swap directions
                    // f1234.xyzw = f1234.zwxy;
                    e1234.x = f1234.z;
                    e1234.y = f1234.w;
                    e1234.z = f1234.x;
                    e1234.w = f1234.y;

                    // f5678.xyzw = f5678.zwxy;
                    e5678.x = f5678.z;
                    e5678.y = f5678.w;
                    e5678.z = f5678.x;
                    e5678.w = f5678.y;

                    rho = 0;
                    u = double2(0, 0);
                    }
                    else // Fluid
                    {
                    // Compute rho and u
                    // Rho is computed by doing a reduction on f
                    double4 temp = f1234 + f5678;
                    rho = f0 + temp.x + temp.y + temp.z + temp.w;

                    // Compute velocity in x and y directions
                    double4 x1234 = make_double4(dirX.s0, dirX.s1, dirX.s2, dirX.s3);
                    double4 x5678 = make_double4(dirX.s4, dirX.s5, dirX.s6, dirX.s7);
                    double4 y1234 = make_double4(dirY.s0, dirY.s1, dirY.s2, dirY.s3);
                    double4 y5678 = make_double4(dirY.s4, dirY.s5, dirY.s6, dirY.s7);
                    u.x = (dot(f1234, x1234) + dot(f5678, x5678)) / rho;
                    u.y = (dot(f1234, y1234) + dot(f5678, y5678)) / rho;

                    // Compute f
                    e0 = ced(rho, weight[0], double2(0, 0), u);
                    e1234.x = ced(rho, weight[1], double2(dirX.s0, dirY.s0), u);
                    e1234.y = ced(rho, weight[2], double2(dirX.s1, dirY.s1), u);
                    e1234.z = ced(rho, weight[3], double2(dirX.s2, dirY.s2), u);
                    e1234.w = ced(rho, weight[4], double2(dirX.s3, dirY.s3), u);
                    e5678.x = ced(rho, weight[5], double2(dirX.s4, dirY.s4), u);
                    e5678.y = ced(rho, weight[6], double2(dirX.s5, dirY.s5), u);
                    e5678.z = ced(rho, weight[7], double2(dirX.s6, dirY.s6), u);
                    e5678.w = ced(rho, weight[8], double2(dirX.s7, dirY.s7), u);

                    e0 = (1.0 - omega) * f0 + omega * e0;
                    e1234 = (1.0 - omega) * f1234 + omega * e1234;
                    e5678 = (1.0 - omega) * f5678 + omega * e5678;
                    }

                    // Propagate
                    bool t3 = idx > 0;          // Not on Left boundary
                    bool t1 = idx < width - 1;  // Not on Right boundary
                    bool t4 = idy > 0;          // Not on Upper boundary
                    bool t2 = idy < height - 1; // Not on lower boundary

                    if (t1 && t2 && t3 && t4) {
                    // New positions to write (Each thread will write 8 values)
                    // Note the propagation sources imply the OLD locations for each thread
                    int8 nX = newPos(idx, dirX);
                    int8 nY = newPos(idy, dirY);
                    int8 nPos = fma8(width, nY, nX);

                    // Write center distribution to thread's location
                    of0[pos] = e0;

                    // Propagate to right cell
                    of1234[nPos.s0].x = e1234.x;

                    // Propagate to Lower cell
                    of1234[nPos.s1].y = e1234.y;

                    // Propagate to left cell
                    of1234[nPos.s2].z = e1234.z;

                    // Propagate to Upper cell
                    of1234[nPos.s3].w = e1234.w;

                    // Propagate to Lower-Right cell
                    of5678[nPos.s4].x = e5678.x;

                    // Propogate to Lower-Left cell
                    of5678[nPos.s5].y = e5678.y;

                    // Propagate to Upper-Left cell
                    of5678[nPos.s6].z = e5678.z;

                    // Propagate to Upper-Right cell
                    of5678[nPos.s7].w = e5678.w;
                    }

                }
            }
        }
    }
}
