#include "kernel.h"

// --- from masking.cu ---
inline double firstRepeatOffsetProb(const double probMult, const int maxRepeatOffset) {
  if (probMult < 1 || probMult > 1)
    return (1 - probMult) / (1 - pow(probMult, (double)maxRepeatOffset));
  else
    return 1.0 / maxRepeatOffset;
}

void maskProbableLetters(const int size,
    unsigned char *seqBeg,
    const float *probabilities,
    const unsigned char *maskTable) {

  const double minMaskProb = 0.5;
  for (int i=0; i<size; i++)
    if (probabilities[i] >= minMaskProb)
      seqBeg[i] = maskTable[seqBeg[i]];
}

int calcRepeatProbs(float *letterProbs,
    const unsigned char *seqBeg,
    const int size,
    const int maxRepeatOffset,
    const double *likelihoodRatioMatrix, // 64 by 64 matrix,
    const double b2b,
    const double f2f0,
    const double f2b,
    const double b2fLast_inv,
    const double *pow_lkp,
    double *foregroundProbs,
    const int scaleStepSize,
    double *scaleFactors)
{

  double backgroundProb = 1.0;
  for (int k=0; k < size ; k++) {

    const int v0 = seqBeg[k];
    const int k_cap = k < maxRepeatOffset ? k : maxRepeatOffset;

    const int pad1 = k_cap - 1;
    const int pad2 = maxRepeatOffset - k_cap; // maxRepeatOffset - k, then 0                   when k > maxRepeatOffset
    const int pad3 = k - k_cap;               // 0                  , then maxRepeatOffset - k when k > maxRepeatOffset

    double accu = 0;

    for (int i = 0; i < k; i++) {

      const int idx1 = pad1 - i;
      const int idx2 = pad2 + i;
      const int idx3 = pad3 + i;

      const int v1 = seqBeg[idx3];
      accu += foregroundProbs[idx1];
      foregroundProbs[idx1] = ( (f2f0 * foregroundProbs[idx1]) +
          (backgroundProb * pow_lkp[idx2]) ) *
        likelihoodRatioMatrix[v0*size+v1];
    }

    backgroundProb = (backgroundProb * b2b) + (accu * f2b);

    if (k % scaleStepSize == scaleStepSize - 1) {
      const double scale = 1 / backgroundProb;
      scaleFactors[k / scaleStepSize] = scale;

      for (int i=0; i< k_cap; i++)
        foregroundProbs[i] = foregroundProbs[i] * scale;

      backgroundProb = 1;
    }

    letterProbs[k] = (float)(backgroundProb);
  }

  double accu = 0;
  for (int i=0 ; i < maxRepeatOffset; i++) {
    accu += foregroundProbs[i];
    foregroundProbs[i] = f2b;
  }

  const double fTot = backgroundProb * b2b + accu * f2b;
  backgroundProb = b2b;

  const double fTot_inv = 1/ fTot ;
  for (int k=(size-1) ; k >= 0 ; k--){

    float nonRepeatProb = letterProbs[k] * backgroundProb * fTot_inv;
    letterProbs[k] = 1.f - nonRepeatProb;

    const int k_cap  = min(k, maxRepeatOffset);

    if (k % scaleStepSize == scaleStepSize - 1) {
      const double scale = scaleFactors[k/ scaleStepSize];

      for (int i=0; i< k_cap; i++)
        foregroundProbs[i] = foregroundProbs[i] * scale;

      backgroundProb *= scale;
    }

    const double c0 = f2b * backgroundProb;
    const int v0= seqBeg[k];

    double accu = 0;
    for (int i = 0; i < k_cap; i++) {

      const int v1 =  seqBeg[k-(i+1)];
      const double f = foregroundProbs[i] * likelihoodRatioMatrix[v0*size+v1];

      accu += pow_lkp[k_cap-(i+1)]*f;
      foregroundProbs[i] = c0 + f2f0 * f;
    }

    const double p = k > maxRepeatOffset ? 1. : pow_lkp[maxRepeatOffset - k]*b2fLast_inv;
    backgroundProb = (b2b * backgroundProb) + accu*p;
  }

  const double bTot = backgroundProb;
  return (fabs(fTot - bTot) > fmax(fTot, bTot) / 1e6);
}
extern "C"

  void maskSequences(unsigned char * seqs,
    const double *  likelihoodRatioMatrix,
    const unsigned char *  maskTable,
    const int size                     ,
    const int maxRepeatOffset          ,
    const double repeatProb            ,
    const double repeatEndProb         ,
    const double repeatOffsetProbDecay ,
    const double firstGapProb          ,
    const double otherGapProb          ,
    const double minMaskProb           ,
    int seqs_len )
{
    #pragma HLS INTERFACE m_axi port=seqs offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=likelihoodRatioMatrix offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=maskTable offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=maxRepeatOffset
    #pragma HLS INTERFACE s_axilite port=repeatProb
    #pragma HLS INTERFACE s_axilite port=repeatEndProb
    #pragma HLS INTERFACE s_axilite port=repeatOffsetProbDecay
    #pragma HLS INTERFACE s_axilite port=firstGapProb
    #pragma HLS INTERFACE s_axilite port=otherGapProb
    #pragma HLS INTERFACE s_axilite port=minMaskProb
    #pragma HLS INTERFACE s_axilite port=seqs_len
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int gid = _bid_x*BLOCK_DIM_X+_tid_x;
            if (gid >= seqs_len) return;

            unsigned char* seqBeg = seqs+gid*33;

            float probabilities[SEQ_LEN];

            const double b2b = 1 - repeatProb;
            const double f2f0 = 1 - repeatEndProb;
            const double f2b = repeatEndProb;

            const double b2fGrowth = 1 / repeatOffsetProbDecay;

            const double  b2fLast = repeatProb * firstRepeatOffsetProb(b2fGrowth, maxRepeatOffset);
            const double b2fLast_inv = 1 / b2fLast ;

            double p = b2fLast;
            double ar_1[50];

            for (int i=0 ; i < maxRepeatOffset; i++){
            ar_1[i] = p ;
            p *= b2fGrowth;
            }

            const int scaleStepSize = 16;

            double scaleFactors[SEQ_LEN / scaleStepSize];

            double foregroundProbs[50];

            for (int i=0 ; i < maxRepeatOffset; i++){
            foregroundProbs[i] = 0;
            };

            const int err  = calcRepeatProbs(probabilities,seqBeg, size,
            maxRepeatOffset, likelihoodRatioMatrix,
            b2b, f2f0, f2b,
            b2fLast_inv,ar_1,foregroundProbs,scaleStepSize, scaleFactors);

            //if (err)  printf("tantan: warning: possible numeric inaccuracy\n");

            maskProbableLetters(size,seqBeg, probabilities, maskTable);

        }
    }
}
