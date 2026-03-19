#include "k_dsa.cpp"
#include <cstdlib>
#include <cstring>

int main() {
    unsigned char kem_cfg = 0;
    const int BUF_SIZE = 4096;

    uint8_t *ret_out            = (uint8_t*)calloc(BUF_SIZE, 1);
    uint8_t *sign_out           = (uint8_t*)calloc(BUF_SIZE, 1);
    uint8_t *sign_in            = (uint8_t*)calloc(BUF_SIZE, 1);
    uint8_t *mu_processed_in    = (uint8_t*)calloc(BUF_SIZE, 1);
    uint8_t *mu_orig_in         = (uint8_t*)calloc(BUF_SIZE, 1);
    uint8_t *mu2_processed_in   = (uint8_t*)calloc(BUF_SIZE, 1);
    uint8_t *sk_in              = (uint8_t*)calloc(BUF_SIZE, 1);
    uint8_t *pk_in              = (uint8_t*)calloc(BUF_SIZE, 1);
    int      *ver_out           = (int*)calloc(1, sizeof(int));
    size_t   mlen_in            = 0;

    mldsa_accelerator(kem_cfg,
                      ret_out,
                      sign_out,
                      sign_in,
                      mu_processed_in,
                      mu_orig_in,
                      mu2_processed_in,
                      sk_in,
                      pk_in,
                      ver_out,
                      mlen_in);

    free(ret_out);
    free(sign_out);
    free(sign_in);
    free(mu_processed_in);
    free(mu_orig_in);
    free(mu2_processed_in);
    free(sk_in);
    free(pk_in);
    free(ver_out);
    return 0;
}
