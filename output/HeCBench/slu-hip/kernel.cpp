#include "kernel.h"

// --- from numeric.cu ---
extern "C"
void RL(
    const unsigned*  sym_c_ptr_dev,
    const unsigned*  sym_r_idx_dev,
    REAL*  val_dev,
    const unsigned*  l_col_ptr_dev,
    const unsigned*  csr_r_ptr_dev,
    const unsigned*  csr_c_idx_dev,
    const unsigned*  csr_diag_ptr_dev,
    const int*  level_idx_dev,
    REAL*  tmpMem,
    const unsigned n,
    const int levelHead,
    const int inLevPos)
{
    #pragma HLS INTERFACE m_axi port=sym_c_ptr_dev offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=sym_r_idx_dev offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=val_dev offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=l_col_ptr_dev offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=csr_r_ptr_dev offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=csr_c_idx_dev offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=csr_diag_ptr_dev offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=level_idx_dev offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=tmpMem offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=levelHead
    #pragma HLS INTERFACE s_axilite port=inLevPos
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int tid = _tid_x;
            const int bid = _bid_x;
            const int wid = _tid_x / 32;

            const unsigned currentCol = level_idx_dev[levelHead+inLevPos+bid];
            const unsigned currentLColSize = sym_c_ptr_dev[currentCol + 1] - l_col_ptr_dev[currentCol] - 1;
            const unsigned currentLPos = l_col_ptr_dev[currentCol] + tid + 1;

            REAL s[4096];

            //update current col

            int offset = 0;
            while (currentLColSize > offset)
            {
            if (tid + offset < currentLColSize)
            {
            unsigned ridx = sym_r_idx_dev[currentLPos + offset];

            val_dev[currentLPos + offset] /= val_dev[l_col_ptr_dev[currentCol]];
            tmpMem[bid*n + ridx]= val_dev[currentLPos + offset];
            }
            offset += BLOCK_DIM_X;
            }

            //broadcast to submatrix
            const unsigned subColPos = csr_diag_ptr_dev[currentCol] + wid + 1;
            const unsigned subMatSize = csr_r_ptr_dev[currentCol + 1] - csr_diag_ptr_dev[currentCol] - 1;
            unsigned subCol;
            const int tidInWarp = _tid_x % 32;
            unsigned subColElem = 0;

            int woffset = 0;
            while (subMatSize > woffset)
            {
            if (wid + woffset < subMatSize)
            {
            offset = 0;
            subCol = csr_c_idx_dev[subColPos + woffset];
            while(offset < sym_c_ptr_dev[subCol + 1] - sym_c_ptr_dev[subCol])
            {
            if (tidInWarp + offset < sym_c_ptr_dev[subCol + 1] - sym_c_ptr_dev[subCol])
            {

            subColElem = sym_c_ptr_dev[subCol] + tidInWarp + offset;
            unsigned ridx = sym_r_idx_dev[subColElem];

            if (ridx == currentCol)
            {
            s[wid] = val_dev[subColElem];
            }
            //Threads in a warp are always synchronized
            //
            if (ridx > currentCol)
            {
            //elem in currentCol same row with subColElem might be 0, so
            //clearing tmpMem is necessary
            (val_dev[subColElem] += -tmpMem[ridx+n*bid]*s[wid]);
            }
            }
            offset += 32;
            }
            }
            woffset += BLOCK_DIM_X/32;
            }
            //Clear tmpMem
            offset = 0;
            while (currentLColSize > offset)
            {
            if (tid + offset < currentLColSize)
            {
            unsigned ridx = sym_r_idx_dev[currentLPos + offset];
            tmpMem[bid*n + ridx]= 0;
            }
            offset += BLOCK_DIM_X;
            }

        }
    }
}
extern "C"

void RL_perturb(
    const unsigned*  sym_c_ptr_dev,
    const unsigned*  sym_r_idx_dev,
    REAL*  val_dev,
    const unsigned*  l_col_ptr_dev,
    const unsigned*  csr_r_ptr_dev,
    const unsigned*  csr_c_idx_dev,
    const unsigned*  csr_diag_ptr_dev,
    const int*  level_idx_dev,
    REAL*  tmpMem,
    const unsigned n,
    const int levelHead,
    const int inLevPos,
    const float pert)
{
    #pragma HLS INTERFACE m_axi port=sym_c_ptr_dev offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=sym_r_idx_dev offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=val_dev offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=l_col_ptr_dev offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=csr_r_ptr_dev offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=csr_c_idx_dev offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=csr_diag_ptr_dev offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=level_idx_dev offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=tmpMem offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=levelHead
    #pragma HLS INTERFACE s_axilite port=inLevPos
    #pragma HLS INTERFACE s_axilite port=pert
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int tid = _tid_x;
            const int bid = _bid_x;
            const int wid = _tid_x / 32;

            const unsigned currentCol = level_idx_dev[levelHead+inLevPos+bid];
            const unsigned currentLColSize = sym_c_ptr_dev[currentCol + 1] - l_col_ptr_dev[currentCol] - 1;
            const unsigned currentLPos = l_col_ptr_dev[currentCol] + tid + 1;

            REAL s[4096];

            //update current col

            int offset = 0;
            while (currentLColSize > offset)
            {
            if (tid + offset < currentLColSize)
            {
            unsigned ridx = sym_r_idx_dev[currentLPos + offset];

            if (abs(val_dev[l_col_ptr_dev[currentCol]]) < pert)
            val_dev[l_col_ptr_dev[currentCol]] = pert;

            val_dev[currentLPos + offset] /= val_dev[l_col_ptr_dev[currentCol]];
            tmpMem[bid*n + ridx]= val_dev[currentLPos + offset];
            }
            offset += BLOCK_DIM_X;
            }

            //broadcast to submatrix
            const unsigned subColPos = csr_diag_ptr_dev[currentCol] + wid + 1;
            const unsigned subMatSize = csr_r_ptr_dev[currentCol + 1] - csr_diag_ptr_dev[currentCol] - 1;
            unsigned subCol;
            const int tidInWarp = _tid_x % 32;
            unsigned subColElem = 0;

            int woffset = 0;
            while (subMatSize > woffset)
            {
            if (wid + woffset < subMatSize)
            {
            offset = 0;
            subCol = csr_c_idx_dev[subColPos + woffset];
            while(offset < sym_c_ptr_dev[subCol + 1] - sym_c_ptr_dev[subCol])
            {
            if (tidInWarp + offset < sym_c_ptr_dev[subCol + 1] - sym_c_ptr_dev[subCol])
            {

            subColElem = sym_c_ptr_dev[subCol] + tidInWarp + offset;
            unsigned ridx = sym_r_idx_dev[subColElem];

            if (ridx == currentCol)
            {
            s[wid] = val_dev[subColElem];
            }
            //Threads in a warp are always synchronized
            //
            if (ridx > currentCol)
            {
            //elem in currentCol same row with subColElem might be 0, so
            //clearing tmpMem is necessary
            (val_dev[subColElem] += -tmpMem[ridx+n*bid]*s[wid]);
            }
            }
            offset += 32;
            }
            }
            woffset += BLOCK_DIM_X/32;
            }
            //Clear tmpMem
            offset = 0;
            while (currentLColSize > offset)
            {
            if (tid + offset < currentLColSize)
            {
            unsigned ridx = sym_r_idx_dev[currentLPos + offset];
            tmpMem[bid*n + ridx]= 0;
            }
            offset += BLOCK_DIM_X;
            }

        }
    }
}
extern "C"

void RL_onecol_factorizeCurrentCol(
    const unsigned*  sym_c_ptr_dev,
    const unsigned*  sym_r_idx_dev,
    REAL*  val_dev,
    const unsigned*  l_col_ptr_dev,
    const unsigned currentCol,
    REAL*  tmpMem,
    const int stream,
    const unsigned n)
{
    #pragma HLS INTERFACE m_axi port=sym_c_ptr_dev offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=sym_r_idx_dev offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=val_dev offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=l_col_ptr_dev offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=currentCol
    #pragma HLS INTERFACE m_axi port=tmpMem offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=stream
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int tid = _tid_x;

            const unsigned currentLColSize = sym_c_ptr_dev[currentCol + 1] - l_col_ptr_dev[currentCol] - 1;
            const unsigned currentLPos = l_col_ptr_dev[currentCol] + tid + 1;

            //update current col

            int offset = 0;
            while (currentLColSize > offset)
            {
            if (tid + offset < currentLColSize)
            {
            unsigned ridx = sym_r_idx_dev[currentLPos + offset];

            val_dev[currentLPos + offset] /= val_dev[l_col_ptr_dev[currentCol]];
            tmpMem[stream * n + ridx]= val_dev[currentLPos + offset];
            }
            offset += BLOCK_DIM_X;
            }

        }
    }
}
extern "C"

void RL_onecol_factorizeCurrentCol_perturb(
    const unsigned*  sym_c_ptr_dev,
    const unsigned*  sym_r_idx_dev,
    REAL*  val_dev,
    const unsigned*  l_col_ptr_dev,
    const unsigned currentCol,
    REAL*  tmpMem,
    const int stream,
    const unsigned n,
    const float pert)
{
    #pragma HLS INTERFACE m_axi port=sym_c_ptr_dev offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=sym_r_idx_dev offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=val_dev offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=l_col_ptr_dev offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=currentCol
    #pragma HLS INTERFACE m_axi port=tmpMem offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=stream
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=pert
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int tid = _tid_x;

            const unsigned currentLColSize = sym_c_ptr_dev[currentCol + 1] - l_col_ptr_dev[currentCol] - 1;
            const unsigned currentLPos = l_col_ptr_dev[currentCol] + tid + 1;

            //update current col

            int offset = 0;
            while (currentLColSize > offset)
            {
            if (tid + offset < currentLColSize)
            {
            unsigned ridx = sym_r_idx_dev[currentLPos + offset];

            if (abs(val_dev[l_col_ptr_dev[currentCol]]) < pert)
            val_dev[l_col_ptr_dev[currentCol]] = pert;

            val_dev[currentLPos + offset] /= val_dev[l_col_ptr_dev[currentCol]];
            tmpMem[stream * n + ridx]= val_dev[currentLPos + offset];
            }
            offset += BLOCK_DIM_X;
            }

        }
    }
}
extern "C"

void RL_onecol_updateSubmat(
    const unsigned*  sym_c_ptr_dev,
    const unsigned*  sym_r_idx_dev,
    REAL*  val_dev,
    const unsigned*  csr_c_idx_dev,
    const unsigned*  csr_diag_ptr_dev,
    const unsigned currentCol,
    REAL*  tmpMem,
    const int stream,
    const unsigned n)
{
    #pragma HLS INTERFACE m_axi port=sym_c_ptr_dev offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=sym_r_idx_dev offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=val_dev offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=csr_c_idx_dev offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=csr_diag_ptr_dev offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=currentCol
    #pragma HLS INTERFACE m_axi port=tmpMem offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=stream
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int tid = _tid_x;
            const int bid = _bid_x;
            REAL s;

            //broadcast to submatrix
            const unsigned subColPos = csr_diag_ptr_dev[currentCol] + bid + 1;
            unsigned subCol;
            unsigned subColElem = 0;

            int offset = 0;
            subCol = csr_c_idx_dev[subColPos];
            while(offset < sym_c_ptr_dev[subCol + 1] - sym_c_ptr_dev[subCol])
            {
            if (tid + offset < sym_c_ptr_dev[subCol + 1] - sym_c_ptr_dev[subCol])
            {
            subColElem = sym_c_ptr_dev[subCol] + tid + offset;
            unsigned ridx = sym_r_idx_dev[subColElem];

            if (ridx == currentCol)
            {
            s = val_dev[subColElem];
            }
            if (ridx > currentCol)
            {
            (val_dev[subColElem] += -tmpMem[stream * n + ridx] * s);
            }
            }
            offset += BLOCK_DIM_X;
            }

        }
    }
}
extern "C"

void RL_onecol_cleartmpMem(
    const unsigned*  sym_c_ptr_dev,
    const unsigned*  sym_r_idx_dev,
    const unsigned*  l_col_ptr_dev,
    const unsigned currentCol,
    REAL*  tmpMem,
    const int stream,
    const unsigned n)
{
    #pragma HLS INTERFACE m_axi port=sym_c_ptr_dev offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=sym_r_idx_dev offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=l_col_ptr_dev offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=currentCol
    #pragma HLS INTERFACE m_axi port=tmpMem offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=stream
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int tid = _tid_x;

            const unsigned currentLColSize = sym_c_ptr_dev[currentCol + 1] - l_col_ptr_dev[currentCol] - 1;
            const unsigned currentLPos = l_col_ptr_dev[currentCol] + tid + 1;

            unsigned offset = 0;
            while (currentLColSize > offset)
            {
            if (tid + offset < currentLColSize)
            {
            unsigned ridx = sym_r_idx_dev[currentLPos + offset];
            tmpMem[stream * n + ridx]= 0;
            }
            offset += BLOCK_DIM_X;
            }

        }
    }
}
