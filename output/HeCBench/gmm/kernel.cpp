#include "kernel.h"

// --- from gaussian_kernel.cu ---
void mvtmeans(float* fcs_data, int num_dimensions, int num_events, float* means) {
  int tid = _tid_x;

  if(tid < num_dimensions) {
    means[tid] = 0.0;

    // Sum up all the values for each dimension
    for(int i = 0; i < num_events; i++) {
      means[tid] += fcs_data[i*num_dimensions+tid];
    }

    // Divide by the # of elements to get the average
    means[tid] /= (float) num_events;
  }
}

void averageVariance(float* fcs_data, float* means, int num_dimensions, int num_events, float* avgvar) {
  int tid = _tid_x;

  float variances[NUM_DIMENSIONS];
  float total_variance;

  // Compute average variance for each dimension
  if(tid < num_dimensions) {
    variances[tid] = 0.0;
    // Sum up all the variance
    for(int i = 0; i < num_events; i++) {
      // variance = (data - mean)^2
      variances[tid] += (fcs_data[i*num_dimensions + tid])*(fcs_data[i*num_dimensions + tid]);
    }
    variances[tid] /= (float) num_events;
    variances[tid] -= means[tid]*means[tid];
  }

  if(tid == 0) {
    total_variance = 0.0;
    for(int i=0; i<num_dimensions;i++) {
      total_variance += variances[i];
    }
    *avgvar = total_variance / (float) num_dimensions;
  }
}

void invert(float* data, int actualsize, float* log_determinant)  {
  int maxsize = actualsize;
  int n = actualsize;

  if(_tid_x == 0) {
    *log_determinant = 0.0;

    // sanity check        
    if (actualsize == 1) {
      *log_determinant = logf(data[0]);
      data[0] = 1.f / data[0];
    } else {

      for (int i=1; i < actualsize; i++) data[i] /= data[0]; // normalize row 0
      for (int i=1; i < actualsize; i++)  { 
        for (int j=i; j < actualsize; j++)  { // do a column of L
          float sum = 0.0;
          for (int k = 0; k < i; k++)  
            sum += data[j*maxsize+k] * data[k*maxsize+i];
          data[j*maxsize+i] -= sum;
        }
        if (i == actualsize-1) continue;
        for (int j=i+1; j < actualsize; j++)  {  // do a row of U
          float sum = 0.0;
          for (int k = 0; k < i; k++)
            sum += data[i*maxsize+k]*data[k*maxsize+j];
          data[i*maxsize+j] = 
            (data[i*maxsize+j]-sum) / data[i*maxsize+i];
        }
      }

      for(int i=0; i<actualsize; i++) {
        *log_determinant += logf(fabs(data[i*n+i]));
      }

      for ( int i = 0; i < actualsize; i++ )  // invert L
        for ( int j = i; j < actualsize; j++ )  {
          float x = 1.f;
          if ( i != j ) {
            x = 0.0;
            for ( int k = i; k < j; k++ ) 
              x -= data[j*maxsize+k]*data[k*maxsize+i];
          }
          data[j*maxsize+i] = x / data[j*maxsize+j];
        }
      for ( int i = 0; i < actualsize; i++ )   // invert U
        for ( int j = i; j < actualsize; j++ )  {
          if ( i == j ) continue;
          float sum = 0.0;
          for ( int k = i; k < j; k++ )
            sum += data[k*maxsize+j]*( (i==k) ? 1.f : data[i*maxsize+k] );
          data[i*maxsize+j] = -sum;
        }
      for ( int i = 0; i < actualsize; i++ )   // final inversion
        for ( int j = 0; j < actualsize; j++ )  {
          float sum = 0.0;
          for ( int k = ((i>j)?i:j); k < actualsize; k++ )  
            sum += ((j==k)?1.f:data[j*maxsize+k])*data[k*maxsize+i];
          data[j*maxsize+i] = sum;
        }
    }
  }
}

void compute_pi(clusters_t* clusters, int num_clusters) {
  float sum;

  if(_tid_x == 0) {
    sum = 0.0;
    for(int i=0; i<num_clusters; i++) {
      sum += clusters->N[i];
    }
  }

  for(int c = _tid_x; c < num_clusters; c += BLOCK_DIM_X) {
    if(clusters->N[c] < 0.5f) {
      clusters->pi[_tid_x] = 1e-10;
    } else {
      clusters->pi[_tid_x] = clusters->N[c] / sum;
    }
  }
}

void compute_constants(clusters_t* clusters, int num_clusters, int num_dimensions) {
  int tid = _tid_x;
  int num_threads = BLOCK_DIM_X;
  int num_elements = num_dimensions*num_dimensions;

  float determinant_arg; // only one thread computes the inverse so we need a shared argument

  float log_determinant;

  float matrix[NUM_DIMENSIONS*NUM_DIMENSIONS];

  // Invert the matrix for every cluster
  int c = _bid_x;
  // Copy the R matrix into shared memory for doing the matrix inversion
  for(int i=tid; i<num_elements; i+= num_threads ) {
    matrix[i] = clusters->R[c*num_dimensions*num_dimensions+i];
  }
#if DIAG_ONLY
  if(tid == 0) { 
    determinant_arg = 1.0f;
    for(int i=0; i < num_dimensions; i++) {
      determinant_arg *= matrix[i*num_dimensions+i];
      matrix[i*num_dimensions+i] = 1.0f / matrix[i*num_dimensions+i];
    }
    determinant_arg = logf(determinant_arg);
  }
#else 
  invert(matrix,num_dimensions,&determinant_arg);
#endif
  log_determinant = determinant_arg;

  // Copy the matrx from shared memory back into the cluster memory
  for(int i=tid; i<num_elements; i+= num_threads) {
    clusters->Rinv[c*num_dimensions*num_dimensions+i] = matrix[i];
  }

  // Compute the constant
  // Equivilent to: log(1/((2*PI)^(M/2)*det(R)^(1/2)))
  // This constant is used in all E-step likelihood calculations
  if(tid == 0) {
    clusters->constant[c] = -num_dimensions*0.5f*logf(2.0f*PI) - 0.5f*log_determinant;
  }
}
extern "C"

void constants_kernel(clusters_t* clusters, int num_clusters, int num_dimensions) {
    #pragma HLS INTERFACE m_axi port=clusters offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=num_clusters
    #pragma HLS INTERFACE s_axilite port=num_dimensions
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=matrix complete dim=1
    #pragma HLS ARRAY_PARTITION variable=matrix complete dim=1
    #pragma HLS ARRAY_PARTITION variable=Rinv complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1


                // compute_constants(clusters,num_clusters,num_dimensions);

                int tid = _tid_x;
                int bid = _bid_x;
                int num_threads = BLOCK_DIM_X;
                int num_elements = num_dimensions*num_dimensions;

                float determinant_arg; // only one thread computes the inverse so we need a shared argument
                float sum;
                float matrix[NUM_DIMENSIONS*NUM_DIMENSIONS];

                float log_determinant;

                // Invert the matrix for every cluster

                // Copy the R matrix into shared memory for doing the matrix inversion
                for(int i=tid; i<num_elements; i+= num_threads ) {
                matrix[i] = clusters->R[bid*num_dimensions*num_dimensions+i];
                }
                #if DIAG_ONLY
                if(tid == 0) {
                determinant_arg = 1.0f;
                for(int i=0; i < num_dimensions; i++) {
                determinant_arg *= matrix[i*num_dimensions+i];
                matrix[i*num_dimensions+i] = 1.0f / matrix[i*num_dimensions+i];
                }
                determinant_arg = logf(determinant_arg);
                }
                #else
                invert(matrix,num_dimensions,&determinant_arg);
                #endif
                log_determinant = determinant_arg;

                // Copy the matrx from shared memory back into the cluster memory
                for(int i=tid; i<num_elements; i+= num_threads) {
                clusters->Rinv[bid*num_dimensions*num_dimensions+i] = matrix[i];
                }

                // Compute the constant
                // Equivilent to: log(1/((2*PI)^(M/2)*det(R)^(1/2)))
                // This constant is used in all E-step likelihood calculations
                if(tid == 0) {
                clusters->constant[bid] = -num_dimensions*0.5f*logf(2.0f*PI) - 0.5f*log_determinant;
                }

                if(bid == 0) {
                // compute_pi(clusters,num_clusters);

                if(tid == 0) {
                sum = 0.0;
                for(int i=0; i<num_clusters; i++) {
                sum += clusters->N[i];
                }
                }

                for(int i = tid; i < num_clusters; i += num_threads) {
                if(clusters->N[i] < 0.5f) {
                clusters->pi[tid] = 1e-10;
                } else {
                clusters->pi[tid] = clusters->N[i] / sum;
                }
                }
                }

            }
        }
    }
}
extern "C"

  void seed_clusters_kernel( const float* fcs_data, 
    clusters_t* clusters, 
    const int num_dimensions, 
    const int num_clusters, 
    const int num_events) 
{
    #pragma HLS INTERFACE m_axi port=fcs_data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=clusters offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=num_dimensions
    #pragma HLS INTERFACE s_axilite port=num_clusters
    #pragma HLS INTERFACE s_axilite port=num_events
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=variances complete dim=1
    #pragma HLS ARRAY_PARTITION variable=matrix complete dim=1
    #pragma HLS ARRAY_PARTITION variable=matrix complete dim=1
    #pragma HLS ARRAY_PARTITION variable=means complete dim=1
    #pragma HLS ARRAY_PARTITION variable=variances complete dim=1
    #pragma HLS ARRAY_PARTITION variable=means complete dim=1
    #pragma HLS ARRAY_PARTITION variable=means complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int tid = _tid_x;
                int num_threads = BLOCK_DIM_X;
                int row, col;
                float seed;

                // Number of elements in the covariance matrix
                int num_elements = num_dimensions*num_dimensions;

                // shared memory
                float means[NUM_DIMENSIONS];
                float avgvar;
                float variances[NUM_DIMENSIONS];
                float total_variance;

                // Compute the means
                // mvtmeans(fcs_data, num_dimensions, num_events, means);

                if(tid < num_dimensions) {
                means[tid] = 0.0;

                // Sum up all the values for each dimension
                for(int i = 0; i < num_events; i++) {
                means[tid] += fcs_data[i*num_dimensions+tid];
                }

                // Divide by the # of elements to get the average
                means[tid] /= (float) num_events;
                }

                // Compute the average variance
                // averageVariance(fcs_data, means, num_dimensions, num_events, &avgvar);

                // Compute average variance for each dimension
                if(tid < num_dimensions) {
                variances[tid] = 0.0;
                // Sum up all the variance
                for(int i = 0; i < num_events; i++) {
                // variance = (data - mean)^2
                variances[tid] += (fcs_data[i*num_dimensions + tid])*(fcs_data[i*num_dimensions + tid]);
                }
                variances[tid] /= (float) num_events;
                variances[tid] -= means[tid]*means[tid];
                }

                if(tid == 0) {
                total_variance = 0.0;
                for(int i=0; i<num_dimensions;i++) {
                total_variance += variances[i];
                }
                avgvar = total_variance / (float) num_dimensions;
                }

                if(num_clusters > 1) {
                seed = (num_events-1.0f)/(num_clusters-1.0f);
                } else {
                seed = 0.0;
                }

                // Seed the pi, means, and covariances for every cluster
                for(int c=0; c < num_clusters; c++) {
                if(tid < num_dimensions) {
                clusters->means[c*num_dimensions+tid] = fcs_data[((int)(c*seed))*num_dimensions+tid];
                }

                for(int i=tid; i < num_elements; i+= num_threads) {
                // Add the average variance divided by a constant, this keeps the cov matrix from becoming singular
                row = (i) / num_dimensions;
                col = (i) % num_dimensions;

                if(row == col) {
                clusters->R[c*num_dimensions*num_dimensions+i] = 1.0f;
                } else {
                clusters->R[c*num_dimensions*num_dimensions+i] = 0.0f;
                }
                }
                if(tid == 0) {
                clusters->pi[c] = 1.0f/((float)num_clusters);
                clusters->N[c] = ((float) num_events) / ((float)num_clusters);
                clusters->avgvar[c] = avgvar / COVARIANCE_DYNAMIC_RANGE;
                }
                }

            }
        }
    }
}

float parallelSum(float* data, const unsigned int ndata) {
  const unsigned int tid = _tid_x;
  float t;

  // Butterfly sum.  ndata MUST be a power of 2.
  for(unsigned int bit = ndata >> 1; bit > 0; bit >>= 1) {
    t = data[tid] + data[tid^bit];
    data[tid] = t;
  }
  return data[tid];
}

void compute_indices(int num_events, int* start, int* stop) {
  // Break up the events evenly between the blocks
  int num_pixels_per_block = num_events / NUM_BLOCKS;
  // Make sure the events being accessed by the block are aligned to a multiple of 16
  num_pixels_per_block = num_pixels_per_block - (num_pixels_per_block % 16);

  *start = _bid_y * num_pixels_per_block + _tid_x;

  // Last block will handle the leftover events
  if(_bid_y == GRID_DIM_Y-1) {
    *stop = num_events;
  } else { 
    *stop = (_bid_y+1) * num_pixels_per_block;
  }
}
extern "C"

void estep1(float* data, clusters_t* clusters, int num_dimensions, int num_events) {
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=clusters offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=num_dimensions
    #pragma HLS INTERFACE s_axilite port=num_events
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=means complete dim=1
    #pragma HLS ARRAY_PARTITION variable=means complete dim=1
    #pragma HLS ARRAY_PARTITION variable=Rinv complete dim=1
    #pragma HLS ARRAY_PARTITION variable=means complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1


                // Cached cluster parameters
                float means[NUM_DIMENSIONS];
                float Rinv[NUM_DIMENSIONS*NUM_DIMENSIONS];
                float cluster_pi;
                float constant;
                const unsigned int tid = _tid_x;

                int start_index;
                int end_index;

                int c = _bid_x;

                compute_indices(num_events,&start_index,&end_index);

                float like;

                // This loop computes the expectation of every event into every cluster
                //
                // P(k|n) = L(x_n|mu_k,R_k)*P(k) / P(x_n)
                //
                // Compute log-likelihood for every cluster for each event
                // L = constant*exp(-0.5*(x-mu)*Rinv*(x-mu))
                // log_L = log_constant - 0.5*(x-u)*Rinv*(x-mu)
                // the constant stored in clusters[c].constant is already the log of the constant

                // copy the means for this cluster into shared memory
                if(tid < num_dimensions) {
                means[tid] = clusters->means[c*num_dimensions+tid];
                }

                // copy the covariance inverse into shared memory
                for(int i=tid; i < num_dimensions*num_dimensions; i+= NUM_THREADS_ESTEP) {
                Rinv[i] = clusters->Rinv[c*num_dimensions*num_dimensions+i];
                }

                cluster_pi = clusters->pi[c];
                constant = clusters->constant[c];

                // Sync to wait for all params to be loaded to shared memory

                for(int event=start_index; event<end_index; event += NUM_THREADS_ESTEP) {
                like = 0.0f;
                // this does the loglikelihood calculation
                #if DIAG_ONLY
                for(int j=0; j<num_dimensions; j++) {
                like += (data[j*num_events+event]-means[j]) * (data[j*num_events+event]-means[j]) * Rinv[j*num_dimensions+j];
                }
                #else
                for(int i=0; i<num_dimensions; i++) {
                for(int j=0; j<num_dimensions; j++) {
                like += (data[i*num_events+event]-means[i]) * (data[j*num_events+event]-means[j]) * Rinv[i*num_dimensions+j];
                }
                }
                #endif
                // numerator of the E-step probability computation
                clusters->memberships[c*num_events+event] = -0.5f * like + constant + logf(cluster_pi);
                }

            }
        }
    }
}
extern "C"

void estep2(float* fcs_data, clusters_t* clusters, int num_dimensions, int num_clusters, int num_events, float* likelihood) {
    #pragma HLS INTERFACE m_axi port=fcs_data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=clusters offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=num_dimensions
    #pragma HLS INTERFACE s_axilite port=num_clusters
    #pragma HLS INTERFACE s_axilite port=num_events
    #pragma HLS INTERFACE m_axi port=likelihood offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=total_likelihoods complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                float temp;
                float thread_likelihood = 0.0f;
                float total_likelihoods[NUM_THREADS_ESTEP];
                float max_likelihood;
                float denominator_sum;

                // Break up the events evenly between the blocks
                int num_pixels_per_block = num_events / GRID_DIM_X;
                // Make sure the events being accessed by the block are aligned to a multiple of 16
                num_pixels_per_block = num_pixels_per_block - (num_pixels_per_block % 16);
                int tid = _tid_x;

                int start_index;
                int end_index;
                start_index = _bid_x * num_pixels_per_block + tid;

                // Last block will handle the leftover events
                if(_bid_x == GRID_DIM_X-1) {
                end_index = num_events;
                } else {
                end_index = (_bid_x+1) * num_pixels_per_block;
                }

                total_likelihoods[tid] = 0.0;

                // P(x_n) = sum of likelihoods weighted by P(k) (their probability, cluster[c].pi)
                //  log(a+b) != log(a) + log(b) so we need to do the log of the sum of the exponentials

                //  For the sake of numerical stability, we first find the max and scale the values
                //  That way, the maximum value ever going into the exp function is 0 and we avoid overflow

                //  log-sum-exp formula:
                //  log(sum(exp(x_i)) = max(z) + log(sum(exp(z_i-max(z))))
                for(int pixel=start_index; pixel<end_index; pixel += NUM_THREADS_ESTEP) {
                // find the maximum likelihood for this event
                max_likelihood = clusters->memberships[pixel];
                for(int c=1; c<num_clusters; c++) {
                max_likelihood = fmaxf(max_likelihood,clusters->memberships[c*num_events+pixel]);
                }

                // Compute P(x_n), the denominator of the probability (sum of weighted likelihoods)
                denominator_sum = 0.0;
                for(int c=0; c<num_clusters; c++) {
                temp = expf(clusters->memberships[c*num_events+pixel]-max_likelihood);
                denominator_sum += temp;
                }
                denominator_sum = max_likelihood + logf(denominator_sum);
                thread_likelihood += denominator_sum;

                // Divide by denominator, also effectively normalize probabilities
                // exp(log(p) - log(denom)) == p / denom
                for(int c=0; c<num_clusters; c++) {
                clusters->memberships[c*num_events+pixel] = expf(clusters->memberships[c*num_events+pixel] - denominator_sum);
                //printf("Probability that pixel #%d is in cluster #%d: %f\n",pixel,c,clusters->memberships[c*num_events+pixel]);
                }
                }

                total_likelihoods[tid] = thread_likelihood;

                temp = parallelSum(total_likelihoods,NUM_THREADS_ESTEP);
                if(tid == 0) {
                likelihood[_bid_x] = temp;
                }

            }
        }
    }
}
extern "C"

void mstep_means(float* fcs_data, clusters_t* clusters, int num_dimensions, int num_clusters, int num_events) {
    #pragma HLS INTERFACE m_axi port=fcs_data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=clusters offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=num_dimensions
    #pragma HLS INTERFACE s_axilite port=num_clusters
    #pragma HLS INTERFACE s_axilite port=num_events
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=means complete dim=1
    #pragma HLS ARRAY_PARTITION variable=means complete dim=1
    #pragma HLS ARRAY_PARTITION variable=temp_sum complete dim=1
    #pragma HLS ARRAY_PARTITION variable=means complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                // One block per cluster, per dimension:  (M x D) grid of blocks
                int tid = _tid_x;
                int num_threads = BLOCK_DIM_X;
                int c = _bid_x; // cluster number
                int d = _bid_y; // dimension number

                float temp_sum[NUM_THREADS_MSTEP];
                float sum = 0.0f;

                for(int event=tid; event < num_events; event+= num_threads) {
                sum += fcs_data[d*num_events+event]*clusters->memberships[c*num_events+event];
                }
                temp_sum[tid] = sum;

                // Reduce partial sums
                sum = parallelSum(temp_sum,NUM_THREADS_MSTEP);
                if(tid == 0) {
                clusters->means[c*num_dimensions+d] = sum;
                }

                /*if(tid == 0) {
                for(int i=1; i < num_threads; i++) {
                temp_sum[0] += temp_sum[i];
                }
                clusters->means[c*num_dimensions+d] = temp_sum[0];
                //clusters->means[c*num_dimensions+d] = temp_sum[0] / clusters->N[c];
                }*/

            }
        }
    }
}
