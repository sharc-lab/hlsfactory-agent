#include "kernel.h"

// --- from kernels.cu ---
extern "C"
void mttkrp_MIHCSR_kernel_slc_atomic_fbrLvlPar(
  const DTYPE * vals,
  const ITYPE * fbrLikeSlcInds,
  const ITYPE * dInds2, 
  const ITYPE * fbrPtr0,
  const ITYPE * fbrPtr1,
  const ITYPE * fbrIdx1,
  ITYPE nFibers,
        DTYPE * dU0,
  const DTYPE * dU1,
  const DTYPE * dU2, 
  ITYPE  mode,
  ITYPE R,
  ITYPE warpPerSlice,
  int logOfWPC,
  int fbrPerWarp,
  int logOfFPW)
{
    #pragma HLS INTERFACE m_axi port=vals offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fbrLikeSlcInds offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dInds2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fbrPtr0 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=fbrPtr1 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=fbrIdx1 offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=nFibers
    #pragma HLS INTERFACE m_axi port=dU0 offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=dU1 offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=dU2 offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=mode
    #pragma HLS INTERFACE s_axilite port=R
    #pragma HLS INTERFACE s_axilite port=warpPerSlice
    #pragma HLS INTERFACE s_axilite port=logOfWPC
    #pragma HLS INTERFACE s_axilite port=fbrPerWarp
    #pragma HLS INTERFACE s_axilite port=logOfFPW
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            ITYPE tId = _tid_x;
            ITYPE laneId = tId & 31;
            ITYPE bdim = BLOCK_DIM_X;
            ITYPE gId = (_bid_x * bdim + tId);
            ITYPE workId = (tId & ((1 << (5 + logOfWPC)) - 1)) >> 5;  //tId >> 5; //tId >> 5;//
            ITYPE fbr = (gId >> (5 + logOfWPC)) << logOfFPW; // 5: minimum 1 WARP (2^5) // _bid_x ;//

            DTYPE tmp = 0, tmp_val;

            if(fbr < nFibers - 1){

            tmp_val = 0;
            bool diffFiber = false;
            unsigned int idx0;

            for (int fr = 0; fr < fbrPerWarp && (fbr+fr) < (nFibers - 1); ++fr){

            diffFiber = false;
            unsigned int idx1 = fbrIdx1[fbr+fr];// dInds1[fbrPtr1[fbr]];
            idx0 = fbrLikeSlcInds[fbr+fr];//slc;
            tmp_val = 0;

            for(unsigned int x = fbrPtr1[fbr+fr] + workId; x < fbrPtr1[fbr+fr+1]; x+=warpPerSlice) {

            unsigned int idx2 = dInds2[x];

            for(unsigned int r=laneId; r<R; r+=32) {
            tmp_val += vals[x] * dU2[idx2 * R + r]; //2MR
            }
            }

            for(unsigned int r=laneId; r<R; r+=32) {
            tmp += tmp_val * dU1[idx1 * R + r] ; //2PR
            }

            if(fbrLikeSlcInds[fbr+fr] != fbrLikeSlcInds[fbr+fr+1]) {

            diffFiber = true;
            for(unsigned int r=laneId; r<R; r+=32) {
            (dU0[idx0 * R + r] += tmp); //2PR
            }
            tmp = 0;
            }
            }

            if(!diffFiber) {
            for(unsigned int r=laneId; r<R; r+=32) {
            (dU0[idx0 * R + r] += tmp);
            }
            }
            }

        }
    }
}
extern "C"

void mttkrp_MIHCSR_kernel_fbrS_atomic_fbrLvlPar_4D(
  const DTYPE * vals,
  const ITYPE * fbrLikeSlcInds,
  const ITYPE * dInds3, 
  const ITYPE * fbrPtr0,
  const ITYPE * fbrPtr1,
  const ITYPE * fbrIdx1,
  const ITYPE * fbrPtr2,
  const ITYPE * fbrIdx2,
  ITYPE nFibers,
  DTYPE * dU0,
  const DTYPE * dU1,
  const DTYPE * dU2,
  const DTYPE * dU3,
  ITYPE mode,
  ITYPE R,
  ITYPE warpPerSlice,
  int logOfWPC)
{
    #pragma HLS INTERFACE m_axi port=vals offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fbrLikeSlcInds offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dInds3 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fbrPtr0 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=fbrPtr1 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=fbrIdx1 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=fbrPtr2 offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=fbrIdx2 offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=nFibers
    #pragma HLS INTERFACE m_axi port=dU0 offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=dU1 offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=dU2 offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=dU3 offset=slave bundle=gmem11
    #pragma HLS INTERFACE s_axilite port=mode
    #pragma HLS INTERFACE s_axilite port=R
    #pragma HLS INTERFACE s_axilite port=warpPerSlice
    #pragma HLS INTERFACE s_axilite port=logOfWPC
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            ITYPE tId = _tid_x;
            ITYPE laneId = tId & 31;
            ITYPE bdim = BLOCK_DIM_X;
            ITYPE gId = (_bid_x * bdim + tId);
            ITYPE workId = (tId & ((1 << (5 + logOfWPC)) - 1)) >> 5;  //tId >> 5; //tId >> 5;//
            ITYPE fbrS = gId >> (5 + logOfWPC); // 5: minimum 1 WARP (2^5) // _bid_x ;//
            DTYPE tmp = 0, tmp_val, tmp2 = 0;

            if(fbrS < nFibers - 1){

            tmp = 0;
            unsigned int idx0 = fbrIdx1[fbrS];// dInds1[fbrPtr1[fbr]];
            unsigned int idx3 = fbrLikeSlcInds[fbrS];//slc;

            for (int fbr = fbrPtr1[fbrS] + workId; fbr < fbrPtr1[fbrS+1]; fbr+=warpPerSlice){
            unsigned int idx1 = fbrIdx2[fbr];
            tmp_val = 0;

            for(unsigned int x = fbrPtr2[fbr]; x < fbrPtr2[fbr+1]; ++x) {
            unsigned int idx2 = dInds3[x];

            for(unsigned int r=laneId; r<R; r+=32)
            tmp_val += vals[x] * dU2[idx2 * R + r] ; //2MR
            }
            for(unsigned int r=laneId; r<R; r+=32)
            tmp += tmp_val * dU1[idx1 * R + r]  ;
            }
            for(unsigned int r=laneId; r<R; r+=32) {
            tmp2 = tmp * dU3[idx3 * R + r];
            (dU0[idx0 * R + r] += tmp2); //2PR
            }
            }

        }
    }
}
extern "C"

void mttkrp_MIHCSR_kernel_fbr_atomic_fbrLvlPar(
  const DTYPE * vals,
  const ITYPE * fbrLikeSlcInds,
  const ITYPE * dInds2, 
  const ITYPE * fbrPtr0,
  const ITYPE * fbrPtr1,
  const ITYPE * fbrIdx1,
  ITYPE nFibers,
        DTYPE * dU0,
  const DTYPE * dU1,
  const DTYPE * dU2, 
  ITYPE mode,
  ITYPE R,
  ITYPE warpPerSlice,
  int logOfWPC)
{
    #pragma HLS INTERFACE m_axi port=vals offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fbrLikeSlcInds offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dInds2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fbrPtr0 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=fbrPtr1 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=fbrIdx1 offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=nFibers
    #pragma HLS INTERFACE m_axi port=dU0 offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=dU1 offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=dU2 offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=mode
    #pragma HLS INTERFACE s_axilite port=R
    #pragma HLS INTERFACE s_axilite port=warpPerSlice
    #pragma HLS INTERFACE s_axilite port=logOfWPC
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            ITYPE tId = _tid_x;
            ITYPE laneId = tId & 31;
            ITYPE bdim = BLOCK_DIM_X;
            ITYPE gId = (_bid_x * bdim + tId);
            ITYPE workId = (tId & ((1 << (5 + logOfWPC)) - 1)) >> 5;  //tId >> 5; //tId >> 5;//
            ITYPE fbr = gId >> (5 + logOfWPC); // 5: minimum 1 WARP (2^5) // _bid_x ;//
            DTYPE tmp = 0, tmp_val;

            if(fbr < nFibers - 1){

            tmp_val = 0;
            unsigned int idx0 = fbrIdx1[fbr];// dInds1[fbrPtr1[fbr]];
            unsigned int idx2 = fbrLikeSlcInds[fbr];//slc;

            for(unsigned int x = fbrPtr1[fbr] + workId; x < fbrPtr1[fbr+1]; x+=warpPerSlice) {

            unsigned int idx1 = dInds2[x];

            for(unsigned int r=laneId; r<R; r+=32) {
            tmp_val += vals[x] * dU1[idx1 * R + r]; //2MR
            }
            }
            for(unsigned int r=laneId; r<R; r+=32) {
            tmp = tmp_val * dU2[idx2 * R + r] ;
            (dU0[idx0 * R + r] += tmp); //2PR
            }
            }

        }
    }
}
extern "C"

void mttkrp_MIHCSR_kernel_fbr_atomic_fbrLvlPar_4D(
  const DTYPE * vals,
  const ITYPE * fbrLikeSlcInds,
  const ITYPE * dInds3, 
  const ITYPE * fbrPtr0,
  const ITYPE * fbrPtr1,
  const ITYPE * fbrIdx1,
  const ITYPE * fbrPtr2,
  const ITYPE * fbrIdx2,
  ITYPE nFibers,
        DTYPE * dU0,
  const DTYPE * dU1,
  const DTYPE * dU2,
  const DTYPE * dU3,
  ITYPE mode,
  ITYPE R,
  ITYPE warpPerSlice,
  int logOfWPC)
{
    #pragma HLS INTERFACE m_axi port=vals offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fbrLikeSlcInds offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dInds3 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fbrPtr0 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=fbrPtr1 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=fbrIdx1 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=fbrPtr2 offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=fbrIdx2 offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=nFibers
    #pragma HLS INTERFACE m_axi port=dU0 offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=dU1 offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=dU2 offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=dU3 offset=slave bundle=gmem11
    #pragma HLS INTERFACE s_axilite port=mode
    #pragma HLS INTERFACE s_axilite port=R
    #pragma HLS INTERFACE s_axilite port=warpPerSlice
    #pragma HLS INTERFACE s_axilite port=logOfWPC
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            ITYPE tId = _tid_x;
            ITYPE laneId = tId & 31;
            ITYPE bdim = BLOCK_DIM_X;
            ITYPE gId = (_bid_x * bdim + tId);
            ITYPE workId = (tId & ((1 << (5 + logOfWPC)) - 1)) >> 5;  //tId >> 5; //tId >> 5;//
            ITYPE fbrS = gId >> (5 + logOfWPC); // 5: minimum 1 WARP (2^5) // _bid_x ;//
            DTYPE tmp;

            if(fbrS < nFibers - 1){

            unsigned int idx2 = fbrLikeSlcInds[fbrS];//slc;
            unsigned int idx3 = fbrIdx1[fbrS];// dInds1[fbrPtr1[fbr]];

            for (int fbr = fbrPtr1[fbrS] + workId; fbr < fbrPtr1[fbrS+1]; fbr+=warpPerSlice){
            unsigned int idx0 = fbrIdx2[fbr];
            tmp = 0;

            for(unsigned int x = fbrPtr2[fbr]; x < fbrPtr2[fbr+1]; ++x) {
            unsigned int idx1 = dInds3[x];

            for(unsigned int r=laneId; r<R; r+=32)
            tmp += vals[x] * dU1[idx1 * R + r]; //2MR
            }
            for(unsigned int r=laneId; r<R; r+=32)  {
            (dU0[idx0 * R + r] += tmp * dU2[idx2 * R + r] * dU3[idx3 * R + r]) ;
            }
            }
            }

        }
    }
}
extern "C"

void mttkrp_MIHCSR_kernel_all_atomic_fbrLvlPar(
  const DTYPE * vals,
  const ITYPE * fbrLikeSlcInds,
  const ITYPE * dInds2, 
  const ITYPE * fbrPtr0,
  const ITYPE * fbrPtr1,
  const ITYPE * fbrIdx1,
  ITYPE nFibers,
        DTYPE * dU0,
  const DTYPE * dU1,
  const DTYPE * dU2, 
  ITYPE mode,
  ITYPE R,
  ITYPE warpPerSlice,
  int logOfWPC)
{
    #pragma HLS INTERFACE m_axi port=vals offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fbrLikeSlcInds offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dInds2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fbrPtr0 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=fbrPtr1 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=fbrIdx1 offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=nFibers
    #pragma HLS INTERFACE m_axi port=dU0 offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=dU1 offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=dU2 offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=mode
    #pragma HLS INTERFACE s_axilite port=R
    #pragma HLS INTERFACE s_axilite port=warpPerSlice
    #pragma HLS INTERFACE s_axilite port=logOfWPC
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            ITYPE tId = _tid_x;
            ITYPE laneId = tId & 31;
            ITYPE bdim = BLOCK_DIM_X;
            ITYPE gId = (_bid_x * bdim + tId);
            ITYPE workId = (tId & ((1 << (5 + logOfWPC)) - 1)) >> 5;  //tId >> 5; //tId >> 5;//
            ITYPE fbr = gId >> (5 + logOfWPC); // 5: minimum 1 WARP (2^5) // _bid_x ;//
            DTYPE tmp = 0, tmp_val;

            if(fbr < nFibers - 1){

            tmp_val = 0;
            unsigned int idx1 = fbrLikeSlcInds[fbr];//slc;
            unsigned int idx2 = fbrIdx1[fbr];// dInds1[fbrPtr1[fbr]];

            for(unsigned int r=laneId; r<R; r+=32)
            tmp = dU1[idx1 * R + r] * dU2[idx2 * R + r] ; //1PR

            for(unsigned int x = fbrPtr1[fbr] + workId; x < fbrPtr1[fbr+1]; x+=warpPerSlice) {

            unsigned int idx0 = dInds2[x];

            for(unsigned int r=laneId; r<R; r+=32) {
            tmp_val = vals[x] * tmp;///dU1[idx1 * R + r] * dU2[idx2 * R + r] ; //2MR
            (dU0[idx0 * R + r] += tmp_val);
            }
            }
            }

        }
    }
}
extern "C"

void mttkrp_MIHCSR_kernel_all_atomic_fbrLvlPar_4D(
  const DTYPE * vals,
  const ITYPE * fbrLikeSlcInds,
  const ITYPE * dInds3, 
  const ITYPE * fbrPtr0,
  const ITYPE * fbrPtr1,
  const ITYPE * fbrIdx1,
  const ITYPE * fbrPtr2,
  const ITYPE * fbrIdx2,
  ITYPE nFibers,
        DTYPE * dU0,
  const DTYPE * dU1,
  const DTYPE * dU2,
  const DTYPE * dU3,
  ITYPE mode,
  ITYPE R,
  ITYPE warpPerSlice,
  int logOfWPC)
{
    #pragma HLS INTERFACE m_axi port=vals offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=fbrLikeSlcInds offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=dInds3 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=fbrPtr0 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=fbrPtr1 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=fbrIdx1 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=fbrPtr2 offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=fbrIdx2 offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=nFibers
    #pragma HLS INTERFACE m_axi port=dU0 offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=dU1 offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=dU2 offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=dU3 offset=slave bundle=gmem11
    #pragma HLS INTERFACE s_axilite port=mode
    #pragma HLS INTERFACE s_axilite port=R
    #pragma HLS INTERFACE s_axilite port=warpPerSlice
    #pragma HLS INTERFACE s_axilite port=logOfWPC
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            ITYPE tId = _tid_x;
            ITYPE laneId = tId & 31;
            ITYPE bdim = BLOCK_DIM_X;
            ITYPE gId = (_bid_x * bdim + tId);
            ITYPE workId = (tId & ((1 << (5 + logOfWPC)) - 1)) >> 5;  //tId >> 5; //tId >> 5;//
            ITYPE fbrS = gId >> (5 + logOfWPC); // 5: minimum 1 WARP (2^5) // _bid_x ;//
            DTYPE tmp = 0, tmp_val = 0;;

            if(fbrS < nFibers - 1){

            tmp = 0;
            unsigned int idx1 = fbrLikeSlcInds[fbrS];//slc;
            unsigned int idx2 = fbrIdx1[fbrS];// dInds1[fbrPtr1[fbr]];

            for(unsigned int r=laneId; r<R; r+=32)
            tmp_val = dU1[idx1 * R + r] * dU2[idx2 * R + r] ; //1PR

            for (int fbr = fbrPtr1[fbrS] + workId; fbr < fbrPtr1[fbrS+1]; fbr+=warpPerSlice){
            ITYPE idx3 = fbrIdx2[fbr];

            for(unsigned int x = fbrPtr2[fbr]; x < fbrPtr2[fbr+1]; ++x) {
            unsigned int idx0 = dInds3[x];

            for(unsigned int r=laneId; r<R; r+=32) {
            tmp = vals[x] * dU3[idx3 * R + r] * tmp_val;//2MR
            (dU0[idx0 * R + r] += tmp);
            }
            }
            }
            }

        }
    }
}
