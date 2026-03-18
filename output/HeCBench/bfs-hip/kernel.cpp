#include "kernel.h"

// --- from bfs.cu ---
extern "C"
void Kernel(const Node*  d_graph_nodes, 
       const int*  d_graph_edges,
       char*  d_graph_mask,
       char*  d_updatind_graph_mask,
       const char * d_graph_visited,
       int*  d_cost,
       const int no_of_nodes) 
{
    #pragma HLS INTERFACE m_axi port=d_graph_nodes offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_graph_edges offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_graph_mask offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_updatind_graph_mask offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=d_graph_visited offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=d_cost offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=no_of_nodes
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int tid = _bid_x * BLOCK_DIM_X + _tid_x;
            if( tid<no_of_nodes && d_graph_mask[tid])
            {
            d_graph_mask[tid]=0;
            const int num_edges = d_graph_nodes[tid].no_of_edges;
            const int starting = d_graph_nodes[tid].starting;

            for(int i=starting; i<(num_edges + starting); i++)
            {
            int id = d_graph_edges[i];
            if(!d_graph_visited[id])
            {
            d_cost[id]=d_cost[tid]+1;
            d_updatind_graph_mask[id]=1;
            }
            }
            }

        }
    }
}
extern "C"

void Kernel2(char*  d_graph_mask,
        char*  d_updatind_graph_mask,
        char*  d_graph_visited,
        char*  d_over,
        const int no_of_nodes)
{
    #pragma HLS INTERFACE m_axi port=d_graph_mask offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_updatind_graph_mask offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_graph_visited offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_over offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=no_of_nodes
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int tid = _bid_x * BLOCK_DIM_X + _tid_x;
            if( tid<no_of_nodes && d_updatind_graph_mask[tid])
            {
            d_graph_mask[tid]=1;
            d_graph_visited[tid]=1;
            *d_over=1;
            d_updatind_graph_mask[tid]=0;
            }

        }
    }
}
