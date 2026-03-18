#include "kernel.h"

// --- from compress_kernel.cu ---
extern "C"
void compress(  const long d_Ne, fp *d_I)
{
    #pragma HLS INTERFACE s_axilite port=d_Ne
    #pragma HLS INTERFACE m_axi port=d_I offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int bx = _bid_x;                      // get current horizontal block index
            int tx = _tid_x;                     // get current horizontal thread index
            int ei = (bx*NUMBER_THREADS)+tx;          // unique thread id, more threads than actual elements !!!

            // copy input to output & log uncompress
            if(ei<d_Ne){                              // do only for the number of elements, omit extra threads

            d_I[ei] = log(d_I[ei])*(fp)255;             // exponentiate input IMAGE and copy to output image

            }

        }
    }
}


// --- from extract_kernel.cu ---
extern "C"
void extract(const  long d_Ne, fp *d_I)
{
    #pragma HLS INTERFACE s_axilite port=d_Ne
    #pragma HLS INTERFACE m_axi port=d_I offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            // indexes
            int bx = _bid_x;                      // get current horizontal block index
            int tx = _tid_x;                     // get current horizontal thread index
            int ei = (bx*NUMBER_THREADS)+tx;          // unique thread id, more threads than actual elements !!!

            // copy input to output & log uncompress
            if(ei<d_Ne){                              // do only for the number of elements, omit extra threads

            d_I[ei] = exp(d_I[ei]/(fp)255);             // exponentiate input IMAGE and copy to output image

            }


        }
    }
}


// --- from prepare_kernel.cu ---
extern "C"
void prepare(const  long d_Ne,
                      const fp *d_I,       // pointer to output image (DEVICE GLOBAL MEMORY)
                      fp *d_sums,          // pointer to input image (DEVICE GLOBAL MEMORY)
                      fp *d_sums2)
{

  // indexes
  int bx = _bid_x;                     // get current horizontal block index
  int tx = _tid_x;                    // get current horizontal thread index
  int ei = (bx*NUMBER_THREADS)+tx;         // unique thread id, more threads than actual elements !!!

  // copy input to output & log uncompress
  if(ei<d_Ne){                             // do only for the number of elements, omit extra threads

    d_sums[ei] = d_I[ei];
    d_sums2[ei] = d_I[ei]*d_I[ei];

  }

}


// --- from reduce_kernel.cu ---
extern "C"
void reduce(const  long d_Ne,  // number of elements in array
                    const int d_no,       // number of sums to reduce
                    const int d_mul,      // increment
                    fp *d_sums,           // pointer to partial sums variable (DEVICE GLOBAL MEMORY)
                    fp *d_sums2){

  // indexes
    int bx = _bid_x;                  // get current horizontal block index
  int tx = _tid_x;                   // get current horizontal thread index
  int ei = (bx*NUMBER_THREADS)+tx;        // unique thread id, more threads than actual elements !!!
  int nf = NUMBER_THREADS-(GRID_DIM_X*NUMBER_THREADS-d_no);        // number of elements assigned to last block
  int df = 0;                             // divisibility factor for the last block

  // statistical
  fp d_psum[NUMBER_THREADS];   // data for block calculations allocated by every block in its shared memory
  fp d_psum2[NUMBER_THREADS];

  // counters
  int i;

  // copy data to shared memory
  if(ei<d_no){                            // do only for the number of elements, omit extra threads

    d_psum[tx] = d_sums[ei*d_mul];
    d_psum2[tx] = d_sums2[ei*d_mul];

  }

  // Lingjie Zhang modifited at Nov 1 / 2015
    // end Lingjie Zhang's modification

  // reduction of sums if all blocks are full (rare case)  
  if(nf == NUMBER_THREADS){
    // sum of every 2, 4, ..., NUMBER_THREADS elements
    for(i=2; i<=NUMBER_THREADS; i=2*i){
      // sum of elements
      if((tx+1) % i == 0){                      // every ith
        d_psum[tx] = d_psum[tx] + d_psum[tx-i/2];
        d_psum2[tx] = d_psum2[tx] + d_psum2[tx-i/2];
      }
      // synchronization
    }
    // final sumation by last thread in every block
    if(tx==(NUMBER_THREADS-1)){                      // block result stored in global memory
      d_sums[bx*d_mul*NUMBER_THREADS] = d_psum[tx];
      d_sums2[bx*d_mul*NUMBER_THREADS] = d_psum2[tx];
    }
  }
  // reduction of sums if last block is not full (common case)
  else{ 
    // for full blocks (all except for last block)
    if(bx != (GRID_DIM_X - 1)){                      //
      // sum of every 2, 4, ..., NUMBER_THREADS elements
      for(i=2; i<=NUMBER_THREADS; i=2*i){                //
        // sum of elements
        if((tx+1) % i == 0){                    // every ith
          d_psum[tx] = d_psum[tx] + d_psum[tx-i/2];
          d_psum2[tx] = d_psum2[tx] + d_psum2[tx-i/2];
        }
        // synchronization
      }
      // final sumation by last thread in every block
      if(tx==(NUMBER_THREADS-1)){                    // block result stored in global memory
        d_sums[bx*d_mul*NUMBER_THREADS] = d_psum[tx];
        d_sums2[bx*d_mul*NUMBER_THREADS] = d_psum2[tx];
      }
    }
    // for not full block (last block)
    else{                                //
      // figure out divisibility
      for(i=2; i<=NUMBER_THREADS; i=2*i){                //
        if(nf >= i){
          df = i;
        }
      }
      // sum of every 2, 4, ..., NUMBER_THREADS elements
      for(i=2; i<=df; i=2*i){                      //
        // sum of elements (only busy threads)
        if((tx+1) % i == 0 && tx<df){                // every ith
          d_psum[tx] = d_psum[tx] + d_psum[tx-i/2];
          d_psum2[tx] = d_psum2[tx] + d_psum2[tx-i/2];
        }
        // synchronization (all threads)
      }
      // remainder / final summation by last thread
      if(tx==(df-1)){                    //
        // compute the remainder and final summation by last busy thread
        for(i=(bx*NUMBER_THREADS)+df; i<(bx*NUMBER_THREADS)+nf; i++){            //
          d_psum[tx] = d_psum[tx] + d_sums[i];
          d_psum2[tx] = d_psum2[tx] + d_sums2[i];
        }
        // final sumation by last thread in every block
        d_sums[bx*d_mul*NUMBER_THREADS] = d_psum[tx];
        d_sums2[bx*d_mul*NUMBER_THREADS] = d_psum2[tx];
      }
    }
  }

}


// --- from srad2_kernel.cu ---
extern "C"
void srad2(const  fp d_lambda, 
                    const int d_Nr, 
                    const int d_Nc, 
                    const long d_Ne, 
                    const int *d_iN, 
                    const int *d_iS, 
                    const int *d_jE, 
                    const int *d_jW,
                    const fp *d_dN, 
                    const fp *d_dS, 
                    const fp *d_dE, 
                    const fp *d_dW, 
                    const fp *d_c, 
                    fp *d_I)
{
    #pragma HLS INTERFACE s_axilite port=d_lambda
    #pragma HLS INTERFACE s_axilite port=d_Nr
    #pragma HLS INTERFACE s_axilite port=d_Nc
    #pragma HLS INTERFACE s_axilite port=d_Ne
    #pragma HLS INTERFACE m_axi port=d_iN offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_iS offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_jE offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_jW offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=d_dN offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=d_dS offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=d_dE offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=d_dW offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=d_c offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=d_I offset=slave bundle=gmem9
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            // indexes
            int bx = _bid_x;                  // get current horizontal block index
            int tx = _tid_x;                   // get current horizontal thread index
            int ei = bx*NUMBER_THREADS+tx;          // more threads than actual elements !!!
            int row;                                // column, x position
            int col;                                // row, y position

            // variables
            fp d_cN,d_cS,d_cW,d_cE;
            fp d_D;

            // figure out row/col location in new matrix
            row = (ei+1) % d_Nr - 1;                // (0-n) row
            col = (ei+1) / d_Nr + 1 - 1;            // (0-n) column
            if((ei+1) % d_Nr == 0){
            row = d_Nr - 1;
            col = col - 1;
            }

            if(ei<d_Ne){                            // make sure that only threads matching jobs run

            // diffusion coefficent
            d_cN = d_c[ei];                       // north diffusion coefficient
            d_cS = d_c[d_iS[row] + d_Nr*col];     // south diffusion coefficient
            d_cW = d_c[ei];                       // west diffusion coefficient
            d_cE = d_c[row + d_Nr * d_jE[col]];   // east diffusion coefficient

            // divergence (equ 58)
            d_D = d_cN*d_dN[ei] + d_cS*d_dS[ei] + d_cW*d_dW[ei] + d_cE*d_dE[ei];// divergence

            // image update (equ 61) (every element of IMAGE)
            d_I[ei] = d_I[ei] + (fp)0.25*d_lambda*d_D;// updates image (based on input time step and divergence)

            }


        }
    }
}


// --- from srad_kernel.cu ---
extern "C"
void srad(  fp d_lambda, 
                   const int d_Nr, 
                   const int d_Nc, 
                   const long d_Ne, 
                   const int *d_iN, 
                   const int *d_iS, 
                   const int *d_jE, 
                   const int *d_jW, 
                   fp *d_dN, 
                   fp *d_dS, 
                   fp *d_dE, 
                   fp *d_dW, 
                   const fp d_q0sqr, 
                   fp *d_c, 
                   const fp *d_I)
{
    #pragma HLS INTERFACE s_axilite port=d_lambda
    #pragma HLS INTERFACE s_axilite port=d_Nr
    #pragma HLS INTERFACE s_axilite port=d_Nc
    #pragma HLS INTERFACE s_axilite port=d_Ne
    #pragma HLS INTERFACE m_axi port=d_iN offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_iS offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_jE offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_jW offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=d_dN offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=d_dS offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=d_dE offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=d_dW offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=d_q0sqr
    #pragma HLS INTERFACE m_axi port=d_c offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=d_I offset=slave bundle=gmem9
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            // indexes
            int bx = _bid_x;                    // get current horizontal block index
            int tx = _tid_x;                   // get current horizontal thread index
            int ei = bx*NUMBER_THREADS+tx;          // more threads than actual elements !!!
            int row;                                // column, x position
            int col;                                // row, y position

            // variables
            fp d_Jc;
            fp d_dN_loc, d_dS_loc, d_dW_loc, d_dE_loc;
            fp d_c_loc;
            fp d_G2,d_L,d_num,d_den,d_qsqr;

            // figure out row/col location in new matrix
            row = (ei+1) % d_Nr - 1;                // (0-n) row
            col = (ei+1) / d_Nr + 1 - 1;            // (0-n) column
            if((ei+1) % d_Nr == 0){
            row = d_Nr - 1;
            col = col - 1;
            }

            if(ei<d_Ne){                            // make sure that only threads matching jobs run

            // directional derivatives, ICOV, diffusion coefficent
            d_Jc = d_I[ei];                       // get value of the current element

            // directional derivates (every element of IMAGE)(try to copy to shared memory or temp files)
            d_dN_loc = d_I[d_iN[row] + d_Nr*col] - d_Jc;            // north direction derivative
            d_dS_loc = d_I[d_iS[row] + d_Nr*col] - d_Jc;            // south direction derivative
            d_dW_loc = d_I[row + d_Nr*d_jW[col]] - d_Jc;            // west direction derivative
            d_dE_loc = d_I[row + d_Nr*d_jE[col]] - d_Jc;            // east direction derivative

            // normalized discrete gradient mag squared (equ 52,53)
            d_G2 = (d_dN_loc*d_dN_loc + d_dS_loc*d_dS_loc + d_dW_loc*d_dW_loc + d_dE_loc*d_dE_loc) / (d_Jc*d_Jc);  // gradient (based on derivatives)

            // normalized discrete laplacian (equ 54)
            d_L = (d_dN_loc + d_dS_loc + d_dW_loc + d_dE_loc) / d_Jc;      // laplacian (based on derivatives)

            // ICOV (equ 31/35)
            d_num  = ((fp)0.5*d_G2) - (((fp)1.0/(fp)16.0)*(d_L*d_L)) ;            // num (based on gradient and laplacian)
            d_den  = (fp)1 + ((fp)0.25*d_L);                        // den (based on laplacian)
            d_qsqr = d_num/(d_den*d_den);                    // qsqr (based on num and den)

            // diffusion coefficent (equ 33) (every element of IMAGE)
            d_den = (d_qsqr-d_q0sqr) / (d_q0sqr * ((fp)1.0+d_q0sqr)) ;        // den (based on qsqr and q0sqr)
            d_c_loc = (fp)1.0 / ((fp)1.0+d_den) ;                    // diffusion coefficient (based on den)

            // saturate diffusion coefficent to 0-1 range
            if (d_c_loc < 0){                          // if diffusion coefficient < 0
            d_c_loc = 0;                          // ... set to 0
            }
            else if (d_c_loc > 1){                        // if diffusion coefficient > 1
            d_c_loc = 1;                          // ... set to 1
            }

            // save data to global memory
            d_dN[ei] = d_dN_loc;
            d_dS[ei] = d_dS_loc;
            d_dW[ei] = d_dW_loc;
            d_dE[ei] = d_dE_loc;
            d_c[ei] = d_c_loc;

            }


        }
    }
}
