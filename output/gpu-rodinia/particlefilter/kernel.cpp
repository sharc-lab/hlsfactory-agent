#include "kernel.h"

// --- from ex_particle_CUDA_float_seq.cu ---
void cdfCalc(double * CDF, double * weights, int Nparticles) {
    int x;
    CDF[0] = weights[0];
    for (x = 1; x < Nparticles; x++) {
        CDF[x] = weights[x] + CDF[x - 1];
    }
}

double d_randu(int * seed, int index) {

    int M = INT_MAX;
    int A = 1103515245;
    int C = 12345;
    int num = A * seed[index] + C;
    seed[index] = num % M;

    return fabs(seed[index] / ((double) M));
}/**

double d_randn(int * seed, int index) {
    //Box-Muller algortihm
    double pi = 3.14159265358979323846;
    double u = d_randu(seed, index);
    double v = d_randu(seed, index);
    double cosine = cos(2 * pi * v);
    double rt = -2 * log(u);
    return sqrt(rt) * cosine;
}

double updateWeights(double * weights, double * likelihood, int Nparticles) {
    int x;
    double sum = 0;
    for (x = 0; x < Nparticles; x++) {
        weights[x] = weights[x] * exp(likelihood[x]);
        sum += weights[x];
    }
    return sum;
}

int findIndexBin(double * CDF, int beginIndex, int endIndex, double value) {
    if (endIndex < beginIndex)
        return -1;
    int middleIndex;
    while (endIndex > beginIndex) {
        middleIndex = beginIndex + ((endIndex - beginIndex) / 2);
        if (CDF[middleIndex] >= value) {
            if (middleIndex == 0)
                return middleIndex;
            else if (CDF[middleIndex - 1] < value)
                return middleIndex;
            else if (CDF[middleIndex - 1] == value) {
                while (CDF[middleIndex] == value && middleIndex >= 0)
                    middleIndex--;
                middleIndex++;
                return middleIndex;
            }
        }
        if (CDF[middleIndex] > value)
            endIndex = middleIndex - 1;
        else
            beginIndex = middleIndex + 1;
    }
    return -1;
}

double dev_round_double(double value) {
    int newValue = (int) (value);
    if (value - newValue < .5f)
        return newValue;
    else
        return newValue++;
}
extern "C"

void find_index_kernel(double * arrayX, double * arrayY, double * CDF, double * u, double * xj, double * yj, double * weights, int Nparticles) {
    #pragma HLS INTERFACE m_axi port=arrayX offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=arrayY offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=CDF offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=xj offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=yj offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=weights offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=Nparticles
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int block_id = _bid_x;
                int i = BLOCK_DIM_X * block_id + _tid_x;

                if (i < Nparticles) {

                int index = -1;
                int x;

                for (x = 0; x < Nparticles; x++) {
                if (CDF[x] >= u[i]) {
                index = x;
                break;
                }
                }
                if (index == -1) {
                index = Nparticles - 1;
                }

                xj[i] = arrayX[index];
                yj[i] = arrayY[index];

                //weights[i] = 1 / ((double) (Nparticles)); //moved this code to the beginning of likelihood kernel

                }

            }
        }
    }
}
extern "C"

void normalize_weights_kernel(double * weights, int Nparticles, double* partial_sums, double * CDF, double * u, int * seed) {
    #pragma HLS INTERFACE m_axi port=weights offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=Nparticles
    #pragma HLS INTERFACE m_axi port=partial_sums offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=CDF offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=seed offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int block_id = _bid_x;
                int i = BLOCK_DIM_X * block_id + _tid_x;
                double u1, sumWeights;

                if(0 == _tid_x)
                sumWeights = partial_sums[0];

                if (i < Nparticles) {
                weights[i] = weights[i] / sumWeights;
                }

                if (i == 0) {
                cdfCalc(CDF, weights, Nparticles);
                u[0] = (1 / ((double) (Nparticles))) * d_randu(seed, i); // do this to allow all threads in all blocks to use the same u1
                }

                if(0 == _tid_x)
                u1 = u[0];

                if (i < Nparticles) {
                u[i] = u1 + i / ((double) (Nparticles));
                }

            }
        }
    }
}
extern "C"

void sum_kernel(double* partial_sums, int Nparticles) {
    #pragma HLS INTERFACE m_axi port=partial_sums offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=Nparticles
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int block_id = _bid_x;
                int i = BLOCK_DIM_X * block_id + _tid_x;

                if (i == 0) {
                int x;
                double sum = 0.0;
                int num_blocks = ceil((double) Nparticles / (double) threads_per_block);
                for (x = 0; x < num_blocks; x++) {
                sum += partial_sums[x];
                }
                partial_sums[0] = sum;
                }

            }
        }
    }
}
extern "C"

void likelihood_kernel(double * arrayX, double * arrayY, double * xj, double * yj, double * CDF, int * ind, int * objxy, double * likelihood, unsigned char * I, double * u, double * weights, int Nparticles, int countOnes, int max_size, int k, int IszY, int Nfr, int *seed, double* partial_sums) {
    #pragma HLS INTERFACE m_axi port=arrayX offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=arrayY offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=xj offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=yj offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=CDF offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=ind offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=objxy offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=likelihood offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=I offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=weights offset=slave bundle=gmem10
    #pragma HLS INTERFACE s_axilite port=Nparticles
    #pragma HLS INTERFACE s_axilite port=countOnes
    #pragma HLS INTERFACE s_axilite port=max_size
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=IszY
    #pragma HLS INTERFACE s_axilite port=Nfr
    #pragma HLS INTERFACE m_axi port=seed offset=slave bundle=gmem11
    #pragma HLS INTERFACE m_axi port=partial_sums offset=slave bundle=gmem12
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buffer complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int block_id = _bid_x;
                int i = BLOCK_DIM_X * block_id + _tid_x;
                int y;

                int indX, indY;
                double buffer[512];
                if (i < Nparticles) {
                arrayX[i] = xj[i];
                arrayY[i] = yj[i];

                weights[i] = 1 / ((double) (Nparticles)); //Donnie - moved this line from end of find_index_kernel to prevent all weights from being reset before calculating position on final iteration.

                arrayX[i] = arrayX[i] + 1.0 + 5.0 * d_randn(seed, i);
                arrayY[i] = arrayY[i] - 2.0 + 2.0 * d_randn(seed, i);

                }

                if (i < Nparticles) {
                for (y = 0; y < countOnes; y++) {
                //added dev_round_double() to be consistent with roundDouble
                indX = dev_round_double(arrayX[i]) + objxy[y * 2 + 1];
                indY = dev_round_double(arrayY[i]) + objxy[y * 2];

                ind[i * countOnes + y] = abs(indX * IszY * Nfr + indY * Nfr + k);
                if (ind[i * countOnes + y] >= max_size)
                ind[i * countOnes + y] = 0;
                }
                likelihood[i] = calcLikelihoodSum(I, ind, countOnes, i);

                likelihood[i] = likelihood[i] / countOnes;

                weights[i] = weights[i] * exp(likelihood[i]); //Donnie Newell - added the missing exponential function call

                }

                buffer[_tid_x] = 0.0;

                if (i < Nparticles) {

                buffer[_tid_x] = weights[i];
                }

                //this doesn't account for the last block that isn't full
                for (unsigned int s = BLOCK_DIM_X / 2; s > 0; s >>= 1) {
                if (_tid_x < s) {
                buffer[_tid_x] += buffer[_tid_x + s];
                }

                }
                if (_tid_x == 0) {
                partial_sums[_bid_x] = buffer[0];
                }



            }
        }
    }
}


// --- from ex_particle_CUDA_naive_seq.cu ---
int findIndexSeq(double * CDF, int lengthCDF, double value)
{
	int index = -1;
	int x;
	for(x = 0; x < lengthCDF; x++)
	{
		if(CDF[x] >= value)
		{
			index = x;
			break;
		}
	}
	if(index == -1)
		return lengthCDF-1;
	return index;
}

int findIndexBin(double * CDF, int beginIndex, int endIndex, double value)
{
	if(endIndex < beginIndex)
		return -1;
	int middleIndex;
	while(endIndex > beginIndex)
	{
		middleIndex = beginIndex + ((endIndex-beginIndex)/2);
		if(CDF[middleIndex] >= value)
		{
			if(middleIndex == 0)
				return middleIndex;
			else if(CDF[middleIndex-1] < value)
				return middleIndex;
			else if(CDF[middleIndex-1] == value)
			{
				while(CDF[middleIndex] == value && middleIndex >= 0)
					middleIndex--;
				middleIndex++;
				return middleIndex;
			}
		}
		if(CDF[middleIndex] > value)
			endIndex = middleIndex-1;
		else
			beginIndex = middleIndex+1;
	}
	return -1;
}
extern "C"

void kernel(double * arrayX, double * arrayY, double * CDF, double * u, double * xj, double * yj, int Nparticles){
    #pragma HLS INTERFACE m_axi port=arrayX offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=arrayY offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=CDF offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=xj offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=yj offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=Nparticles
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int block_id = _bid_x;// + GRID_DIM_X * _bid_y;
                int i = BLOCK_DIM_X * block_id + _tid_x;

                if(i < Nparticles){

                int index = -1;
                int x;

                for(x = 0; x < Nparticles; x++){
                if(CDF[x] >= u[i]){
                index = x;
                break;
                }
                }
                if(index == -1){
                index = Nparticles-1;
                }

                xj[i] = arrayX[index];
                yj[i] = arrayY[index];

                }

            }
        }
    }
}
