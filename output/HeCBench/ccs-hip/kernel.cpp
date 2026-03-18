#include "kernel.h"

// --- from main.cu ---
struct pair_r compute(
  float *genekj,
  float *geneij,
  const char *sample,
  int wid,int k,int i,int D,
  const float *gene)
{
  int j;
  float sx = 0.f, sxx = 0.f, sy = 0.f, sxy = 0.f, syy = 0.f;
  float sx_n = 0.f, sxx_n = 0.f, sy_n = 0.f, sxy_n = 0.f, syy_n = 0.f;

  struct pair_r rval = {0.f, 0.f};

  for (j = 0; j < D; j++) {
    genekj[j]=gene[k*(D+1)+j];
    if(sample[j]=='1')
      sx += genekj[j];
    else
      sx_n += genekj[j];
  }

  sx /= wid;
  sx_n /= (D-wid);

  for (j = 0; j < D; j++) {
    if(sample[j]=='1')
      sxx += (sx-genekj[j]) * (sx-genekj[j]);
    else
      sxx_n += (sx_n-genekj[j]) * (sx_n-genekj[j]);
  }

  sxx = sqrtf(sxx);
  sxx_n = sqrtf(sxx_n);

  for (j = 0; j < D; j++) {
    geneij[j]=gene[i*(D+1)+j];
    if(sample[j]=='1')
      sy += geneij[j];
    else
      sy_n += geneij[j];
  }

  sy /= wid; 
  sy_n /= (D-wid); 

  for (j = 0; j < D; j++)
  {
    if(sample[j]=='1') {
      sxy += (sx - genekj[j]) * (sy - geneij[j]);
      syy += (sy - geneij[j]) * (sy - geneij[j]);
    }
    else {
      sxy_n += (sx_n - genekj[j]) * (sy_n - geneij[j]);
      syy_n += (sy_n - geneij[j]) * (sy_n - geneij[j]);
    }
  }

  syy = sqrtf(syy);
  syy_n = sqrtf(syy_n);
  rval.r = fabsf(sxy/(sxx * syy));
  rval.n_r = fabsf(sxy_n/(sxx_n * syy_n));

  return rval;
}
extern "C"

void compute_bicluster(
  const float * gene, 
  const int n,
  const int maxbcn,
  const int D,
  const float thr,
  char * maxbc_sample,
  char * maxbc_data,
  float * maxbc_score,
  int * maxbc_datacount,
  int * maxbc_samplecount,
  char * tmpbc_sample,
  char * tmpbc_data)
{
    #pragma HLS INTERFACE m_axi port=gene offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=maxbcn
    #pragma HLS INTERFACE s_axilite port=D
    #pragma HLS INTERFACE s_axilite port=thr
    #pragma HLS INTERFACE m_axi port=maxbc_sample offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=maxbc_data offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=maxbc_score offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=maxbc_datacount offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=maxbc_samplecount offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=tmpbc_sample offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=tmpbc_data offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_genekj complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_geneij complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_vect complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            float s_genekj[MAXSAMPLE];
            float s_geneij[MAXSAMPLE];
            char s_vect[3*MAXSAMPLE];

            int k=_bid_x*BLOCK_DIM_X+_tid_x;

            if(k<maxbcn) {
            float jcc,mean_k,mean_i;
            int i,j,l,vl,wid,wid_0,wid_1,wid_2,l_i,t_tot,t_dif;
            int dif,tot;
            struct pair_r rval;
            int tmpbc_datacount,tmpbc_samplecount;

            float genekj,geneij;

            maxbc_score[k]=1.f;
            maxbc_datacount[k]=0;

            //calculate mean expression for gene k

            mean_k=gene[k*(D+1)+D];

            for (i = k+1; i < n; i++) //pair k,i
            {
            //calculate mean expression for gene i
            mean_i=gene[i*(D+1)+D];

            wid_0=0; wid_1=0; wid_2=0;

            for (j = 0; j < D; j++)
            {
            genekj=gene[k*(D+1)+j];
            geneij=gene[i*(D+1)+j];

            if ((genekj - mean_k)>=0 && (geneij - mean_i)>=0) //i and k upregulated : positive correlation
            {
            s_vect[0*3+j] = '1';
            s_vect[1*3+j] = '0';
            s_vect[2*3+j] = '0';
            wid_0++;
            }
            else if ((genekj - mean_k)<0 && (geneij - mean_i)<0)  // i and k down regulated : positive correlation
            {
            s_vect[0*3+j] = '0';
            s_vect[1*3+j] = '1';
            s_vect[2*3+j] = '0';
            wid_1++;
            }
            else if ((genekj - mean_k)*(geneij - mean_i)<0) //betwenn i and k one is up regulated and the other one is down regulated : negative correlation
            {
            s_vect[0*3+j] = '0';
            s_vect[1*3+j] = '0';
            s_vect[2*3+j] = '1';
            wid_2++;
            }
            }

            for (vl = 0; vl < 3; vl++)
            {
            dif=0; tot=0;
            if(vl==0)
            wid=wid_0;
            else if(vl==1)
            wid=wid_1;
            if(vl==2)
            wid=wid_2;

            if(wid>minsample) { //minimum samples required to form a bicluster module. Default minimum set to 10 in ccs.h

            rval=compute(s_genekj, s_geneij, s_vect+vl*MAXSAMPLE, wid, k, i, D, gene);
            }
            else {
            continue;
            }

            if (rval.r > thr)
            {
            tot++;
            if(rval.n_r>thr)
            dif++;

            for (j = 0;j < D; j++)
            tmpbc_sample[k*D+j] = s_vect[vl*MAXSAMPLE+j];

            for (j = 0;j < n; j++)
            tmpbc_data[k*n+j] = '0';

            tmpbc_data[k*n+k] = '1';
            tmpbc_data[k*n+i] = '1';
            tmpbc_datacount = 2;
            tmpbc_samplecount = wid;

            for (l = 0; l < n; l++)  { //bicluster augmentation
            if (l != i && l != k) {
            t_tot=0; t_dif=0;
            for(l_i=0;l_i<n;l_i++) {
            if(tmpbc_data[k*n+l_i]=='1')  {
            rval=compute(s_genekj, s_geneij, s_vect + vl*MAXSAMPLE, wid, l, l_i, D, gene);

            if(rval.r>thr)
            t_tot+=1;
            else {
            t_tot=0;
            break;
            }
            if(rval.n_r>thr)
            t_dif+=1;
            }
            }

            if(t_tot>0)  {
            tmpbc_data[k*n+l] = '1';
            tmpbc_datacount+=1;
            tot+=t_tot; dif+=t_dif;
            }
            }
            }  // end of augmentation

            // Compute Jaccard score

            if(tot>0)
            jcc=(float)dif/tot;
            else
            jcc=1.f;

            /* Select bicluster candidate as the largest (maxbc[k].datacount<tmpbc.datacount)
            of all condition dependent (jaccard score <0.01) bicluster for k. Minimum number of gene
            for a bicluster is set at 10. See the mingene at ccs.h */

            if(jcc<0.01f && maxbc_datacount[k]<tmpbc_datacount && tmpbc_datacount>mingene)
            {
            maxbc_score[k]=jcc;
            for (j = 0; j < n; j++)
            maxbc_data[k*n+j]=tmpbc_data[k*n+j];
            for (j = 0; j < D; j++)
            maxbc_sample[k*D+j]=tmpbc_sample[k*D+j];
            maxbc_datacount[k]=tmpbc_datacount;
            maxbc_samplecount[k]=tmpbc_samplecount;
            }
            }    //end of r>thr condition
            }    //end of loop for vl
            }  // end of i loop
            }

        }
    }
}
