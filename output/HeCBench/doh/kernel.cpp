#include "kernel.h"

// --- from main.cu ---
inline int _clip(const int x, const int low, const int high)
{
  if (x > high)
    return high;
  else if (x < low)
    return low;
  else
    return x;
}

inline IMAGE_T _integ(const IMAGE_T * img,
                      const INT_T img_rows,
                      const INT_T img_cols,
                      int r,
                      int c,
                      const int rl,
                      const int cl)
{
  r = _clip(r, 0, img_rows - 1);
  c = _clip(c, 0, img_cols - 1);

  const int r2 = _clip(r + rl, 0, img_rows - 1);
  const int c2 = _clip(c + cl, 0, img_cols - 1);

  IMAGE_T ans = img[r * img_cols + c] + img[r2 * img_cols + c2] -
                img[r * img_cols + c2] - img[r2 * img_cols + c];

  return max((IMAGE_T)0, ans);
}
extern "C"

void hessian_matrix_det(const IMAGE_T* img,
                        const INT_T img_rows,
                        const INT_T img_cols,
                        const IMAGE_T sigma,
                        IMAGE_T* out)
{
    #pragma HLS INTERFACE m_axi port=img offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=img_rows
    #pragma HLS INTERFACE s_axilite port=img_cols
    #pragma HLS INTERFACE s_axilite port=sigma
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int tid = BLOCK_DIM_X * _bid_x + _tid_x;

            if (tid >= img_rows*img_cols) return;

            const int r = tid / img_cols;
            const int c = tid % img_cols;

            int size = (int)((IMAGE_T)3.0 * sigma);

            const int b = (size - 1) / 2 + 1;
            const int l = size / 3;
            const int w = size;

            const IMAGE_T w_i = (IMAGE_T)1.0 / (size * size);

            const IMAGE_T tl = _integ(img, img_rows, img_cols, r - l, c - l, l, l); // top left
            const IMAGE_T br = _integ(img, img_rows, img_cols, r + 1, c + 1, l, l); // bottom right
            const IMAGE_T bl = _integ(img, img_rows, img_cols, r - l, c + 1, l, l); // bottom left
            const IMAGE_T tr = _integ(img, img_rows, img_cols, r + 1, c - l, l, l); // top right

            IMAGE_T dxy = bl + tr - tl - br;
            dxy = -dxy * w_i;

            IMAGE_T mid = _integ(img, img_rows, img_cols, r - l + 1, c - l, 2 * l - 1, w);  // middle box
            IMAGE_T side = _integ(img, img_rows, img_cols, r - l + 1, c - l / 2, 2 * l - 1, l);  // sides

            IMAGE_T dxx = mid - (IMAGE_T)3 * side;
            dxx = -dxx * w_i;

            mid = _integ(img, img_rows, img_cols, r - l, c - b + 1, w, 2 * b - 1);
            side = _integ(img, img_rows, img_cols, r - b / 2, c - b + 1, b, 2 * b - 1);

            IMAGE_T dyy = mid - (IMAGE_T)3 * side;
            dyy = -dyy * w_i;

            out[tid] = (dxx * dyy - (IMAGE_T)0.81 * (dxy * dxy));

        }
    }
}
