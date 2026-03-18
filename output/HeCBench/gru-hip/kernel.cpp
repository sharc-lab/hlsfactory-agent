#include "kernel.h"

// --- from main.cu ---
T sigmoid(T in)  {
  T one = static_cast<T>(1.0);
  return one / (one + exp(-in));
}
extern "C"

void gru_cell_forward(
            scalar_t * Input,
            scalar_t * Hidden,
            scalar_t * Bias1,
            scalar_t * Bias2,
            scalar_t * _hx,   // h(t-1)
            scalar_t * _hy,   // h(t)
            scalar_t * storage,
            index_type hsz,
            index_type totalElements)
{
  index_type linearIndex = _bid_x * BLOCK_DIM_X + _tid_x;
  if (linearIndex < totalElements) {
    index_type offset = (linearIndex/hsz)*3*hsz+linearIndex%hsz;

    scalar_t ir = DEVICE_LINEAR_GET(Input,  offset+0*hsz);
    scalar_t ii = DEVICE_LINEAR_GET(Input,  offset+1*hsz);
    scalar_t in = DEVICE_LINEAR_GET(Input,  offset+2*hsz);
    scalar_t hr = DEVICE_LINEAR_GET(Hidden, offset+0*hsz);
    scalar_t hi = DEVICE_LINEAR_GET(Hidden, offset+1*hsz);
    scalar_t hn = DEVICE_LINEAR_GET(Hidden, offset+2*hsz);

    scalar_t hx = DEVICE_LINEAR_GET(_hx, linearIndex);
    scalar_t* hy = &DEVICE_LINEAR_GET(_hy, linearIndex);

    scalar_t b1r, b1i, b1n, b2r, b2i, b2n;

    b1r = DEVICE_BIAS_GET(Bias1, linearIndex%hsz+0*hsz);
    b1i = DEVICE_BIAS_GET(Bias1, linearIndex%hsz+1*hsz);
    b1n = DEVICE_BIAS_GET(Bias1, linearIndex%hsz+2*hsz);

    b2r = DEVICE_BIAS_GET(Bias2, linearIndex%hsz+0*hsz);
    b2i = DEVICE_BIAS_GET(Bias2, linearIndex%hsz+1*hsz);
    b2n = DEVICE_BIAS_GET(Bias2, linearIndex%hsz+2*hsz);

    offset = (linearIndex/hsz)*5*hsz+linearIndex%hsz;

    accscalar_t rg, ig, ng;

    // reset: ir = Wr * xt , hr = Ur * h(t-1)
    rg = sigmoid(H2F(ir) + H2F(hr) + H2F(b1r) + H2F(b2r));

    // update: ii = Wz * xt , hi = Uz * h(t-1)
    ig = sigmoid(H2F(ii) + H2F(hi) + H2F(b1i) + H2F(b2i));

    // in = Wh * xt, hn = Uh * h(t-1)
    ng = H2F(in) + H2F(b1n) + rg*( H2F(hn) + H2F(b2n) );
    ng = tanh(ng); // h'

    // z * h(t-1) + (1-z)*h', hx = h(t-1)
    *hy = F2H( ng + ig * ( H2F(hx)-ng ) );

    //save for backwards
    DEVICE_LINEAR_GET(storage, offset+0*hsz) = F2H(rg);
    DEVICE_LINEAR_GET(storage, offset+1*hsz) = F2H(ig);
    DEVICE_LINEAR_GET(storage, offset+2*hsz) = F2H(ng);
    DEVICE_LINEAR_GET(storage, offset+3*hsz) = hx;
    DEVICE_LINEAR_GET(storage, offset+4*hsz) = F2H(H2F(hn) + H2F(b2n));
  }
}
