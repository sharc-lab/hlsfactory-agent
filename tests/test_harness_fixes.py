from pathlib import Path

from hlsfactory_agent.rewrite import _rewrite_types, outside_strings
from hlsfactory_agent.targets.catapult import CATAPULT
from hlsfactory_agent.translate import FIXED_SEED, compare_outputs, seed_random_sources
from hlsfactory_agent.translate import testbench_strings_kept as strings_kept


def test_seed_random_sources(tmp_path: Path):
    tb = tmp_path / "testbench.cpp"
    tb.write_text("std::random_device rd;\nstd::mt19937 g(rd());\nint s = std::random_device{}();\nsrand(time(NULL));\n", encoding="utf-8")
    assert seed_random_sources(tmp_path) == ["testbench.cpp"]
    text = tb.read_text(encoding="utf-8")
    assert f"std::mt19937 rd({FIXED_SEED});" in text and f"int s = {FIXED_SEED}u" in text and f"srand({FIXED_SEED})" in text
    assert "random_device" not in text.replace("was std::random_device", "")
    assert seed_random_sources(tmp_path) == []


def test_prepass_leaves_string_literals():
    log = {"residue": [], "types": []}
    line = 'printf("type ap_fixed<32, 4> x\\n"); ap_fixed<32, 4> v; ap_uint<8> "ap_int<3>" ap_int<3> w;'
    out = _rewrite_types(line, CATAPULT, log, "tb.cpp", 1)
    assert out == 'printf("type ap_fixed<32, 4> x\\n"); ac_fixed<32, 4, true> v; ac_int<8, false> "ap_int<3>" ac_int<3, true> w;'
    assert outside_strings('a "b" c', str.upper) == 'A "b" C'


def test_testbench_strings_kept(tmp_path: Path):
    inp, out = tmp_path / "in", tmp_path / "out"
    inp.mkdir(); out.mkdir()
    (inp / "synth.tcl").write_text("add_files k.cpp\nadd_files -tb tb.cpp\n", encoding="utf-8")
    (inp / "k.cpp").write_text('const char *k = "kernel only";\n', encoding="utf-8")
    (inp / "tb.cpp").write_text('#include "ap_int.h"\n// "comment"\nprintf("Data type: ap_fixed<32, 4>\\n"); printf("PASS\\\\n"); cout << "x = " << x;\n', encoding="utf-8")
    (out / "testbench.cpp").write_text('printf("Data type: ac_fixed<32, 4, true>\\n"); printf("PASS\\n"); printf("x = %d", x);\n', encoding="utf-8")
    assert strings_kept(inp, out) == ["Data type: ap_fixed<32, 4>\\n", "PASS\\\\n"]
    (out / "testbench.cpp").write_text('printf("Data type: ap_fixed<32, 4>\\n"); printf("PASS\\\\n"); printf("x = %d", x);\n', encoding="utf-8")
    assert strings_kept(inp, out) == []


def test_compare_outputs_ignores_ap_int_warnings():
    orig = "WARNING: Bitsize mismatch for ap_[u]int|=ap_[u]int.\nresult 3\nPASS\n"
    assert compare_outputs(orig, "result 3\nPASS\n")["identical"] is True


def test_prepass_folds_log2_and_sizes_calloc_vla_constexpr(tmp_path: Path):
    from hlsfactory_agent.rewrite import rewrite_design

    d = tmp_path / "design"
    d.mkdir()
    (d / "cfg.h").write_text("static constexpr int kPackets = 16 / 4;\n#define M 256\n#define Mlog log2(M)\n#define Nbits ceil(log2(10))\n", encoding="utf-8")
    (d / "k.cpp").write_text('#include "cfg.h"\nvoid top(int *a, unsigned char *s, int *v, int n) { int w = Mlog + Nbits; int q = log2(n); (void)w; (void)q; }\n', encoding="utf-8")
    (d / "testbench.cpp").write_text('#include "cfg.h"\nint main() { int rows = 4; int cols = 16; int size = rows * cols; int sp = size / kPackets; int a[sp];\n'
                                     'unsigned char *s = (unsigned char*)calloc(M, 1); int *v = (int*)calloc(8, sizeof(int)); top(a, s, v, 5); return 0; }\n', encoding="utf-8")
    log = rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    k = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert "void top(int a[16], unsigned char s[256], int v[8], int n)" in k
    assert "#define Mlog 8\n#define Nbits 4\n" in (tmp_path / "out" / "cfg.h").read_text(encoding="utf-8")
    assert "log2(n)" in k and any("log2" in r["reason"] for r in log["residue"])


def test_dataflow_stage_under_a_runtime_loop_stays_inline(tmp_path: Path):
    from hlsfactory_agent.rewrite import _conditional_call, dataflow_blocks

    d = tmp_path / "design"
    d.mkdir()
    (d / "k.cpp").write_text(
        "#include <hls_stream.h>\nvoid merge(hls::stream<int> &i, hls::stream<int> &o) { o.write(i.read()); }\n"
        "void fixed(hls::stream<int> &i, hls::stream<int> &o) { o.write(i.read()); }\n"
        "void guarded(hls::stream<int> &i, hls::stream<int> &o) { o.write(i.read()); }\n"
        "void top(hls::stream<int> &n_in, hls::stream<int> &i, hls::stream<int> &o) {\n#pragma HLS dataflow\n    int n = n_in.read();\n"
        "    for (int k = 0; k < n; ++k)\n    AGGRESSIVE_UNROLL\n    {\n        merge(i, o);\n    }\n"
        "    for (int k = 0; k < N_MAX; ++k) { fixed(i, o); }\n    if (n > 1) guarded(i, o);\n}\n",
        encoding="utf-8",
    )
    assert dataflow_blocks(d, "top") == {"fixed"}
    for body in ("void f() { for (int k = 0; k < n; ++k) g(); }", "void f() { do { g(); } while (x); }", "void f() { if (a) { b(); } else g(); }"):
        assert _conditional_call(body, body.index("g()")) is True, body
    body = "void f() { for (int k = 0; k < N; ++k) { x(); g(); } }"
    assert _conditional_call(body, body.index("g()")) is False


def test_catapult_degrade_multicycle_timeout_and_clock(tmp_path: Path):
    import json

    from hlsfactory_agent.targets.catapult import catapult_degrade_step

    failed = tmp_path / "failed"
    failed.mkdir()
    (failed / "run.tcl").write_text("go libraries\ndirective set -CLOCKS {clk {-CLOCK_PERIOD 5.0}}\ngo assembly\ngo architect\n", encoding="utf-8")
    (failed / "k.cpp").write_text("#pragma hls_unroll 4\nfor(;;){}\n#pragma hls_unroll yes\nfor(;;){}\n#pragma hls_pipeline_init_interval 1\n", encoding="utf-8")
    (failed / "catapult.log").write_text("# Warning: Couldn't find library component for operator 'div(32,1,32,1,32)' - multicycle component prohibited by directive SCHED_USE_MULTICYCLE=false (SIF-4)\n", encoding="utf-8")
    assert catapult_degrade_step(failed, tmp_path / "m") == "multicycle"
    assert "directive set -SCHED_USE_MULTICYCLE true\ndirective set -CLOCKS" in (tmp_path / "m" / "run.tcl").read_text(encoding="utf-8")
    (failed / "catapult.log").write_text("# Error: $PROJECT_HOME/k.cpp(112): Resource '/top/size:rsc' with variable connected to multiple sub-blocks not mapped to '[DirectInput]' (MEM-71)\n", encoding="utf-8")
    assert catapult_degrade_step(failed, tmp_path / "d") == "direct_input:size"
    assert "go assembly\ndirective set /top/size:rsc -MAP_TO_MODULE {[DirectInput]}\ngo architect" in (tmp_path / "d" / "run.tcl").read_text(encoding="utf-8")
    (tmp_path / "d" / "catapult.log").write_text((failed / "catapult.log").read_text(encoding="utf-8"), encoding="utf-8")
    assert catapult_degrade_step(tmp_path / "d", tmp_path / "d2") is None
    (failed / "catapult.log").write_text("# Info: Running transformation 'dpfsm'\n", encoding="utf-8")
    (failed / "synth_result.json").write_text(json.dumps({"status": "TIMEOUT"}), encoding="utf-8")
    assert catapult_degrade_step(failed, tmp_path / "t1") == "unroll_cap:partial:1"
    assert "hls_unroll 4" not in (tmp_path / "t1" / "k.cpp").read_text(encoding="utf-8")
    (tmp_path / "t1" / "synth_result.json").write_text(json.dumps({"status": "TIMEOUT"}), encoding="utf-8")
    assert catapult_degrade_step(tmp_path / "t1", tmp_path / "t2") == "unroll_cap:full:1"
    assert "hls_unroll" not in (tmp_path / "t2" / "k.cpp").read_text(encoding="utf-8")
    (failed / "synth_result.json").unlink()
    (failed / "catapult.log").write_text("#  Minimum II of '9' is required to schedule the design (SCHD-87)\n# Error: Schedule failed, sequential delay violated. (SCHD-20)\n", encoding="utf-8")
    assert catapult_degrade_step(failed, tmp_path / "c") == "clock:10"
    assert "-CLOCK_PERIOD 10}" in (tmp_path / "c" / "run.tcl").read_text(encoding="utf-8")
    (tmp_path / "c" / "catapult.log").write_text('# Error: Schedule failed, view the Gantt chart for a failed schedule using "view schedule". (SCHD-26)\n', encoding="utf-8")
    assert catapult_degrade_step(tmp_path / "c", tmp_path / "c2") == "unpipeline:all:1"  # the clock is relaxed once; then loops run sequentially
    assert "hls_pipeline" not in (tmp_path / "c2" / "k.cpp").read_text(encoding="utf-8")
    (tmp_path / "c2" / "catapult.log").write_text((tmp_path / "c" / "catapult.log").read_text(encoding="utf-8"), encoding="utf-8")
    assert catapult_degrade_step(tmp_path / "c2", tmp_path / "c3") is None
    (tmp_path / "t2" / "synth_result.json").write_text(json.dumps({"status": "TIMEOUT"}), encoding="utf-8")
    assert catapult_degrade_step(tmp_path / "t2", tmp_path / "t3") == "unpipeline:all:1"  # a timeout with no unroll left


def test_vitis_script_two_tops_and_std_includes(tmp_path: Path):
    from hlsfactory_agent.extraction_repair import repair_std_includes
    from hlsfactory_agent.translate import normalize_vitis_script

    d = tmp_path / "design"
    d.mkdir()
    (d / "synth.tcl").write_text("open_project p\nset_top readEdgesCU\nadd_files scatter.cpp\nadd_files apply.cpp\nadd_files k.h\nadd_files -tb testbench.cpp\nset_top vertexApply\ncsynth_design\n", encoding="utf-8")
    (d / "scatter.cpp").write_text('#include "k.h"\nextern "C" { void readEdgesCU(int *e) { helper(e); } }\n', encoding="utf-8")
    (d / "apply.cpp").write_text('#include "k.h"\nextern "C" { void vertexApply(int *v) { helper(v); } }\n', encoding="utf-8")
    (d / "k.h").write_text("void helper(int *x) { x[0] = INT_MAX; }\nuint32_t h; // uint64_t in a comment only counts as use\n", encoding="utf-8")
    (d / "testbench.cpp").write_text("#include <stdint.h>\n#include <limits.h>\nint main() { uint32_t x = INT_MAX; return 0; }\n", encoding="utf-8")
    assert normalize_vitis_script(d)["tops_dropped"] == ["readEdgesCU"]
    tcl = (d / "synth.tcl").read_text(encoding="utf-8")
    assert tcl.count("set_top vertexApply") == 2 and "readEdgesCU" not in tcl and "add_files -tb scatter.cpp" in tcl and "add_files apply.cpp" in tcl
    assert repair_std_includes(d) == {"k.h": ["#include <stdint.h>", "#include <limits.h>"]}
    assert (d / "k.h").read_text(encoding="utf-8").startswith("#include <stdint.h>  // hlsfactory: extraction repair\n#include <limits.h>")
    assert repair_std_includes(d) == {}


def test_repair_ap_cint_moves_c_units_to_cpp(tmp_path: Path):
    from hlsfactory_agent.extraction_repair import repair_ap_cint

    d = tmp_path / "design"
    d.mkdir()
    (d / "synth.tcl").write_text("set_top k\nadd_files k.c\nadd_files -tb testbench.cpp\n", encoding="utf-8")
    (d / "k.c").write_text('#include "ap_cint.h"\nint8 k(int8 x) { return x; }\n', encoding="utf-8")
    (d / "testbench.cpp").write_text('#include "k.c"\nint main() { return k(1) != 1; }\n', encoding="utf-8")
    assert repair_ap_cint(d) == {"standin": "ap_cint.h", "renamed": {"k.c": "k.cpp"}}
    assert (d / "ap_cint.h").exists() and (d / "k.cpp").exists() and not (d / "k.c").exists()
    assert "add_files k.cpp\n" in (d / "synth.tcl").read_text(encoding="utf-8")
    assert '#include "k.cpp"' in (d / "testbench.cpp").read_text(encoding="utf-8")
    assert repair_ap_cint(d) == {}


def test_leftover_scan_ignores_string_literals(tmp_path: Path):
    from hlsfactory_agent.translate import check_translated_design

    out = tmp_path / "out"
    out.mkdir()
    (out / "k.cpp").write_text('#include <ac_int.h>\n#pragma hls_design top\nvoid top(ac_int<8, true> x) { const char *s = "ap_int<8> x"; }\n', encoding="utf-8")
    (out / "testbench.cpp").write_text('#include "ap_int.h"\nint main() { printf("ap_fixed<32, 4>\\n"); return 0; }\n', encoding="utf-8")
    r = check_translated_design(out, CATAPULT)
    assert [l["line"] for l in r["leftovers"] if l["file"] == "testbench.cpp"] == [1]  # the include, not the printf
    assert not [l for l in r["leftovers"] if l["file"] == "k.cpp" and "ap_int<8> x" in l["text"]]


def test_two_top_script_keeps_a_top_per_solution(tmp_path: Path):
    from hlsfactory_agent.translate import normalize_vitis_script

    d = tmp_path / "design"
    d.mkdir()
    (d / "synth.tcl").write_text("set_top a\nadd_files a.cpp\nadd_files b.cpp\nopen_solution s1\ncsynth_design\nset_top b\nopen_solution s2\ncsynth_design\n", encoding="utf-8")
    (d / "a.cpp").write_text("void a(int *x) {}\n", encoding="utf-8")
    (d / "b.cpp").write_text("void b(int *x) {}\n", encoding="utf-8")
    assert normalize_vitis_script(d)["tops_dropped"] == ["a"]
    assert (d / "synth.tcl").read_text(encoding="utf-8") == "set_top b\nadd_files -tb a.cpp\nadd_files b.cpp\nopen_solution s1\ncsynth_design\nset_top b\nopen_solution s2\ncsynth_design\n"


def test_prepass_sizes_byte_malloc_and_parenthesised_malloc_and_maps_c_puns(tmp_path: Path):
    from hlsfactory_agent.rewrite import rewrite_design

    d = tmp_path / "design"
    d.mkdir()
    (d / "k.h").write_text("#define M 4\nstruct dtype { int data; };\n", encoding="utf-8")
    (d / "k.cpp").write_text('#include "k.h"\nvoid top(unsigned char *sk, dtype *A, int *v, float id) { int out = *((int*)(&id)); *((int*)&v[0]) = out; (void)sk; (void)A; }\n', encoding="utf-8")
    (d / "testbench.cpp").write_text('#include "k.h"\n#define KEYBYTES 32\nint main() { unsigned char *sk = (unsigned char*)malloc(KEYBYTES); dtype *A = (dtype*)(malloc(sizeof(dtype)*M*M));'
                                     ' int *v = (int*)malloc(M); top(sk, A, v, 1.0f); return 0; }\n', encoding="utf-8")
    rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    k = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert "void top(unsigned char sk[32], dtype A[16], int *v, " in k  # malloc(N) sizes a byte buffer only
    assert "int out = cat_load<int>((&id));" in k and "cat_store<int>(&v[0], out);" in k


def test_define_continuation_and_single_assignment_locals(tmp_path):
    from hlsfactory_agent.scan import collect_defines, resolve_int
    (tmp_path / "k.h").write_text("#define A (2*B \\\n + 1)\n#define B 3\t/* note */\n", encoding="utf-8")
    assert resolve_int("A", collect_defines(tmp_path)) == 7


def test_sizing_from_local_chain_and_lone_test_program(tmp_path):
    from hlsfactory_agent.rewrite import rewrite_design
    d = tmp_path / "in"; d.mkdir()
    (d / "k.h").write_text("#define D 4\n#define W 16\n#define N 8\n", encoding="utf-8")
    (d / "k.cpp").write_text('#include "k.h"\nvoid top(int in[], int out[], int q[]) { out[0] = in[0] + q[0]; }\n', encoding="utf-8")
    (d / "k_test.cpp").write_text(
        '#include "k.h"\n#include <stdlib.h>\nvoid top(int in[], int out[], int q[]);\nint main() {\n'
        "  int wq = (D + W - 1) / W;\n  int total = wq + N * wq;\n  int m = 2;\n  if (m < 8) m = 8;\n"
        "  int *in = (int*)malloc(total * sizeof(int));\n  int out[4];\n  int q[m];\n  top(in, out, q);\n  return 0;\n}\n", encoding="utf-8")
    log = rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    assert (tmp_path / "out" / "testbench.cpp").exists() and not (tmp_path / "out" / "k_test.cpp").exists()
    sig = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert "in[9]" in sig and "out[4]" in sig and "q[]" in sig  # m is reassigned, so q stays unsized


def test_oracle_fail_line():
    from hlsfactory_agent.translate import oracle_fail_line
    assert oracle_fail_line("x\nFAIL\n0\n") == "FAIL"
    assert oracle_fail_line("*** Test FAILED ***\n") == "*** Test FAILED ***"
    assert oracle_fail_line("FAIL: 0\nfailures: 0\nPASS\n") == ""


def test_dataflow_stage_with_a_port_is_a_block_unless_shared_or_local(tmp_path):
    from hlsfactory_agent.rewrite import dataflow_blocks
    d = tmp_path / "in"; d.mkdir()
    (d / "k.cpp").write_text(
        "#include <hls_stream.h>\n"
        "static void load(int *in, hls::stream<int> &o, int n) { o.write(in[0]); }\n"
        "static void loadb(int *in, hls::stream<int> &o, int n) { o.write(in[1]); }\n"
        "static void scratch(int *buf, hls::stream<int> &o) { o.write(buf[0]); }\n"
        "static void store(int *out, hls::stream<int> &i, int n) { out[0] = i.read(); }\n"
        "void top(int *in1, int *in2, int *out, int n) {\n#pragma HLS dataflow\n    static hls::stream<int> a, b, c;\n    int tmp[4];\n"
        "    load(in1, a, n);\n    loadb(in1, b, n);\n    scratch(tmp, c);\n    store(out, c, n);\n}\n", encoding="utf-8")
    assert dataflow_blocks(d, "top") == {"store"}  # in1 is shared by two stages, tmp is local


def test_conditional_region_keeps_no_blocks_and_repeated_stage_gets_instances(tmp_path):
    from hlsfactory_agent.rewrite import rewrite_design
    d = tmp_path / "in"; d.mkdir()
    (d / "k.cpp").write_text(
        "#include <hls_stream.h>\n"
        "template <class T> static void dup(hls::stream<T> &a, hls::stream<T> &b, int n) { b.write(a.read()); }\n"
        "static void load(int *in, hls::stream<int> &o, int n) { o.write(in[0]); }\n"
        "static void sink(hls::stream<int> &i, int *out) { out[0] = i.read(); }\n"
        "static void sign(int *in, int *out) {\n#pragma HLS dataflow\n    static hls::stream<int> a, b;\n    load(in, a, 1);\n    sink(b, out);\n}\n"
        "void top(int *in1, int *in2, int *out, int cfg) {\n#pragma HLS dataflow\n    static hls::stream<int> a, b, c;\n"
        "    if (cfg == 0) { sign(in1, out); return; }\n"
        "    load(in1, a, 1);\n    load(in2, b, 1);\n    dup<int>(a, c, 1);\n    dup<int>(b, c, 1);\n    sink(c, out);\n}\n", encoding="utf-8")
    (d / "testbench.cpp").write_text("void top(int *in1, int *in2, int *out, int cfg);\nint main() { int x[4], y[8], o[2]; top(x, y, o, 1); return 0; }\n", encoding="utf-8")
    log = rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    c = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert "static void load(int in[8]," in c  # the larger of the two ports it receives
    assert "#pragma hls_design block\nstatic void sign(" not in c and c.count("#pragma hls_design block") == 5  # sink + 2 x 2 wrappers
    assert "#pragma hls_design block\nstatic void load(" not in c and "block\ntemplate <class T> static void dup(" not in c  # inlined into wrappers
    assert "static void load__2(int in[8], hls::stream<int> &o, int n) {\n    load(in, o, n);\n}" in c.replace("ac_channel", "hls::stream")
    assert "template <class T> static void dup__1(" in c and "dup<T>(a, b, n);" in c
    assert "    load__1(in1, a, 1);\n    load__2(in2, b, 1);\n    dup__1<int>(a, c, 1);\n    dup__2<int>(b, c, 1);" in c
    assert any("called 2 times" in b.get("reason", "") for b in log["blocks"])
