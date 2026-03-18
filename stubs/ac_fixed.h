/*
 * Minimal Catapult ac_fixed stub
 * For compilation testing with clang++ only - NOT for synthesis
 */

#ifndef __AC_FIXED_H__
#define __AC_FIXED_H__

#include <type_traits>

#include "ap_fixed.h"

#ifndef AC_TRN
#define AC_TRN AP_TRN
#endif

#ifndef AC_RND
#define AC_RND AP_RND
#endif

#ifndef AC_WRAP
#define AC_WRAP AP_WRAP
#endif

#ifndef AC_SAT
#define AC_SAT AP_SAT
#endif

template <int W, int I, bool Signed = true, int Q = AC_TRN, int O = AC_WRAP>
using ac_fixed =
    typename std::conditional<Signed, ap_fixed<W, I, Q, O>, ap_ufixed<W, I, Q, O>>::type;

template <int W, int I, int Q = AC_TRN, int O = AC_WRAP>
using ac_ufixed = ap_ufixed<W, I, Q, O>;

#endif // __AC_FIXED_H__
