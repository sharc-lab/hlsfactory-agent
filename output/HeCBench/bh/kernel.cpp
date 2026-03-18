#include "kernel.h"

// --- from main.cu ---
extern "C"
void BoundingBoxKernel(
    const int nnodesd,
    const int nbodiesd,
    int* const  startd,
    int* const  childd,
    float4* const  posMassd,
    float3* const  maxd,
    float3* const  mind,
    float* const  radiusd,
    int* const  bottomd,
    int* const  stepd,
    unsigned int* const  blkcntd)
{
    #pragma HLS INTERFACE s_axilite port=nnodesd
    #pragma HLS INTERFACE s_axilite port=nbodiesd
    #pragma HLS INTERFACE m_axi port=startd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=childd offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=posMassd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=maxd offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=mind offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=radiusd offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=bottomd offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=stepd offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=blkcntd offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sminx complete dim=1
    #pragma HLS ARRAY_PARTITION variable=child complete dim=1
    #pragma HLS ARRAY_PARTITION variable=pos complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i, j, k;
            float val;
            float3 min, max;
            float sminx[THREADS1],
            smaxx[THREADS1],
            sminy[THREADS1],
            smaxy[THREADS1],
            sminz[THREADS1],
            smaxz[THREADS1];

            // initialize with valid data (in case #bodies < #threads)
            const float4 p0 = posMassd[0];
            min.x = max.x = p0.x;
            min.y = max.y = p0.y;
            min.z = max.z = p0.z;

            // scan all bodies
            i = _tid_x;
            int inc = THREADS1 * GRID_DIM_X;
            for (j = i + _bid_x * THREADS1; j < nbodiesd; j += inc) {
            const float4 p = posMassd[j];
            val = p.x;
            min.x = fminf(min.x, val);
            max.x = fmaxf(max.x, val);
            val = p.y;
            min.y = fminf(min.y, val);
            max.y = fmaxf(max.y, val);
            val = p.z;
            min.z = fminf(min.z, val);
            max.z = fmaxf(max.z, val);
            }

            // reduction in shared memory
            sminx[i] = min.x;
            smaxx[i] = max.x;
            sminy[i] = min.y;
            smaxy[i] = max.y;
            sminz[i] = min.z;
            smaxz[i] = max.z;

            for (j = THREADS1 / 2; j > 0; j /= 2) {
            if (i < j) {
            k = i + j;
            sminx[i] = min.x = fminf(min.x, sminx[k]);
            smaxx[i] = max.x = fmaxf(max.x, smaxx[k]);
            sminy[i] = min.y = fminf(min.y, sminy[k]);
            smaxy[i] = max.y = fmaxf(max.y, smaxy[k]);
            sminz[i] = min.z = fminf(min.z, sminz[k]);
            smaxz[i] = max.z = fmaxf(max.z, smaxz[k]);
            }
            }

            // write block result to global memory
            if (i == 0) {
            k = _bid_x;
            mind[k] = min;
            maxd[k] = max;
            __threadfence();

            inc = GRID_DIM_X - 1;
            if (inc == atomicInc(blkcntd, inc)) {
            // I'm the last block, so combine all block results
            for (j = 0; j <= inc; j++) {
            float3 minp = mind[j];
            float3 maxp = maxd[j];
            min.x = fminf(min.x, minp.x);
            max.x = fmaxf(max.x, maxp.x);
            min.y = fminf(min.y, minp.y);
            max.y = fmaxf(max.y, maxp.y);
            min.z = fminf(min.z, minp.z);
            max.z = fmaxf(max.z, maxp.z);
            }

            // compute radius
            val = fmaxf(max.x - min.x, max.y - min.y);
            *radiusd = fmaxf(val, max.z - min.z) * 0.5f;

            // create root node
            k = nnodesd;
            *bottomd = k;

            startd[k] = 0;
            float4 p;
            p.x = (min.x + max.x) * 0.5f;
            p.y = (min.y + max.y) * 0.5f;
            p.z = (min.z + max.z) * 0.5f;
            p.w = -1.0f;
            posMassd[k] = p;
            k *= 8;
            for (i = 0; i < 8; i++) childd[k + i] = -1;
            (*stepd)++;
            }
            }

        }
    }
}
extern "C"

void ClearKernel1(const int nnodesd, const int nbodiesd, int* const  childd)
{
    #pragma HLS INTERFACE s_axilite port=nnodesd
    #pragma HLS INTERFACE s_axilite port=nbodiesd
    #pragma HLS INTERFACE m_axi port=childd offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=child complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int top = 8 * nnodesd;
            int bottom = 8 * nbodiesd;
            int inc = BLOCK_DIM_X * GRID_DIM_X;
            int k = (bottom & (-WARPSIZE)) + _tid_x + _bid_x * BLOCK_DIM_X;  // align to warp size
            if (k < bottom) k += inc;

            // iterate over all cells assigned to thread
            while (k < top) {
            childd[k] = -1;
            k += inc;
            }

        }
    }
}
extern "C"

void TreeBuildingKernel(
    const int nnodesd,
    const int nbodiesd,
    volatile int* const  childd,
    const float4* const  posMassd,
    const float* const __restrict radiusd,
            int* const __restrict bottomd
)
{
    #pragma HLS INTERFACE s_axilite port=nnodesd
    #pragma HLS INTERFACE s_axilite port=nbodiesd
    #pragma HLS INTERFACE m_axi port=childd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=posMassd offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=radiusd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=bottomd offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=child complete dim=1
    #pragma HLS ARRAY_PARTITION variable=pos complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i, j, depth, skip, inc;
            float x, y, z, r;
            float dx, dy, dz;
            int ch, n, cell, locked, patch;
            float radius;

            // cache root data
            radius = *radiusd * 0.5f;
            const float4 root = posMassd[nnodesd];

            skip = 1;
            inc = BLOCK_DIM_X * GRID_DIM_X;
            i = _tid_x + _bid_x * BLOCK_DIM_X;

            // iterate over all bodies assigned to thread
            while (i < nbodiesd) {
            const float4 p = posMassd[i];
            if (skip != 0) {
            // new body, so start traversing at root
            skip = 0;
            n = nnodesd;
            depth = 1;
            r = radius;
            dx = dy = dz = -r;
            j = 0;
            // determine which child to follow
            if (root.x < p.x) {j = 1; dx = r;}
            if (root.y < p.y) {j |= 2; dy = r;}
            if (root.z < p.z) {j |= 4; dz = r;}
            x = root.x + dx;
            y = root.y + dy;
            z = root.z + dz;
            }

            // follow path to leaf cell
            ch = childd[n*8+j];
            while (ch >= nbodiesd) {
            n = ch;
            depth++;
            r *= 0.5f;
            dx = dy = dz = -r;
            j = 0;
            // determine which child to follow
            if (x < p.x) {j = 1; dx = r;}
            if (y < p.y) {j |= 2; dy = r;}
            if (z < p.z) {j |= 4; dz = r;}
            x += dx;
            y += dy;
            z += dz;
            ch = childd[n*8+j];
            }

            if (ch != -2) {  // skip if child pointer is locked and try again later
            locked = n*8+j;
            if (ch == -1) {
            if (ch == atomicCAS((int*)&childd[locked], ch, i)) {  // if null, just insert the new body
            i += inc;  // move on to next body
            skip = 1;
            }
            } else {  // there already is a body at this position
            if (ch == atomicCAS((int*)&childd[locked], ch, -2)) {  // try to lock
            patch = -1;
            const float4 chp = posMassd[ch];
            // create new cell(s) and insert the old and new bodies
            do {
            depth++;
            cell = atomicSub(bottomd, 1) - 1;

            if (patch != -1) {
            childd[n*8+j] = cell;
            }
            patch = max(patch, cell);

            j = 0;
            if (x < chp.x) j = 1;
            if (y < chp.y) j |= 2;
            if (z < chp.z) j |= 4;
            childd[cell*8+j] = ch;

            n = cell;
            r *= 0.5f;
            dx = dy = dz = -r;
            j = 0;
            if (x < p.x) {j = 1; dx = r;}
            if (y < p.y) {j |= 2; dy = r;}
            if (z < p.z) {j |= 4; dz = r;}
            x += dx;
            y += dy;
            z += dz;

            ch = childd[n*8+j];
            // repeat until the two bodies are different children
            } while (ch >= 0);
            childd[n*8+j] = i;

            i += inc;  // move on to next body
            skip = 2;
            }
            }
            }

            if (skip == 2) {
            childd[locked] = patch;
            }
            }

        }
    }
}
extern "C"

void ClearKernel2(
    const int nnodesd, 
    int* const  startd, 
    float4* const  posMassd,
    int* const  bottomd)
{
    #pragma HLS INTERFACE s_axilite port=nnodesd
    #pragma HLS INTERFACE m_axi port=startd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=posMassd offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bottomd offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=pos complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int k, inc, bottom;

            bottom = *bottomd;
            inc = BLOCK_DIM_X * GRID_DIM_X;
            k = (bottom & (-WARPSIZE)) + _tid_x + _bid_x * BLOCK_DIM_X;  // align to warp size
            if (k < bottom) k += inc;

            // iterate over all cells assigned to thread
            while (k < nnodesd) {
            posMassd[k].w = -1.0f;
            startd[k] = -1;
            k += inc;
            }

        }
    }
}
extern "C"

void SummarizationKernel(
    const int nnodesd, 
    const int nbodiesd,
    volatile int* const  countd,
    const int* const  childd,
    volatile float4* const  posMassd, // will cause hanging for 2048 bodies without volatile
    int* const __restrict bottomd)
{
    #pragma HLS INTERFACE s_axilite port=nnodesd
    #pragma HLS INTERFACE s_axilite port=nbodiesd
    #pragma HLS INTERFACE m_axi port=countd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=childd offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=posMassd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=bottomd offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=child complete dim=1
    #pragma HLS ARRAY_PARTITION variable=mass complete dim=1
    #pragma HLS ARRAY_PARTITION variable=pos complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int child[THREADS3 * 8];
            float mass[THREADS3 * 8];

            int i, j, ch, cnt;
            float cm, px, py, pz, m;
            int bottom = *bottomd;
            int inc = BLOCK_DIM_X * GRID_DIM_X;
            int k = (bottom & (-WARPSIZE)) + _tid_x + _bid_x * BLOCK_DIM_X;  // align to warp size
            if (k < bottom) k += inc;

            int restart = k;
            for (j = 0; j < 3; j++) {  // wait-free pre-passes
            // iterate over all cells assigned to thread
            while (k <= nnodesd) {
            if (posMassd[k].w < 0.0f) {
            for (i = 0; i < 8; i++) {
            ch = childd[k*8+i];
            child[i*THREADS3+_tid_x] = ch;  // cache children
            if ((ch >= nbodiesd) && ((mass[i*THREADS3+_tid_x] = posMassd[ch].w) < 0.0f)) {
            break;
            }
            }
            if (i == 8) {
            // all children are ready
            cm = 0.0f;
            px = 0.0f;
            py = 0.0f;
            pz = 0.0f;
            cnt = 0;
            for (i = 0; i < 8; i++) {
            ch = child[i*THREADS3+_tid_x];
            if (ch >= 0) {
            const float chx = posMassd[ch].x;
            const float chy = posMassd[ch].y;
            const float chz = posMassd[ch].z;
            const float chw = posMassd[ch].w;
            if (ch >= nbodiesd) {  // count bodies (needed later)
            m = mass[i*THREADS3+_tid_x];
            cnt += countd[ch];
            } else {
            m = chw;
            cnt++;
            }
            // add child's contribution
            cm += m;
            px += chx * m;
            py += chy * m;
            pz += chz * m;
            }
            }
            countd[k] = cnt;
            m = 1.0f / cm;
            posMassd[k].x = px * m;
            posMassd[k].y = py * m;
            posMassd[k].z = pz * m;
            posMassd[k].w = cm;
            }
            }
            k += inc;  // move on to next cell
            }
            k = restart;
            }

            j = 0;
            // iterate over all cells assigned to thread
            while (k <= nnodesd) {
            if (posMassd[k].w >= 0.0f) {
            k += inc;
            } else {
            if (j == 0) {
            j = 8;
            for (i = 0; i < 8; i++) {
            ch = childd[k*8+i];
            child[i*THREADS3+_tid_x] = ch;  // cache children
            if ((ch < nbodiesd) || ((mass[i*THREADS3+_tid_x] = posMassd[ch].w) >= 0.0f)) {
            j--;
            }
            }
            } else {
            j = 8;
            for (i = 0; i < 8; i++) {
            ch = child[i*THREADS3+_tid_x];
            if ((ch < nbodiesd) || (mass[i*THREADS3+_tid_x] >= 0.0f) || ((mass[i*THREADS3+_tid_x] = posMassd[ch].w) >= 0.0f)) {
            j--;
            }
            }
            }

            if (j == 0) {
            // all children are ready
            cm = 0.0f;
            px = 0.0f;
            py = 0.0f;
            pz = 0.0f;
            cnt = 0;
            for (i = 0; i < 8; i++) {
            ch = child[i*THREADS3+_tid_x];
            if (ch >= 0) {
            // four reads due to missing copy constructor for "volatile float4"
            const float chx = posMassd[ch].x;
            const float chy = posMassd[ch].y;
            const float chz = posMassd[ch].z;
            const float chw = posMassd[ch].w;
            if (ch >= nbodiesd) {  // count bodies (needed later)
            m = mass[i*THREADS3+_tid_x];
            cnt += countd[ch];
            } else {
            m = chw;
            cnt++;
            }
            // add child's contribution
            cm += m;
            px += chx * m;
            py += chy * m;
            pz += chz * m;
            }
            }
            countd[k] = cnt;
            m = 1.0f / cm;
            // four writes due to missing copy constructor for "volatile float4"
            posMassd[k].x = px * m;
            posMassd[k].y = py * m;
            posMassd[k].z = pz * m;
            posMassd[k].w = cm;
            k += inc;
            }
            }
            }

        }
    }
}
extern "C"

void SortKernel(
    const int nnodesd,
    const int nbodiesd, 
    int* const  sortd,
    const int* const  countd,
    volatile int* const  startd,
    int* const  childd,
    int* const  bottomd)
{
    #pragma HLS INTERFACE s_axilite port=nnodesd
    #pragma HLS INTERFACE s_axilite port=nbodiesd
    #pragma HLS INTERFACE m_axi port=sortd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=countd offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=startd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=childd offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=bottomd offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=child complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i, j;
            int bottom = *bottomd;
            int dec = BLOCK_DIM_X * GRID_DIM_X;
            int k = nnodesd + 1 - dec + _tid_x + _bid_x * BLOCK_DIM_X;

            // iterate over all cells assigned to thread
            while (k >= bottom) {
            int start = startd[k];
            if (start >= 0) {
            j = 0;
            for (i = 0; i < 8; i++) {
            int ch = childd[k*8+i];
            if (ch >= 0) {
            if (i != j) {
            // move children to front (needed later for speed)
            childd[k*8+i] = -1;
            childd[k*8+j] = ch;
            }
            j++;
            if (ch >= nbodiesd) {
            // child is a cell
            startd[ch] = start;  // set start ID of child
            start += countd[ch];  // add #bodies in subtree
            } else {
            // child is a body
            sortd[start] = ch;  // record body in 'sorted' array
            start++;
            }
            }
            }
            k -= dec;  // move on to next cell
            }
            }

        }
    }
}
extern "C"

void ForceCalculationKernel(
    const int nnodesd, 
    const int nbodiesd,
    const float dthfd,
    const float itolsqd,
    const float epssqd,
    const int* const  sortd,
    const int* const  childd,
    const float4* const  posMassd,
    float2* const  veld,
    float4* const  accVeld,
    const float* const  radiusd,
    const int* const  stepd)
{
    #pragma HLS INTERFACE s_axilite port=nnodesd
    #pragma HLS INTERFACE s_axilite port=nbodiesd
    #pragma HLS INTERFACE s_axilite port=dthfd
    #pragma HLS INTERFACE s_axilite port=itolsqd
    #pragma HLS INTERFACE s_axilite port=epssqd
    #pragma HLS INTERFACE m_axi port=sortd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=childd offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=posMassd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=veld offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=accVeld offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=radiusd offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=stepd offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=child complete dim=1
    #pragma HLS ARRAY_PARTITION variable=pos complete dim=1
    #pragma HLS ARRAY_PARTITION variable=dq complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i, j, k, n, depth, base, sbase, diff, pd, nd;
            float ax, ay, az, dx, dy, dz, tmp;
            int pos[THREADS5], node[THREADS5];
            float dq[THREADS5];

            if (0 == _tid_x) {
            tmp = *radiusd * 2;
            // precompute values that depend only on tree level
            dq[0] = tmp * tmp * itolsqd;
            for (i = 1; i < MAXDEPTH; i++) {
            dq[i] = dq[i - 1] * 0.25f;
            dq[i - 1] += epssqd;
            }
            dq[i - 1] += epssqd;
            }

            // figure out first thread in each warp (lane 0)
            base = _tid_x / WARPSIZE;
            sbase = base * WARPSIZE;
            j = base * MAXDEPTH;

            diff = _tid_x - sbase;
            // make multiple copies to avoid index calculations later
            if (diff < MAXDEPTH) {
            dq[diff+j] = dq[diff];
            }

            // iterate over all bodies assigned to thread
            for (k = _tid_x + _bid_x * BLOCK_DIM_X; k < nbodiesd; k += BLOCK_DIM_X * GRID_DIM_X) {
            i = sortd[k];  // get permuted/sorted index
            // cache position info
            const float4 pi = posMassd[i];

            ax = 0.0f;
            ay = 0.0f;
            az = 0.0f;

            // initialize iteration stack, i.e., push root node onto stack
            depth = j;
            if (sbase == _tid_x) {
            pos[j] = 0;
            node[j] = nnodesd * 8;
            }

            do {
            // stack is not empty
            pd = pos[depth];
            nd = node[depth];
            while (pd < 8) {
            // node on top of stack has more children to process
            n = childd[nd + pd];  // load child pointer
            pd++;

            if (n >= 0) {
            const float4 pn = posMassd[n];
            dx = pn.x - pi.x;
            dy = pn.y - pi.y;
            dz = pn.z - pi.z;
            tmp = dx*dx + (dy*dy + (dz*dz + epssqd));  // compute distance squared (plus softening)
            if ((n < nbodiesd) || __all_sync(0xffffffff, tmp >= dq[depth])) {
            // check if all threads agree that cell is far enough away (or is a body)
            tmp = rsqrtf(tmp);  // compute distance
            tmp = pn.w * tmp * tmp * tmp;
            ax += dx * tmp;
            ay += dy * tmp;
            az += dz * tmp;
            } else {
            // push cell onto stack
            if (sbase == _tid_x) {
            pos[depth] = pd;
            node[depth] = nd;
            }
            depth++;
            pd = 0;
            nd = n * 8;
            }
            } else {
            pd = 8;  // early out because all remaining children are also zero
            }
            }
            depth--;  // done with this level
            } while (depth >= j);

            float4 acc = accVeld[i];
            if (*stepd > 0) {
            // update velocity
            float2 v = veld[i];
            v.x += (ax - acc.x) * dthfd;
            v.y += (ay - acc.y) * dthfd;
            acc.w += (az - acc.z) * dthfd;
            veld[i] = v;
            }

            // save computed acceleration
            acc.x = ax;
            acc.y = ay;
            acc.z = az;
            accVeld[i] = acc;
            }

        }
    }
}
extern "C"

void IntegrationKernel(
     const int nbodiesd,
     const float dtimed,
     const float dthfd,
     float4* const  posMass,
     float2* const  veld,
     float4* const  accVeld)
{
    #pragma HLS INTERFACE s_axilite port=nbodiesd
    #pragma HLS INTERFACE s_axilite port=dtimed
    #pragma HLS INTERFACE s_axilite port=dthfd
    #pragma HLS INTERFACE m_axi port=posMass offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=veld offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=accVeld offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=pos complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i, inc;
            float dvelx, dvely, dvelz;
            float velhx, velhy, velhz;

            // iterate over all bodies assigned to thread
            inc = BLOCK_DIM_X * GRID_DIM_X;
            for (i = _tid_x + _bid_x * BLOCK_DIM_X; i < nbodiesd; i += inc) {
            // integrate
            float4 acc = accVeld[i];
            dvelx = acc.x * dthfd;
            dvely = acc.y * dthfd;
            dvelz = acc.z * dthfd;

            float2 v = veld[i];
            velhx = v.x + dvelx;
            velhy = v.y + dvely;
            velhz = acc.w + dvelz;

            float4 p = posMass[i];
            p.x += velhx * dtimed;
            p.y += velhy * dtimed;
            p.z += velhz * dtimed;
            posMass[i] = p;

            v.x = velhx + dvelx;
            v.y = velhy + dvely;
            acc.w = velhz + dvelz;
            veld[i] = v;
            accVeld[i] = acc;
            }

        }
    }
}
extern "C"

void InitializationKernel(int *step, unsigned int *blkcnt)
{
    #pragma HLS INTERFACE m_axi port=step offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=blkcnt offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            *step = -1;
            *blkcnt = 0;

        }
    }
}
