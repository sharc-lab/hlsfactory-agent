#include "kernel.h"

// --- from blockmatching.cu ---
__inline__ uint flp2 (uint x)
{
  return (0x80000000u >> __clz(x));
}

__inline__ T L2p2(const T i1, const T i2)
{
  T diff = i1 - i2;
  return diff*diff;
}
extern "C"

void block_matching(
    const  uchar* __restrict image, //IN: Original image
    ushort* __restrict g_stacks,         //OUT: For each reference patch contains addresses of similar patches (patch is adressed by top left corner) [..LOC_Y(sbyte)..|..LOC_X(sbyte)..]
    uint* __restrict g_num_patches_in_stack,  //OUT: For each reference patch contains number of similar patches
    const uint2 image_dim,      //IN: Image dimensions
    const uint2 stacks_dim,      //IN: Size of area, where reference patches could be located
    const Params params,      //IN: Denoising parameters
    const uint2 start_point)    //IN: Address of the top-left reference patch of a batch
{
  //One block is processing warpSize patches (because each warp is computing distance of same warpSize patches from different displaced patches)
  int tid = _tid_x % warpSize;
  int wid = _tid_x / warpSize;
  int num_warps = BLOCK_DIM_X/warpSize;

  //p_block denotes reference rectangle on which current cuda block is computing
  uint p_rectangle_width = ((warpSize-1) * params.p) + params.k;
  uint p_rectangle_start = start_point.x + _bid_x * warpSize * params.p;

  //Shared arrays
  uint s_data[4096];
  uint *s_diff = s_data; //SIZE: p_rectangle_width*num_warps
  uint *s_stacks = &s_data[p_rectangle_width*num_warps]; //SIZE: params.N*num_warps*warpSize
  uchar *s_patches_in_stack = (uchar*)&s_data[num_warps*(p_rectangle_width + params.N*warpSize)]; //SIZE: num_warps*warpSize
  uchar *s_image_p = (uchar*)&s_patches_in_stack[num_warps*warpSize]; //SIZE: p_rectangle_width*params.k

  s_diff += idx2(0, wid, p_rectangle_width);

  //Initialize s_patches_in_stack to zero
  s_patches_in_stack[ idx2(tid, wid, warpSize) ] = 0;

  int2 p; //Address of reference patch
  int2 q; //Address of patch against which the difference is computed

  p.x = p_rectangle_start + (tid*params.p);
  p.y = start_point.y + (_bid_y*params.p);

  //Ensure, that the bottom most patches will be taken as reference patches regardless the p parameter.
  if (p.y >= stacks_dim.y && p.y < stacks_dim.y + params.p - 1)
    p.y = stacks_dim.y - 1;
  else if (p.y >= stacks_dim.y) return;

  //Ensure, that the right most patches will be taken as reference patches regardless the p parameter.
  uint inner_p_x = tid*params.p;
  if (p.x >= stacks_dim.x && p.x < stacks_dim.x + params.p - 1)
  {
    inner_p_x -= (p.x - (stacks_dim.x - 1));
    p.x = stacks_dim.x - 1;
  }

  //Load reference patches needed by actual block to shared memory
  for(int i = _tid_x; i < p_rectangle_width*params.k; i+=BLOCK_DIM_X)
  {
    int sx = i % p_rectangle_width;
    int sy = i / p_rectangle_width;
    if (p_rectangle_start+sx >= image_dim.x) continue;
    s_image_p[i] = image[idx2(p_rectangle_start+sx,p.y+sy,image_dim.x)];
  }

  //scale difference so that it can fit ushort
  uint shift = (__clz(params.Tn) < 16u) ? 16u - (uint)__clz(params.Tn) : 0;

  //Ensure that displaced patch coordinates (q) will be positive
  int2 from;
  from.y = (p.y - (int)params.n < 0) ? -p.y : -(int)params.n;
  from.x = (((int)p_rectangle_start) - (int)params.n < 0) ? -((int)p_rectangle_start) : -(int)params.n;
  from.x += wid;

  //For each displacement (x,y) in n neighbourhood
  for(int y = from.y; y <= (int)params.n; ++y)
  {
    q.y = p.y + y;
    if (q.y >= stacks_dim.y) break;

    for(int x = from.x; x <= (int)params.n; x += num_warps)
    {
      //Reference patch is always the most similar to itself (there is no need to copute it)
      if (x == 0 && y == 0) continue; 

      //Each warp is computing the same patch with slightly different displacement.
      //Compute distance of reference patch p from current patch q which is dispaced by (x+tid,y)

      //q_block denotes displaced rectangle which is processed by the current warp
      uint q_rectangle_start = p_rectangle_start + x;
      q.x = q_rectangle_start + inner_p_x;

      //Compute distance for each column of reference patch
      for(uint i = tid; i < p_rectangle_width && p_rectangle_start+i < image_dim.x && 
                        q_rectangle_start+i < image_dim.x; i+=warpSize)
      {
        uint dist = 0;
        for(uint iy = 0; iy < params.k; ++iy)
        {
          dist += L2p2((int)s_image_p[ idx2(i, iy, p_rectangle_width) ], 
                       (int)image[ idx2(q_rectangle_start+i, q.y+iy, image_dim.x) ]);
        }
        s_diff[i] = dist;
      }

      if (p.x >= stacks_dim.x || q.x >= stacks_dim.x) continue;

      //Sum column distances to obtain patch distance
      uint diff = 0;
      for (uint i = 0; i < params.k; ++i) 
        diff += s_diff[inner_p_x + i];

      //Distance threshold
      if(diff < params.Tn)
      {
        uint loc_y = (uint)((q.y - p.y) & 0xFF); //relative location y (-127 to 127)
        uint loc_x = (uint)((q.x - p.x) & 0xFF); //relative location x (-127 to 127)
        diff >>= shift;
        diff <<= 16u; // [..DIFF(ushort)..|..LOC_Y(sbyte)..|..LOC_X(sbyte)..]
        diff |= (loc_y << 8u);
        diff |= loc_x;

        //Add current patch to s_stacks
        add_to_matched_image( 
            &s_stacks[ params.N * idx2(tid, wid, warpSize) ],
            &s_patches_in_stack[ idx2(tid, wid, warpSize) ],
            diff,
            params
            );
      }
    }
  }

  uint batch_size = GRID_DIM_X*warpSize;
  uint block_address_x = _bid_x*warpSize+tid;

  if (wid > 0) return;
  //Select N most similar patches for each reference patch from stacks in shared memory and save them to global memory
  //Each thread represents one reference patch 
  //Each thread will find N most similar blocks in num_warps stacks (which were computed by different warps) and save them into global memory
  //In shared memory the most similar patch is at the end, in global memory the order does not matter
  //DEV: performance impact cca 8%
  if (p.x >= stacks_dim.x) return;

  int j;
  for (j = 0; j < params.N; ++j)
  {
    uint count = 0;
    uint minIdx = 0;
    uint minVal = 0xFFFFFFFF; //INF

    //Finds patch with minimal value of remaining
    for (int i = minIdx; i < num_warps; ++i)
    {
      count = (uint)s_patches_in_stack[ idx2(tid, i, warpSize) ];
      if (count == 0) continue;

      uint newMinVal = s_stacks[ idx3(count-1,tid,i,params.N,warpSize) ];
      if (newMinVal < minVal)
      {
        minVal = newMinVal;
        minIdx = i;
      }
    }
    if (minVal == 0xFFFFFFFF) break; //All stacks are empty

    //Remove patch from shared stack
    s_patches_in_stack[ idx2(tid, minIdx, warpSize) ]--;

    //Adds patch to stack in global memory
    g_stacks[idx3(j, block_address_x, _bid_y, params.N, batch_size)] = (ushort)(minVal & 0xFFFF);
  }
  //Save to the global memory the number of similar patches rounded to the nearest lower power of two
  g_num_patches_in_stack[ idx2(block_address_x ,_bid_y, batch_size) ] = flp2((uint)j+1)-1;
}


// --- from dct8x8.cu ---
void InplaceIDCTvector(float *Vect0, int Step)
{
    float *Vect1 = Vect0 + Step;
    float *Vect2 = Vect1 + Step;
    float *Vect3 = Vect2 + Step;
    float *Vect4 = Vect3 + Step;
    float *Vect5 = Vect4 + Step;
    float *Vect6 = Vect5 + Step;
    float *Vect7 = Vect6 + Step;

    float Y04P   = (*Vect0) + (*Vect4);
    float Y2b6eP = C_b * (*Vect2) + C_e * (*Vect6);

    float Y04P2b6ePP = Y04P + Y2b6eP;
    float Y04P2b6ePM = Y04P - Y2b6eP;
    float Y7f1aP3c5dPP = C_f * (*Vect7) + C_a * (*Vect1) + C_c * (*Vect3) + C_d * (*Vect5);
    float Y7a1fM3d5cMP = C_a * (*Vect7) - C_f * (*Vect1) + C_d * (*Vect3) - C_c * (*Vect5);

    float Y04M   = (*Vect0) - (*Vect4);
    float Y2e6bM = C_e * (*Vect2) - C_b * (*Vect6);

    float Y04M2e6bMP = Y04M + Y2e6bM;
    float Y04M2e6bMM = Y04M - Y2e6bM;
    float Y1c7dM3f5aPM = C_c * (*Vect1) - C_d * (*Vect7) - C_f * (*Vect3) - C_a * (*Vect5);
    float Y1d7cP3a5fMM = C_d * (*Vect1) + C_c * (*Vect7) - C_a * (*Vect3) + C_f * (*Vect5);

    (*Vect0) = C_norm * (Y04P2b6ePP + Y7f1aP3c5dPP);
    (*Vect7) = C_norm * (Y04P2b6ePP - Y7f1aP3c5dPP);
    (*Vect4) = C_norm * (Y04P2b6ePM + Y7a1fM3d5cMP);
    (*Vect3) = C_norm * (Y04P2b6ePM - Y7a1fM3d5cMP);

    (*Vect1) = C_norm * (Y04M2e6bMP + Y1c7dM3f5aPM);
    (*Vect5) = C_norm * (Y04M2e6bMM - Y1d7cP3a5fMM);
    (*Vect2) = C_norm * (Y04M2e6bMM + Y1d7cP3a5fMM);
    (*Vect6) = C_norm * (Y04M2e6bMP - Y1c7dM3f5aPM);
}
extern "C"

void DCT2D8x8(float *__restrict dst, const float *__restrict src, const uint size)
{
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float block[KER2_BLOCK_HEIGHT * KER2_SMEMBLOCK_STRIDE];

                    if (_bid_x * KER2_BLOCK_HEIGHT * KER2_BLOCK_WIDTH + (_tid_y+1) * BLOCK_SIZE*BLOCK_SIZE-1 >= size) return;

                    int offset = _tid_y * (BLOCK_SIZE*BLOCK_SIZE) + _tid_x;

                    //Get macro-block address
                    src += _bid_x * KER2_BLOCK_HEIGHT * KER2_BLOCK_WIDTH;
                    dst += _bid_x * KER2_BLOCK_HEIGHT * KER2_BLOCK_WIDTH;

                    //8x1 blocks in one macro-block (_tid_y - index of block inside the macro-block)
                    //Get the first element of the column in the block with index _tid_y
                    src += offset;
                    dst += offset;

                    float *bl_ptr = block + offset;

                    #pragma unroll

                    for (unsigned int i = 0; i < BLOCK_SIZE; i++)
                    bl_ptr[i * BLOCK_SIZE] = src[i * BLOCK_SIZE]; //Load column to the shared mem

                    //process rows
                    InplaceDCTvector(bl_ptr - _tid_x + BLOCK_SIZE * _tid_x, 1);

                    //process columns
                    InplaceDCTvector(bl_ptr, BLOCK_SIZE);

                    for (unsigned int i = 0; i < BLOCK_SIZE; i++)
                    dst[i * BLOCK_SIZE] = bl_ptr[i * BLOCK_SIZE];

                }
            }
        }
    }
}
extern "C"

void IDCT2D8x8(float *__restrict dst, const float *__restrict src, const uint size)
{
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float block[KER2_BLOCK_HEIGHT * KER2_SMEMBLOCK_STRIDE];

                    if (_bid_x * KER2_BLOCK_HEIGHT * KER2_BLOCK_WIDTH + (_tid_y+1) * BLOCK_SIZE*BLOCK_SIZE-1 >= size) return;

                    int offset = _tid_y * (BLOCK_SIZE*BLOCK_SIZE) + _tid_x;

                    src += _bid_x * KER2_BLOCK_HEIGHT * KER2_BLOCK_WIDTH;
                    dst += _bid_x * KER2_BLOCK_HEIGHT * KER2_BLOCK_WIDTH;

                    src += offset;
                    dst += offset;

                    float *bl_ptr = block + offset;

                    #pragma unroll

                    for (unsigned int i = 0; i < BLOCK_SIZE; i++)
                    bl_ptr[i * BLOCK_SIZE] = src[i * BLOCK_SIZE];

                    //process rows
                    InplaceIDCTvector(bl_ptr - _tid_x + BLOCK_SIZE * _tid_x, 1);

                    //process columns
                    InplaceIDCTvector(bl_ptr, BLOCK_SIZE);

                    for (unsigned int i = 0; i < BLOCK_SIZE; i++)
                    dst[i * BLOCK_SIZE] = bl_ptr[i * BLOCK_SIZE];

                }
            }
        }
    }
}


// --- from filtering.cu ---
inline T warpReduceSum(T val) 
{
  for (int offset = warpSize/2; offset > 0; offset /= 2)
    val += 0;
  return val;
}

__inline__ float blockReduceSum(T* shared, T val, int tid, int tcount) 
{
  int lane = tid % warpSize;
  int wid = tid / warpSize;

  val = warpReduceSum(val);     // Each warp performs partial reduction

  if (lane==0) shared[wid]=val; // Write reduced value to shared memory

  //read from shared memory only if that warp existed
  val = (tid < tcount / warpSize) ? shared[lane] : 0;

  if (wid==0) val = warpReduceSum(val); //Final reduce within first warp

  return val;
}

float abspow2(float & a)
{
  return a * a;
}

__inline__ uint ilog2(IntType n)
{
  uint l;
  for (l = 0; n; n >>= 1, ++l);
  return l;
}

__inline__ void rotate(T& a, T& b)
{
  T tmp;
  tmp = a;
  a = tmp + b;
  b = tmp - b;
}

__inline__ void fwht(T *data, uint n)
{
  unsigned l2 = ilog2(n) - 1;
  for ( uint i = 0; i < l2; ++i )
  {
    for (uint j = 0; j < n; j += (1 << (i + 1)))
    for (uint k = 0; k < (uint)(1 << i); ++k)
      rotate(data[j + k], data[j + k + (uint)(1 << i)]);
  }
}

inline void get_block_addresses(
  const uint2 & start_point,    //IN: first reference patch of a batch
  const uint & patch_stack_size,  //IN: maximal size of a 3D group
  const uint2 & stacks_dim,    //IN: Size of area, where reference patches could be located
  const Params & params,      //IN: Denoising parameters
  uint2 & outer_address,      //OUT: Coordinetes of reference patch in the image
  uint & start_idx)        //OUT: Address of a first element of the 3D group in stacks array
{
  //One block handles one patch_stack, data are in array one after one.
  start_idx = patch_stack_size * idx2(_bid_x,_bid_y,GRID_DIM_X);
  
  outer_address.x = start_point.x + (_bid_x * params.p);
  outer_address.y = start_point.y + (_bid_y * params.p);

  //Ensure, that the bottom most patches will be taken as reference patches regardless the p parameter.
  if (outer_address.y >= stacks_dim.y && outer_address.y < stacks_dim.y + params.p - 1)
    outer_address.y = stacks_dim.y - 1;
  //Ensure, that the right most patches will be taken as reference patches regardless the p parameter.
  if (outer_address.x >= stacks_dim.x && outer_address.x < stacks_dim.x + params.p - 1)
    outer_address.x = stacks_dim.x - 1;
}
extern "C"

void get_block(
    const uint2 start_point,                       //IN: first reference patch of a batch
    const uchar* __restrict image,                 //IN: image
    const ushort* __restrict stacks,               //IN: array of adresses of similar patches
    const uint* __restrict g_num_patches_in_stack, //IN: numbers of patches in 3D groups
    float* __restrict patch_stack,                 //OUT: assembled 3D groups
    const uint2 image_dim,                         //IN: image dimensions
    const uint2 stacks_dim,                        //IN: dimensions limiting addresses of reference patches
    const Params params)                           //IN: denoising parameters
{
  
  
  uint startidx;
  uint2 outer_address;
  get_block_addresses(start_point,  params.k*params.k*(params.N+1), stacks_dim, params, outer_address, startidx);

  if (outer_address.x >= stacks_dim.x || outer_address.y >= stacks_dim.y) return;
  
  patch_stack += startidx;
  
  const ushort* z_ptr = &stacks[ idx3(0, _bid_x, _bid_y, params.N,  GRID_DIM_X) ];

  uint num_patches = g_num_patches_in_stack[ idx2(_bid_x, _bid_y, GRID_DIM_X) ];
  
  patch_stack[ idx3(_tid_x, _tid_y, 0, params.k, params.k) ] = (float)(image[ idx2(outer_address.x+_tid_x, outer_address.y+_tid_y, image_dim.x)]);
  for(uint i = 0; i < num_patches; ++i)
  {
    int x = (int)((signed char)(z_ptr[i] & 0xFF));
    int y = (int)((signed char)((z_ptr[i] >> 8) & 0xFF));
    patch_stack[ idx3(_tid_x, _tid_y, i+1, params.k, params.k) ] = (float)(image[ idx2(outer_address.x+x+_tid_x, outer_address.y+y+_tid_y, image_dim.x)]);
  }
}
extern "C"

void hard_treshold_block(
  const uint2 start_point,                        //IN: first reference patch of a batch
  float* __restrict patch_stack,                  //IN/OUT: 3D groups with thransfomed patches
  float* __restrict  w_P,                         //OUT: weight of each 3D group
  const uint* __restrict g_num_patches_in_stack,  //IN: numbers of patches in 3D groups
  uint2 stacks_dim,                               //IN: dimensions limiting addresses of reference patches
  const Params params,                            //IN: denoising parameters
  const uint sigma                                //IN: noise variance
)
{
    #pragma HLS INTERFACE s_axilite port=start_point
    #pragma HLS INTERFACE m_axi port=patch_stack offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=w_P offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_num_patches_in_stack offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=stacks_dim
    #pragma HLS INTERFACE s_axilite port=params
    #pragma HLS INTERFACE s_axilite port=variance
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1
    #pragma HLS ARRAY_PARTITION variable=block complete dim=1
    #pragma HLS ARRAY_PARTITION variable=data complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float data[4096];

                    int paramN = params.N+1;
                    uint tcount = BLOCK_DIM_X*BLOCK_DIM_Y;
                    uint tid = idx2(_tid_x, _tid_y, BLOCK_DIM_X);
                    uint patch_stack_size = tcount * paramN;

                    uint startidx;
                    uint2 outer_address;
                    get_block_addresses(start_point, patch_stack_size, stacks_dim, params, outer_address, startidx);

                    if (outer_address.x >= stacks_dim.x || outer_address.y >= stacks_dim.y) return;

                    uint num_patches = g_num_patches_in_stack[ idx2(_bid_x, _bid_y, GRID_DIM_X) ]+1; //+1 for the reference patch.
                    float* s_patch_stack = data + (tid * (num_patches+1)); //+1 for avoiding bank conflicts //TODO:sometimes
                    patch_stack = patch_stack + startidx + tid;

                    //Load to the shared memory
                    for(uint i = 0; i < num_patches; ++i)
                    s_patch_stack[i] = patch_stack[ i*tcount ];

                    //1D Transform
                    fwht(s_patch_stack, num_patches);

                    //Hard-thresholding + counting of nonzero coefficients
                    uint nonzero = 0;
                    float threshold = params.L3D * sqrtf((float)(num_patches * sigma));
                    for(int i = 0; i < num_patches; ++i)
                    {
                    if (fabsf(s_patch_stack[ i ]) < threshold)
                    {
                    s_patch_stack[ i ] = 0.0f;
                    }
                    else
                    ++nonzero;
                    }

                    //Inverse 1D Transform
                    fwht(s_patch_stack, num_patches);

                    //Normalize and save to global memory
                    for (uint i = 0; i < num_patches; ++i)
                    {
                    patch_stack[ i*tcount ] = s_patch_stack[i] / num_patches;
                    }

                    //Reuse the shared memory for 32 partial sums
                    uint* shared = (uint*)data;
                    //Sum the number of non-zero coefficients for a 3D group
                    nonzero = blockReduceSum<uint>(shared, nonzero, tid, tcount);

                    //Save the weight of a 3D group (1/nonzero coefficients)
                    if (tid == 0)
                    {
                    if (nonzero < 1) nonzero = 1;
                    w_P[ idx2(_bid_x, _bid_y, GRID_DIM_X ) ] = 1.0f/(float)nonzero;
                    }

                }
            }
        }
    }
}
extern "C"

void aggregate_block(
  const uint2 start_point,                        //IN: first reference patch of a batch
  const float* __restrict patch_stack,            //IN: 3D groups with thransfomed patches
  const float* __restrict w_P,                    //IN: weight for each 3D group
  const ushort* __restrict stacks,                //IN: array of adresses of similar patches
  const float* __restrict kaiser_window,          //IN: kaiser window
  float* __restrict numerator,                    //IN/OUT: numerator aggregation buffer (have to be initialized to 0)
  float* __restrict denominator,                  //IN/OUT: denominator aggregation buffer (have to be initialized to 0)
  const uint* __restrict g_num_patches_in_stack,  //IN: numbers of patches in 3D groups
  const uint2 image_dim,                          //IN: image dimensions
  const uint2 stacks_dim,                         //IN: dimensions limiting addresses of reference patches
  const Params params                             //IN: denoising parameters
)
{    
  uint startidx;
  uint2 outer_address;
  get_block_addresses(start_point, params.k*params.k*(params.N+1), stacks_dim, params, outer_address, startidx);
  
  if (outer_address.x >= stacks_dim.x || outer_address.y >= stacks_dim.y) return;

  patch_stack += startidx;

  uint num_patches = g_num_patches_in_stack[ idx2(_bid_x, _bid_y, GRID_DIM_X) ]+1;

  float wp = w_P[ idx2(_bid_x, _bid_y, GRID_DIM_X ) ];
  
  const ushort* z_ptr = &stacks[ idx3(0, _bid_x, _bid_y, params.N,  GRID_DIM_X) ];

  float kaiser_value = kaiser_window[ idx2(_tid_x, _tid_y, params.k) ];

  for(uint z = 0; z < num_patches; ++z)
  {
    int x = 0;
    int y = 0;
    if (z > 0) {
      x = (int)((signed char)(z_ptr[z-1] & 0xFF));
      y = (int)((signed char)((z_ptr[z-1] >> 8) & 0xFF));
    }

    float value = ( patch_stack[ idx3(_tid_x, _tid_y, z, params.k, params.k) ]);
    int idx = idx2(outer_address.x + x + _tid_x, outer_address.y + y + _tid_y, image_dim.x);
    atomicAdd(numerator + idx, value * kaiser_value * wp);
    atomicAdd(denominator + idx, kaiser_value * wp);
  }
}
extern "C"

void aggregate_final(
  const float* __restrict numerator,    //IN: numerator aggregation buffer
  const float* __restrict denominator,  //IN: denominator aggregation buffer
  const uint2 image_dim,                //IN: image dimensions
  uchar*__restrict result)              //OUT: image estimate
{
  uint idx = _bid_x * BLOCK_DIM_X + _tid_x;
  uint idy = _bid_y * BLOCK_DIM_Y + _tid_y;
  if (idx >= image_dim.x || idy >= image_dim.y) return;

  int value = lrintf(numerator[ idx2(idx,idy,image_dim.x) ] / denominator[ idx2(idx,idy,image_dim.x) ] );
  if (value < 0) value = 0;
  if (value > 255) value = 255;
  result[ idx2(idx,idy,image_dim.x) ] = (uchar)value;
}
