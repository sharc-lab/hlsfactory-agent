"""Mechanical Vitis-to-target rewrite: everything the mapping table can do without judgment.

Runs on the host before the agent starts. It copies the design, swaps headers and types, moves
loop pragmas to the target's placement, drops pragmas with no equivalent, inserts the top-function
marker, and writes ``rewrite_log.json``. Anything it cannot do with certainty goes into the log's
``residue`` list for the agent. It never guesses at numerics.
"""

from __future__ import annotations

import json
import os
import re
import shutil
from collections import Counter
from pathlib import Path

from hlsfactory_agent.scan import collect_defines, resolve_int
from hlsfactory_agent.translate import RE_COMMENT, SOURCE_SUFFIXES, TargetSpec, in_class_body, testbench_files

HEADER_MAPS: dict[str, dict[str, str]] = {
    "catapult": {"ap_int.h": "ac_int.h", "ap_fixed.h": "ac_fixed.h", "hls_stream.h": "ac_channel.h"},
    "xlscc": {"ap_int.h": "ac_int.h", "ap_fixed.h": "ac_fixed.h", "hls_stream.h": "xls_emu.h"},
    "bambu": {"ap_int.h": "ac_int.h", "ap_fixed.h": "ac_fixed.h", "hls_stream.h": "ac_channel.h"},
    "dynamatic": {"ap_int.h": "stdint.h", "ap_fixed.h": "stdint.h", "hls_stream.h": "stdint.h"},  # fixed point and streams are blocked before the pre-pass
}
# how a target spells loop and function pragmas; a None form has no equivalent and the pragma is dropped
PRAGMA_FORMS: dict[str, dict[str, str | None]] = {
    "catapult": {"pipeline": "#pragma hls_pipeline_init_interval {v}", "unroll": "#pragma hls_unroll {v}", "unroll_full": "#pragma hls_unroll yes", "inline": "#pragma hls_design inline"},
    "bambu": {"pipeline": None, "unroll": "#pragma HLS unroll factor={v}", "unroll_full": "#pragma HLS unroll", "inline": "#pragma HLS inline", "label_own_line": True},
    "dynamatic": {"pipeline": None, "unroll": "#pragma clang loop unroll_count({v})", "unroll_full": "#pragma clang loop unroll(full)", "inline": None, "label_own_line": True},
}
RE_LABELED_LOOP = re.compile(r"^(\s*)([A-Za-z_]\w*)\s*:\s*((?:for|while)\b.*)$")
STREAM_MAPS: dict[str, str] = {"catapult": "ac_channel<", "xlscc": "__xls_channel<"}
MODE_MAP = {
    "AP_TRN": "AC_TRN",
    "AP_TRN_ZERO": "AC_TRN_ZERO",
    "AP_RND": "AC_RND",
    "AP_RND_ZERO": "AC_RND_ZERO",
    "AP_RND_MIN_INF": "AC_RND_MIN_INF",
    "AP_RND_INF": "AC_RND_INF",
    "AP_RND_CONV": "AC_RND_CONV",
    "AP_WRAP": "AC_WRAP",
    "AP_SAT": "AC_SAT",
    "AP_SAT_ZERO": "AC_SAT_ZERO",
    "AP_SAT_SYM": "AC_SAT_SYM",
}

RE_INCLUDE = re.compile(r'^(\s*#\s*include\s*)([<"])(ap_int\.h|ap_fixed\.h|hls_stream\.h)([>"])')
RE_AP_INT = re.compile(r"\bap_int\s*<\s*([^<>,]+?)\s*>")
RE_AP_UINT = re.compile(r"\bap_uint\s*<\s*([^<>,]+?)\s*>")
RE_AP_FIXED = re.compile(
    r"\b(ap_fixed|ap_ufixed)\s*<\s*([^<>,]+?)\s*,\s*([^<>,]+?)\s*(?:,\s*([A-Z_]+)\s*)?(?:,\s*([A-Z_]+)\s*)?>"
)
RE_STREAM = re.compile(r"\bhls::stream\s*<")
RE_CHANNEL_DEPTH = re.compile(r"\bac_channel\s*<\s*([^,<>]*(?:<[^<>]*>)?[^,<>]*)\s*,\s*[^<>]+>")  # hls::stream<T, depth>: the depth is a directive
RE_PRAGMA = re.compile(r"^(\s*)#\s*pragma\s+HLS\s+([A-Za-z_]+)(.*)$", re.IGNORECASE)
RE_LOOP = re.compile(r"^\s*(?:[A-Za-z_]\w*\s*:\s*)?(for|while)\b")
RE_FN_HEADER_END = re.compile(r"\)\s*(const\s*)?\{?\s*$")
# Methods of ap_int/ap_fixed with no same-named ac_int/ac_fixed equivalent. to_int, to_uint, to_double and the
# *_reduce family exist on both families and are left alone.
RE_VENDOR_METHOD = re.compile(r"\.(range|reverse|length|set_bit|get_bit|bit|test|invert|rrotate|lrotate|concat)\s*\(")
RE_INT_LITERAL = re.compile(r"^\d+$")

# Catapult cannot synthesize std::complex (CIN-15), native float arithmetic (CIN-290) or float math calls
# (CIN-16); its front end does not include <stdint.h> transitively (CRD-20); local channels must be static (HIER-6).
DIALECT_TYPES: dict[str, dict[str, str]] = {
    "catapult": {"std::complex": "ac_complex", "complex": "ac_complex", "float": "ac_ieee_float32", "double": "ac_ieee_float64"},
    "bambu": {"std::complex": "ac_complex", "complex": "ac_complex"},  # Bambu synthesizes native float with its soft libm
}
# hls_math.h declares nothing the target needs: the float math comes from the shims (Catapult) or libm (Bambu)
DIALECT_HEADERS: dict[str, dict[str, str]] = {
    t: {"<complex>": "<ac_complex.h>", '"hls_math.h"': "<cmath>", "<hls_math.h>": "<cmath>"} for t in ("catapult", "bambu")
}
DIALECT_HEADERS["dynamatic"] = {'"hls_math.h"': "<math.h>", "<hls_math.h>": "<math.h>"}
# Dynamatic is C: ap_int<N> becomes the stdint type of that width or a bit-precise _BitInt(N)
STDINT_WIDTHS = ("8", "16", "32")
RE_VITIS_LIB_COPY = re.compile(r"\b_AP_[WIQON]\b")  # the template parameters of a copied ap_int/ap_fixed library header
# Vendor headers with no target equivalent, written next to the design as a local synthesizable stand-in when
# the source includes them. hls::vector is a plain aggregate; hls_fft.h/hls_streamofblocks.h are empty, so
# any use of their APIs still fails the compile check and reaches the agent as residue.
SHIM_HEADERS: dict[str, dict[str, str]] = {
    "catapult": {
        "hls_vector.h": (
            "#pragma once\n"
            "namespace hls {\n"
            "template <typename T, int N> struct vector {\n"
            "    T data[N];\n"
            "    T &operator[](int i) { return data[i]; }\n"
            "    const T &operator[](int i) const { return data[i]; }\n"
            "};\n"
            "}\n"
        ),
        "hls_fft.h": "#pragma once\n",
        "hls_streamofblocks.h": "#pragma once\n",
        # the Vitis AXI-stream side-band struct over ac_int: the include resolves and typedefs of it stay valid, while the
        # ports themselves are plain channels in Catapult. A zero-width field keeps one bit (ac_int<0> is not a type).
        "ap_axi_sdata.h": (
            "#pragma once\n"
            "#include <ac_int.h>\n"
            "template <int WData, int WUser = 0, int WId = 0, int WDest = 0> struct ap_axis {\n"
            "    ac_int<WData, true> data;\n"
            "    ac_int<(WData / 8 > 0 ? WData / 8 : 1), false> keep, strb;\n"
            "    ac_int<(WUser > 0 ? WUser : 1), false> user;\n"
            "    ac_int<1, false> last;\n"
            "    ac_int<(WId > 0 ? WId : 1), false> id;\n"
            "    ac_int<(WDest > 0 ? WDest : 1), false> dest;\n"
            "};\n"
            "template <int WData, int WUser = 0, int WId = 0, int WDest = 0> struct ap_axiu {\n"
            "    ac_int<WData, false> data;\n"
            "    ac_int<(WData / 8 > 0 ? WData / 8 : 1), false> keep, strb;\n"
            "    ac_int<(WUser > 0 ? WUser : 1), false> user;\n"
            "    ac_int<1, false> last;\n"
            "    ac_int<(WId > 0 ? WId : 1), false> id;\n"
            "    ac_int<(WDest > 0 ? WDest : 1), false> dest;\n"
            "};\n"
        ),
        "cat_abs.h": "#pragma once\n// Catapult has no synthesizable body for the C library abs(); the pre-pass rewrites abs( to cat_abs(.\ntemplate <typename T> inline T cat_abs(const T &x) { return x < 0 ? T(-x) : x; }\n",
        "ac_float_trig.h": '#pragma once\n// Catapult synthesizes no trig on its float types: fixed-point CORDIC from ac_math, converted at the boundary.\n// Angle in radians, |x| < 2^15; about 1e-9 absolute error. Written next to the design by the translation pre-pass.\n#include <ac_std_float.h>\n#include <ac_math/ac_sincos_cordic.h>\n#include <ac_math/ac_sqrt.h>\nnamespace ac_float_trig {\ntypedef ac_fixed<48, 16, true, AC_RND, AC_SAT> angle_t;\ntypedef ac_fixed<40, 2, true> unit_t;\nstatic const ac_fixed<48, 2, true> INV_PI = 0.31830988618379067154;\ntemplate <typename F> inline F sin_of(const F &x) {\n    angle_t a = x.template convert_to_ac_fixed<48, 16, true, AC_RND, AC_SAT>() * INV_PI;\n    unit_t s;\n    ac_math::ac_sin_cordic(a, s);\n    return F(s);\n}\ntemplate <typename F> inline F cos_of(const F &x) {\n    angle_t a = x.template convert_to_ac_fixed<48, 16, true, AC_RND, AC_SAT>() * INV_PI;\n    unit_t c;\n    ac_math::ac_cos_cordic(a, c);\n    return F(c);\n}\n}\ninline ac_ieee_float32 sin(const ac_ieee_float32 &x) { return ac_float_trig::sin_of(x); }\ninline ac_ieee_float32 cos(const ac_ieee_float32 &x) { return ac_float_trig::cos_of(x); }\ninline ac_ieee_float64 sin(const ac_ieee_float64 &x) { return ac_float_trig::sin_of(x); }\ninline ac_ieee_float64 cos(const ac_ieee_float64 &x) { return ac_float_trig::cos_of(x); }\ninline ac_ieee_float32 sinf(const ac_ieee_float32 &x) { return sin(x); }\ninline ac_ieee_float32 cosf(const ac_ieee_float32 &x) { return cos(x); }\nnamespace ac_float_trig {\ntemplate <typename F> inline F sqrt_of(const F &x) {\n    F r;\n    ac_math::ac_sqrt(x, r);  // ac_math has the float overload; a fixed-point detour overflowed past 2^16 (NaN in the testbench)\n    return r;\n}\n}\ninline ac_ieee_float32 sqrt(const ac_ieee_float32 &x) { return ac_float_trig::sqrt_of(x); }\ninline ac_ieee_float64 sqrt(const ac_ieee_float64 &x) { return ac_float_trig::sqrt_of(x); }\ninline ac_ieee_float32 sqrtf(const ac_ieee_float32 &x) { return sqrt(x); }\n',
        "ac_float_exp.h": '#pragma once\n// Catapult synthesizes no exp/log/pow on its float types: hyperbolic CORDIC from ac_math, which takes the float\n// types directly (30 fractional bits in the temporaries). Written next to the design by the translation pre-pass.\n#include <ac_std_float.h>\n#include <ac_math/ac_hcordic.h>\nnamespace ac_float_exp {\ntemplate <typename F> inline F exp_of(const F &x) { F z; ac_math::ac_exp_cordic<true, 30>(x, z); return z; }\ntemplate <typename F> inline F log_of(const F &x) { F z; ac_math::ac_log_cordic<true, 30>(x, z); return z; }\ntemplate <typename F> inline F pow_of(const F &a, const F &b) { F z; ac_math::ac_pow_cordic<true, 30>(a, b, z); return z; }\n}\ninline ac_ieee_float32 exp(const ac_ieee_float32 &x) { return ac_float_exp::exp_of(x); }\ninline ac_ieee_float64 exp(const ac_ieee_float64 &x) { return ac_float_exp::exp_of(x); }\ninline ac_ieee_float32 log(const ac_ieee_float32 &x) { return ac_float_exp::log_of(x); }\ninline ac_ieee_float64 log(const ac_ieee_float64 &x) { return ac_float_exp::log_of(x); }\ninline ac_ieee_float32 pow(const ac_ieee_float32 &a, const ac_ieee_float32 &b) { return ac_float_exp::pow_of(a, b); }\ninline ac_ieee_float64 pow(const ac_ieee_float64 &a, const ac_ieee_float64 &b) { return ac_float_exp::pow_of(a, b); }\ninline ac_ieee_float32 expf(const ac_ieee_float32 &x) { return exp(x); }\ninline ac_ieee_float32 logf(const ac_ieee_float32 &x) { return log(x); }\ninline ac_ieee_float32 powf(const ac_ieee_float32 &a, const ac_ieee_float32 &b) { return pow(a, b); }\n',
        "cat_bytes.h": (
            "#pragma once\n"
            "// Catapult rejects memcpy/memset and reinterpret_cast between unrelated types (CIN-71). Typed equivalents:\n"
            "// a copy between arrays of one type is an element loop; a copy between an array and wide ac_int words packs\n"
            "// or unpacks each element's bits at a stride of 8*sizeof(T), the layout memcpy gives on the host. The byte\n"
            "// count must be a constant. cat_store<V>(p, v) and cat_load<V>(p) stand for a store and a load through a pointer\n"
            "// cast to V. Written next to the design by the translation pre-pass.\n"
            "#include <cstddef>\n#include <type_traits>\n#include <ac_int.h>\n#include <ac_fixed.h>\n#include <ac_std_float.h>\n"
            "namespace cat_bytes {\n"
            "// raw bits of one element; overloads, so a class derived from a float type deduces its base\n"
            "template <class T> inline typename std::enable_if<std::is_integral<T>::value, ac_int<8 * sizeof(T), false> >::type get(const T &x) { return x; }\n"
            "template <int N, bool S> inline ac_int<N, false> get(const ac_int<N, S> &x) { return x; }\n"
            "template <int N, int I, bool S, ac_q_mode Q, ac_o_mode O> inline ac_int<N, false> get(const ac_fixed<N, I, S, Q, O> &x) { return x.template slc<N>(0); }\n"
            "template <ac_ieee_float_format F> inline ac_int<ac_ieee_float<F>::width, false> get(const ac_ieee_float<F> &x) { return x.data_ac_int(); }\n"
            "template <class T, int W> inline typename std::enable_if<std::is_integral<T>::value>::type set(T &x, const ac_int<W, false> &b) { x = (T)b.to_uint64(); }\n"
            "template <int N, bool S, int W> inline void set(ac_int<N, S> &x, const ac_int<W, false> &b) { x = b; }\n"
            "template <int N, int I, bool S, ac_q_mode Q, ac_o_mode O, int W> inline void set(ac_fixed<N, I, S, Q, O> &x, const ac_int<W, false> &b) { x.set_slc(0, b); }\n"
            "template <ac_ieee_float_format F, int W> inline void set(ac_ieee_float<F> &x, const ac_int<W, false> &b) { x.set_data(ac_int<ac_ieee_float<F>::width, true>(b)); }\n"
            "}\n"
            "template <class T> inline void cat_memset0(T *p, std::size_t bytes) {\n"
            "    for (std::size_t i = 0; i < bytes / sizeof(T); i++) p[i] = T(0);\n"
            "}\n"
            "template <class T> inline void cat_memcpy(T *d, const T *s, std::size_t bytes) {\n"
            "    for (std::size_t i = 0; i < bytes / sizeof(T); i++) d[i] = s[i];\n"
            "}\n"
            "template <int W, bool S> inline void cat_memcpy(ac_int<W, S> *d, const ac_int<W, S> *s, std::size_t bytes) {\n"
            "    for (std::size_t i = 0; i < bytes / sizeof(ac_int<W, S>); i++) d[i] = s[i];\n"
            "}\n"
            "template <class T, int W, bool S> inline void cat_memcpy(ac_int<W, S> *d, const T *s, std::size_t bytes) {  // pack\n"
            "    for (std::size_t i = 0; i < bytes / sizeof(T); i++)\n"
            "        d[i * 8 * sizeof(T) / W].set_slc(i * 8 * sizeof(T) % W, cat_bytes::get(s[i]));\n"
            "}\n"
            "template <class T, int W, bool S> inline void cat_memcpy(T *d, const ac_int<W, S> *s, std::size_t bytes) {  // unpack\n"
            "    typedef decltype(cat_bytes::get(*d)) B;\n"
            "    for (std::size_t i = 0; i < bytes / sizeof(T); i++)\n"
            "        cat_bytes::set(d[i], s[i * 8 * sizeof(T) / W].template slc<B::width>(i * 8 * sizeof(T) % W));\n"
            "}\n"
            "template <class V, class T> inline void cat_store(T *p, const V &v) { cat_memcpy(p, &v, sizeof(V)); }\n"
            "template <class V, class T> inline V cat_load(const T *p) { V v; cat_memcpy(&v, p, sizeof(V)); return v; }\n"
        ),
    }
}
SHIM_HEADERS["bambu"] = {k: v for k, v in SHIM_HEADERS["catapult"].items() if k.startswith("hls_")}  # float math is native there
RE_ANY_INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]')
RE_DIALECT_TYPE = re.compile(r"\b(std::complex|complex|float|double)\b")
RE_CONST_TABLE = re.compile(r"\b(?:const\s+)?(float|double)\b[^=;]*\[[^=;]*=\s*\{|\bconst\s+(float|double)\b[^=;]*\[|\bconstexpr\s+(float|double)\b")  # a literal table or constexpr stays native
RE_MAIN = re.compile(r"\bint\s+main\s*\(")
RE_SDS_PRAGMA = re.compile(r"^\s*#\s*pragma\s+SDS\b", re.I)  # SDSoC pragmas have no Catapult meaning
RE_FLOAT_TYPEDEF = re.compile(r"\b(?:typedef\s+(?:float|double)\s+([A-Za-z_]\w*)|using\s+([A-Za-z_]\w*)\s*=\s*(?:float|double))\s*;")
# a bare float literal as a ternary branch: `T v = c ? x : 0.0;` mixes the float type with double (CRD-42)
RE_TERNARY_LITERAL = re.compile(r"(\?\s*|:\s*)(-?\d+\.\d*(?:[eE][-+]?\d+)?|-?\d+[eE][-+]?\d+)(?=\s*[:;])")
RE_FLOAT_MATH = re.compile(r"\b(tan|atan2?)f?\s*\(")  # cos/sin/sqrt and exp/log/pow come from the ac_float_trig and ac_float_exp shims
RE_STD_TRIG = re.compile(r"\bstd::((?:cos|sin|sqrt)f?)\s*\(")
# byte-level idioms Catapult rejects (CIN-71), mapped to the typed helpers in cat_bytes.h
RE_MEMSET0 = re.compile(r"\b(?:std::)?memset\s*\(\s*([^,]+?)\s*,\s*0\s*,")
RE_MEMCPY = re.compile(r"\b(?:std::)?memcpy\s*\(")
RE_CAST_STORE = re.compile(r"\*\s*reinterpret_cast\s*<\s*((?:[^<>]|<[^<>]*>)+?)\s*\*\s*>\s*\(([^()]+)\)\s*=\s*([^;]+);")
RE_CAST_LOAD = re.compile(r"\*\s*reinterpret_cast\s*<\s*((?:[^<>]|<[^<>]*>)+?)\s*\*\s*>\s*\(([^()]+)\)")
RE_VOID_PTR_FN = re.compile(r"^(\s*)((?:(?:inline|static)\s+)*[A-Za-z_][\w:<>]*\s+[A-Za-z_]\w*\s*\()([^()]*\bvoid\s*\*[^()]*)\)")
RE_ABS = re.compile(r"\b(?:std::)?abs\s*\(")  # the C library abs has no synthesizable body (CIN-16); cat_abs.h replaces it
RE_FLOAT_LITERAL_DECL = re.compile(r"(?<![\w.])(ac_ieee_float(?:32|64))\s+(\w+)\s*=\s*(-?\d+(?:\.\d*)?(?:[eE][-+]?\d+)?[fF]?)\s*;")
RE_UNSIZED_PARAM = re.compile(r"\b([A-Za-z_]\w*)\s*\[\s*(?:/\*.*?\*/)?\s*\]")  # `a[]` or `a[/*2 * n*/]`
RE_POINTER_PARAM = re.compile(r"^(?:const\s+)?[A-Za-z_][\w:<>, ]*?\s*\*\s*([A-Za-z_]\w*)$")  # `T *x`, a pointer port
RE_BIT_RANGE = re.compile(r"\.range\(\s*([^,()]+?)\s*,\s*\1\s*\)")  # x.range(i, i) is the single bit x[i]
# `ac_ieee_floatNN v = <native arithmetic>;` — Catapult cannot convert a folded constant to double (CIN-291),
# so each operand is wrapped in the float type and the arithmetic happens on it. Binary64 only: it keeps the C
# double semantics; a float32 declaration would change precision and is left as residue.
RE_FLOAT_DECL = re.compile(r"^(\s*)(ac_ieee_float(?:32|64))(\s+\w+\s*=\s*)([^;(){}\[\],\"']*[-+*/][^;(){}\[\],\"']*);(\s*)$")
RE_OPERAND = re.compile(r"\b(\d+\.?\d*(?:[eE][-+]?\d+)?|[A-Za-z_]\w*)\b")
RE_TYPED_LITERAL = re.compile(r"\d\.?\d*[fFuUlL]\b")
# a local channel, possibly an array, possibly with the hls::stream name argument ac_channel has no constructor for
RE_LOCAL_CHANNEL = re.compile(r"^(\s+)(ac_channel\s*<(?:[^;&()]|\([^()]*\))*>\s+[A-Za-z_]\w*(?:\s*\[[^\]]+\])*)\s*(?:\(\s*\"[^\"]*\"\s*\))?\s*;")
RE_MACRO_CHANNEL = re.compile(r"^(\s*#\s*define\s+\w+(?:\([^)]*\))?\s+)(ac_channel\s*<(?:[^;&()]|\([^()]*\))*>\s+[\w#]+\s*;)")
# a name that stands for a channel type: `#define stream_t(x) hls::stream<x>`, `typedef ac_channel<T> ch_t;`
RE_CHANNEL_ALIAS = re.compile(
    r"^\s*(?:#\s*define\s+(\w+)\s*\([^)]*\)\s*(?:ac_channel|hls::stream)\s*<"
    r"|typedef\s+(?:ac_channel|hls::stream)\s*<.*>\s*(\w+)\s*;"
    r"|using\s+(\w+)\s*=\s*(?:ac_channel|hls::stream)\s*<)",
    re.M,
)


DROP_INCLUDES = ("gmp.h", "gmpxx.h")  # host-only libraries the Vitis flow links for simulation; nothing to synthesize


def _resolve_include(line: str, path: str, p: Path, dir_out: Path) -> str:
    """A header the extraction copied next to the sources is still included by its repository-relative path
    (`../../include/x.h`), which the compiler cannot find; the include takes the path the file now has."""
    if path in DROP_INCLUDES:
        return f"// {line.strip()}  // host-only library, not synthesized"
    name = Path(path).name
    if "/" in path and not (p.parent / path).exists():
        found = next((c for c in (p.parent / name, dir_out / name) if c.exists()), None)
        if found is not None:
            return line.replace(path, os.path.relpath(found, p.parent).replace(os.sep, "/"), 1)
    return line


def channel_aliases(texts) -> list[str]:
    return sorted({n for t in texts for m in RE_CHANNEL_ALIAS.finditer(t) for n in m.groups() if n})


def _local_alias_channel(aliases: list[str]) -> re.Pattern | None:
    """A local declaration through a channel alias, `stream_t(T) s[N];` or `ch_t s;`, which needs `static` too."""
    if not aliases:
        return None
    return re.compile(r"^(\s+)((?:" + "|".join(map(re.escape, aliases)) + r")\s*(?:\((?:[^;()&]|\([^()]*\))*\))?\s+[A-Za-z_]\w*(?:\s*\[[^\]]+\])*)\s*;")
LOOKBACK_LINES = 3
SIGNATURE_LINES = 8


RE_DATAFLOW = re.compile(r"#\s*pragma\s+HLS\s+dataflow\b", re.I)
# a macro whose body is a dataflow pragma, e.g. `#define DF PRAGMA( hls dataflow )`
RE_DATAFLOW_MACRO = re.compile(r"(?m)^[ \t]*#\s*define\s+(\w+)(?:\([^)]*\))?[ \t]+[^\n]*\bHLS\s+dataflow\b", re.I)
RE_INLINE_ON = re.compile(r"#\s*pragma\s+HLS\s+inline\s*(?://.*)?$", re.I | re.M)
RE_FN_DEF = re.compile(r"(?m)^[ \t]*(?:template\s*<[^>]*>\s*)?[A-Za-z_][\w:<>,\s\*&]*?\b([A-Za-z_]\w*)\s*\(([^;{}]*)\)\s*\{")
BLOCK_MARKER = "#pragma hls_design block"
KEYWORDS = {"if", "for", "while", "switch", "else", "return", "catch", "sizeof"}


def _has_memory_param(params: str) -> bool:
    """An array or pointer parameter, with template argument lists removed so `UF*2` is not a pointer."""
    while (stripped := re.sub(r"<[^<>]*>", "", params)) != params:
        params = stripped
    return re.search(r"[\[*]", params) is not None


def _body_end(text: str, start: int) -> int:
    depth = 0
    for j in range(text.index("{", start), len(text)):
        depth += text[j] == "{"
        depth -= text[j] == "}"
        if depth == 0:
            return j
    return len(text)


def dataflow_blocks(dir_in: Path, top: str | None) -> set[str]:
    """Functions called from a `#pragma HLS dataflow` region, or a macro expanding to one, that take only
    channels and scalars. Catapult blocks may share nothing but channels (ASM-35), so a stage that receives
    an array or returns a value stays inline; a stage with its own `#pragma HLS inline` stays inline too.
    Stages may be defined in any design file."""
    files = [p for p in sorted(Path(dir_in).rglob("*")) if p.is_file() and p.suffix.lower() in SOURCE_SUFFIXES and p.name != "testbench.cpp"]
    texts = [p.read_text(encoding="utf-8", errors="replace") for p in files]
    macros = sorted({m.group(1) for t in texts for m in RE_DATAFLOW_MACRO.finditer(t)})
    marker = RE_DATAFLOW
    if macros:
        uses = r"^(?![ \t]*#\s*define)[^\n]*?\b(?:" + "|".join(macros) + r")\b"
        marker = re.compile(RE_DATAFLOW.pattern + "|" + uses, re.I | re.M)
    per_file, params, bodies = [], {}, {}
    for text in texts:
        found = [m for m in RE_FN_DEF.finditer(text) if m.group(1) not in KEYWORDS]
        defs = {m.group(1): (m.start(), m.group(2), re.search(r"\bvoid\b", text[m.start() : m.start(1)])) for m in found}
        spans = {n: (s, _body_end(text, s) + 1) for n, (s, _, _) in defs.items()}
        per_file.append((text, spans))
        for n, (s, e) in spans.items():
            if defs[n][2]:  # a block returns void
                params.setdefault(n, defs[n][1])
            bodies.setdefault(n, text[s:e])
    blocks: set[str] = set()
    for text, spans in per_file:
        for m in marker.finditer(text):
            inside = [n for n, (s, e) in spans.items() if s < m.start() < e]
            if not inside:
                continue
            region = max(inside, key=lambda n: spans[n][0])
            s, e = spans[region]
            for c in re.findall(r"\b([A-Za-z_]\w*)\s*(?:<(?:[^;()]|\([^()]*\))*>)?\s*\(", text[s:e]):
                if c in params and c not in (region, top) and not _has_memory_param(params[c]) and not RE_INLINE_ON.search(bodies[c]):
                    blocks.add(c)
    return blocks


RE_ANON_NAMESPACE = re.compile(r"^(\s*)namespace\s*\{")
# what an anonymous namespace in a header may hold and still be safely named: no object or out-of-line
# function definitions, which would then be defined once per translation unit
RE_LINKAGE_FREE = re.compile(r"^\s*(?:template\b|struct\b|class\b|union\b|enum\b|using\b|typedef\b|inline\b|constexpr\b|static\s+constexpr\b|static_assert\b|#|//|/\*|\*|\}|$)")


def _name_anonymous_namespaces(lines: list[str], log: dict, rel: str) -> list[str]:
    """An anonymous namespace in a header gives each translation unit its own copy of the types in it, so a
    class outside it that uses them differs between units; Catapult compiles every unit of a design together
    and rejects that (CRD-1066). A named inline namespace keeps the unqualified lookup and makes it one type."""
    out = list(lines)
    name = "anon_" + re.sub(r"\W", "_", Path(rel).stem)
    for i, line in enumerate(lines):
        m = RE_ANON_NAMESPACE.match(line)
        if not m:
            continue
        depth, start, safe = 0, True, True
        for l in lines[i:]:
            code = re.sub(r'"(?:\\.|[^"\\])*"', '""', l.split("//", 1)[0])
            if depth == 1 and start and code.strip() and not RE_LINKAGE_FREE.match(code):
                safe = False
                break
            if depth == 1 and code.strip() and not code.lstrip().startswith("#"):
                start = False
            for ch in code:
                depth += (ch == "{") - (ch == "}")
            if depth == 1 and code.rstrip().endswith((";", "}")):
                start = True
            if depth <= 0:
                break
        if safe and depth <= 0:
            out[i] = RE_ANON_NAMESPACE.sub(rf"\1inline namespace {name} {{", line, count=1)
            log["types"].append({"file": rel, "line": i + 1, "before": line.strip(), "after": out[i].strip()})
    return out


def _insert_block_markers(lines: list[str], blocks: set[str], log: dict, rel: str) -> list[str]:
    for name in sorted(blocks):
        rx = _fn_def_regex(name)
        for idx in range(len(lines)):
            if not rx.match("\n".join(lines[idx : idx + SIGNATURE_LINES])):
                continue
            while idx < len(lines) - 1 and not lines[idx].strip():
                idx += 1
            at = idx - 1 if idx and lines[idx - 1].lstrip().startswith("template") else idx
            if at and lines[at - 1].lstrip().startswith("#pragma hls_design"):
                break
            lines.insert(at, BLOCK_MARKER)
            log["blocks"].append({"file": rel, "line": at + 1, "function": name})
            break
    return lines


def _fn_def_regex(top: str) -> re.Pattern:
    # a definition: return type, the top name, an argument list that may span lines, then the opening brace
    return re.compile(r"^\s*[A-Za-z_][\w:<>,\s\*&]*?\b" + re.escape(top) + r"\s*\([^;{}]*\)\s*\{")


def _c_int(m: re.Match, signed: bool) -> str:
    n = m.group(1).strip()
    if n in STDINT_WIDTHS:
        return f"{'' if signed else 'u'}int{n}_t"
    return f"{'' if signed else 'unsigned '}_BitInt({n})"


# Dynamatic captures the test program's stdout as the profiling vectors (one line per kernel argument), so
# `main()` may write nothing there; the rewrite moves every print to stderr and leaves `puts`/`putchar` to the check.
RE_STDOUT = (
    (re.compile(r"(?<![\w.])printf\s*\("), "fprintf(stderr, "),
    (re.compile(r"\bstd::cout\b"), "std::cerr"),
    (re.compile(r"(?<![\w:])cout\b"), "cerr"),
    (re.compile(r"\bstdout\b"), "stderr"),
)


def redirect_stdout(line: str) -> str:
    for rx, rep in RE_STDOUT:
        line = rx.sub(rep, line)
    return line


def _rewrite_types(line: str, target: TargetSpec, log: dict, rel: str, i: int) -> str:
    before = line
    if target.name == "dynamatic":
        line = RE_AP_INT.sub(lambda m: _c_int(m, True), line)
        line = RE_AP_UINT.sub(lambda m: _c_int(m, False), line)
    else:
        line = RE_AP_INT.sub(r"ac_int<\1, true>", line)
        line = RE_AP_UINT.sub(r"ac_int<\1, false>", line)

    def fixed(m: re.Match) -> str:
        signed = "true" if m.group(1) == "ap_fixed" else "false"
        parts = [m.group(2), m.group(3), signed]
        for mode in (m.group(4), m.group(5)):
            if not mode:
                continue
            if mode in MODE_MAP:
                parts.append(MODE_MAP[mode])
            else:
                log["residue"].append(
                    {"file": rel, "line": i, "reason": f"no {target.display_name} mapping for fixed-point mode {mode}; numerics must be preserved"}
                )
                parts.append(mode)
        return f"ac_fixed<{', '.join(parts)}>"

    line = RE_AP_FIXED.sub(fixed, line)
    line = RE_STREAM.sub(STREAM_MAPS.get(target.name, "ac_channel<"), line)
    line = RE_CHANNEL_DEPTH.sub(r"ac_channel<\1>", line)  # ac_channel takes one template argument; FIFO_DEPTH comes from run.tcl
    if line != before:
        log["types"].append({"file": rel, "line": i, "before": before.strip(), "after": line.strip()})
    m = RE_VENDOR_METHOD.search(line)
    if m and not line.lstrip().startswith("//") and not (m.group(0).startswith(".range") and RE_BIT_RANGE.search(line)):
        # a single-bit .range(i, i) is rewritten mechanically later and is not residue
        log["residue"].append({"file": rel, "line": i, "text": line.strip(), "reason": f"vendor method needs manual translation: {m.group(0)}"})
    return line


def float_typedefs(dir_in: Path) -> set[str]:
    """Names typedef'd to float/double in the DUT: declarations of these types need the literal rule too."""
    names: set[str] = set()
    for p in sorted(Path(dir_in).rglob("*")):
        if p.is_file() and p.suffix.lower() in SOURCE_SUFFIXES and p.name != "testbench.cpp":
            names |= {a or b for a, b in RE_FLOAT_TYPEDEF.findall(p.read_text(encoding="utf-8", errors="replace"))}
    return names


def _rewrite_dialect(lines: list[str], target: TargetSpec, log: dict, rel: str, testbench: bool, alias_channel: re.Pattern | None = None) -> list[str]:
    """Constructs the target's synthesizer rejects but a host compiler accepts. The testbench keeps host
    types (it is compiled, never synthesized); mixing at the DUT boundary is left to the agent as residue."""
    types = DIALECT_TYPES.get(target.name, {})
    if not types:  # a target with header swaps only (Dynamatic: hls_math.h -> math.h)
        out = []
        for i, line in enumerate(lines, start=1):
            for h, new_h in DIALECT_HEADERS.get(target.name, {}).items():
                if line.lstrip().startswith("#") and h in line:
                    log["headers"].append({"file": rel, "line": i, "before": line.strip(), "after": line.replace(h, new_h).strip()})
                    line = line.replace(h, new_h)
                    break
            out.append(line)
        return out
    catapult = target.name == "catapult"  # the float shims, cat_abs and static local channels are Catapult rules
    out: list[str] = []
    for i, line in enumerate(lines, start=1):
        code = line.split("//", 1)[0]
        if testbench:
            if log.get("dialect_dut_changed") and RE_DIALECT_TYPE.search(code):
                log["residue"].append({"file": rel, "line": i, "reason": "testbench uses std::complex/float; the DUT now uses ac_complex/ac_ieee_float32, convert at the boundary with .real()/.imag() or a cast"})
                break
            continue
        for h, new_h in DIALECT_HEADERS.get(target.name, {}).items():
            if re.match(r"\s*#\s*include\s*" + re.escape(h), line):
                line = line.replace(h, new_h)
        directive = line.lstrip().startswith("#") and not re.match(r"\s*#\s*define\b", line)  # a #define'd type is a type
        if not RE_CONST_TABLE.search(code) and not directive:
            new = RE_STD_TRIG.sub(r"\1(", RE_DIALECT_TYPE.sub(lambda m: types.get(m.group(1), m.group(0)), line))
            if new != line:
                log["types"].append({"file": rel, "line": i, "before": line.strip(), "after": new.strip()})
                log["dialect_dut_changed"] = True
                line = new
        m = (RE_LOCAL_CHANNEL.match(line) or (alias_channel and alias_channel.match(line))) if catapult else None
        if m and not in_class_body(lines, i - 1):
            line = f"{m.group(1)}static {m.group(2)};"
            log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": line.strip()})
        if catapult and RE_MACRO_CHANNEL.match(line):  # a macro that declares a channel expands inside a function
            line = RE_MACRO_CHANNEL.sub(r"\1static \2", line)
            log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": line.strip()})
        if RE_BIT_RANGE.search(code):
            line = RE_BIT_RANGE.sub(r"[\1]", line)
            log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": line.strip()})
        if catapult and RE_ABS.search(code):
            line = RE_ABS.sub("cat_abs(", line)
            log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": line.strip()})
        if catapult and re.search(r"\b(memset|memcpy|reinterpret_cast)\b|\bvoid\s*\*", code) and not line.lstrip().startswith("#"):
            new = RE_CAST_LOAD.sub(r"cat_load<\1>(\2)", RE_CAST_STORE.sub(r"cat_store<\1>(\2, \3);", line))
            new = RE_MEMCPY.sub("cat_memcpy(", RE_MEMSET0.sub(r"cat_memset0(\1,", new))
            m = RE_VOID_PTR_FN.match(new)
            if m:  # a void* parameter erases the element type the typed copies need: it becomes a template parameter
                params, k = m.group(3), 0
                while re.search(r"\bvoid\s*\*", params):
                    params, k = re.sub(r"\bvoid\s*\*", f"P{k} *", params, count=1), k + 1
                new = f"{m.group(1)}template <{', '.join(f'class P{j}' for j in range(k))}> {m.group(2)}{params}){new[m.end():]}"
            if new != line:
                log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": new.strip()})
                line = new
        lit_rx = log.get("_float_literal_rx") or RE_FLOAT_LITERAL_DECL
        if lit_rx.search(line.split("//", 1)[0]) and not line.lstrip().startswith("#"):  # after the type rewrite
            # the float types' constructors are explicit: copy-initialization from a literal does not compile (CRD-415)
            line = lit_rx.sub(r"\1 \2(\3);", line)
            log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": line.strip()})
        decl = re.match(r"\s*" + lit_rx.pattern.split(r"\s+(\w+)", 1)[0] + r"\s+\w+\s*=[^;]*\?", line)  # the literal rule's type
        if catapult and decl and not line.lstrip().startswith("#"):
            new = RE_TERNARY_LITERAL.sub(lambda t: f"{t.group(1)}{decl.group(1).replace('const', '').strip()}({t.group(2)})", line)
            if new != line:
                log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": new.strip()})
                line = new
        if RE_SDS_PRAGMA.match(line):
            log["pragmas_dropped"].append({"file": rel, "line": i, "kind": "sds", "before": line.strip(), "category": "absent", "reason": "SDSoC pragma, no target equivalent"})
            continue
        m = RE_FLOAT_DECL.match(line)
        if m and not line.lstrip().startswith("#"):
            if m.group(2) == "ac_ieee_float64" and not RE_TYPED_LITERAL.search(m.group(4)):
                rhs = RE_OPERAND.sub(lambda o: f"{m.group(2)}({o.group(1)})", m.group(4))
                line = f"{m.group(1)}{m.group(2)}{m.group(3)}{rhs};{m.group(5)}"
                log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": line.strip()})
            else:
                log["residue"].append({"file": rel, "line": i, "text": line.strip(), "reason": f"native float arithmetic feeding a {m.group(2)} is not synthesizable in {target.display_name} (CIN-291): wrap each operand in {m.group(2)}(...) or fold it to a table"})
        if catapult and RE_FLOAT_MATH.search(code):
            log["residue"].append({"file": rel, "line": i, "text": line.strip(), "reason": f"no synthesizable float math in {target.display_name}: {RE_FLOAT_MATH.search(code).group(0)}) — constant-fold to a table or use a fixed-point ac_math function (cos/sin/sqrt come from ac_float_trig.h, exp/log/pow from ac_float_exp.h)"})
        out.append(line)
    if testbench:
        return lines
    text = "\n".join(out)
    code = RE_COMMENT.sub("", text)
    for needed, inc in reversed(target.required_includes):
        if re.search(needed, code) and inc not in text and inc.replace("<stdint.h>", "<cstdint>") not in text:
            out.insert(0, inc)
            log["headers"].append({"file": rel, "line": 1, "before": "", "after": inc})
    return out


def _find_back(out: list[str], predicate) -> int | None:
    j = len(out) - 1
    while j >= 0 and j >= len(out) - LOOKBACK_LINES:
        if predicate(out[j]):
            return j
        j -= 1
    return None


def _category_for(kind: str, rest: str, target: TargetSpec) -> str:
    kind_l = kind.lower()
    has_factor = "factor" in rest.lower()
    for rule in target.pragma_rules:
        src = rule.source.lower()
        if f"hls {kind_l}" not in src:
            continue
        if kind_l == "unroll" and ("factor" in src) != has_factor:
            continue
        return rule.category
    return "absent"


def _rewrite_pragmas(lines: list[str], target: TargetSpec, log: dict, rel: str, defines: dict[str, str] | None = None) -> list[str]:
    out: list[str] = []
    defines = defines or {}
    for i, line in enumerate(lines, start=1):
        m = RE_PRAGMA.match(line)
        if not m:
            out.append(line)
            continue
        kind, rest = m.group(2).lower(), m.group(3)
        entry = {"file": rel, "line": i, "kind": kind, "before": line.strip(), "category": _category_for(kind, rest, target)}
        forms = PRAGMA_FORMS.get(target.name, PRAGMA_FORMS["catapult"])

        if kind == "pipeline":
            if re.search(r"\boff\b", rest, re.IGNORECASE) or forms["pipeline"] is None:
                entry["reason"] = "pipeline off has no target equivalent" if forms["pipeline"] else f"no loop pipelining in {target.display_name}"
                log["pragmas_dropped"].append(entry)
                continue
            ii = re.search(r"II\s*=\s*([A-Za-z_0-9]+)", rest, re.IGNORECASE)
            value = ii.group(1) if ii else "1"
            if not RE_INT_LITERAL.match(value) and resolve_int(value, defines) is not None:
                value = str(resolve_int(value, defines))  # target pragmas are not macro-expanded; a #define resolves mechanically
            new = forms["pipeline"].format(v=value)
            if not RE_INT_LITERAL.match(value):
                log["residue"].append({"file": rel, "line": i, "text": new, "reason": f"pipeline II `{value}` is not an integer literal; target pragmas are not macro-expanded, substitute the numeric value"})
            anchor = _find_back(out, RE_LOOP.match)
        elif kind == "unroll":
            f = re.search(r"factor\s*=\s*([A-Za-z_0-9]+)", rest, re.IGNORECASE)
            if f:
                value = f.group(1)
                if not RE_INT_LITERAL.match(value) and resolve_int(value, defines) is not None:
                    value = str(resolve_int(value, defines))
                new = forms["unroll"].format(v=value)
                if not RE_INT_LITERAL.match(value):
                    log["residue"].append({"file": rel, "line": i, "text": new, "reason": f"unroll factor `{value}` is not an integer literal; target pragmas are not macro-expanded, substitute the numeric value"})
            else:
                new = forms["unroll_full"]
            anchor = _find_back(out, RE_LOOP.match)
        elif kind == "inline":
            new = forms["inline"]
            code_of = lambda s: s.split("//", 1)[0].rstrip()  # `) {  // L90`
            anchor = _find_back(out, lambda s: bool(RE_FN_HEADER_END.search(code_of(s))) and not RE_LOOP.match(s) and not s.lstrip().startswith("if"))
            if anchor is not None:  # a signature that spans lines: the pragma goes before its first line, not inside the parameter list
                depth = 0
                for j in range(anchor, -1, -1):
                    depth += code_of(out[j]).count(")") - code_of(out[j]).count("(")
                    if depth <= 0 and "(" in code_of(out[j]):
                        anchor = j
                        break
        else:
            entry["reason"] = "no source-level equivalent in target"
            log["pragmas_dropped"].append(entry)
            continue

        if anchor is None:
            log["residue"].append({"file": rel, "line": i, "reason": f"could not find where to place `{new}` for `{line.strip()}`; place it manually"})
            continue
        indent = out[anchor][: len(out[anchor]) - len(out[anchor].lstrip())]
        lab = RE_LABELED_LOOP.match(out[anchor]) if forms.get("label_own_line") else None
        if lab:  # Bambu's pragma must be followed by the loop itself, so the label moves above the pragma
            out[anchor] = f"{lab.group(1)}{lab.group(3)}"
            out.insert(anchor, f"{indent}{lab.group(2)}:")
            anchor += 1
        out.insert(anchor, f"{indent}{new}")
        entry["after"] = new
        log["pragmas_moved"].append(entry)
    return _drop_pipeline_on_unrolled(out, log, rel)


def _drop_pipeline_on_unrolled(out: list[str], log: dict, rel: str) -> list[str]:
    """Vitis accepts PIPELINE together with a complete UNROLL; Catapult refuses to pipeline a fully unrolled loop,
    so the pipeline pragma is dropped from any pragma block that also fully unrolls."""
    is_pragma = lambda s: s.lstrip().startswith("#pragma hls_")
    drop: set[int] = set()
    for i, line in enumerate(out):
        if line.strip() != "#pragma hls_unroll yes":
            continue
        j = i
        while j > 0 and is_pragma(out[j - 1]):
            j -= 1
        k = i
        while k + 1 < len(out) and is_pragma(out[k + 1]):
            k += 1
        drop.update(n for n in range(j, k + 1) if out[n].lstrip().startswith("#pragma hls_pipeline_init_interval"))
    for n in sorted(drop):
        log["pragmas_dropped"].append({"file": rel, "line": n + 1, "kind": "pipeline", "before": out[n].strip(), "category": "lossy",
                                       "reason": "a fully unrolled loop cannot be pipelined in Catapult"})
    return [l for n, l in enumerate(out) if n not in drop]


def _size_unsized_params(dir_out: Path, top: str, log: dict) -> None:
    """`T x[]` parameters of the top have no size Catapult can use (CIN-84, it assumes 1024 words); the size
    comes from the array the testbench passes at the call site, resolved through the testbench's defines."""
    from hlsfactory_agent.scan import array_dims, top_signature

    sig = top_signature(dir_out, top)
    tb = dir_out / "testbench.cpp"
    if not sig or not tb.exists():
        return
    params = [re.sub(r"//[^\n]*", "", p).strip() for p in re.split(r",(?![^<]*>)", sig)]
    params = [p for p in params if p]
    tb_text = tb.read_text(encoding="utf-8", errors="replace")
    args: list[str] = []
    for call in re.finditer(r"\b" + re.escape(top) + r"\s*\(([^;{}]*)\)\s*;", tb_text):  # the call, not a prototype
        cand = [a.strip().lstrip("&*") for a in re.split(r",(?![^()]*\))", call.group(1))]
        if not any("[" in a or re.match(r"(const|unsigned|signed|int|double|float|char|bool|short|long|void|ac_|ap_|std::)\b", a) for a in cand):
            args = cand
            break
    if not args:
        return
    defines = collect_defines(dir_out)
    # `int n = 1024;` in the testbench resolves like a #define for the heap idioms below
    locals_ = {m.group(1): m.group(2) for m in re.finditer(r"\b(?:const\s+)?(?:int|unsigned|size_t|long)\s+([A-Za-z_]\w*)\s*=\s*([^;(){}]+?)\s*;", tb_text)}
    sizes: dict[str, int] = {}
    pointers: set[str] = set()  # `T *x` ports
    for k, p in enumerate(params):
        m = RE_UNSIZED_PARAM.search(p)
        ptr = None if m else RE_POINTER_PARAM.match(p)
        name = m.group(1) if m else ptr.group(1) if ptr else None
        if not (name and k < len(args) and re.fullmatch(r"[A-Za-z_]\w*", args[k])):
            continue
        dims = array_dims(dir_out, args[k], defines)
        n = dims[0] if dims and len(dims) == 1 else None
        heap = re.search(
            r"\b" + re.escape(args[k]) + r"\s*=\s*(?:\([^()]*\)\s*)?(?:malloc\s*\(\s*([^;]*?)\s*\*\s*sizeof"
            r"|malloc\s*\(\s*sizeof\s*\([^()]*\)\s*\*\s*([^;()]+?)\s*\)|new\s+[\w:<>]+\s*\[\s*([^\]]+?)\s*\])",
            tb_text,
        )
        if n is None and heap:
            n = resolve_int(next(g for g in heap.groups() if g), {**defines, **locals_})
        if n:
            sizes[name] = n
            if ptr:
                pointers.add(name)
    if not sizes:
        return
    # a pointer port becomes a sized array parameter (the same C++ type after decay), in the definition only
    for p in sorted(dir_out.rglob("*")):
        if not pointers or not p.is_file() or p.suffix.lower() not in (".cpp", ".cc", ".c") or p.name == "testbench.cpp":
            continue
        text = p.read_text(encoding="utf-8", errors="replace")
        d = re.search(r"\b" + re.escape(top) + r"\s*\(([^;{]*)\)\s*\{", text, re.S)
        if not d:
            continue
        head = d.group(1)
        for name in sorted(pointers):
            head = re.sub(r"\*\s*" + re.escape(name) + r"\b", f"{name}[{sizes[name]}]", head, count=1)
        p.write_text(text[: d.start(1)] + head + text[d.end(1):], encoding="utf-8")
        log["types"].append({"file": str(p.relative_to(dir_out)), "line": 0, "before": "pointer top parameters " + ", ".join(sorted(pointers)), "after": ", ".join(f"{k}[{sizes[k]}]" for k in sorted(pointers))})
        break
    sizes = {k: v for k, v in sizes.items() if k not in pointers}
    for p in sorted(dir_out.rglob("*")):
        if not p.is_file() or p.suffix.lower() not in SOURCE_SUFFIXES or p.name == "testbench.cpp":
            continue
        text = p.read_text(encoding="utf-8", errors="replace")
        new = RE_UNSIZED_PARAM.sub(lambda m: f"{m.group(1)}[{sizes[m.group(1)]}]" if m.group(1) in sizes else m.group(0), text)
        if new != text:
            p.write_text(new, encoding="utf-8")
            log["types"].append({"file": str(p.relative_to(dir_out)), "line": 0, "before": "unsized top parameters " + ", ".join(sizes), "after": ", ".join(f"{k}[{v}]" for k, v in sizes.items())})


RE_SCALAR_TYPE = re.compile(r"^(?:const\s+)?(?:unsigned\s+|signed\s+)?(?:void|bool|char|short|int|long|long\s+long|float|double|size_t|u?int(?:8|16|32|64)_t)$")
RET_PARAM = "ret_out"


def _void_class_return(dir_out: Path, top: str, log: dict) -> None:
    """Bambu 2024.10 aborts (`map::at`) on a top that returns a class by value (the hidden sret pointer):
    `T top(a)` becomes `void top(a, T &ret_out)` and every `return x;` of its body assigns first. The
    testbench call sites are left to the agent as residue."""
    rx = re.compile(r"(?m)^([ \t]*)((?:const\s+)?[A-Za-z_][\w:<>, ]*?)\s+(" + re.escape(top) + r")\s*\(([^;{}]*)\)\s*(\{|;)")
    files = [p for p in sorted(dir_out.rglob("*")) if p.is_file() and p.suffix.lower() in SOURCE_SUFFIXES and p.name != "testbench.cpp"]
    if any(p.suffix.lower() == ".c" for p in files):
        return  # a C design has no class to return
    scalar_names = {m.group(2) for p in files for m in re.finditer(r"\btypedef\s+([\w ]+?)\s+(\w+)\s*;", p.read_text(encoding="utf-8", errors="replace")) if RE_SCALAR_TYPE.match(m.group(1).strip())}
    changed = False
    for p in files:
        text = p.read_text(encoding="utf-8", errors="replace")
        pos, new = 0, ""
        for m in rx.finditer(text):
            ret = m.group(2).strip()
            if ret == "void" or "*" in ret or RE_SCALAR_TYPE.match(ret) or ret.split()[-1] in scalar_names | KEYWORDS:
                continue
            params = m.group(4).strip()
            params = f"{params}, " if params and params != "void" else ""
            sig = f"{m.group(1)}void {top}({params}{ret} &{RET_PARAM})"
            new += text[pos : m.start()] + sig
            if m.group(5) == "{":
                end = _body_end(text, m.end() - 1)
                body = re.sub(r"\breturn\s+([^;]+);", lambda r: f"{{ {RET_PARAM} = {r.group(1)}; return; }}", text[m.end() - 1 : end + 1])
                new += " " + body
                pos = end + 1
            else:
                new += ";"
                pos = m.end()
            log["types"].append({"file": str(p.relative_to(dir_out)), "line": text.count("\n", 0, m.start()) + 1, "before": m.group(0).strip(), "after": sig.strip() + (" {" if m.group(5) == "{" else ";")})
            changed = True
        if pos:
            p.write_text(new + text[pos:], encoding="utf-8")
    tb = dir_out / "testbench.cpp"
    if not changed or not tb.exists():
        return
    for i, line in enumerate(tb.read_text(encoding="utf-8", errors="replace").splitlines(), start=1):
        if re.search(r"\b" + re.escape(top) + r"\s*\(", line) and not line.lstrip().startswith("//"):
            log["residue"].append({"file": "testbench.cpp", "line": i, "text": line.strip(), "reason": f"the top now returns void with a trailing `{RET_PARAM}` reference (Bambu cannot synthesize a class returned by value); pass the result variable as the last argument here"})


def _relocate_residue(log: dict, rel: str, out: list[str]) -> None:
    """Insertions shift lines; a residue entry that recorded its text is re-pointed at that text's nearest copy."""
    for r in log["residue"]:
        if r.get("file") != rel or not r.get("text"):
            continue
        hits = [n for n, l in enumerate(out, start=1) if l.strip() == r["text"]]
        if hits:
            r["line"] = min(hits, key=lambda n: abs(n - int(r["line"])))


def _insert_top_marker(lines: list[str], top: str, target: TargetSpec, log: dict, rel: str) -> list[str]:
    rx = _fn_def_regex(top)
    for idx, line in enumerate(lines):
        if rx.match("\n".join(lines[idx : idx + SIGNATURE_LINES])):
            lines.insert(idx, target.top_marker)
            log["top_marker"] = {"file": rel, "line": idx + 1, "inserted": True}
            return lines
    return lines


def rewrite_design(dir_in: Path, dir_out: Path, target: TargetSpec, top: str | None) -> dict:
    dir_in, dir_out = Path(dir_in), Path(dir_out)
    dir_out.mkdir(parents=True, exist_ok=True)
    log: dict = {
        "target": target.name,
        "headers": [],
        "types": [],
        "pragmas_moved": [],
        "pragmas_dropped": [],
        "top_marker": {"inserted": False},
        "blocks": [],
        "stdout": [],
        "residue": [],
    }
    header_map = HEADER_MAPS.get(target.name, HEADER_MAPS["catapult"])
    blocks = dataflow_blocks(dir_in, top) if target.name == "catapult" else set()
    defines = collect_defines(dir_in)
    names = float_typedefs(dir_in) if target.name == "catapult" else set()
    if names:  # `const w_dt v = 1.0;` with `typedef float w_dt` needs direct initialization like the float types themselves
        log["_float_literal_rx"] = re.compile(r"(?<![\w.])((?:const\s+)?(?:ac_ieee_float(?:32|64)|" + "|".join(map(re.escape, sorted(names))) + r"))\s+(\w+)\s*=\s*(-?\d+(?:\.\d*)?(?:[eE][-+]?\d+)?[fF]?)\s*;")

    for p in sorted(dir_in.rglob("*")):
        if not p.is_file() or p.name == "synth.tcl":
            continue
        if p.suffix.lower() in SOURCE_SUFFIXES and p.name != "testbench.cpp" and RE_MAIN.search(p.read_text(encoding="utf-8", errors="replace")):
            log["pragmas_dropped"].append({"file": str(p.relative_to(dir_in)), "line": 0, "kind": "file", "before": "second main()", "category": "absent",
                                           "reason": "a second test program next to testbench.cpp cannot be linked; not copied"})
            continue
        if p.suffix.lower() in (".h", ".hpp") and RE_VITIS_LIB_COPY.search(p.read_text(encoding="utf-8", errors="replace")):
            log["headers"].append({"file": str(p.relative_to(dir_in)), "line": 0, "before": "copy of a Vitis library header", "after": "not copied; its includes map to the target's types"})
            continue
        dst = dir_out / p.relative_to(dir_in)
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(p, dst)

    if top and target.name in ("catapult", "bambu", "dynamatic"):  # Dynamatic's translator segfaults on `T x[]`
        _size_unsized_params(dir_out, top, log)
    if top and target.name == "bambu":
        _void_class_return(dir_out, top, log)

    tb_files = testbench_files(dir_out)
    sources = [q for q in dir_out.rglob("*") if q.is_file() and q.suffix.lower() in SOURCE_SUFFIXES]
    alias_channel = _local_alias_channel(channel_aliases(q.read_text(encoding="utf-8", errors="replace") for q in sources))
    for p in sorted(dir_out.rglob("*"), key=lambda q: (q.name == "testbench.cpp", str(q))):
        if not p.is_file() or p.suffix.lower() not in SOURCE_SUFFIXES:
            continue
        rel = str(p.relative_to(dir_out))
        lines = p.read_text(encoding="utf-8", errors="replace").splitlines()
        new_lines: list[str] = []
        for i, line in enumerate(lines, start=1):
            m = RE_INCLUDE.match(line)
            if m:
                new = f"{m.group(1)}{m.group(2)}{header_map[m.group(3)]}{m.group(4)}"
                log["headers"].append({"file": rel, "line": i, "before": line.strip(), "after": new.strip()})
                new_lines.append(new)
                continue
            m = RE_ANY_INCLUDE.match(line)
            if m and target.name == "catapult":
                new = _resolve_include(line, m.group(1), p, dir_out)
                if new != line:
                    log["headers"].append({"file": rel, "line": i, "before": line.strip(), "after": new.strip()})
                    new_lines.append(new)
                    continue
            if target.name == "dynamatic" and (new := redirect_stdout(line)) != line:
                log["stdout"].append({"file": rel, "line": i, "before": line.strip(), "after": new.strip()})
                line = new
            new_lines.append(_rewrite_types(line, target, log, rel, i))
        new_lines = _rewrite_dialect(new_lines, target, log, rel, testbench=rel in tb_files, alias_channel=alias_channel)
        for line in new_lines:
            m = RE_ANY_INCLUDE.match(line)
            shim = SHIM_HEADERS.get(target.name, {}).get(m.group(1)) if m else None
            if shim is not None and not (dir_out / m.group(1)).exists():
                (dir_out / m.group(1)).write_text(shim, encoding="utf-8")
                log["headers"].append({"file": m.group(1), "line": 0, "before": "", "after": "shim written"})
        new_lines = _rewrite_pragmas(new_lines, target, log, rel, defines)
        if top and target.top_marker and p.suffix.lower() in (".cpp", ".cc", ".c") and not log["top_marker"]["inserted"]:
            new_lines = _insert_top_marker(new_lines, top, target, log, rel)
        if blocks and rel not in tb_files:
            new_lines = _insert_block_markers(new_lines, blocks, log, rel)
        if target.name == "catapult" and p.suffix.lower() in (".h", ".hpp"):
            new_lines = _name_anonymous_namespaces(new_lines, log, rel)
        _relocate_residue(log, rel, new_lines)
        text = "\n".join(new_lines) + "\n"
        for k, l in enumerate(text.splitlines(), start=1):
            if l.lstrip().startswith("//"):
                continue
            for pat in target.forbidden_patterns + (() if rel in tb_files else target.dut_forbidden_patterns):
                if re.search(pat, l) and not ("ac_channel" in pat and in_class_body(new_lines, k - 1)):
                    log["residue"].append({"file": rel, "line": k, "reason": f"still matches `{pat}`: {l.strip()}"})
                    break
        p.write_text(text, encoding="utf-8")

    log.pop("_float_literal_rx", None)
    log["category_counts"] = dict(Counter(e["category"] for e in log["pragmas_moved"] + log["pragmas_dropped"]))
    (dir_out / "rewrite_log.json").write_text(json.dumps(log, indent=2), encoding="utf-8")
    return log
