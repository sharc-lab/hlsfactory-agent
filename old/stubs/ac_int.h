/*
 * Minimal Catapult ac_int stub
 * For compilation testing with clang++ only - NOT for synthesis
 */

#ifndef __AC_INT_H__
#define __AC_INT_H__

#include <type_traits>

#include "ap_int.h"

template <int W, bool Signed = true>
using ac_int = typename std::conditional<Signed, ap_int<W>, ap_uint<W>>::type;

template <int W>
using ac_uint = ac_int<W, false>;

#endif // __AC_INT_H__
