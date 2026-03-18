#include "kernel.h"

// --- from petri_kernel.cu ---
void fire_transition(char* g_places, int* conflict_array, int tr, 
		     int tc, int step, int N, int thd_thrd) 
{
  int val1, val2, val3, to_update;
  int mark1, mark2;
	
  to_update = 0;
  if (_tid_x<thd_thrd) 
    {
      // check if the transition is enabled and conflict-free
      val1 = (tr==0)? (N+N)-1: tr-1;
      val2 = (tr & 0x1)? (tc==N-1? 0: tc+1): tc;
      val3 = (tr==(N+N)-1)? 0: tr+1;
      mark1 = g_places[val1*N+val2];
      mark2 = g_places[tr*N+tc];
      if ( (mark1>0) && (mark2>0) ) 
	{
	  to_update = 1;
	  conflict_array[tr*N+tc] = step;
	}
    }

  if (to_update) 
    {
      // If there are conflicts, transitions on even/odd rows are 
      // kept when the step is even/odd
      to_update = ((step & 0x01) == (tr & 0x01) ) || 
	( (conflict_array[val1*N+val2]!=step) && 
	  (conflict_array[val3*N+((val2==0)? N-1: val2-1)]!=step) );
    }

  // now update state
  // 6 kernel memory accesses 
  if (to_update) 
    {
      g_places[val1*N+val2] = mark1-1;  // the place above
      g_places[tr*N+tc] = mark2-1; // the place on the left
    }
  if (to_update) 
    {
      g_places[val3*N+val2]++;  // the place below
      g_places[tr*N+(tc==N-1? 0: tc+1)]++; // the place on the right
    }
}

void initialize_grid(int* g_places, int NSQUARE2, int seed) 
{
  // N is an even number
  int i;
  int loop_num = NSQUARE2 >> (BLOCK_SIZE_BITS+2);
	
  for (i=0; i<loop_num; i++) 
    {
      g_places[_tid_x+(i<<BLOCK_SIZE_BITS)] = 0x01010101;
    }
    
  if (_tid_x < (NSQUARE2>>2)-(loop_num<<BLOCK_SIZE_BITS)) 
    {
      g_places[_tid_x+(loop_num<<BLOCK_SIZE_BITS)] = 0x01010101;
    }
	
  RandomInit(_bid_x+seed);
}

void run_trajectory(int* g_places, int N, int max_steps) 
{
  int step, NSQUARE2, val;

  step = 0;
  NSQUARE2 = (N+N)*N;
	
  while (step<max_steps) 
    {
      BRandom(); // select the next MERS_N (624) transitions

      // process 256 transitions
      val = mt[_tid_x]%NSQUARE2;
      fire_transition((char*)g_places, g_places+(NSQUARE2>>2), 
		      val/N, val%N, step+7, N, BLOCK_SIZE);
      
      // process 256 transitions
      val = mt[_tid_x+BLOCK_SIZE]%NSQUARE2;
      fire_transition((char*)g_places, g_places+(NSQUARE2>>2), 
		      val/N, val%N, step+11, N, BLOCK_SIZE);
		                
      // process 112 transitions
      if (  _tid_x < MERS_N-(BLOCK_SIZE<<1)  ) 
	{
	  val = mt[_tid_x+(BLOCK_SIZE<<1)]%NSQUARE2;
	}
      fire_transition((char*)g_places, g_places+(NSQUARE2>>2), 
		      val/N, val%N, step+13, N, MERS_N-(BLOCK_SIZE<<1));

      step += MERS_N>>1; 
      // experiments show that for N>2000 and max_step<20000, 
      // the step increase is larger than 320
    }
}

void compute_reward_stat(int * g_places,
                         float*  g_vars,
                         int*  g_maxs, 
			 int NSQUARE2) 
{
  float sum = 0;
  int i;
  int max = 0;
  int temp, data; 
  int loop_num = NSQUARE2 >> (BLOCK_SIZE_BITS+2);
  for (i=0; i<=loop_num-1; i++) 
    {  // a bug. i<loop_num should be changed to i<=loop_num-1
      data = g_places[_tid_x+(i<<BLOCK_SIZE_BITS)];
	    
      temp = data & 0x0FF;
      sum += temp*temp;
      max = max<temp? temp: max;
      temp = (data>>8) & 0x0FF;
      sum += temp*temp;
      max = max<temp? temp: max;
      temp = (data>>16) & 0x0FF;
      sum += temp*temp;
      max = max<temp? temp: max;
      temp = (data>>24) & 0x0FF;
      sum += temp*temp;
      max = max<temp? temp: max;
    }

  i = NSQUARE2>>2;
  i &= 0x0FF;
  loop_num *= BLOCK_SIZE; 
  // I do not know why loop_num<<=BLOCK_SIZE_BITS does not work
  if (_tid_x <= i-1) 
    {
      data = g_places[_tid_x+loop_num];
	    
      temp = data & 0x0FF;
      sum += temp*temp;
      max = max<temp? temp: max;
      temp = (data>>8) & 0x0FF;
      sum += temp*temp;
      max = max<temp? temp: max;
      temp = (data>>16) & 0x0FF;
      sum += temp*temp;
      max = max<temp? temp: max;
      temp = (data>>24) & 0x0FF;
      sum += temp*temp;
      max = max<temp? temp: max;
    }
	
  ((float*)mt)[_tid_x] = (float)sum;
  mt[_tid_x+BLOCK_SIZE] = (uint32)max;
		
  for (i=(BLOCK_SIZE>>1); i>0; i = (i>>1) ) 
    {
      if (_tid_x<i) 
	{
	  ((float*)mt)[_tid_x] += ((float*)mt)[_tid_x+i];
	  if (mt[_tid_x+BLOCK_SIZE]<mt[_tid_x+i+BLOCK_SIZE])
	    mt[_tid_x+BLOCK_SIZE] = mt[_tid_x+i+BLOCK_SIZE];
	}
    }
		
  if (_tid_x==0) 
    {
      g_vars[_bid_x] = (((float*)mt)[0])/NSQUARE2-1; 
      // D(X)=E(X^2)-E(X)^2, E(X)=1
      g_maxs[_bid_x] = (int)mt[BLOCK_SIZE];
    }
}
extern "C"

void PetrinetKernel(int*  g_s,
                    float*  g_v,
                    int* g_m,
                    int n, int s, int seed) 
{
    #pragma HLS INTERFACE m_axi port=g_s offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_v offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_m offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=s
    #pragma HLS INTERFACE s_axilite port=seed
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE m_axi port=g_s offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_v offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_m offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=s
    #pragma HLS INTERFACE s_axilite port=seed
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            // block size must be 256
            // n is an even number
            int NSQUARE2 = n*n*2;
            int* g_places = g_s+_bid_x*((NSQUARE2>>2)+NSQUARE2);
            // place numbers, conflict_array
            initialize_grid(g_places, NSQUARE2, seed);

            run_trajectory(g_places, n, s);
            compute_reward_stat(g_places, g_v, g_m, NSQUARE2);

        }
    }
SQUARE2>>2)+NSQUARE2);
            // place numbers, conflict_array
            initialize_grid(g_places, NSQUARE2, seed);

            run_trajectory(g_places, n, s);
            compute_reward_stat(g_places, g_v, g_m, NSQUARE2);

        }
    }
}


// --- from rand_gen.cu ---
void RandomInit(uint32 seed) 
{
  int i;
  // re-seed generator
  if(_tid_x == 0)
    {
      mt[0]= seed & 0xffffffffUL;
      for (i=1; i < MERS_N; i++) 
	{
	  mt[i] = (1812433253UL * (mt[i-1] ^ (mt[i-1] >> 30)) + i);
  	}
    }
}

void BRandom() 
{
  // generate 32 random bits
  uint32 y;
  int thdx;

  // block size is 256
  // step 1: 0-226, MERS_N-MERS_M=227
  if (_tid_x<MERS_N-MERS_M) 
    {
      y = (mt[_tid_x] & UPPER_MASK) | (mt[_tid_x+1] & LOWER_MASK);
      y = mt[_tid_x+MERS_M] ^ (y >> 1) ^ ( (y & 1)? MERS_A: 0);
    }
  if (_tid_x<MERS_N-MERS_M) 
    {
      mt[_tid_x] = y;
    }
  
  // step 2: 227-453
  thdx = _tid_x + (MERS_N-MERS_M);
  if (_tid_x<MERS_N-MERS_M) 
    {
      y = (mt[thdx] & UPPER_MASK) | (mt[thdx+1] & LOWER_MASK);
      y = mt[_tid_x] ^ (y >> 1) ^ ( (y & 1)? MERS_A: 0);
    }
  if (_tid_x<MERS_N-MERS_M) 
    {
      mt[thdx] = y;
    }
  
  // step 3: 454-622
  thdx += (MERS_N-MERS_M);
  if (thdx < MERS_N-1) 
    {
      y = (mt[thdx] & UPPER_MASK) | (mt[thdx+1] & LOWER_MASK);
      y = mt[_tid_x+(MERS_N-MERS_M)] ^ (y >> 1) ^ ( (y & 1)? MERS_A: 0);
    }
  if (thdx < MERS_N-1) 
    {
      mt[thdx] = y;
    }

  // step 4: 623
  if (_tid_x == 0) 
    {
      y = (mt[MERS_N-1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
      mt[MERS_N-1] = mt[MERS_M-1] ^ (y >> 1) ^ ( (y & 1)? MERS_A: 0);
    }

  // Tempering (May be omitted):
  y ^=  y >> MERS_U;
  y ^= (y << MERS_S) & MERS_B;
  y ^= (y << MERS_T) & MERS_C;
  y ^=  y >> MERS_L;

}
