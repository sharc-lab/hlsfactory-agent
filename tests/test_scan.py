from pathlib import Path

from hlsfactory_agent.scan import scan_design, write_scan

SRC = """#include "ap_fixed.h"
#include "hls_stream.h"
typedef ap_fixed<16, 8> data_t;
void top(hls::stream<data_t> &in, ap_uint<8> n) {
#pragma HLS INTERFACE axis port=in
#pragma HLS ARRAY_PARTITION variable=coef complete dim=1
    for (int i = 0; i < 8; i++) {
#pragma HLS PIPELINE II=1
        // #pragma HLS UNROLL
        int *p = (int *)malloc(4);
    }
}
"""


def make_design(tmp_path: Path) -> Path:
    d = tmp_path / "design"
    d.mkdir()
    (d / "top.cpp").write_text(SRC, encoding="utf-8")
    (d / "top.h").write_text('#include "ap_int.h"\n', encoding="utf-8")
    return d


def test_scan_counts_pragmas_types_headers(tmp_path: Path):
    s = scan_design(make_design(tmp_path))
    assert s["counts"]["pragmas"] == {"interface": 1, "array_partition": 1, "pipeline": 1}
    assert s["counts"]["types"] == {"ap_fixed": 1, "hls::stream": 1, "ap_uint": 1}
    assert s["counts"]["headers"] == {"ap_fixed.h": 1, "hls_stream.h": 1, "ap_int.h": 1}


def test_scan_skips_commented_pragmas_and_records_constructs(tmp_path: Path):
    s = scan_design(make_design(tmp_path))
    kinds = [p["kind"] for p in s["pragmas"]]
    assert "unroll" not in kinds
    assert s["counts"]["constructs"] == {"dynamic_memory": 1}
    item = s["pragmas"][0]
    assert item["file"] == "top.cpp" and item["line"] == 5 and "INTERFACE" in item["text"]


def test_scan_flags_recursion_candidate(tmp_path: Path):
    d = tmp_path / "design"
    d.mkdir()
    (d / "r.cpp").write_text("int fact(int n) {\n    if (n < 2) return 1;\n    return n * fact(n - 1);\n}\n", encoding="utf-8")
    s = scan_design(d)
    assert s["counts"]["constructs"] == {"recursion_candidate": 1}
    assert s["constructs"][0]["text"] == "fact"


def test_write_scan_writes_json(tmp_path: Path):
    d = make_design(tmp_path)
    out = write_scan(d, tmp_path)
    assert out.name == "scan.json" and out.exists()
