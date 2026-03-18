#include "kernel.h"

// --- from kernel.cu ---
DOUBLE get_time_eikonal(DOUBLE a, DOUBLE b, DOUBLE c, DOUBLE s)
{
  DOUBLE ret, tmp;

  // a > b > c
  if(a < b) { tmp = a; a = b; b = tmp; }
  if(b < c) { tmp = b; b = c; c = tmp; }
  if(a < b) { tmp = a; a = b; b = tmp; }

  ret = INF;

  if(c < INF)
  {
    ret = c + s;

    if(ret > b) 
    {  
      tmp = ((b+c) + sqrtf(2.0f*s*s-(b-c)*(b-c)))*0.5f;

      if(tmp > b) ret = tmp; 

      if(ret > a)  {      
        tmp = (a+b+c)/3.0f + sqrtf(2.0f*(a*(b-a)+b*(c-b)+c*(a-c))+3.0f*s*s)/3.0f; 

        if(tmp > a) ret = tmp;
      }
    }
  }

  return ret;
}
extern "C"

void run_solver(
  const double* spd,
  const bool* mask,
  const DOUBLE * sol_in,
  DOUBLE * sol_out,
  bool * con,
  const uint* list,
  int xdim, int ydim, int zdim,
  int nIter, uint nActiveBlock)
{
    #pragma HLS INTERFACE m_axi port=spd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=mask offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=sol_in offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=sol_out offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=con offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=list offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=xdim
    #pragma HLS INTERFACE s_axilite port=ydim
    #pragma HLS INTERFACE s_axilite port=zdim
    #pragma HLS INTERFACE s_axilite port=nIter
    #pragma HLS INTERFACE s_axilite port=nActiveBlock
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=_sol complete dim=1
    #pragma HLS ARRAY_PARTITION variable=_sol complete dim=1

    #pragma HLS INTERFACE m_axi port=spd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=mask offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=sol_in offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=sol_out offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=con offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=list offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=xdim
    #pragma HLS INTERFACE s_axilite port=ydim
    #pragma HLS INTERFACE s_axilite port=zdim
    #pragma HLS INTERFACE s_axilite port=nIter
    #pragma HLS INTERFACE s_axilite port=nActiveBlock
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=_sol complete dim=1
    #pragma HLS ARRAY_PARTITION variable=_sol complete dim=1

    for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        uint list_idx = _bid_y*GRID_DIM_X + _bid_x;

                        if(list_idx < nActiveBlock)
                        {
                        // retrieve actual block index from the active list
                        uint block_idx = list[list_idx];

                        double F;
                        bool isValid;
                        uint blocksize = BLOCK_LENGTH*BLOCK_LENGTH*BLOCK_LENGTH;
                        uint base_addr = block_idx*blocksize;

                        uint xgridlength = xdim/BLOCK_LENGTH;
                        uint ygridlength = ydim/BLOCK_LENGTH;
                        uint zgridlength = zdim/BLOCK_LENGTH;

                        // compute block index
                        uint bx = block_idx%xgridlength;
                        uint tmpIdx = (block_idx - bx)/xgridlength;
                        uint by = tmpIdx%ygridlength;
                        uint bz = (tmpIdx-by)/ygridlength;

                        uint tx = _tid_x;
                        uint ty = _tid_y;
                        uint tz = _tid_z;
                        uint tIdx = tz*BLOCK_LENGTH*BLOCK_LENGTH + ty*BLOCK_LENGTH + tx;

                        DOUBLE _sol[BLOCK_LENGTH+2][BLOCK_LENGTH+2][BLOCK_LENGTH+2];

                        // copy global to shared memory
                        dim3 idx(tx+1,ty+1,tz+1);

                        SOL(idx.x,idx.y,idx.z) = sol_in[base_addr + tIdx];
                        F = spd[base_addr + tIdx];
                        if(F > 0) F = 1.0/F; // F = 1/f
                        isValid = mask[base_addr + tIdx];

                        uint new_base_addr, new_tIdx;

                        // 1-neighborhood values
                        if(tx == 0)
                        {
                        if(bx == 0) // end of the grid
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx + BLOCK_LENGTH-1;
                        new_base_addr = (block_idx - 1)*blocksize;
                        }

                        SOL(tx,idx.y,idx.z) = sol_in[new_base_addr + new_tIdx];
                        }

                        if(tx == BLOCK_LENGTH-1)
                        {
                        if(bx == xgridlength-1) // end of the grid
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx - (BLOCK_LENGTH-1);
                        new_base_addr = (block_idx + 1)*blocksize;
                        }
                        SOL(tx+2,idx.y,idx.z) = sol_in[new_base_addr + new_tIdx];
                        }

                        if(ty == 0)
                        {
                        if(by == 0)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx + (BLOCK_LENGTH-1)*BLOCK_LENGTH;
                        new_base_addr = (block_idx - xgridlength)*blocksize;
                        }

                        SOL(idx.x,ty,idx.z) = sol_in[new_base_addr + new_tIdx];
                        }

                        if(ty == BLOCK_LENGTH-1)
                        {
                        if(by == ygridlength-1)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx - (BLOCK_LENGTH-1)*BLOCK_LENGTH;
                        new_base_addr = (block_idx + xgridlength)*blocksize;
                        }

                        SOL(idx.x,ty+2,idx.z) = sol_in[new_base_addr + new_tIdx];
                        }

                        if(tz == 0)
                        {
                        if(bz == 0)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx + (BLOCK_LENGTH-1)*BLOCK_LENGTH*BLOCK_LENGTH;
                        new_base_addr = (block_idx - xgridlength*ygridlength)*blocksize;
                        }

                        SOL(idx.x,idx.y,tz) = sol_in[new_base_addr + new_tIdx];
                        }

                        if(tz == BLOCK_LENGTH-1)
                        {
                        if(bz == zgridlength-1)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx - (BLOCK_LENGTH-1)*BLOCK_LENGTH*BLOCK_LENGTH;
                        new_base_addr = (block_idx + xgridlength*ygridlength)*blocksize;
                        }

                        SOL(idx.x,idx.y,tz+2) = sol_in[new_base_addr + new_tIdx];
                        }

                        DOUBLE a,b,c,oldT,newT;

                        for(int iter=0; iter<nIter; iter++)
                        {
                        //
                        // compute new value
                        //
                        oldT = newT = SOL(idx.x,idx.y,idx.z);

                        if(isValid)
                        {
                        a = min(SOL(tx,idx.y,idx.z),SOL(tx+2,idx.y,idx.z));
                        b = min(SOL(idx.x,ty,idx.z),SOL(idx.x,ty+2,idx.z));
                        c = min(SOL(idx.x,idx.y,tz),SOL(idx.x,idx.y,tz+2));

                        DOUBLE tmp = (DOUBLE) get_time_eikonal(a, b, c, F);

                        newT = min(tmp,oldT);
                        }

                        if(isValid) SOL(idx.x,idx.y,idx.z) = newT;
                        }

                        DOUBLE residue = oldT - newT;

                        // write back to global memory
                        con[base_addr + tIdx] = (residue < EPS) ? true : false;
                        sol_out[base_addr + tIdx] = newT;
                        }

                    }
                }
            }
        }
    }
                   if(by == ygridlength-1)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx - (BLOCK_LENGTH-1)*BLOCK_LENGTH;
                        new_base_addr = (block_idx + xgridlength)*blocksize;
                        }

                        SOL(idx.x,ty+2,idx.z) = sol_in[new_base_addr + new_tIdx];
                        }

                        if(tz == 0)
                        {
                        if(bz == 0)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx + (BLOCK_LENGTH-1)*BLOCK_LENGTH*BLOCK_LENGTH;
                        new_base_addr = (block_idx - xgridlength*ygridlength)*blocksize;
                        }

                        SOL(idx.x,idx.y,tz) = sol_in[new_base_addr + new_tIdx];
                        }

                        if(tz == BLOCK_LENGTH-1)
                        {
                        if(bz == zgridlength-1)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx - (BLOCK_LENGTH-1)*BLOCK_LENGTH*BLOCK_LENGTH;
                        new_base_addr = (block_idx + xgridlength*ygridlength)*blocksize;
                        }

                        SOL(idx.x,idx.y,tz+2) = sol_in[new_base_addr + new_tIdx];
                        }

                        DOUBLE a,b,c,oldT,newT;

                        for(int iter=0; iter<nIter; iter++)
                        {
                        //
                        // compute new value
                        //
                        oldT = newT = SOL(idx.x,idx.y,idx.z);

                        if(isValid)
                        {
                        a = min(SOL(tx,idx.y,idx.z),SOL(tx+2,idx.y,idx.z));
                        b = min(SOL(idx.x,ty,idx.z),SOL(idx.x,ty+2,idx.z));
                        c = min(SOL(idx.x,idx.y,tz),SOL(idx.x,idx.y,tz+2));

                        DOUBLE tmp = (DOUBLE) get_time_eikonal(a, b, c, F);

                        newT = min(tmp,oldT);
                        }

                        if(isValid) SOL(idx.x,idx.y,idx.z) = newT;
                        }

                        DOUBLE residue = oldT - newT;

                        // write back to global memory
                        con[base_addr + tIdx] = (residue < EPS) ? true : false;
                        sol_out[base_addr + tIdx] = newT;
                        }

                    }
                }
            }
        }
    }
}
extern "C"

void run_reduction(
  const bool * con,
  bool * listVol,
  const uint * list,
  uint nActiveBlock)
{
    #pragma HLS INTERFACE m_axi port=con offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=listVol offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=list offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=nActiveBlock
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=conv complete dim=1

    #pragma HLS INTERFACE m_axi port=con offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=listVol offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=list offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=nActiveBlock
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=conv complete dim=1

    for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        uint list_idx = _bid_y*GRID_DIM_X + _bid_x;

                        if(list_idx < nActiveBlock)
                        {
                        uint block_idx = list[list_idx];

                        bool conv[BLOCK_LENGTH*BLOCK_LENGTH*BLOCK_LENGTH];

                        uint blocksize = BLOCK_LENGTH*BLOCK_LENGTH*BLOCK_LENGTH/2;
                        uint base_addr = block_idx*blocksize*2;
                        uint tx = _tid_x;
                        uint ty = _tid_y;
                        uint tz = _tid_z;
                        uint tIdx = tz*BLOCK_LENGTH*BLOCK_LENGTH + ty*BLOCK_LENGTH + tx;

                        conv[tIdx] = con[base_addr + tIdx];
                        conv[tIdx + blocksize] = con[base_addr + tIdx + blocksize];

                        for(uint i=blocksize; i>0; i/=2)
                        {
                        if(tIdx < i)
                        {
                        bool b1, b2;
                        b1 = conv[tIdx];
                        b2 = conv[tIdx+i];
                        conv[tIdx] = (b1 && b2) ? true : false ;
                        }
                        }

                        if(tIdx == 0)
                        {
                        listVol[block_idx] = !conv[0]; // active list is negation of tile convergence (active = not converged)
                        }
                        }

                    }
                }
            }
        }
    }
 ty = _tid_y;
                        uint tz = _tid_z;
                        uint tIdx = tz*BLOCK_LENGTH*BLOCK_LENGTH + ty*BLOCK_LENGTH + tx;

                        conv[tIdx] = con[base_addr + tIdx];
                        conv[tIdx + blocksize] = con[base_addr + tIdx + blocksize];

                        for(uint i=blocksize; i>0; i/=2)
                        {
                        if(tIdx < i)
                        {
                        bool b1, b2;
                        b1 = conv[tIdx];
                        b2 = conv[tIdx+i];
                        conv[tIdx] = (b1 && b2) ? true : false ;
                        }
                        }

                        if(tIdx == 0)
                        {
                        listVol[block_idx] = !conv[0]; // active list is negation of tile convergence (active = not converged)
                        }
                        }

                    }
                }
            }
        }
    }
}
extern "C"

void run_check_neighbor(
  const double* spd,
  const bool* mask,
  const DOUBLE * sol_in,
  DOUBLE * sol_out,
  bool * con,
  const uint* list,
  int xdim, int ydim, int zdim,
  uint nActiveBlock, uint nTotalBlock)
{
    #pragma HLS INTERFACE m_axi port=spd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=mask offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=sol_in offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=sol_out offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=con offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=list offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=xdim
    #pragma HLS INTERFACE s_axilite port=ydim
    #pragma HLS INTERFACE s_axilite port=zdim
    #pragma HLS INTERFACE s_axilite port=nActiveBlock
    #pragma HLS INTERFACE s_axilite port=nTotalBlock
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=_sol complete dim=1
    #pragma HLS ARRAY_PARTITION variable=_sol complete dim=1

    #pragma HLS INTERFACE m_axi port=spd offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=mask offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=sol_in offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=sol_out offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=con offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=list offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=xdim
    #pragma HLS INTERFACE s_axilite port=ydim
    #pragma HLS INTERFACE s_axilite port=zdim
    #pragma HLS INTERFACE s_axilite port=nActiveBlock
    #pragma HLS INTERFACE s_axilite port=nTotalBlock
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=_sol complete dim=1
    #pragma HLS ARRAY_PARTITION variable=_sol complete dim=1

    for (int _tid_z = 0; _tid_z < BLOCK_DIM_Z; _tid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1


                        uint list_idx = _bid_y*GRID_DIM_X + _bid_x;

                        if(list_idx < nTotalBlock)
                        {
                        double F;
                        bool isValid;
                        DOUBLE _sol[BLOCK_LENGTH+2][BLOCK_LENGTH+2][BLOCK_LENGTH+2];

                        uint block_idx = list[list_idx];
                        uint blocksize = BLOCK_LENGTH*BLOCK_LENGTH*BLOCK_LENGTH;
                        uint base_addr = block_idx*blocksize;

                        uint tx = _tid_x;
                        uint ty = _tid_y;
                        uint tz = _tid_z;
                        uint tIdx = tz*BLOCK_LENGTH*BLOCK_LENGTH + ty*BLOCK_LENGTH + tx;

                        if(list_idx < nActiveBlock) // copy value
                        {
                        sol_out[base_addr + tIdx] = sol_in[base_addr + tIdx];
                        }
                        else
                        {
                        uint xgridlength = xdim/BLOCK_LENGTH;
                        uint ygridlength = ydim/BLOCK_LENGTH;
                        uint zgridlength = zdim/BLOCK_LENGTH;

                        // compute block index
                        uint bx = block_idx%xgridlength;
                        uint tmpIdx = (block_idx - bx)/xgridlength;
                        uint by = tmpIdx%ygridlength;
                        uint bz = (tmpIdx-by)/ygridlength;

                        // copy global to shared memory
                        dim3 idx(tx+1,ty+1,tz+1);
                        _sol[idx.x][idx.y][idx.z] = sol_in[base_addr + tIdx];
                        F = spd[base_addr + tIdx];
                        if(F > 0) F = 1.0/F;
                        isValid = mask[base_addr + tIdx];

                        uint new_base_addr, new_tIdx;

                        // 1-neighborhood values
                        if(tx == 0)
                        {
                        if(bx == 0) // end of the grid
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx + BLOCK_LENGTH-1;
                        new_base_addr = (block_idx - 1)*blocksize;
                        }
                        _sol[tx][idx.y][idx.z] = sol_in[new_base_addr + new_tIdx];
                        }

                        if(tx == BLOCK_LENGTH-1)
                        {
                        if(bx == xgridlength-1) // end of the grid
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx - (BLOCK_LENGTH-1);
                        new_base_addr = (block_idx + 1)*blocksize;
                        }
                        _sol[tx+2][idx.y][idx.z] = sol_in[new_base_addr + new_tIdx];
                        }

                        if(ty == 0)
                        {
                        if(by == 0)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx + (BLOCK_LENGTH-1)*BLOCK_LENGTH;
                        new_base_addr = (block_idx - xgridlength)*blocksize;
                        }
                        _sol[idx.x][ty][idx.z] = sol_in[new_base_addr + new_tIdx];
                        }

                        if(ty == BLOCK_LENGTH-1)
                        {
                        if(by == ygridlength-1)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx - (BLOCK_LENGTH-1)*BLOCK_LENGTH;
                        new_base_addr = (block_idx + xgridlength)*blocksize;
                        }
                        _sol[idx.x][ty+2][idx.z] = sol_in[new_base_addr + new_tIdx];
                        }

                        if(tz == 0)
                        {
                        if(bz == 0)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx + (BLOCK_LENGTH-1)*BLOCK_LENGTH*BLOCK_LENGTH;
                        new_base_addr = (block_idx - xgridlength*ygridlength)*blocksize;
                        }
                        _sol[idx.x][idx.y][tz] = sol_in[new_base_addr + new_tIdx];
                        }

                        if(tz == BLOCK_LENGTH-1)
                        {
                        if(bz == zgridlength-1)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx - (BLOCK_LENGTH-1)*BLOCK_LENGTH*BLOCK_LENGTH;
                        new_base_addr = (block_idx + xgridlength*ygridlength)*blocksize;
                        }
                        _sol[idx.x][idx.y][tz+2] = sol_in[new_base_addr + new_tIdx];
                        }

                        DOUBLE a,b,c,oldT,newT;

                        //
                        // compute new value
                        //
                        oldT = newT = _sol[idx.x][idx.y][idx.z];

                        if(isValid)
                        {
                        a = min(_sol[tx][idx.y][idx.z],_sol[tx+2][idx.y][idx.z]);
                        b = min(_sol[idx.x][ty][idx.z],_sol[idx.x][ty+2][idx.z]);
                        c = min(_sol[idx.x][idx.y][tz],_sol[idx.x][idx.y][tz+2]);

                        DOUBLE tmp = (DOUBLE) get_time_eikonal(a, b, c, F);
                        newT = min(tmp,oldT);

                        sol_out[base_addr + tIdx] = newT;
                        }
                        // write back to global memory
                        DOUBLE residue = oldT - newT;
                        con[base_addr + tIdx] = (residue < EPS) ? true : false;
                        }
                        }

                    }
                }
            }
        }
    }
1)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx - (BLOCK_LENGTH-1)*BLOCK_LENGTH;
                        new_base_addr = (block_idx + xgridlength)*blocksize;
                        }
                        _sol[idx.x][ty+2][idx.z] = sol_in[new_base_addr + new_tIdx];
                        }

                        if(tz == 0)
                        {
                        if(bz == 0)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx + (BLOCK_LENGTH-1)*BLOCK_LENGTH*BLOCK_LENGTH;
                        new_base_addr = (block_idx - xgridlength*ygridlength)*blocksize;
                        }
                        _sol[idx.x][idx.y][tz] = sol_in[new_base_addr + new_tIdx];
                        }

                        if(tz == BLOCK_LENGTH-1)
                        {
                        if(bz == zgridlength-1)
                        {
                        new_tIdx = tIdx;
                        new_base_addr = base_addr;
                        }
                        else
                        {
                        new_tIdx = tIdx - (BLOCK_LENGTH-1)*BLOCK_LENGTH*BLOCK_LENGTH;
                        new_base_addr = (block_idx + xgridlength*ygridlength)*blocksize;
                        }
                        _sol[idx.x][idx.y][tz+2] = sol_in[new_base_addr + new_tIdx];
                        }

                        DOUBLE a,b,c,oldT,newT;

                        //
                        // compute new value
                        //
                        oldT = newT = _sol[idx.x][idx.y][idx.z];

                        if(isValid)
                        {
                        a = min(_sol[tx][idx.y][idx.z],_sol[tx+2][idx.y][idx.z]);
                        b = min(_sol[idx.x][ty][idx.z],_sol[idx.x][ty+2][idx.z]);
                        c = min(_sol[idx.x][idx.y][tz],_sol[idx.x][idx.y][tz+2]);

                        DOUBLE tmp = (DOUBLE) get_time_eikonal(a, b, c, F);
                        newT = min(tmp,oldT);

                        sol_out[base_addr + tIdx] = newT;
                        }
                        // write back to global memory
                        DOUBLE residue = oldT - newT;
                        con[base_addr + tIdx] = (residue < EPS) ? true : false;
                        }
                        }

                    }
                }
            }
        }
    }
}
