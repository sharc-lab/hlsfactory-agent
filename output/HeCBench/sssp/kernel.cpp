#include "kernel.h"

// --- from main.cu ---
extern "C"
void SSSP_gpu(
    const Node * graph_nodes_av,
    const Edge * graph_edges_av,
    int * cost,
    int * color,
    const int * q1,
          int * q2,
    const int * n_t,
    int * head,
    int * tail,
    int * overflow,
    const int * gray_shade,
    int * iter)
{
    #pragma HLS INTERFACE m_axi port=graph_nodes_av offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=graph_edges_av offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=cost offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=color offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=q1 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=q2 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=n_t offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=head offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=tail offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=overflow offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=gray_shade offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=iter offset=slave bundle=gmem11
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int l_mem[W_QUEUE_SIZE+2];
            int tail_bin;
            int* l_q2 = l_mem;
            int* shift = l_mem + W_QUEUE_SIZE;
            int* base = l_mem + W_QUEUE_SIZE + 1;

            const int tid     = _tid_x;
            const int gtid    = _bid_x * BLOCK_DIM_X + _tid_x;
            const int WG_SIZE = BLOCK_DIM_X;

            int n_t_local = *n_t; // (*n_t += 0);
            int gray_shade_local = *gray_shade; // (gray_shade[0] += 0);

            if(tid == 0) {
            // Reset queue
            tail_bin = 0;
            }

            // Fetch frontier elements from the queue
            if(tid == 0)
            *base = (head[0] += WG_SIZE);

            int my_base = *base;
            while(my_base < n_t_local) {

            // If local queue might overflow
            if(tail_bin >= W_QUEUE_SIZE / 2) {
            if(tid == 0) {
            // Add local tail_bin to tail
            *shift = (tail[0] += tail_bin);
            }
            int local_shift = tid;
            while(local_shift < tail_bin) {
            q2[*shift + local_shift] = l_q2[local_shift];
            // Multiple threads are copying elements at the same time, so we shift by multiple elements for next iteration
            local_shift += WG_SIZE;
            }
            if(tid == 0) {
            // Reset local queue
            tail_bin = 0;
            }
            }

            if(my_base + tid < n_t_local && *overflow == 0) {
            // Visit a node from the current frontier
            int pid = q1[my_base + tid];
            //////////////// Visit node ///////////////////////////
            (color[pid] = BLACK); // Node visited
            int  cur_cost = cost[pid]; // (cost[pid] += 0); // Look up shortest-path distance to this node
            Node cur_node;
            cur_node.x = graph_nodes_av[pid].x;
            cur_node.y = graph_nodes_av[pid].y;
            Edge cur_edge;
            // For each outgoing edge
            for(int i = cur_node.x; i < cur_node.y + cur_node.x; i++) {
            cur_edge.x = graph_edges_av[i].x;
            cur_edge.y = graph_edges_av[i].y;
            int id     = cur_edge.x;
            int cost_local   = cur_edge.y;
            cost_local += cur_cost;
            int orig_cost = (cost[id] = max(cost[id], cost_local));
            if(orig_cost < cost_local) {
            int old_color = (color[id] = max(color[id], gray_shade_local));
            if(old_color != gray_shade_local) {
            // Push to the queue
            int tail_index = (tail_bin += 1);
            if(tail_index >= W_QUEUE_SIZE) {
            *overflow = 1;
            } else
            l_q2[tail_index] = id;
            }
            }
            }
            }

            if(tid == 0)
            *base = (head[0] += WG_SIZE); // Fetch more frontier elements from the queue
            my_base = *base;
            }
            /////////////////////////////////////////////////////////
            // Compute size of the output and allocate space in the global queue
            if(tid == 0) {
            *shift = (tail[0] += tail_bin);
            }
            ///////////////////// CONCATENATE INTO GLOBAL MEMORY /////////////////////
            int local_shift = tid;
            while(local_shift < tail_bin) {
            q2[*shift + local_shift] = l_q2[local_shift];
            // Multiple threads are copying elements at the same time, so we shift by multiple elements for next iteration
            local_shift += WG_SIZE;
            }
            //////////////////////////////////////////////////////////////////////////

            if(gtid == 0) {
            (iter[0] += 1);
            }

        }
    }
}
