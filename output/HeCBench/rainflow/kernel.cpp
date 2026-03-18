#include "kernel.h"

// --- from main.cu ---
void Extrema(const double* history, const int history_length, double *result, int& result_length)
{
  result[0] = history[0];

  int eidx = 0;
  for (int i = 1; i < history_length - 1; i++)
    if ((history[i] > result[eidx] && history[i] > history[i + 1]) ||
        (history[i] < result[eidx] && history[i] < history[i + 1]))
      result[++eidx] = history[i];

  result[++eidx] = history[history_length - 1];
  result_length = eidx + 1;
}

void Execute(const double* history, const int history_length,
             double *extrema, int* points, double3 *results,
             int *results_length )
{
  int extrema_length = 0;
  Extrema(history, history_length, extrema, extrema_length);

  int pidx = -1, eidx = -1, ridx = -1;

  for (int i = 0; i < extrema_length; i++)
  {
    points[++pidx] = ++eidx;
    double xRange, yRange;
    while (pidx >= 2 && (xRange = fabs(extrema[points[pidx - 1]] - extrema[points[pidx]]))
           >= (yRange = fabs(extrema[points[pidx - 2]] - extrema[points[pidx - 1]])))
    {
      double yMean = 0.5 * (extrema[points[pidx - 2]] + extrema[points[pidx - 1]]);

      if (pidx == 2)
      {
        results[++ridx] = make_double3( 0.5, yRange, yMean );
        points[0] = points[1];
        points[1] = points[2];
        pidx = 1;
      }
      else
      {
        results[++ridx] = make_double3( 1.0, yRange, yMean );
        points[pidx - 2] = points[pidx];
        pidx -= 2;
      }
    }
  }

  for (int i = 0; i <= pidx - 1; i++)
  {
    double range = fabs(extrema[points[i]] - extrema[points[i + 1]]);
    double mean = 0.5 * (extrema[points[i]] + extrema[points[i + 1]]);
    results[++ridx] = make_double3 ( 0.5, range, mean );
  }

  *results_length = ridx + 1;
}
extern "C"

void rainflow_count(const double * history,
                    const int * history_lengths,
                    double * extrema,
                       int *  points,
                    double3 * results,
                    int * result_length,
                    const int num_history )
{
    #pragma HLS INTERFACE m_axi port=history offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=history_lengths offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=extrema offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=points offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=results offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=result_length offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=num_history
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i >= num_history) return;

            const int offset = history_lengths[i];
            const int history_length = history_lengths[i+1] - offset;
            Execute(history + offset,
            history_length,
            extrema + offset,
            points + offset,
            results + offset,
            result_length + i);

        }
    }
}
