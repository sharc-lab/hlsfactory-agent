#include "k_kem.cpp"
#include <cstring>
#include <cstdlib>

int main() {
    // Parameters (example values)
    unsigned char kem_cfg = 0;
    const int BUF_SIZE = 1024;

    // Allocate and zero-initialize buffers
    uint8_t *ct_in        = (uint8_t*)calloc(BUF_SIZE, sizeof(uint8_t));
    uint8_t *ct_out       = (uint8_t*)calloc(BUF_SIZE, sizeof(uint8_t));
    uint8_t *ss_enc_out   = (uint8_t*)calloc(BUF_SIZE, sizeof(uint8_t));
    uint8_t *ss_dec_out   = (uint8_t*)calloc(BUF_SIZE, sizeof(uint8_t));
    uint8_t *buf_in       = (uint8_t*)calloc(BUF_SIZE, sizeof(uint8_t));
    uint8_t *pk_in        = (uint8_t*)calloc(BUF_SIZE, sizeof(uint8_t));
    uint8_t *sk_in        = (uint8_t*)calloc(BUF_SIZE, sizeof(uint8_t));

    // Call the accelerator kernel
    mlkem_accelerator(kem_cfg, ct_in, ct_out, ss_enc_out,
                      ss_dec_out, buf_in, pk_in, sk_in);

    // Clean up
    free(ct_in);
    free(ct_out);
    free(ss_enc_out);
    free(ss_dec_out);
    free(buf_in);
    free(pk_in);
    free(sk_in);
    return 0;
}
