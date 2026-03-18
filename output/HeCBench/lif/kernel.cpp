#include "kernel.h"

// --- from main.cu ---
extern "C"
void lif (
    size_t numNeurons, int neurons_per_item, float dt,
    const float* encode_result,
          float* voltage_array,
          float* reftime_array,
    float tau_rc, float tau_ref,
    const float* bias,
    const float* gain,
          float* spikes)
{
    #pragma HLS INTERFACE s_axilite port=numNeurons
    #pragma HLS INTERFACE s_axilite port=neurons_per_item
    #pragma HLS INTERFACE s_axilite port=dt
    #pragma HLS INTERFACE m_axi port=encode_result offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=voltage_array offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=reftime_array offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=tau_rc
    #pragma HLS INTERFACE s_axilite port=tau_ref
    #pragma HLS INTERFACE m_axi port=bias offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=gain offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=spikes offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t i = (size_t)_bid_x * BLOCK_DIM_X + _tid_x;
            if (i < numNeurons)
            {
            int neuron_index = i % neurons_per_item;
            int item_index = i / neurons_per_item;

            float voltage = voltage_array[i];
            float ref_time = reftime_array[i];
            float current = bias[neuron_index] + gain[neuron_index] * encode_result[item_index];
            float dV, spike, mult;

            dV = -expm1f(-dt / tau_rc) * (current - voltage);
            voltage = fmaxf(voltage + dV, 0.f);

            ref_time -= dt;

            mult = ref_time;
            mult *= -1.f / dt;
            mult += 1.f;

            mult = fminf(mult, 1.f);
            mult = fmaxf(mult, 0.f);

            voltage *= mult;

            if(voltage > 1.f){
            spike = 1.f / dt;
            ref_time = tau_ref + dt * (1.f - (voltage - 1.f) / dV);
            voltage = 0.f;
            }else{
            spike = 0.f;
            }

            reftime_array[i] = ref_time;
            voltage_array[i] = voltage;
            spikes[i] = spike;
            }

        }
    }
}
