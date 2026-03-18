#include "kernel.h"

// --- from main.cu ---
T2 exp_i( T phi ) {
  return (T2){ cos(phi), sin(phi) };
}

T2 cmplx_mul( T2 a, T2 b ) { return (T2){ a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x }; }

T2 cm_fl_mul( T2 a, T  b ) { return (T2){ b*a.x, b*a.y }; }

T2 cmplx_add( T2 a, T2 b ) { return (T2){ a.x + b.x, a.y + b.y }; }

T2 cmplx_sub( T2 a, T2 b ) { return (T2){ a.x - b.x, a.y - b.y }; }
