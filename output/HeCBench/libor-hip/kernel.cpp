#include "kernel.h"

// --- from main.cu ---
void path_calc(float *L, 
               const float *z, 
               const float *lambda, 
               const float delta,
               const int Nmat, 
               const int N)
{
  int   i, n;
  float sqez, lam, con1, v, vrat;

  for(n=0; n<Nmat; n++) {
    sqez = sqrtf(delta)*z[n];
    v = 0.f;

    for (i=n+1; i<N; i++) {
      lam  = lambda[i-n-1];
      con1 = delta*lam;
      v   += __fdividef(con1*L[i],1.f+delta*L[i]);
      vrat = __expf(con1*v + lam*(sqez-0.5f*con1));
      L[i] = L[i]*vrat;
    }
  }
}

void path_calc_b1(float *L, 
                  const float *z, 
                  float *L2,
                  const float *lambda,
                  const float delta,
                  const int Nmat,
                  const int N)
{
  int   i, n;
  float sqez, lam, con1, v, vrat;

  for (i=0; i<N; i++) L2[i] = L[i];
   
  for(n=0; n<Nmat; n++) {
    sqez = sqrtf(delta)*z[n];
    v = 0.f;

    for (i=n+1; i<N; i++) {
      lam  = lambda[i-n-1];
      con1 = delta*lam;
      v   += __fdividef(con1*L[i],1.f+delta*L[i]);
      vrat = __expf(con1*v + lam*(sqez-0.5f*con1));
      L[i] = L[i]*vrat;

      // store these values for reverse path
      L2[i+(n+1)*N] = L[i];
    }
  }
}

void path_calc_b2(float *L_b, 
                  const float *z, 
                  const float *L2, 
                  const float *lambda, 
                  const float delta,
                  const int Nmat,
                  const int N)
{
  int   i, n;
  float faci, v1;

  for (n=Nmat-1; n>=0; n--) {
    v1 = 0.f;
    for (i=N-1; i>n; i--) {
      v1    += lambda[i-n-1]*L2[i+(n+1)*N]*L_b[i];
      faci   = __fdividef(delta,1.f+delta*L2[i+n*N]);
      L_b[i] = L_b[i]*__fdividef(L2[i+(n+1)*N],L2[i+n*N])
              + v1*lambda[i-n-1]*faci*faci;
 
    }
  }
}

float portfolio_b(float *L, 
                  float *L_b,
                  const float *lambda, 
                  const   int *maturities, 
                  const float *swaprates, 
                  const float delta,
                  const int Nmat,
                  const int N,
                  const int Nopt)
{
  int   m, n;
  float b, s, swapval,v;
  float B[NMAT], S[NMAT], B_b[NMAT], S_b[NMAT];

  b = 1.f;
  s = 0.f;
  for (m=0; m<N-Nmat; m++) {
    n    = m + Nmat;
    b    = __fdividef(b,1.f+delta*L[n]);
    s    = s + delta*b;
    B[m] = b;
    S[m] = s;
  }

  v = 0.f;

  for (m=0; m<NMAT; m++) {
    B_b[m] = 0.f;
    S_b[m] = 0.f;
  }

  for (n=0; n<Nopt; n++){
    m = maturities[n] - 1;
    swapval = B[m] + swaprates[n]*S[m] - 1.f;
    if (swapval<0) {
      v     += -100.f*swapval;
      S_b[m] += -100.f*swaprates[n];
      B_b[m] += -100.f;
    }
  }

  for (m=N-Nmat-1; m>=0; m--) {
    n = m + Nmat;
    B_b[m] += delta*S_b[m];
    L_b[n]  = -B_b[m]*B[m]*__fdividef(delta,1.f+delta*L[n]);
    if (m>0) {
      S_b[m-1] += S_b[m];
      B_b[m-1] += __fdividef(B_b[m],1.f+delta*L[n]);
    }
  }

  // apply discount

  b = 1.f;
  for (n=0; n<Nmat; n++) b = b/(1.f+delta*L[n]);

  v = b*v;

  for (n=0; n<Nmat; n++){
    L_b[n] = -v*delta/(1.f+delta*L[n]);
  }

  for (n=Nmat; n<N; n++){
    L_b[n] = b*L_b[n];
  }

  return v;
}

float portfolio(float *L,
                const float *lambda, 
                const   int *maturities, 
                const float *swaprates, 
                const float delta,
                const int Nmat,
                const int N,
                const int Nopt)
{
  int   n, m, i;
  float v, b, s, swapval, B[40], S[40];
	
  b = 1.f;
  s = 0.f;

  for(n=Nmat; n<N; n++) {
    b = b/(1.f+delta*L[n]);
    s = s + delta*b;
    B[n-Nmat] = b;
    S[n-Nmat] = s;
  }

  v = 0.f;

  for(i=0; i<Nopt; i++){
    m = maturities[i] - 1;
    swapval = B[m] + swaprates[i]*S[m] - 1.f;
    if(swapval<0)
      v += -100.f*swapval;
  }

  // apply discount

  b = 1.f;
  for (n=0; n<Nmat; n++) b = b/(1.f+delta*L[n]);

  v = b*v;

  return v;
}
extern "C"

void Pathcalc_Portfolio_KernelGPU(
  float * d_v, 
  float * d_Lb,
  const float * lambda, 
  const   int * maturities, 
  const float * swaprates, 
  const float delta,
  const int Nmat,
  const int N,
  const int Nopt)
{
    #pragma HLS INTERFACE m_axi port=d_v offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_Lb offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=lambda offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=maturities offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=swaprates offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=delta
    #pragma HLS INTERFACE s_axilite port=Nmat
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=Nopt
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int     tid = BLOCK_DIM_X * _bid_x + _tid_x;
            const int threadN = BLOCK_DIM_X * GRID_DIM_X;

            int   i,path;
            float L[NN], L2[L2_SIZE], z[NN];
            float *L_b = L;

            // Monte Carlo LIBOR path calculation

            for(path = tid; path < NPATH; path += threadN){
            // initialise the data for current thread
            for (i=0; i<N; i++) {
            // for real application, z should be randomly generated
            z[i] = 0.3f;
            L[i] = 0.05f;
            }
            path_calc_b1(L, z, L2, lambda, delta, Nmat, N);
            d_v[path] = portfolio_b(L, L_b, lambda, maturities, swaprates, delta, Nmat, N, Nopt);
            path_calc_b2(L_b, z, L2, lambda, delta, Nmat, N);
            d_Lb[path] = L_b[NN-1];
            }

        }
    }
}
extern "C"

void Pathcalc_Portfolio_KernelGPU2(
  float * d_v, 
  const float * lambda, 
  const   int * maturities, 
  const float * swaprates, 
  const float delta,
  const int Nmat,
  const int N,
  const int Nopt)
{
    #pragma HLS INTERFACE m_axi port=d_v offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=lambda offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=maturities offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=swaprates offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=delta
    #pragma HLS INTERFACE s_axilite port=Nmat
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=Nopt
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int     tid = BLOCK_DIM_X * _bid_x + _tid_x;
            const int threadN = BLOCK_DIM_X * GRID_DIM_X;

            int   i, path;
            float L[NN], z[NN];

            // Monte Carlo LIBOR path calculation

            for(path = tid; path < NPATH; path += threadN){
            // initialise the data for current thread
            for (i=0; i<N; i++) {
            // for real application, z should be randomly generated
            z[i] = 0.3f;
            L[i] = 0.05f;
            }
            path_calc(L, z, lambda, delta, Nmat, N);
            d_v[path] = portfolio(L, lambda, maturities, swaprates, delta, Nmat, N, Nopt);
            }

        }
    }
}
