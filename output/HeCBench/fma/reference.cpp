template <typename T>
void reference (const T*  a,
                const T*  b,
                T*  c,
                const int*  in_indices,
                const int*  out_indices,
                int num_ops,
                int C,
                int N_A,
                int N_C)
{
  for (int op_idx = 0; op_idx < num_ops; op_idx++) {
    int in_idx = in_indices[op_idx];
    int out_idx = out_indices[op_idx];

    // Bounds checking
    if (in_idx < 0 || in_idx >= N_A || out_idx < 0 || out_idx >= N_C) {
        continue;
    }

    for (int ch_idx = 0; ch_idx < C; ch_idx++) {
      T a_val = a[in_idx * C + ch_idx];
      T b_val = b[ch_idx];

      // Direct write since out_rows are unique (no conflicts)
      c[out_idx * C + ch_idx] += a_val * b_val;
    }
  }
}
