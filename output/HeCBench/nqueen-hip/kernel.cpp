#include "kernel.h"

// --- from main.cu ---
bool queens_stillLegal(const char *board, const int r)
{
  bool safe = true;
  // Check vertical
  for (int i = 0; i < r; ++i)
    if (board[i] == board[r]) safe = false;
  // Check diagonals
  int ld = board[r];  //left diagonal columns
  int rd = board[r];  // right diagonal columns
  for (int i = r-1; i >= 0; --i) {
    --ld; ++rd;
    if (board[i] == ld || board[i] == rd) safe = false;
  }
  return safe;
}
extern "C"

void BP_queens_root_dfs(
  int N, unsigned int nPreFixos, int depthPreFixos,
  const QueenRoot * root_prefixes,
  unsigned long long * vector_of_tree_size,
  unsigned long long * sols)
{
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=nPreFixos
    #pragma HLS INTERFACE s_axilite port=depthPreFixos
    #pragma HLS INTERFACE m_axi port=root_prefixes offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=vector_of_tree_size offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=sols offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int idx = _bid_x * BLOCK_DIM_X + _tid_x;
            if (idx < nPreFixos) {
            unsigned int flag = 0;
            unsigned int bit_test = 0;
            char vertice[20];
            int N_l = N;
            int i, depth;
            unsigned long long  qtd_solutions_thread = 0ULL;
            int depthGlobal = depthPreFixos;
            unsigned long long tree_size = 0ULL;

            #pragma unroll 2
            for (i = 0; i < N_l; ++i) {
            vertice[i] = _EMPTY_;
            }

            flag = root_prefixes[idx].control;

            #pragma unroll 2
            for (i = 0; i < depthGlobal; ++i)
            vertice[i] = root_prefixes[idx].board[i];

            depth = depthGlobal;

            do {
            vertice[depth]++;
            bit_test = 0;
            bit_test |= (1<<vertice[depth]);
            if(vertice[depth] == N_l){
            vertice[depth] = _EMPTY_;
            } else if (!(flag & bit_test ) && queens_stillLegal(vertice, depth)){
            ++tree_size;
            flag |= (1ULL<<vertice[depth]);
            depth++;
            if (depth == N_l) { //sol
            ++qtd_solutions_thread;
            } else continue;
            } else continue;
            depth--;
            flag &= ~(1ULL<<vertice[depth]);
            } while(depth >= depthGlobal);

            sols[idx] = qtd_solutions_thread;
            vector_of_tree_size[idx] = tree_size;
            }//if

        }
    }
}//kernel
