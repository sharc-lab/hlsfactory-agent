#include "kernel.h"

// --- from main.cu ---
extern "C"
void vanGenuchten(
  const double * Ksat,
  const double * psi,
        double * C,
        double * theta,
        double * K,
  const int size)
{
    #pragma HLS INTERFACE m_axi port=Ksat offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=psi offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=theta offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=K offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            double Se, _theta, _psi, lambda, m, t;

            int i = _tid_x + _bid_x * BLOCK_DIM_X;
            if (i < size)
            {
            lambda = n - 1.0;
            m = lambda/n;

            // Compute the volumetric moisture content [eqn 21]
            _psi = psi[i] * 100.0;
            if ( _psi < 0.0 )
            _theta = (theta_S - theta_R) / pow(1.0 + pow((alpha * (-_psi)), n), m) + theta_R;
            else
            _theta = theta_S;

            theta[i] = _theta;

            // Compute the effective saturation [eqn 2]
            Se = (_theta - theta_R) / (theta_S - theta_R);

            // Compute the hydraulic conductivity [eqn 8]
            t = 1.0 - pow(1.0 - pow(Se, 1.0 / m), m);
            K[i] = Ksat[i] * sqrt(Se) * t * t;

            // Compute the specific moisture storage derivative of eqn (21).
            // So we have to calculate C = d(theta)/dh. Then the unit is converted into [1/m].
            if (_psi < 0.0)
            C[i] = 100.0 * alpha * n * (1.0 /n - 1.0) * pow(alpha * fabs(_psi), n - 1.0)
            * (theta_R - theta_S) * pow(pow(alpha * fabs(_psi), n) + 1.0, 1.0 / n - 2.0);
            else
            C[i] = 0.0;
            }

        }
    }
}
