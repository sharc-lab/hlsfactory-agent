#include "kernel.h"

// --- from main.cu ---
extern "C"
void kalman(
  const double* ys,
  int nobs,
  const double* T,
  const double* Z,
  const double* RQR,
  const double* P,
  const double* alpha,
  bool intercept,
  const double* d_mu,
  int batch_size,
  double* vs,
  double* Fs,
  double* sum_logFs,
  int n_diff,
  int fc_steps = 0,
  double* d_fc = nullptr,
  bool conf_int = false,
  double* d_F_fc = nullptr)
{
    #pragma HLS INTERFACE m_axi port=ys offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=nobs
    #pragma HLS INTERFACE m_axi port=T offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=Z offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=RQR offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=P offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=alpha offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=intercept
    #pragma HLS INTERFACE m_axi port=d_mu offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=batch_size
    #pragma HLS INTERFACE m_axi port=vs offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=Fs offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=sum_logFs offset=slave bundle=gmem9
    #pragma HLS INTERFACE s_axilite port=n_diff
    #pragma HLS INTERFACE s_axilite port=0
    #pragma HLS INTERFACE m_axi port=nullptr offset=slave bundle=gmem10
    #pragma HLS INTERFACE s_axilite port=false
    #pragma HLS INTERFACE m_axi port=nullptr offset=slave bundle=gmem11
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            constexpr int rd2 = rd * rd;
            double l_RQR[rd2] = {0.0};
            double l_T[rd2] = {0.0};
            double l_Z[rd] = {0.0};
            double l_P[rd2] = {0.0};
            double l_alpha[rd] = {0.0};
            double l_K[rd] = {0.0};
            double l_tmp[rd2] = {0.0};
            double l_TP[rd2] = {0.0};

            int bid = BLOCK_DIM_X * _bid_x + _tid_x;
            if (bid < batch_size) {
            // Load global mem into registers
            int b_rd_offset  = bid * rd;
            int b_rd2_offset = bid * rd2;
            for (int i = 0; i < rd2; i++) {
            l_RQR[i] = RQR[b_rd2_offset + i];
            l_T[i]   = T[b_rd2_offset + i];
            l_P[i]   = P[b_rd2_offset + i];
            }
            for (int i = 0; i < rd; i++) {
            if (n_diff > 0) l_Z[i] = Z[b_rd_offset + i];
            l_alpha[i] = alpha[b_rd_offset + i];
            }

            double b_sum_logFs = 0.0;
            const double* b_ys = ys + bid * nobs;
            double* b_vs       = vs + bid * nobs;
            double* b_Fs       = Fs + bid * nobs;

            double mu = intercept ? d_mu[bid] : 0.0;

            for (int it = 0; it < nobs; it++) {
            // 1. v = y - Z*alpha
            double vs_it = b_ys[it];
            if (n_diff == 0)
            vs_it -= l_alpha[0];
            else {
            for (int i = 0; i < rd; i++) {
            vs_it -= l_alpha[i] * l_Z[i];
            }
            }
            b_vs[it] = vs_it;

            // 2. F = Z*P*Z'
            double _Fs;
            if (n_diff == 0)
            _Fs = l_P[0];
            else {
            _Fs = 0.0;
            for (int i = 0; i < rd; i++) {
            for (int j = 0; j < rd; j++) {
            _Fs += l_P[j * rd + i] * l_Z[i] * l_Z[j];
            }
            }
            }
            b_Fs[it] = _Fs;
            if (it >= n_diff) b_sum_logFs += log(_Fs);

            // 3. K = 1/Fs[it] * T*P*Z'
            // TP = T*P
            MM_l<rd>(l_T, l_P, l_TP);
            // K = 1/Fs[it] * TP*Z'
            double _1_Fs = 1.0 / _Fs;
            if (n_diff == 0) {
            for (int i = 0; i < rd; i++) {
            l_K[i] = _1_Fs * l_TP[i];
            }
            } else
            Mv_l<rd>(_1_Fs, l_TP, l_Z, l_K);

            // 4. alpha = T*alpha + K*vs[it] + c
            // tmp = T*alpha
            Mv_l<rd>(l_T, l_alpha, l_tmp);
            // alpha = tmp + K*vs[it]
            for (int i = 0; i < rd; i++) {
            l_alpha[i] = l_tmp[i] + l_K[i] * vs_it;
            }
            // alpha = alpha + c
            l_alpha[n_diff] += mu;

            // 5. L = T - K * Z
            // L = T (L is tmp)
            for (int i = 0; i < rd2; i++) {
            l_tmp[i] = l_T[i];
            }
            // L = L - K * Z
            if (n_diff == 0) {
            for (int i = 0; i < rd; i++) {
            l_tmp[i] -= l_K[i];
            }
            } else {
            for (int i = 0; i < rd; i++) {
            for (int j = 0; j < rd; j++) {
            l_tmp[j * rd + i] -= l_K[i] * l_Z[j];
            }
            }
            }

            // 6. P = T*P*L' + R*Q*R'
            // P = TP*L'
            MM_l<rd, false, true>(l_TP, l_tmp, l_P);
            // P = P + RQR
            for (int i = 0; i < rd2; i++) {
            l_P[i] += l_RQR[i];
            }
            }
            sum_logFs[bid] = b_sum_logFs;

            // Forecast
            double* b_fc   = fc_steps ? d_fc + bid * fc_steps : nullptr;
            double* b_F_fc = conf_int ? d_F_fc + bid * fc_steps : nullptr;
            for (int it = 0; it < fc_steps; it++) {
            if (n_diff == 0)
            b_fc[it] = l_alpha[0];
            else {
            double pred = 0.0;
            for (int i = 0; i < rd; i++) {
            pred += l_alpha[i] * l_Z[i];
            }
            b_fc[it] = pred;
            }

            // alpha = T*alpha + c
            Mv_l<rd>(l_T, l_alpha, l_tmp);
            for (int i = 0; i < rd; i++) {
            l_alpha[i] = l_tmp[i];
            }
            l_alpha[n_diff] += mu;

            if (conf_int) {
            if (n_diff == 0)
            b_F_fc[it] = l_P[0];
            else {
            double _Fs = 0.0;
            for (int i = 0; i < rd; i++) {
            for (int j = 0; j < rd; j++) {
            _Fs += l_P[j * rd + i] * l_Z[i] * l_Z[j];
            }
            }
            b_F_fc[it] = _Fs;
            }

            // P = T*P*T' + RR'
            // TP = T*P
            MM_l<rd>(l_T, l_P, l_TP);
            // P = TP*T'
            MM_l<rd, false, true>(l_TP, l_T, l_P);
            // P = P + RR'
            for (int i = 0; i < rd2; i++) {
            l_P[i] += l_RQR[i];
            }
            }
            }
            }

        }
    }
}
