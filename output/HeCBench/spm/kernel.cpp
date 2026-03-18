#include "kernel.h"

// --- from main.cu ---
float interp(const int3 d, const unsigned char f[], float x, float y, float z)
{
  int ix, iy, iz;
  float dx1, dy1, dz1, dx2, dy2, dz2;
  int k111,k112,k121,k122,k211,k212,k221,k222;
  float vf;
  const unsigned char *ff;

  ix = floorf(x); dx1=x-ix; dx2=1.f-dx1;
  iy = floorf(y); dy1=y-iy; dy2=1.f-dy1;
  iz = floorf(z); dz1=z-iz; dz2=1.f-dz1;

  ff   = f + ix-1+d.x*(iy-1+d.y*(iz-1));
  k222 = ff[   0]; k122 = ff[     1];
  k212 = ff[d.x]; k112 = ff[d.x+1];
  ff  += d.x*d.y;
  k221 = ff[   0]; k121 = ff[     1];
  k211 = ff[d.x]; k111 = ff[d.x+1];

  vf = (((k222*dx2+k122*dx1)*dy2 + (k212*dx2+k112*dx1)*dy1))*dz2 +
       (((k221*dx2+k121*dx1)*dy2 + (k211*dx2+k111*dx1)*dy1))*dz1;

  return(vf);
}
extern "C"

void spm (
  const float * M, 
  const int data_size,
  const unsigned char * g_d,
  const unsigned char * f_d,
  const int3 dg,
  const int3 df,
  unsigned char * ivf_d,
  unsigned char * ivg_d,
  bool * data_threshold_d)
{
    #pragma HLS INTERFACE m_axi port=M offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=data_size
    #pragma HLS INTERFACE m_axi port=g_d offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=f_d offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=dg
    #pragma HLS INTERFACE s_axilite port=df
    #pragma HLS INTERFACE m_axi port=ivf_d offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=ivg_d offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=data_threshold_d offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            // 97 random values
            const float ran[] = {
            0.656619,0.891183,0.488144,0.992646,0.373326,0.531378,0.181316,0.501944,0.422195,
            0.660427,0.673653,0.95733,0.191866,0.111216,0.565054,0.969166,0.0237439,0.870216,
            0.0268766,0.519529,0.192291,0.715689,0.250673,0.933865,0.137189,0.521622,0.895202,
            0.942387,0.335083,0.437364,0.471156,0.14931,0.135864,0.532498,0.725789,0.398703,
            0.358419,0.285279,0.868635,0.626413,0.241172,0.978082,0.640501,0.229849,0.681335,
            0.665823,0.134718,0.0224933,0.262199,0.116515,0.0693182,0.85293,0.180331,0.0324186,
            0.733926,0.536517,0.27603,0.368458,0.0128863,0.889206,0.866021,0.254247,0.569481,
            0.159265,0.594364,0.3311,0.658613,0.863634,0.567623,0.980481,0.791832,0.152594,
            0.833027,0.191863,0.638987,0.669,0.772088,0.379818,0.441585,0.48306,0.608106,
            0.175996,0.00202556,0.790224,0.513609,0.213229,0.10345,0.157337,0.407515,0.407757,
            0.0526927,0.941815,0.149972,0.384374,0.311059,0.168534,0.896648};

            const int idx = _bid_x * NUM_THREADS + _tid_x;

            int x_datasize=(dg.x-2);
            int y_datasize=(dg.y-2);

            for(int i = idx; i < data_size; i += NUM_THREADS*NUM_BLOCKS)
            {
            float xx_temp = (i%x_datasize)+1.f;
            float yy_temp = ((int)floorf((float)i/x_datasize)%y_datasize)+1.f;
            float zz_temp = (floorf((float)i/x_datasize))/y_datasize+1.f;

            // generate rx,ry,rz coordinates
            float rx = xx_temp + ran[i%97];
            float ry = yy_temp + ran[i%97];
            float rz = zz_temp + ran[i%97];

            // rigid transformation over rx,ry,rz coordinates
            float xp = M[0]*rx + M[4]*ry + M[ 8]*rz + M[12];
            float yp = M[1]*rx + M[5]*ry + M[ 9]*rz+ M[13];
            float zp = M[2]*rx + M[6]*ry + M[10]*rz+ M[14];

            if (zp>=1.f && zp<df.z && yp>=1.f && yp<df.y && xp>=1.f && xp<df.x)
            {
            // interpolation
            ivf_d[i] = floorf(interp(df, f_d, xp,yp,zp)+0.5f);
            ivg_d[i] = floorf(interp(dg, g_d, rx,ry,rz)+0.5f);
            data_threshold_d[i] = true;
            }
            else
            {
            ivf_d[i] = 0;
            ivg_d[i] = 0;
            data_threshold_d[i] = false;
            }
            }

        }
    }
}
