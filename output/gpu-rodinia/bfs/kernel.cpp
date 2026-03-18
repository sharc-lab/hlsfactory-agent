#include "kernel.h"

// --- from kernel.cu ---
extern "C"
void Kernel( Node* g_graph_nodes, int* g_graph_edges, bool* g_graph_mask, bool* g_updating_graph_mask, bool *g_graph_visited, int* g_cost, int no_of_nodes) 
{
    #pragma HLS INTERFACE m_axi port=g_graph_nodes offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_graph_edges offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_graph_mask offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g_updating_graph_mask offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=g_graph_visited offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=g_cost offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=no_of_nodes
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int tid = _bid_x*MAX_THREADS_PER_BLOCK + _tid_x;
            if( tid<no_of_nodes && g_graph_mask[tid])
            {
            g_graph_mask[tid]=false;
            for(int i=g_graph_nodes[tid].starting; i<(g_graph_nodes[tid].no_of_edges + g_graph_nodes[tid].starting); i++)
            {
            int id = g_graph_edges[i];
            if(!g_graph_visited[id])
            {
            g_cost[id]=g_cost[tid]+1;
            g_updating_graph_mask[id]=true;
            }
            }
            }

        }
    }
}


// --- from kernel2.cu ---
extern "C"
void Kernel2( bool* g_graph_mask, bool *g_updating_graph_mask, bool* g_graph_visited, bool *g_over, int no_of_nodes)
{
    #pragma HLS INTERFACE m_axi port=g_graph_mask offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_updating_graph_mask offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_graph_visited offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g_over offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=no_of_nodes
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int tid = _bid_x*MAX_THREADS_PER_BLOCK + _tid_x;
            if( tid<no_of_nodes && g_updating_graph_mask[tid])
            {

            g_graph_mask[tid]=true;
            g_graph_visited[tid]=true;
            *g_over=true;
            g_updating_graph_mask[tid]=false;
            }

        }
    }
}
