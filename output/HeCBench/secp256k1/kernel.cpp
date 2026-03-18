#include "kernel.h"

// --- from main.cu ---
void secp256k1_fe_from_storage(secp256k1_fe *r, const secp256k1_fe_storage *a) {
  r->n[0] = a->n[0] & 0x3FFFFFFUL;
  r->n[1] = a->n[0] >> 26 | ((a->n[1] << 6) & 0x3FFFFFFUL);
  r->n[2] = a->n[1] >> 20 | ((a->n[2] << 12) & 0x3FFFFFFUL);
  r->n[3] = a->n[2] >> 14 | ((a->n[3] << 18) & 0x3FFFFFFUL);
  r->n[4] = a->n[3] >> 8 | ((a->n[4] << 24) & 0x3FFFFFFUL);
  r->n[5] = (a->n[4] >> 2) & 0x3FFFFFFUL;
  r->n[6] = a->n[4] >> 28 | ((a->n[5] << 4) & 0x3FFFFFFUL);
  r->n[7] = a->n[5] >> 22 | ((a->n[6] << 10) & 0x3FFFFFFUL);
  r->n[8] = a->n[6] >> 16 | ((a->n[7] << 16) & 0x3FFFFFFUL);
  r->n[9] = a->n[7] >> 10;
}

void secp256k1_fe_sqr_inner(uint *r, const uint *a) {
  ulong c, d;
  ulong u0, u1, u2, u3, u4, u5, u6, u7, u8;
  uint t9, t0, t1, t2, t3, t4, t5, t6, t7;
  const uint M = 0x3FFFFFFUL, R0 = 0x3D10UL, R1 = 0x400UL;

  d  = (ulong)(a[0]*2) * a[9]
    + (ulong)(a[1]*2) * a[8]
    + (ulong)(a[2]*2) * a[7]
    + (ulong)(a[3]*2) * a[6]
    + (ulong)(a[4]*2) * a[5];
  t9 = d & M; d >>= 26;
  c  = (ulong)a[0] * a[0];
  d += (ulong)(a[1]*2) * a[9]
    + (ulong)(a[2]*2) * a[8]
    + (ulong)(a[3]*2) * a[7]
    + (ulong)(a[4]*2) * a[6]
    + (ulong)a[5] * a[5];
  u0 = d & M; d >>= 26; c += u0 * R0;
  t0 = c & M; c >>= 26; c += u0 * R1;
  c += (ulong)(a[0]*2) * a[1];
  d += (ulong)(a[2]*2) * a[9]
    + (ulong)(a[3]*2) * a[8]
    + (ulong)(a[4]*2) * a[7]
    + (ulong)(a[5]*2) * a[6];
  u1 = d & M; d >>= 26; c += u1 * R0;
  t1 = c & M; c >>= 26; c += u1 * R1;
  c += (ulong)(a[0]*2) * a[2]
    + (ulong)a[1] * a[1];
  d += (ulong)(a[3]*2) * a[9]
    + (ulong)(a[4]*2) * a[8]
    + (ulong)(a[5]*2) * a[7]
    + (ulong)a[6] * a[6];
  u2 = d & M; d >>= 26; c += u2 * R0;
  t2 = c & M; c >>= 26; c += u2 * R1;
  c += (ulong)(a[0]*2) * a[3]
    + (ulong)(a[1]*2) * a[2];
  d += (ulong)(a[4]*2) * a[9]
    + (ulong)(a[5]*2) * a[8]
    + (ulong)(a[6]*2) * a[7];
  u3 = d & M; d >>= 26; c += u3 * R0;
  t3 = c & M; c >>= 26; c += u3 * R1;
  c += (ulong)(a[0]*2) * a[4]
    + (ulong)(a[1]*2) * a[3]
    + (ulong)a[2] * a[2];
  d += (ulong)(a[5]*2) * a[9]
    + (ulong)(a[6]*2) * a[8]
    + (ulong)a[7] * a[7];
  u4 = d & M; d >>= 26; c += u4 * R0;
  t4 = c & M; c >>= 26; c += u4 * R1;
  c += (ulong)(a[0]*2) * a[5]
    + (ulong)(a[1]*2) * a[4]
    + (ulong)(a[2]*2) * a[3];
  d += (ulong)(a[6]*2) * a[9]
    + (ulong)(a[7]*2) * a[8];
  u5 = d & M; d >>= 26; c += u5 * R0;
  t5 = c & M; c >>= 26; c += u5 * R1;
  c += (ulong)(a[0]*2) * a[6]
    + (ulong)(a[1]*2) * a[5]
    + (ulong)(a[2]*2) * a[4]
    + (ulong)a[3] * a[3];
  d += (ulong)(a[7]*2) * a[9]
    + (ulong)a[8] * a[8];
  u6 = d & M; d >>= 26; c += u6 * R0;
  t6 = c & M; c >>= 26; c += u6 * R1;
  c += (ulong)(a[0]*2) * a[7]
    + (ulong)(a[1]*2) * a[6]
    + (ulong)(a[2]*2) * a[5]
    + (ulong)(a[3]*2) * a[4];
  d += (ulong)(a[8]*2) * a[9];
  u7 = d & M; d >>= 26; c += u7 * R0;
  t7 = c & M; c >>= 26; c += u7 * R1;
  c += (ulong)(a[0]*2) * a[8]
    + (ulong)(a[1]*2) * a[7]
    + (ulong)(a[2]*2) * a[6]
    + (ulong)(a[3]*2) * a[5]
    + (ulong)a[4] * a[4];
  d += (ulong)a[9] * a[9];
  u8 = d & M; d >>= 26; c += u8 * R0;
  r[3] = t3;
  r[4] = t4;
  r[5] = t5;
  r[6] = t6;
  r[7] = t7;
  r[8] = c & M; c >>= 26; c += u8 * R1;
  c   += d * R0 + t9;
  r[9] = c & (M >> 4); c >>= 22; c += d * (R1 << 4);
  d    = c * (R0 >> 4) + t0;
  r[0] = d & M; d >>= 26;
  d   += c * (R1 >> 4) + t1;
  r[1] = d & M; d >>= 26;
  d   += t2;
  r[2] = d;
}

void secp256k1_fe_sqr(secp256k1_fe *r, const secp256k1_fe *a) {
  secp256k1_fe_sqr_inner(r->n, a->n);
}

void secp256k1_fe_normalize_weak(secp256k1_fe *r) {
  uint t0 = r->n[0], t1 = r->n[1], t2 = r->n[2], t3 = r->n[3], t4 = r->n[4],
     t5 = r->n[5], t6 = r->n[6], t7 = r->n[7], t8 = r->n[8], t9 = r->n[9];

  /* Reduce t9 at the start so there will be at most a single carry from the first pass */
  uint x = t9 >> 22; t9 &= 0x03FFFFFUL;

  /* The first pass ensures the magnitude is 1, ... */
  t0 += x * 0x3D1UL; t1 += (x << 6);
  t1 += (t0 >> 26); t0 &= 0x3FFFFFFUL;
  t2 += (t1 >> 26); t1 &= 0x3FFFFFFUL;
  t3 += (t2 >> 26); t2 &= 0x3FFFFFFUL;
  t4 += (t3 >> 26); t3 &= 0x3FFFFFFUL;
  t5 += (t4 >> 26); t4 &= 0x3FFFFFFUL;
  t6 += (t5 >> 26); t5 &= 0x3FFFFFFUL;
  t7 += (t6 >> 26); t6 &= 0x3FFFFFFUL;
  t8 += (t7 >> 26); t7 &= 0x3FFFFFFUL;
  t9 += (t8 >> 26); t8 &= 0x3FFFFFFUL;

  r->n[0] = t0; r->n[1] = t1; r->n[2] = t2; r->n[3] = t3; r->n[4] = t4;
  r->n[5] = t5; r->n[6] = t6; r->n[7] = t7; r->n[8] = t8; r->n[9] = t9;
}

void secp256k1_fe_mul_inner(uint *r, const uint *a, const uint * b) {
  ulong c, d;
  ulong u0, u1, u2, u3, u4, u5, u6, u7, u8;
  uint t9, t1, t0, t2, t3, t4, t5, t6, t7;
  const uint M = 0x3FFFFFFUL, R0 = 0x3D10UL, R1 = 0x400UL;
  d  = (ulong)a[0] * b[9]
    + (ulong)a[1] * b[8]
    + (ulong)a[2] * b[7]
    + (ulong)a[3] * b[6]
    + (ulong)a[4] * b[5]
    + (ulong)a[5] * b[4]
    + (ulong)a[6] * b[3]
    + (ulong)a[7] * b[2]
    + (ulong)a[8] * b[1]
    + (ulong)a[9] * b[0];
  /* VERIFY_BITS(d, 64); */
  /* [d 0 0 0 0 0 0 0 0 0] = [p9 0 0 0 0 0 0 0 0 0] */
  t9 = d & M; d >>= 26;

  /* [d t9 0 0 0 0 0 0 0 0 0] = [p9 0 0 0 0 0 0 0 0 0] */

  c  = (ulong)a[0] * b[0];

  /* [d t9 0 0 0 0 0 0 0 0 c] = [p9 0 0 0 0 0 0 0 0 p0] */
  d += (ulong)a[1] * b[9]
    + (ulong)a[2] * b[8]
    + (ulong)a[3] * b[7]
    + (ulong)a[4] * b[6]
    + (ulong)a[5] * b[5]
    + (ulong)a[6] * b[4]
    + (ulong)a[7] * b[3]
    + (ulong)a[8] * b[2]
    + (ulong)a[9] * b[1];

  /* [d t9 0 0 0 0 0 0 0 0 c] = [p10 p9 0 0 0 0 0 0 0 0 p0] */
  u0 = d & M; d >>= 26; c += u0 * R0;

  /* [d u0 t9 0 0 0 0 0 0 0 0 c-u0*R0] = [p10 p9 0 0 0 0 0 0 0 0 p0] */
  t0 = c & M; c >>= 26; c += u0 * R1;

  /* [d u0 t9 0 0 0 0 0 0 0 c-u0*R1 t0-u0*R0] = [p10 p9 0 0 0 0 0 0 0 0 p0] */
  /* [d 0 t9 0 0 0 0 0 0 0 c t0] = [p10 p9 0 0 0 0 0 0 0 0 p0] */

  c += (ulong)a[0] * b[1]
    + (ulong)a[1] * b[0];

  /* [d 0 t9 0 0 0 0 0 0 0 c t0] = [p10 p9 0 0 0 0 0 0 0 p1 p0] */
  d += (ulong)a[2] * b[9]
    + (ulong)a[3] * b[8]
    + (ulong)a[4] * b[7]
    + (ulong)a[5] * b[6]
    + (ulong)a[6] * b[5]
    + (ulong)a[7] * b[4]
    + (ulong)a[8] * b[3]
    + (ulong)a[9] * b[2];

  /* [d 0 t9 0 0 0 0 0 0 0 c t0] = [p11 p10 p9 0 0 0 0 0 0 0 p1 p0] */
  u1 = d & M; d >>= 26; c += u1 * R0;

  /* [d u1 0 t9 0 0 0 0 0 0 0 c-u1*R0 t0] = [p11 p10 p9 0 0 0 0 0 0 0 p1 p0] */
  t1 = c & M; c >>= 26; c += u1 * R1;

  /* [d u1 0 t9 0 0 0 0 0 0 c-u1*R1 t1-u1*R0 t0] = [p11 p10 p9 0 0 0 0 0 0 0 p1 p0] */
  /* [d 0 0 t9 0 0 0 0 0 0 c t1 t0] = [p11 p10 p9 0 0 0 0 0 0 0 p1 p0] */

  c += (ulong)a[0] * b[2]
    + (ulong)a[1] * b[1]
    + (ulong)a[2] * b[0];

  /* [d 0 0 t9 0 0 0 0 0 0 c t1 t0] = [p11 p10 p9 0 0 0 0 0 0 p2 p1 p0] */
  d += (ulong)a[3] * b[9]
    + (ulong)a[4] * b[8]
    + (ulong)a[5] * b[7]
    + (ulong)a[6] * b[6]
    + (ulong)a[7] * b[5]
    + (ulong)a[8] * b[4]
    + (ulong)a[9] * b[3];

  /* [d 0 0 t9 0 0 0 0 0 0 c t1 t0] = [p12 p11 p10 p9 0 0 0 0 0 0 p2 p1 p0] */
  u2 = d & M; d >>= 26; c += u2 * R0;
  /* [d u2 0 0 t9 0 0 0 0 0 0 c-u2*R0 t1 t0] = [p12 p11 p10 p9 0 0 0 0 0 0 p2 p1 p0] */
  t2 = c & M; c >>= 26; c += u2 * R1;

  /* [d u2 0 0 t9 0 0 0 0 0 c-u2*R1 t2-u2*R0 t1 t0] = [p12 p11 p10 p9 0 0 0 0 0 0 p2 p1 p0] */
  /* [d 0 0 0 t9 0 0 0 0 0 c t2 t1 t0] = [p12 p11 p10 p9 0 0 0 0 0 0 p2 p1 p0] */
  c += (ulong)a[0] * b[3]
    + (ulong)a[1] * b[2]
    + (ulong)a[2] * b[1]
    + (ulong)a[3] * b[0];

  d += (ulong)a[4] * b[9]
    + (ulong)a[5] * b[8]
    + (ulong)a[6] * b[7]
    + (ulong)a[7] * b[6]
    + (ulong)a[8] * b[5]
    + (ulong)a[9] * b[4];
  u3 = d & M; d >>= 26; c += u3 * R0;

  /* VERIFY_BITS(c, 64); */
  /* [d u3 0 0 0 t9 0 0 0 0 0 c-u3*R0 t2 t1 t0] = [p13 p12 p11 p10 p9 0 0 0 0 0 p3 p2 p1 p0] */
  t3 = c & M; c >>= 26; c += u3 * R1;

  c += (ulong)a[0] * b[4]
    + (ulong)a[1] * b[3]
    + (ulong)a[2] * b[2]
    + (ulong)a[3] * b[1]
    + (ulong)a[4] * b[0];

  /* [d 0 0 0 0 t9 0 0 0 0 c t3 t2 t1 t0] = [p13 p12 p11 p10 p9 0 0 0 0 p4 p3 p2 p1 p0] */
  d += (ulong)a[5] * b[9]
    + (ulong)a[6] * b[8]
    + (ulong)a[7] * b[7]
    + (ulong)a[8] * b[6]
    + (ulong)a[9] * b[5];

  /* [d 0 0 0 0 t9 0 0 0 0 c t3 t2 t1 t0] = [p14 p13 p12 p11 p10 p9 0 0 0 0 p4 p3 p2 p1 p0] */
  u4 = d & M; d >>= 26; c += u4 * R0;

  /* VERIFY_BITS(c, 64); */
  /* [d u4 0 0 0 0 t9 0 0 0 0 c-u4*R0 t3 t2 t1 t0] = [p14 p13 p12 p11 p10 p9 0 0 0 0 p4 p3 p2 p1 p0] */
  t4 = c & M; c >>= 26; c += u4 * R1;

  /* [d u4 0 0 0 0 t9 0 0 0 c-u4*R1 t4-u4*R0 t3 t2 t1 t0] = [p14 p13 p12 p11 p10 p9 0 0 0 0 p4 p3 p2 p1 p0] */
  /* [d 0 0 0 0 0 t9 0 0 0 c t4 t3 t2 t1 t0] = [p14 p13 p12 p11 p10 p9 0 0 0 0 p4 p3 p2 p1 p0] */

  c += (ulong)a[0] * b[5]
    + (ulong)a[1] * b[4]
    + (ulong)a[2] * b[3]
    + (ulong)a[3] * b[2]
    + (ulong)a[4] * b[1]
    + (ulong)a[5] * b[0];

  /* [d 0 0 0 0 0 t9 0 0 0 c t4 t3 t2 t1 t0] = [p14 p13 p12 p11 p10 p9 0 0 0 p5 p4 p3 p2 p1 p0] */
  d += (ulong)a[6] * b[9]
    + (ulong)a[7] * b[8]
    + (ulong)a[8] * b[7]
    + (ulong)a[9] * b[6];

  /* [d 0 0 0 0 0 t9 0 0 0 c t4 t3 t2 t1 t0] = [p15 p14 p13 p12 p11 p10 p9 0 0 0 p5 p4 p3 p2 p1 p0] */
  u5 = d & M; d >>= 26; c += u5 * R0;

  /* VERIFY_BITS(c, 64); */
  /* [d u5 0 0 0 0 0 t9 0 0 0 c-u5*R0 t4 t3 t2 t1 t0] = [p15 p14 p13 p12 p11 p10 p9 0 0 0 p5 p4 p3 p2 p1 p0] */
  t5 = c & M; c >>= 26; c += u5 * R1;

  /* [d u5 0 0 0 0 0 t9 0 0 c-u5*R1 t5-u5*R0 t4 t3 t2 t1 t0] = [p15 p14 p13 p12 p11 p10 p9 0 0 0 p5 p4 p3 p2 p1 p0] */
  /* [d 0 0 0 0 0 0 t9 0 0 c t5 t4 t3 t2 t1 t0] = [p15 p14 p13 p12 p11 p10 p9 0 0 0 p5 p4 p3 p2 p1 p0] */

  c += (ulong)a[0] * b[6]
    + (ulong)a[1] * b[5]
    + (ulong)a[2] * b[4]
    + (ulong)a[3] * b[3]
    + (ulong)a[4] * b[2]
    + (ulong)a[5] * b[1]
    + (ulong)a[6] * b[0];

  /* [d 0 0 0 0 0 0 t9 0 0 c t5 t4 t3 t2 t1 t0] = [p15 p14 p13 p12 p11 p10 p9 0 0 p6 p5 p4 p3 p2 p1 p0] */
  d += (ulong)a[7] * b[9]
    + (ulong)a[8] * b[8]
    + (ulong)a[9] * b[7];

  /* [d 0 0 0 0 0 0 t9 0 0 c t5 t4 t3 t2 t1 t0] = [p16 p15 p14 p13 p12 p11 p10 p9 0 0 p6 p5 p4 p3 p2 p1 p0] */
  u6 = d & M; d >>= 26; c += u6 * R0;

  /* VERIFY_BITS(c, 64); */
  /* [d u6 0 0 0 0 0 0 t9 0 0 c-u6*R0 t5 t4 t3 t2 t1 t0] = [p16 p15 p14 p13 p12 p11 p10 p9 0 0 p6 p5 p4 p3 p2 p1 p0] */
  t6 = c & M; c >>= 26; c += u6 * R1;

  /* [d u6 0 0 0 0 0 0 t9 0 c-u6*R1 t6-u6*R0 t5 t4 t3 t2 t1 t0] = [p16 p15 p14 p13 p12 p11 p10 p9 0 0 p6 p5 p4 p3 p2 p1 p0] */
  /* [d 0 0 0 0 0 0 0 t9 0 c t6 t5 t4 t3 t2 t1 t0] = [p16 p15 p14 p13 p12 p11 p10 p9 0 0 p6 p5 p4 p3 p2 p1 p0] */

  c += (ulong)a[0] * b[7]
    + (ulong)a[1] * b[6]
    + (ulong)a[2] * b[5]
    + (ulong)a[3] * b[4]
    + (ulong)a[4] * b[3]
    + (ulong)a[5] * b[2]
    + (ulong)a[6] * b[1]
    + (ulong)a[7] * b[0];
  /* VERIFY_BITS(c, 64); */

  /* [d 0 0 0 0 0 0 0 t9 0 c t6 t5 t4 t3 t2 t1 t0] = [p16 p15 p14 p13 p12 p11 p10 p9 0 p7 p6 p5 p4 p3 p2 p1 p0] */
  d += (ulong)a[8] * b[9]
    + (ulong)a[9] * b[8];

  /* [d 0 0 0 0 0 0 0 t9 0 c t6 t5 t4 t3 t2 t1 t0] = [p17 p16 p15 p14 p13 p12 p11 p10 p9 0 p7 p6 p5 p4 p3 p2 p1 p0] */
  u7 = d & M; d >>= 26; c += u7 * R0;

  t7 = c & M; c >>= 26; c += u7 * R1;

  c += (ulong)a[0] * b[8]
    + (ulong)a[1] * b[7]
    + (ulong)a[2] * b[6]
    + (ulong)a[3] * b[5]
    + (ulong)a[4] * b[4]
    + (ulong)a[5] * b[3]
    + (ulong)a[6] * b[2]
    + (ulong)a[7] * b[1]
    + (ulong)a[8] * b[0];
  /* VERIFY_BITS(c, 64); */

  /* [d 0 0 0 0 0 0 0 0 t9 c t7 t6 t5 t4 t3 t2 t1 t0] = [p17 p16 p15 p14 p13 p12 p11 p10 p9 p8 p7 p6 p5 p4 p3 p2 p1 p0] */
  d += (ulong)a[9] * b[9];

  /* [d 0 0 0 0 0 0 0 0 t9 c t7 t6 t5 t4 t3 t2 t1 t0] = [p18 p17 p16 p15 p14 p13 p12 p11 p10 p9 p8 p7 p6 p5 p4 p3 p2 p1 p0] */
  u8 = d & M; d >>= 26; c += u8 * R0;

  /* [d u8 0 0 0 0 0 0 0 0 t9 c-u8*R0 t7 t6 t5 t4 t3 t2 t1 t0] = [p18 p17 p16 p15 p14 p13 p12 p11 p10 p9 p8 p7 p6 p5 p4 p3 p2 p1 p0] */

  r[3] = t3;
  r[4] = t4;
  r[5] = t5;
  r[6] = t6;
  r[7] = t7;
  r[8] = c & M; c >>= 26; c += u8 * R1;
  c   += d * R0 + t9;
  r[9] = c & (M >> 4); c >>= 22; c += d * (R1 << 4);
  d    = c * (R0 >> 4) + t0;
  r[0] = d & M; d >>= 26;
  d   += c * (R1 >> 4) + t1;
  r[1] = d & M; d >>= 26;
  d   += t2;
  r[2] = d;
}

void secp256k1_fe_mul(secp256k1_fe *r, const secp256k1_fe *a, const secp256k1_fe * b) {
  secp256k1_fe_mul_inner(r->n, a->n, b->n);
}

void secp256k1_fe_add(secp256k1_fe *r, const secp256k1_fe *a) {
  r->n[0] += a->n[0];
  r->n[1] += a->n[1];
  r->n[2] += a->n[2];
  r->n[3] += a->n[3];
  r->n[4] += a->n[4];
  r->n[5] += a->n[5];
  r->n[6] += a->n[6];
  r->n[7] += a->n[7];
  r->n[8] += a->n[8];
  r->n[9] += a->n[9];
}

void secp256k1_fe_negate(secp256k1_fe *r, const secp256k1_fe *a, int m) {
  r->n[0] = 0x3FFFC2FUL * 2 * (m + 1) - a->n[0];
  r->n[1] = 0x3FFFFBFUL * 2 * (m + 1) - a->n[1];
  r->n[2] = 0x3FFFFFFUL * 2 * (m + 1) - a->n[2];
  r->n[3] = 0x3FFFFFFUL * 2 * (m + 1) - a->n[3];
  r->n[4] = 0x3FFFFFFUL * 2 * (m + 1) - a->n[4];
  r->n[5] = 0x3FFFFFFUL * 2 * (m + 1) - a->n[5];
  r->n[6] = 0x3FFFFFFUL * 2 * (m + 1) - a->n[6];
  r->n[7] = 0x3FFFFFFUL * 2 * (m + 1) - a->n[7];
  r->n[8] = 0x3FFFFFFUL * 2 * (m + 1) - a->n[8];
  r->n[9] = 0x03FFFFFUL * 2 * (m + 1) - a->n[9];
}

void secp256k1_fe_mul_int(secp256k1_fe *r, int a) {
  r->n[0] *= a;
  r->n[1] *= a;
  r->n[2] *= a;
  r->n[3] *= a;
  r->n[4] *= a;
  r->n[5] *= a;
  r->n[6] *= a;
  r->n[7] *= a;
  r->n[8] *= a;
  r->n[9] *= a;
}

void secp256k1_fe_set_int(secp256k1_fe *r, int a) {
  r->n[0] = a;
  r->n[1] = r->n[2] = r->n[3] = r->n[4] = r->n[5] = r->n[6] = r->n[7] = r->n[8] = r->n[9] = 0;
}

void secp256k1_fe_normalize_var(secp256k1_fe *r) {
  uint t0 = r->n[0];
  uint t1 = r->n[1];
  uint t2 = r->n[2];
  uint t3 = r->n[3];
  uint t4 = r->n[4];
  uint t5 = r->n[5];
  uint t6 = r->n[6];
  uint t7 = r->n[7];
  uint t8 = r->n[8];
  uint t9 = r->n[9];

  /* Reduce t9 at the start so there will be at most a single carry from the first pass */
  uint m;
  uint x = t9 >> 22; t9 &= 0x03FFFFFUL;

  /* The first pass ensures the magnitude is 1, ... */
  t0 += x * 0x3D1UL; t1 += (x << 6);
  t1 += (t0 >> 26); t0 &= 0x3FFFFFFUL;
  t2 += (t1 >> 26); t1 &= 0x3FFFFFFUL;
  t3 += (t2 >> 26); t2 &= 0x3FFFFFFUL; m = t2;
  t4 += (t3 >> 26); t3 &= 0x3FFFFFFUL; m &= t3;
  t5 += (t4 >> 26); t4 &= 0x3FFFFFFUL; m &= t4;
  t6 += (t5 >> 26); t5 &= 0x3FFFFFFUL; m &= t5;
  t7 += (t6 >> 26); t6 &= 0x3FFFFFFUL; m &= t6;
  t8 += (t7 >> 26); t7 &= 0x3FFFFFFUL; m &= t7;
  t9 += (t8 >> 26); t8 &= 0x3FFFFFFUL; m &= t8;

  /* At most a single final reduction is needed; check if the value is >= the field characteristic */
  x = (t9 >> 22) | ((t9 == 0x03FFFFFUL) & (m == 0x3FFFFFFUL)
      & ((t1 + 0x40UL + ((t0 + 0x3D1UL) >> 26)) > 0x3FFFFFFUL));

  if (x) {
    t0 += 0x3D1UL; t1 += (x << 6);
    t1 += (t0 >> 26); t0 &= 0x3FFFFFFUL;
    t2 += (t1 >> 26); t1 &= 0x3FFFFFFUL;
    t3 += (t2 >> 26); t2 &= 0x3FFFFFFUL;
    t4 += (t3 >> 26); t3 &= 0x3FFFFFFUL;
    t5 += (t4 >> 26); t4 &= 0x3FFFFFFUL;
    t6 += (t5 >> 26); t5 &= 0x3FFFFFFUL;
    t7 += (t6 >> 26); t6 &= 0x3FFFFFFUL;
    t8 += (t7 >> 26); t7 &= 0x3FFFFFFUL;
    t9 += (t8 >> 26); t8 &= 0x3FFFFFFUL;

    t9 &= 0x03FFFFFUL;
  }

  r->n[0] = t0; r->n[1] = t1; r->n[2] = t2; r->n[3] = t3; r->n[4] = t4;
  r->n[5] = t5; r->n[6] = t6; r->n[7] = t7; r->n[8] = t8; r->n[9] = t9;
}

void secp256k1_fe_inv(secp256k1_fe *r, const secp256k1_fe *a) {
  secp256k1_fe x2, x3, x6, x9, x11, x22, x44, x88, x176, x220, x223, t1;
  int j;

  secp256k1_fe_sqr(&x2, a);
  secp256k1_fe_mul(&x2, &x2, a);

  secp256k1_fe_sqr(&x3, &x2);
  secp256k1_fe_mul(&x3, &x3, a);

  x6 = x3;
  for (j=0; j<3; j++) {
    secp256k1_fe_sqr(&x6, &x6);
  }
  secp256k1_fe_mul(&x6, &x6, &x3);

  x9 = x6;
  for (j=0; j<3; j++) {
    secp256k1_fe_sqr(&x9, &x9);
  }
  secp256k1_fe_mul(&x9, &x9, &x3);

  x11 = x9;
  for (j=0; j<2; j++) {
    secp256k1_fe_sqr(&x11, &x11);
  }
  secp256k1_fe_mul(&x11, &x11, &x2);

  x22 = x11;
  for (j=0; j<11; j++) {
    secp256k1_fe_sqr(&x22, &x22);
  }
  secp256k1_fe_mul(&x22, &x22, &x11);

  x44 = x22;
  for (j=0; j<22; j++) {
    secp256k1_fe_sqr(&x44, &x44);
  }
  secp256k1_fe_mul(&x44, &x44, &x22);

  x88 = x44;
  for (j=0; j<44; j++) {
    secp256k1_fe_sqr(&x88, &x88);
  }
  secp256k1_fe_mul(&x88, &x88, &x44);

  x176 = x88;
  for (j=0; j<88; j++) {
    secp256k1_fe_sqr(&x176, &x176);
  }
  secp256k1_fe_mul(&x176, &x176, &x88);

  x220 = x176;
  for (j=0; j<44; j++) {
    secp256k1_fe_sqr(&x220, &x220);
  }
  secp256k1_fe_mul(&x220, &x220, &x44);

  x223 = x220;
  for (j=0; j<3; j++) {
    secp256k1_fe_sqr(&x223, &x223);
  }
  secp256k1_fe_mul(&x223, &x223, &x3);

  t1 = x223;
  for (j=0; j<23; j++) {
    secp256k1_fe_sqr(&t1, &t1);
  }
  secp256k1_fe_mul(&t1, &t1, &x22);
  for (j=0; j<5; j++) {
    secp256k1_fe_sqr(&t1, &t1);
  }
  secp256k1_fe_mul(&t1, &t1, a);
  for (j=0; j<3; j++) {
    secp256k1_fe_sqr(&t1, &t1);
  }
  secp256k1_fe_mul(&t1, &t1, &x2);
  for (j=0; j<2; j++) {
    secp256k1_fe_sqr(&t1, &t1);
  }
  secp256k1_fe_mul(r, a, &t1);
}

void secp256k1_fe_get_b32(unsigned char *r, const secp256k1_fe *a) {
  r[0] = (a->n[9] >> 14) & 0xff;
  r[1] = (a->n[9] >> 6) & 0xff;
  r[2] = ((a->n[9] & 0x3F) << 2) | ((a->n[8] >> 24) & 0x3);
  r[3] = (a->n[8] >> 16) & 0xff;
  r[4] = (a->n[8] >> 8) & 0xff;
  r[5] = a->n[8] & 0xff;
  r[6] = (a->n[7] >> 18) & 0xff;
  r[7] = (a->n[7] >> 10) & 0xff;
  r[8] = (a->n[7] >> 2) & 0xff;
  r[9] = ((a->n[7] & 0x3) << 6) | ((a->n[6] >> 20) & 0x3f);
  r[10] = (a->n[6] >> 12) & 0xff;
  r[11] = (a->n[6] >> 4) & 0xff;
  r[12] = ((a->n[6] & 0xf) << 4) | ((a->n[5] >> 22) & 0xf);
  r[13] = (a->n[5] >> 14) & 0xff;
  r[14] = (a->n[5] >> 6) & 0xff;
  r[15] = ((a->n[5] & 0x3f) << 2) | ((a->n[4] >> 24) & 0x3);
  r[16] = (a->n[4] >> 16) & 0xff;
  r[17] = (a->n[4] >> 8) & 0xff;
  r[18] = a->n[4] & 0xff;
  r[19] = (a->n[3] >> 18) & 0xff;
  r[20] = (a->n[3] >> 10) & 0xff;
  r[21] = (a->n[3] >> 2) & 0xff;
  r[22] = ((a->n[3] & 0x3) << 6) | ((a->n[2] >> 20) & 0x3f);
  r[23] = (a->n[2] >> 12) & 0xff;
  r[24] = (a->n[2] >> 4) & 0xff;
  r[25] = ((a->n[2] & 0xf) << 4) | ((a->n[1] >> 22) & 0xf);
  r[26] = (a->n[1] >> 14) & 0xff;
  r[27] = (a->n[1] >> 6) & 0xff;
  r[28] = ((a->n[1] & 0x3f) << 2) | ((a->n[0] >> 24) & 0x3);
  r[29] = (a->n[0] >> 16) & 0xff;
  r[30] = (a->n[0] >> 8) & 0xff;
  r[31] = a->n[0] & 0xff;
}

void secp256k1_ge_from_storage(secp256k1_ge *r,  const secp256k1_ge_storage *a) {
  secp256k1_fe_from_storage(&r->x, &a->x);
  secp256k1_fe_from_storage(&r->y, &a->y);
}

void secp256k1_gej_set_ge(secp256k1_gej *r, const secp256k1_ge *a) {
  r->x = a->x;
  r->y = a->y;
  secp256k1_fe_set_int(&r->z, 1);
}

void secp256k1_gej_add_ge_var(secp256k1_gej *r, const secp256k1_gej *a, const secp256k1_ge *b, secp256k1_fe *rzr) {
  /* 8 mul, 3 sqr, 4 normalize, 12 mul_int/add/negate */
  secp256k1_fe z12, u1, u2, s1, s2, h, i, i2, h2, h3, t;

  secp256k1_fe_sqr(&z12, &a->z);
  u1 = a->x; secp256k1_fe_normalize_weak(&u1);
  secp256k1_fe_mul(&u2, &b->x, &z12);
  s1 = a->y; secp256k1_fe_normalize_weak(&s1);
  secp256k1_fe_mul(&s2, &b->y, &z12); secp256k1_fe_mul(&s2, &s2, &a->z);
  secp256k1_fe_negate(&h, &u1, 1); secp256k1_fe_add(&h, &u2);
  secp256k1_fe_negate(&i, &s1, 1); secp256k1_fe_add(&i, &s2);
  secp256k1_fe_sqr(&i2, &i);
  secp256k1_fe_sqr(&h2, &h);
  secp256k1_fe_mul(&h3, &h, &h2);
  if (rzr) {
    *rzr = h;
  }
  secp256k1_fe_mul(&r->z, &a->z, &h);
  secp256k1_fe_mul(&t, &u1, &h2);
  r->x = t; secp256k1_fe_mul_int(&r->x, 2); secp256k1_fe_add(&r->x, &h3); secp256k1_fe_negate(&r->x, &r->x, 3); secp256k1_fe_add(&r->x, &i2);
  secp256k1_fe_negate(&r->y, &r->x, 5); secp256k1_fe_add(&r->y, &t); secp256k1_fe_mul(&r->y, &r->y, &i);
  secp256k1_fe_mul(&h3, &h3, &s1); secp256k1_fe_negate(&h3, &h3, 1);
  secp256k1_fe_add(&r->y, &h3);
}

void secp256k1_ge_set_gej(secp256k1_ge *r, secp256k1_gej *a) {
  secp256k1_fe z2, z3;
  secp256k1_fe_inv(&a->z, &a->z);
  secp256k1_fe_sqr(&z2, &a->z);
  secp256k1_fe_mul(&z3, &a->z, &z2);
  secp256k1_fe_mul(&a->x, &a->x, &z2);
  secp256k1_fe_mul(&a->y, &a->y, &z3);
  secp256k1_fe_set_int(&a->z, 1);
  r->x = a->x;
  r->y = a->y;
}
extern "C"

void secp256k1(const secp256k1_ge_storage *prec, unsigned char* result)
{
    #pragma HLS INTERFACE m_axi port=prec offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

  secp256k1_ge ge[512];
  secp256k1_gej sum;

  secp256k1_ge_from_storage(&ge[0], &prec[0]);
  secp256k1_gej_set_ge(&sum, &ge[0]);
  secp256k1_fe z_all = sum.z;

  for (uint i=1; i<512; ++i) {
    secp256k1_ge_from_storage(&ge[i], &prec[i]);
    secp256k1_gej_add_ge_var(&sum, &sum, &ge[i], 0);
    secp256k1_fe_mul(&z_all, &z_all, &sum.z);
  }
  secp256k1_fe_inv(&z_all, &z_all);
  secp256k1_fe_get_b32(result, &z_all);
}
