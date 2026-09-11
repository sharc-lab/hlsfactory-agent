# Conversion Agent Next Steps Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Turn the working Vitis-to-Catapult translation flow into the merged architecture: deterministic inventory and reconciliation, output equivalence against the original, a mechanical pre-pass so the model only handles residue, a bounded fixer loop, and XLS as the second target.

**Architecture:** Everything sits around the existing `HLSTranslationRun` in `hlsfactory_agent/translate.py`. New deterministic modules (`scan.py`, `rewrite.py`) run on the host before and after the agent; the agent's job narrows to the residue the pre-pass reports; checks never trust the agent's own claims. A second target is a second `TargetSpec` entry plus a small emulation shim.

**Tech Stack:** Python 3.11+ (project venv), pytest, Docker (`hlsfactory-agent` image with clang), Pi coding agent via OpenRouter, vendored hlslibs ac_types headers.

**Spec:** `docs/specs/2026-09-05-allo-hls-conversion-agent-design.md` (sections 5.2, 5.4, 6, 12 as revised in the 2026-09-10 note; the hub is the portable layer, of which the Catapult table is the first column).

## Global Constraints

- Every file read and write passes `encoding="utf-8"`; reads add `errors="replace"`. Windows default encoding crashed a run once.
- Never delete a directory with bare `shutil.rmtree`; use `_rmtree_robust` from `translate.py`.
- Commit messages carry no `Co-Authored-By` trailer.
- No results, transcripts, or run outputs are committed. `exp/run_translate/runs/` and `results/` are git-ignored.
- Tests run with `.venv/Scripts/python -m pytest tests -q` from the repository root and must not need Docker or an API key unless marked.
- Scripts under `exp/` are run with `PYTHONPATH=.` because the package is not installed into the venv.
- Numerics never change in a translation: widths, signedness, rounding and overflow modes are preserved or the design fails with a recorded reason.

---

## File map

| File | Responsibility |
|---|---|
| `hlsfactory_agent/scan.py` (new) | Inventory of pragmas, vendor types, headers, risky constructs in a design folder |
| `hlsfactory_agent/rewrite.py` (new) | Mechanical Vitis-to-target rewrite: headers, types, pragma moves and drops, top marker; residue log |
| `hlsfactory_agent/translate.py` (modify) | Report reconciliation check, output comparison, pre-pass wiring, fixer loop, XLS target |
| `hlsfactory_agent/xls_emu_include/xls_emu.h` (new) | Host-side emulation of `__xls_channel` and XLS pragmas so translated XLS code compiles with clang |
| `exp/run_translate/run_repo.py` (modify) | Pass scan into checks, compare outputs, pass attempts through |
| `tests/test_scan.py`, `tests/test_rewrite.py`, `tests/test_translate.py` | Tests |

---

### Task 1: Construct scanner

**Files:**
- Create: `hlsfactory_agent/scan.py`
- Test: `tests/test_scan.py`

**Interfaces:**
- Produces: `scan_design(dir_design: Path) -> dict` with keys `pragmas` (list of `{"kind","file","line","text"}`), `types` (same shape, `kind` is the vendor type name), `headers` (same shape), `constructs` (same shape, kinds `dynamic_memory`, `stl_container`, `recursion_candidate`), and `counts` (`{"pragmas": {kind: n}, "types": {...}, "headers": {...}, "constructs": {...}}`). Commented-out lines (first non-space characters `//`) are skipped.
- Produces: `write_scan(dir_design: Path, dir_out: Path) -> Path` that writes `scan.json` and returns its path.

- [ ] **Step 1: Write the failing tests**

```python
# tests/test_scan.py
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


def test_write_scan_writes_json(tmp_path: Path):
    d = make_design(tmp_path)
    out = write_scan(d, tmp_path)
    assert out.name == "scan.json" and out.exists()
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `.venv/Scripts/python -m pytest tests/test_scan.py -q`
Expected: FAIL with `ModuleNotFoundError: No module named 'hlsfactory_agent.scan'`

- [ ] **Step 3: Write the implementation**

```python
# hlsfactory_agent/scan.py
"""Deterministic inventory of what a Vitis HLS design contains, so nothing can be dropped silently."""

from __future__ import annotations

import json
import re
from collections import Counter
from pathlib import Path

SOURCE_SUFFIXES = (".cpp", ".cc", ".c", ".h", ".hpp")

RE_PRAGMA = re.compile(r"^\s*#\s*pragma\s+HLS\s+([A-Za-z_]+)", re.IGNORECASE)
RE_TYPES = re.compile(r"\b(ap_int|ap_uint|ap_fixed|ap_ufixed|hls::stream|hls::vector)\b")
RE_HEADER = re.compile(r'^\s*#\s*include\s*[<"]((?:ap_int|ap_fixed|hls_stream|hls_math|hls_vector|ap_axi_sdata)\.h)[>"]')
RE_CONSTRUCTS = {
    "dynamic_memory": re.compile(r"\bmalloc\s*\(|\bcalloc\s*\(|\bnew\s+[A-Za-z_]"),
    "stl_container": re.compile(r"\bstd::(vector|map|set|string|list|deque)\b"),
}


def _is_comment(line: str) -> bool:
    return line.lstrip().startswith("//")


def scan_design(dir_design: Path) -> dict:
    dir_design = Path(dir_design)
    pragmas: list[dict] = []
    types: list[dict] = []
    headers: list[dict] = []
    constructs: list[dict] = []
    files = sorted(p for p in dir_design.rglob("*") if p.is_file() and p.suffix.lower() in SOURCE_SUFFIXES)
    for p in files:
        rel = str(p.relative_to(dir_design))
        text = p.read_text(encoding="utf-8", errors="replace")
        for i, line in enumerate(text.splitlines(), start=1):
            if _is_comment(line):
                continue
            m = RE_PRAGMA.match(line)
            if m:
                pragmas.append({"kind": m.group(1).lower(), "file": rel, "line": i, "text": line.strip()})
                continue
            m = RE_HEADER.match(line)
            if m:
                headers.append({"kind": m.group(1), "file": rel, "line": i, "text": line.strip()})
                continue
            for t in RE_TYPES.findall(line):
                types.append({"kind": t, "file": rel, "line": i, "text": line.strip()})
            for kind, rx in RE_CONSTRUCTS.items():
                if rx.search(line):
                    constructs.append({"kind": kind, "file": rel, "line": i, "text": line.strip()})
    # a function name appearing inside its own body is a recursion candidate
    for p in files:
        text = p.read_text(encoding="utf-8", errors="replace")
        for m in re.finditer(r"^\s*[A-Za-z_][\w:<>,\s\*&]*?\b([A-Za-z_]\w*)\s*\([^;{]*\)\s*\{", text, re.MULTILINE):
            name = m.group(1)
            body_start = m.end()
            depth, j = 1, body_start
            while j < len(text) and depth:
                depth += text[j] == "{"
                depth -= text[j] == "}"
                j += 1
            if re.search(rf"\b{re.escape(name)}\s*\(", text[body_start:j]):
                line_no = text.count("\n", 0, m.start()) + 1
                constructs.append({"kind": "recursion_candidate", "file": str(p.relative_to(dir_design)), "line": line_no, "text": name})
    return {
        "pragmas": pragmas,
        "types": types,
        "headers": headers,
        "constructs": constructs,
        "counts": {
            "pragmas": dict(Counter(x["kind"] for x in pragmas)),
            "types": dict(Counter(x["kind"] for x in types)),
            "headers": dict(Counter(x["kind"] for x in headers)),
            "constructs": dict(Counter(x["kind"] for x in constructs)),
        },
    }


def write_scan(dir_design: Path, dir_out: Path) -> Path:
    out = Path(dir_out) / "scan.json"
    out.write_text(json.dumps(scan_design(dir_design), indent=2), encoding="utf-8")
    return out
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `.venv/Scripts/python -m pytest tests/test_scan.py -q`
Expected: 3 passed

- [ ] **Step 5: Commit**

```bash
git add hlsfactory_agent/scan.py tests/test_scan.py
git commit -m "Add deterministic construct scanner for Vitis designs"
```

---

### Task 2: Report reconciliation check

**Files:**
- Modify: `hlsfactory_agent/translate.py` (`check_translated_design`, `HLSTranslationRun.prepare_run_area`, `build_translate_prompt`)
- Modify: `exp/run_translate/run_repo.py` (pass scan to resumed static checks)
- Test: `tests/test_translate.py`

**Interfaces:**
- Consumes: `scan_design` from Task 1.
- Produces: `check_translated_design(dir_output, target, scan: dict | None = None)`; when `scan` is given, adds check `report_covers_all_pragma_kinds` and result key `unaccounted_pragma_kinds` (list of kinds from `scan["counts"]["pragmas"]` that the report never mentions). A kind is mentioned if the report text contains `#pragma HLS <kind>` case-insensitively or the bare kind name as a word.
- Produces: `prepare_run_area` writes `scan.json` at the run-area root; the prompt tells the agent it exists and that every kind in it must appear in the report.

- [ ] **Step 1: Write the failing tests**

```python
# append to tests/test_translate.py
def test_reconciliation_flags_pragma_kind_missing_from_report(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "translation_report.md").write_text("| mac.cpp | `#pragma HLS PIPELINE II=1` | moved |\n", encoding="utf-8")
    scan = {"counts": {"pragmas": {"pipeline": 1, "array_partition": 2}}}
    r = check_translated_design(out, CATAPULT, scan=scan)
    assert not r["passed"]
    assert r["unaccounted_pragma_kinds"] == ["array_partition"]
    assert r["checks"]["report_covers_all_pragma_kinds"] is False


def test_reconciliation_accepts_report_naming_every_kind(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "translation_report.md").write_text(
        "| mac.cpp | `#pragma HLS PIPELINE II=1` | moved |\n| mac.cpp | all 2 ARRAY_PARTITION pragmas | DROPPED |\n",
        encoding="utf-8",
    )
    scan = {"counts": {"pragmas": {"pipeline": 1, "array_partition": 2}}}
    r = check_translated_design(out, CATAPULT, scan=scan)
    assert r["passed"], r["failures"]
    assert r["unaccounted_pragma_kinds"] == []


def test_check_without_scan_is_unchanged(tmp_path: Path):
    out = make_good_design(tmp_path)
    r = check_translated_design(out, CATAPULT)
    assert r["passed"] and "report_covers_all_pragma_kinds" not in r["checks"]
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `.venv/Scripts/python -m pytest tests/test_translate.py -q -k reconciliation`
Expected: FAIL with `TypeError: check_translated_design() got an unexpected keyword argument 'scan'`

- [ ] **Step 3: Implement the check**

In `check_translated_design`, change the signature to `def check_translated_design(dir_output: Path, target: TargetSpec, scan: dict | None = None) -> dict:` and insert before `result["passed"] = ...`:

```python
    if scan is not None:
        report = dir_output / "translation_report.md"
        report_text = report.read_text(encoding="utf-8", errors="replace").lower() if report.exists() else ""
        kinds = sorted(scan.get("counts", {}).get("pragmas", {}).keys())
        unaccounted = [
            k for k in kinds
            if not re.search(rf"#\s*pragma\s+hls\s+{re.escape(k)}\b", report_text)
            and not re.search(rf"\b{re.escape(k)}\b", report_text)
        ]
        checks["report_covers_all_pragma_kinds"] = not unaccounted
        result["unaccounted_pragma_kinds"] = unaccounted
        if unaccounted:
            result["failures"].append(f"report does not account for pragma kinds: {unaccounted}")
```

In `prepare_run_area`, after the copies, add:

```python
        from hlsfactory_agent.scan import write_scan
        write_scan(dir_run_area / INPUT_DIR_NAME, dir_run_area)
```

In `run` (translate.py), where the static check is called, load the scan and pass it:

```python
            scan_path = dir_run_area / "scan.json"
            scan = json.loads(scan_path.read_text(encoding="utf-8")) if scan_path.exists() else None
            check_data: dict[str, Any] = {
                "static": check_translated_design(dir_output, self.target, scan=scan),
                "container": run_container_checks(container, self.target),
            }
```

In `build_translate_prompt`, add to Step 1's bullets:

```python
        f"- `{CONTAINER_RUN_AREA}/scan.json` lists every `#pragma HLS` kind, vendor type, and header in the input with counts. "
        "Every pragma kind listed there must appear in your report, translated or DROPPED.\n"
```

In `run_repo.py` `translate_one`, in the resume branch, load the scan before recomputing the static check:

```python
            scan_path = dir_run / "run_area" / "scan.json"
            scan = json.loads(scan_path.read_text(encoding="utf-8")) if scan_path.exists() else None
            check["static"] = check_translated_design(dir_run / "run_area" / OUTPUT_DIR_NAME, get_target(target), scan=scan)
```

- [ ] **Step 4: Run all tests**

Run: `.venv/Scripts/python -m pytest tests -q`
Expected: all pass (16 + 3 scan)

- [ ] **Step 5: Commit**

```bash
git add hlsfactory_agent/translate.py exp/run_translate/run_repo.py tests/test_translate.py
git commit -m "Reconcile translation reports against the scanner inventory"
```

---

### Task 3: Output equivalence against the original

**Files:**
- Modify: `hlsfactory_agent/translate.py` (new `compare_outputs`)
- Modify: `exp/run_translate/run_repo.py` (`translate_one` compares oracle stdout with translated testbench stdout)
- Test: `tests/test_translate.py`

**Interfaces:**
- Produces: `compare_outputs(original: str, translated: str) -> dict` with keys `identical: bool`, `original_lines: int`, `translated_lines: int`, `differing: int`, `sample: list[str]` (up to 5 `"orig | trans"` pairs). Normalization: strip trailing whitespace, collapse runs of spaces, replace `-0.0000` style negative zeros with `0.0000`, drop empty lines. Compared line by line after normalization.

- [ ] **Step 1: Write the failing tests**

```python
# append to tests/test_translate.py
from hlsfactory_agent.translate import compare_outputs


def test_compare_outputs_identical_after_normalization():
    r = compare_outputs("a  b\n(-0.0000,0.0000)\nPASS \n", "a b\n(0.0000,-0.0000)\n\nPASS\n")
    assert r["identical"] and r["differing"] == 0


def test_compare_outputs_reports_differences():
    r = compare_outputs("x=1\nPASS\n", "x=2\nPASS\n")
    assert not r["identical"] and r["differing"] == 1 and r["sample"] == ["x=1 | x=2"]
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `.venv/Scripts/python -m pytest tests/test_translate.py -q -k compare_outputs`
Expected: FAIL with `ImportError: cannot import name 'compare_outputs'`

- [ ] **Step 3: Implement**

Add to `translate.py` after `run_container_checks`:

```python
_RE_NEG_ZERO = re.compile(r"-(0\.0+)\b")


def _normalize_output(text: str) -> list[str]:
    lines = []
    for raw in text.splitlines():
        s = " ".join(raw.split())
        s = _RE_NEG_ZERO.sub(r"\1", s)
        if s:
            lines.append(s)
    return lines


def compare_outputs(original: str, translated: str) -> dict:
    a, b = _normalize_output(original), _normalize_output(translated)
    n = max(len(a), len(b))
    differing = 0
    sample: list[str] = []
    for i in range(n):
        x = a[i] if i < len(a) else ""
        y = b[i] if i < len(b) else ""
        if x != y:
            differing += 1
            if len(sample) < 5:
                sample.append(f"{x} | {y}")
    return {
        "identical": differing == 0,
        "original_lines": len(a),
        "translated_lines": len(b),
        "differing": differing,
        "sample": sample,
    }
```

In `run_repo.py` `translate_one`, after `check` is available (both branches), add:

```python
        row["output_match"] = compare_outputs(
            (oracle.get("run") or {}).get("output", ""),
            (check.get("container", {}).get("testbench_run") or {}).get("output", ""),
        )
```

and add `output_match` to the imports and to the README table as a column `outputs match` showing `yes`/`no`/`n/a` (`n/a` when either side had no run).

- [ ] **Step 4: Run all tests**

Run: `.venv/Scripts/python -m pytest tests -q`
Expected: all pass

- [ ] **Step 5: Commit**

```bash
git add hlsfactory_agent/translate.py exp/run_translate/run_repo.py tests/test_translate.py
git commit -m "Compare translated testbench output against the original's"
```

---

### Task 4: Mechanical rewrite pre-pass

**Files:**
- Create: `hlsfactory_agent/rewrite.py`
- Modify: `hlsfactory_agent/translate.py` (`HLSTranslationRun.__init__` gains `prepass: bool = True`; `prepare_run_area` runs it; prompt gets an addendum)
- Test: `tests/test_rewrite.py`

**Interfaces:**
- Produces: `rewrite_design(dir_in: Path, dir_out: Path, target: TargetSpec, top: str | None) -> dict` that copies every file from `dir_in` to `dir_out` except `synth.tcl`, rewrites sources in place, and returns a log `{"headers": [...], "types": [...], "pragmas_moved": [...], "pragmas_dropped": [...], "top_marker": {...}, "residue": [...]}`, also written to `dir_out / "rewrite_log.json"`. Each entry has `file`, `line`, `before`, `after` (or `reason` for residue).
- Residue means: a line that still matches a forbidden pattern after rewriting, a `.range(` or `.to_int(` style vendor method, a rounding or overflow mode with no mapping, or a loop pragma whose loop header could not be found within 3 lines above.

- [ ] **Step 1: Write the failing tests**

```python
# tests/test_rewrite.py
from pathlib import Path

from hlsfactory_agent.rewrite import rewrite_design
from hlsfactory_agent.translate import CATAPULT, check_translated_design

ROOT = Path(__file__).resolve().parents[1]
FIXTURE = ROOT / "exp" / "run_translate" / "designs" / "vitis_mac"
EXPECTED = ROOT / "exp" / "run_translate" / "expected" / "catapult_mac"


def _norm(p: Path) -> str:
    return "\n".join(" ".join(l.split()) for l in p.read_text(encoding="utf-8").splitlines() if l.strip())


def test_prepass_reproduces_hand_translation_of_fixture(tmp_path: Path):
    log = rewrite_design(FIXTURE, tmp_path / "out", CATAPULT, top="mac")
    for name in ("mac.h", "mac.cpp", "testbench.cpp"):
        assert _norm(tmp_path / "out" / name) == _norm(EXPECTED / name), name
    assert log["residue"] == []
    assert {d["kind"] for d in log["pragmas_dropped"]} == {"interface", "array_partition"}
    assert len(log["pragmas_moved"]) == 1 and "hls_pipeline_init_interval 1" in log["pragmas_moved"][0]["after"]
    assert not (tmp_path / "out" / "synth.tcl").exists()


def test_prepass_maps_explicit_modes_and_flags_unknown_ones(tmp_path: Path):
    d = tmp_path / "in"
    d.mkdir()
    (d / "k.cpp").write_text(
        '#include "ap_fixed.h"\n'
        "typedef ap_fixed<8, 4, AP_RND, AP_SAT> a_t;\n"
        "typedef ap_fixed<8, 4, AP_RND_ZERO, AP_SAT_SYM> b_t;\n"
        "void top(a_t x) { ap_uint<3> y = x.range(2, 0); }\n",
        encoding="utf-8",
    )
    log = rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    text = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert "ac_fixed<8, 4, true, AC_RND, AC_SAT> a_t" in text
    assert any("AP_RND_ZERO" in r["reason"] for r in log["residue"])
    assert any(".range(" in r["reason"] for r in log["residue"])


def test_prepass_output_passes_static_check(tmp_path: Path):
    rewrite_design(FIXTURE, tmp_path / "out", CATAPULT, top="mac")
    (tmp_path / "out" / "run.tcl").write_text("solution file add\ngo analyze\ngo extract\n", encoding="utf-8")
    (tmp_path / "out" / "translation_report.md").write_text("pipeline interface array_partition\n", encoding="utf-8")
    r = check_translated_design(tmp_path / "out", CATAPULT)
    assert r["passed"], r["failures"]
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `.venv/Scripts/python -m pytest tests/test_rewrite.py -q`
Expected: FAIL with `ModuleNotFoundError: No module named 'hlsfactory_agent.rewrite'`

- [ ] **Step 3: Implement**

```python
# hlsfactory_agent/rewrite.py
"""Mechanical Vitis-to-target rewrite. Everything the mapping table can do without judgment.

What it cannot do is reported as residue for the agent. It never guesses at numerics.
"""

from __future__ import annotations

import json
import re
import shutil
from pathlib import Path

from hlsfactory_agent.translate import SOURCE_SUFFIXES, TargetSpec

HEADER_MAP = {"ap_int.h": "ac_int.h", "ap_fixed.h": "ac_fixed.h", "hls_stream.h": "ac_channel.h"}
MODE_MAP = {"AP_TRN": "AC_TRN", "AP_RND": "AC_RND", "AP_WRAP": "AC_WRAP", "AP_SAT": "AC_SAT"}

RE_INCLUDE = re.compile(r'^(\s*#\s*include\s*)([<"])(ap_int\.h|ap_fixed\.h|hls_stream\.h)([>"])')
RE_AP_INT = re.compile(r"\bap_int\s*<\s*([^<>,]+?)\s*>")
RE_AP_UINT = re.compile(r"\bap_uint\s*<\s*([^<>,]+?)\s*>")
RE_AP_FIXED = re.compile(r"\b(ap_fixed|ap_ufixed)\s*<\s*([^<>,]+?)\s*,\s*([^<>,]+?)\s*(?:,\s*([A-Z_]+)\s*)?(?:,\s*([A-Z_]+)\s*)?>")
RE_STREAM = re.compile(r"\bhls::stream\s*<")
RE_PRAGMA = re.compile(r"^(\s*)#\s*pragma\s+HLS\s+([A-Za-z_]+)(.*)$", re.IGNORECASE)
RE_LOOP = re.compile(r"^\s*(for|while)\b")
RE_VENDOR_METHOD = re.compile(r"\.(range|to_int|to_uint|to_double|to_float|reverse|and_reduce|or_reduce)\s*\(")
def _fn_def_regex(top: str) -> re.Pattern:
    # a definition line: return type, the top name, an argument list, optionally the opening brace; never ends with ';'
    return re.compile(r"^\s*[A-Za-z_][\w:<>,\s\*&]*?\b" + re.escape(top) + r"\s*\([^;]*\)\s*\{?\s*$")


def _rewrite_types(line: str, log: dict, rel: str, i: int) -> str:
    before = line
    line = RE_AP_INT.sub(r"ac_int<\1, true>", line)
    line = RE_AP_UINT.sub(r"ac_int<\1, false>", line)

    def fixed(m: re.Match) -> str:
        signed = "true" if m.group(1) == "ap_fixed" else "false"
        parts = [m.group(2), m.group(3), signed]
        for mode in (m.group(4), m.group(5)):
            if mode:
                if mode in MODE_MAP:
                    parts.append(MODE_MAP[mode])
                else:
                    log["residue"].append({"file": rel, "line": i, "reason": f"no Catapult mapping for fixed-point mode {mode}"})
                    parts.append(mode)
        return f"ac_fixed<{', '.join(parts)}>"

    line = RE_AP_FIXED.sub(fixed, line)
    line = RE_STREAM.sub("ac_channel<", line)
    if line != before:
        log["types"].append({"file": rel, "line": i, "before": before.strip(), "after": line.strip()})
    if RE_VENDOR_METHOD.search(line):
        log["residue"].append({"file": rel, "line": i, "reason": f"vendor method needs manual translation: {RE_VENDOR_METHOD.search(line).group(0)}"})
    return line


def _rewrite_pragmas(lines: list[str], target: TargetSpec, log: dict, rel: str) -> list[str]:
    out: list[str] = []
    pending_moves: list[tuple[int, str]] = []  # (index in out where loop header sits, pragma text)
    for i, line in enumerate(lines, start=1):
        m = RE_PRAGMA.match(line)
        if not m:
            out.append(line)
            continue
        indent, kind, rest = m.group(1), m.group(2).lower(), m.group(3)
        entry = {"file": rel, "line": i, "kind": kind, "before": line.strip()}
        if kind == "pipeline":
            ii = re.search(r"II\s*=\s*(\d+)", rest, re.IGNORECASE)
            new = f"#pragma hls_pipeline_init_interval {ii.group(1) if ii else 1}"
        elif kind == "unroll":
            f = re.search(r"factor\s*=\s*(\d+)", rest, re.IGNORECASE)
            new = f"#pragma hls_unroll {f.group(1)}" if f else "#pragma hls_unroll yes"
        elif kind == "inline":
            out.append(f"{indent}#pragma hls_design inline")
            entry["after"] = "#pragma hls_design inline"
            log["pragmas_moved"].append(entry)
            continue
        else:
            entry["reason"] = "no source-level equivalent in target"
            log["pragmas_dropped"].append(entry)
            continue
        # move before the nearest loop header within 3 lines above
        j = len(out) - 1
        found = None
        while j >= 0 and j >= len(out) - 3:
            if RE_LOOP.match(out[j]):
                found = j
                break
            j -= 1
        if found is None:
            log["residue"].append({"file": rel, "line": i, "reason": f"could not find loop header for `{line.strip()}`; place it manually"})
            continue
        loop_indent = re.match(r"^\s*", out[found]).group(0)
        out.insert(found, f"{loop_indent}{new}")
        entry["after"] = new
        log["pragmas_moved"].append(entry)
    return out


def _insert_top_marker(lines: list[str], top: str, target: TargetSpec, log: dict, rel: str) -> list[str]:
    rx = _fn_def_regex(top)
    for idx, line in enumerate(lines):
        if rx.match(line) and not line.rstrip().endswith(";"):
            lines.insert(idx, target.top_marker)
            log["top_marker"] = {"file": rel, "line": idx + 1, "inserted": True}
            return lines
    return lines


def rewrite_design(dir_in: Path, dir_out: Path, target: TargetSpec, top: str | None) -> dict:
    dir_in, dir_out = Path(dir_in), Path(dir_out)
    dir_out.mkdir(parents=True, exist_ok=True)
    log: dict = {"headers": [], "types": [], "pragmas_moved": [], "pragmas_dropped": [], "top_marker": {"inserted": False}, "residue": []}
    for p in sorted(dir_in.rglob("*")):
        if not p.is_file() or p.name == "synth.tcl":
            continue
        dst = dir_out / p.relative_to(dir_in)
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(p, dst)
    for p in sorted(dir_out.rglob("*")):
        if not p.is_file() or p.suffix.lower() not in SOURCE_SUFFIXES:
            continue
        rel = str(p.relative_to(dir_out))
        lines = p.read_text(encoding="utf-8", errors="replace").splitlines()
        new_lines: list[str] = []
        for i, line in enumerate(lines, start=1):
            m = RE_INCLUDE.match(line)
            if m:
                new = f"{m.group(1)}{m.group(2)}{HEADER_MAP[m.group(3)]}{m.group(4)}"
                log["headers"].append({"file": rel, "line": i, "before": line.strip(), "after": new.strip()})
                new_lines.append(new)
                continue
            new_lines.append(_rewrite_types(line, log, rel, i))
        new_lines = _rewrite_pragmas(new_lines, target, log, rel)
        if top and p.suffix.lower() in (".cpp", ".cc", ".c") and not log["top_marker"]["inserted"]:
            new_lines = _insert_top_marker(new_lines, top, target, log, rel)
        text = "\n".join(new_lines) + "\n"
        for pat in target.forbidden_patterns:
            for k, l in enumerate(text.splitlines(), start=1):
                if re.search(pat, l) and not l.lstrip().startswith("//"):
                    log["residue"].append({"file": rel, "line": k, "reason": f"still matches `{pat}`: {l.strip()}"})
        p.write_text(text, encoding="utf-8")
    (dir_out / "rewrite_log.json").write_text(json.dumps(log, indent=2), encoding="utf-8")
    return log
```

Then wire it into `translate.py`:

- `HLSTranslationRun.__init__` gains `prepass: bool = True` stored as `self.prepass`.
- In `prepare_run_area`, replace the empty output folder creation with:

```python
        dir_output = dir_run_area / OUTPUT_DIR_NAME
        if self.prepass:
            from hlsfactory_agent.rewrite import rewrite_design
            rewrite_design(dir_run_area / INPUT_DIR_NAME, dir_output, self.target, find_top_from_synth_tcl(self.dir_design))
        else:
            dir_output.mkdir(parents=True, exist_ok=True)
        os.chmod(dir_output, 0o777)
```

- `build_translate_prompt(design_name, target, prepass=False)`; when `prepass` is true, Step 2 is replaced by:

```python
        "### Step 2: A mechanical pre-pass already ran\n"
        f"'./{OUTPUT_DIR_NAME}' already contains every input file with headers, types, and loop pragmas rewritten by a script, "
        f"and `rewrite_log.json` listing what it changed, what it dropped, and a `residue` list of what it could not handle.\n"
        "- Read `rewrite_log.json` first. Fix every residue item by hand.\n"
        "- Review the moved pragmas: the script places a loop pragma on the line before the nearest loop header; correct any it placed wrongly.\n"
        "- Do not redo the mechanical work; do verify it in Step 6.\n"
```

and `run` passes `prepass=self.prepass` when building the prompt.

- [ ] **Step 4: Run all tests**

Run: `.venv/Scripts/python -m pytest tests -q`
Expected: all pass. If `test_prepass_reproduces_hand_translation_of_fixture` fails on whitespace only, fix `_norm`; if it fails on content, fix the rewriter, never the expected folder.

- [ ] **Step 5: Live check, Docker and key needed**

Run: `PYTHONPATH=. .venv/Scripts/python exp/run_translate/run.py --design exp/run_translate/designs/vitis_mac --target catapult`
Expected: `passed=True` and the session shows fewer tool calls than the 22 of the no-prepass run.

- [ ] **Step 6: Commit**

```bash
git add hlsfactory_agent/rewrite.py hlsfactory_agent/translate.py tests/test_rewrite.py
git commit -m "Add mechanical rewrite pre-pass; the agent handles residue"
```

---

### Task 5: Fixer loop in the harness

**Files:**
- Modify: `hlsfactory_agent/translate.py` (`HLSTranslationRun.__init__` gains `attempts: int = 1`; new `build_retry_prompt`; `run` loops)
- Modify: `exp/run_translate/run_repo.py` (`--attempts` flag passed through)
- Test: `tests/test_translate.py`

**Interfaces:**
- Produces: `build_retry_prompt(base_prompt: str, check_data: dict) -> str` that appends a section `## Previous attempt failed` listing static failures, unaccounted pragma kinds, every syntax check with non-zero exit and its last 40 lines, and the testbench build or run output tail, and instructs: fix in place, do not start over, rerun Step 6.
- Produces: `check_data["attempts"]` (list of per-attempt `{"attempt": n, "passed": bool, "failures": [...]}`) and `check_data["attempts_used"]`.

- [ ] **Step 1: Write the failing tests**

```python
# append to tests/test_translate.py
from hlsfactory_agent.translate import build_retry_prompt


def test_retry_prompt_carries_failures_and_tool_output():
    check = {
        "static": {"failures": ["missing required file run.tcl"], "unaccounted_pragma_kinds": ["dataflow"]},
        "container": {
            "syntax_check": {"a.cpp": {"exit_code": 1, "output": "a.cpp:3: error: unknown type"}},
            "testbench_build": {"exit_code": 1, "output": "ld: undefined reference"},
            "testbench_run": {"exit_code": None, "output": "not run: build failed"},
        },
    }
    p = build_retry_prompt("BASE", check)
    assert p.startswith("BASE")
    for needle in ("Previous attempt failed", "missing required file run.tcl", "dataflow", "unknown type", "undefined reference", "Do not start over"):
        assert needle in p, needle
```

- [ ] **Step 2: Run test to verify it fails**

Run: `.venv/Scripts/python -m pytest tests/test_translate.py -q -k retry_prompt`
Expected: FAIL with `ImportError: cannot import name 'build_retry_prompt'`

- [ ] **Step 3: Implement**

Add to `translate.py`:

```python
def build_retry_prompt(base_prompt: str, check_data: dict) -> str:
    static = check_data.get("static", {})
    cont = check_data.get("container", {})
    lines = ["", "## Previous attempt failed", "The harness checked your previous output and found these problems. Fix them in place in "
             f"'./{OUTPUT_DIR_NAME}'. Do not start over. Then rerun Step 6 and the checklist.", ""]
    for f in static.get("failures", []):
        lines.append(f"- static check: {f}")
    for k in static.get("unaccounted_pragma_kinds", []):
        lines.append(f"- report does not mention pragma kind `{k}`")
    for name, v in (cont.get("syntax_check") or {}).items():
        if v.get("exit_code") not in (0, None):
            tail = "\n".join((v.get("output") or "").splitlines()[-40:])
            lines += [f"- syntax check failed for {name}:", "```", tail, "```"]
    for key in ("testbench_build", "testbench_run"):
        v = cont.get(key) or {}
        if v.get("exit_code") not in (0, None):
            tail = "\n".join((v.get("output") or "").splitlines()[-40:])
            lines += [f"- {key.replace('_', ' ')} failed:", "```", tail, "```"]
    return base_prompt + "\n".join(lines) + "\n"
```

In `HLSTranslationRun.__init__` add `attempts: int = 1` stored as `self.attempts`. In `run`, wrap the agent call, session capture, and checks in a loop:

```python
            attempts_log: list[dict] = []
            prompt_this = prompt
            for attempt in range(1, self.attempts + 1):
                cmd = f"umask 000 && timeout {self.agent_timeout_s}s pi -p {shlex.quote(prompt_this)}"
                exit_code, output_agent = container.exec_run(["sh", "-lc", cmd], environment={"OPENROUTER_API_KEY": self.api_key}, workdir=CONTAINER_RUN_AREA)
                # (session capture unchanged, but collect every session file: run_data["session_data"] becomes a list per attempt)
                check_data = {"static": check_translated_design(dir_output, self.target, scan=scan), "container": run_container_checks(container, self.target)}
                passed = bool(check_data["static"]["passed"] and check_data["container"]["testbench_ok"])
                attempts_log.append({"attempt": attempt, "passed": passed, "failures": check_data["static"]["failures"]})
                if passed:
                    break
                prompt_this = build_retry_prompt(prompt, check_data)
            check_data["attempts"] = attempts_log
            check_data["attempts_used"] = len(attempts_log)
```

Session capture per attempt: after each agent call, glob `*.jsonl` in the sessions folder, take the newest by mtime, load it into `run_data.setdefault("sessions", []).append(...)`, and export its HTML. Keep `run_data["session_data"]` pointing at the last attempt's events so `session_stats` in `run_repo.py` still works.

In `run_repo.py`, add `parser.add_argument("--attempts", type=int, default=1)` and pass `attempts=attempts` into `HLSTranslationRun(...)` through `translate_one`'s signature.

- [ ] **Step 4: Run all tests**

Run: `.venv/Scripts/python -m pytest tests -q`
Expected: all pass

- [ ] **Step 5: Live check, Docker and key needed**

Run the fixture with `--attempts 2` through `run_repo.py --only` on a design that failed before, `n256_no_StagePipeline` from the HP-FFT-HLS extraction if it is still under `runs/`, and confirm `check_data.json` shows `attempts_used` of 2 and a pass on the second.

- [ ] **Step 6: Commit**

```bash
git add hlsfactory_agent/translate.py exp/run_translate/run_repo.py tests/test_translate.py
git commit -m "Add bounded fixer loop driven by harness check results"
```

---

### Task 6: XLS target through xlscc, emulation rung only

**Files:**
- Create: `hlsfactory_agent/xls_emu_include/xls_emu.h`
- Modify: `hlsfactory_agent/translate.py` (new `XLSCC` `TargetSpec`, registered in `TARGETS`; `render_xlscc_driver`; `TargetSpec.include_dirs` support)
- Modify: `hlsfactory_agent/rewrite.py` (target-aware pragma spellings and top marker)
- Test: `tests/test_translate.py`, `tests/test_rewrite.py`

**Interfaces:**
- The XLS version uses the same ac types as Catapult, because xlscc ships ac-compatible integer and fixed headers. Streams become `__xls_channel<T>`. The top gets `#pragma hls_top`. Loop pragmas use the Catapult spellings `hls_pipeline_init_interval` and `hls_unroll yes`. Everything else is dropped.
- `xls_emu.h` defines, for host compilation only: `template <typename T> class __xls_channel { std::deque<T> q; public: T read(); void write(const T&); bool nb_read(T&); }` and nothing else. Emulation compiles with `-I ac_types_include -I xls_emu_include`.
- Produces: `render_xlscc_driver(top: str, sources: list[str]) -> str` writing `run_xlscc.sh` with `xlscc <kernel>.cpp --top <top> > <top>.ir`, `opt_main <top>.ir > <top>.opt.ir`, `codegen_main --generator=pipeline --delay_model=unit --pipeline_stages=1 <top>.opt.ir > <top>.v`. Rung 3 for XLS is deferred until the XLS binaries are in the image; the driver is written and checked for its three commands.
- `TargetSpec` gains `extra_include_dirs: tuple[tuple[Path, str], ...] = ()` copied into the run area alongside `include_dir`; `run_container_checks` and `run_checks_standalone` add `-I` for each.

- [ ] **Step 1: Write the failing tests**

```python
# append to tests/test_translate.py
from hlsfactory_agent.translate import XLSCC, render_xlscc_driver


def test_xlscc_target_registered():
    assert get_target("xlscc") is XLSCC
    assert XLSCC.top_marker == "#pragma hls_top"
    assert XLSCC.driver_file == "run_xlscc.sh"
    assert any("__xls_channel" in v for v in XLSCC.type_map.values())


def test_xlscc_driver_has_three_tools():
    d = render_xlscc_driver("fft", ["fft.cpp"])
    for tok in ("xlscc fft.cpp --top fft", "opt_main", "codegen_main", "--pipeline_stages=1"):
        assert tok in d


def test_xlscc_prompt_mentions_channel_and_top():
    p = build_translate_prompt("k", XLSCC)
    assert "__xls_channel" in p and "#pragma hls_top" in p and "run_xlscc.sh" in p
```

```python
# append to tests/test_rewrite.py
from hlsfactory_agent.translate import XLSCC


def test_prepass_xlscc_uses_channels_and_top_pragma(tmp_path: Path):
    rewrite_design(FIXTURE, tmp_path / "out", XLSCC, top="mac")
    cpp = (tmp_path / "out" / "mac.cpp").read_text(encoding="utf-8")
    h = (tmp_path / "out" / "mac.h").read_text(encoding="utf-8")
    assert "#pragma hls_top" in cpp and "__xls_channel<data_t>" in h and "hls_pipeline_init_interval 1" in cpp
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `.venv/Scripts/python -m pytest tests -q -k xlscc`
Expected: FAIL with `ImportError: cannot import name 'XLSCC'`

- [ ] **Step 3: Implement**

`hlsfactory_agent/xls_emu_include/xls_emu.h`:

```cpp
// Host-side emulation of the xlscc channel type so translated XLS designs compile and run with clang.
// Only for testbench emulation. xlscc itself provides the real __xls_channel.
#ifndef XLS_EMU_H
#define XLS_EMU_H
#include <deque>
#include <stdexcept>
template <typename T>
class __xls_channel {
  std::deque<T> q_;
 public:
  T read() {
    if (q_.empty()) throw std::runtime_error("read on empty __xls_channel");
    T v = q_.front(); q_.pop_front(); return v;
  }
  void write(const T &v) { q_.push_back(v); }
  bool nb_read(T &v) { if (q_.empty()) return false; v = q_.front(); q_.pop_front(); return true; }
  bool empty() const { return q_.empty(); }
};
#endif
```

In `translate.py`, add the field `extra_include_dirs: tuple[tuple[Path, str], ...] = field(default_factory=tuple)` to `TargetSpec`, copy each `(src, name)` into the run area in `prepare_run_area` and mount each in `run_checks_standalone`, and build the `-I` list in `run_container_checks` from `[target.include_dir_name] + [name for _, name in target.extra_include_dirs]`. Then:

```python
DIR_XLS_EMU_INCLUDE = DIR_CURRENT / "xls_emu_include"


def render_xlscc_driver(top: str, sources: list[str]) -> str:
    kernel = " ".join(sources)
    return (
        "#!/bin/sh\n# XLS run script generated by hlsfactory_agent.translate\nset -e\n"
        f"xlscc {kernel} --top {top} > {top}.ir\n"
        f"opt_main {top}.ir > {top}.opt.ir\n"
        f"codegen_main --generator=pipeline --delay_model=unit --pipeline_stages=1 {top}.opt.ir > {top}.v\n"
    )


XLSCC = TargetSpec(
    name="xlscc",
    display_name="Google XLS (xlscc)",
    include_dir=DIR_AC_TYPES_INCLUDE,
    include_dir_name="ac_types_include",
    extra_include_dirs=((DIR_XLS_EMU_INCLUDE, "xls_emu_include"),),
    cxx_std_check="c++17",
    header_map={"ap_int.h": "ac_int.h", "ap_fixed.h": "ac_fixed.h", "hls_stream.h": "xls_emu.h  (for emulation; xlscc provides __xls_channel itself)"},
    type_map={
        "ap_int<N>": "ac_int<N, true>",
        "ap_uint<N>": "ac_int<N, false>",
        "ap_fixed<W, I>": "ac_fixed<W, I, true>",
        "ap_ufixed<W, I>": "ac_fixed<W, I, false>",
        "hls::stream<T>": "__xls_channel<T>",
        "stream.read()": "channel.read()",
        "stream.write(v)": "channel.write(v)",
        "float / double": "NOT SUPPORTED by XLS; if the design needs floating point, stop and report it as untranslatable",
    },
    pragma_rules=(
        PragmaRule("#pragma HLS PIPELINE II=n  (inside the loop body)", "#pragma hls_pipeline_init_interval n", "on the line directly BEFORE the `for` statement", note="xlscc honors II=1 reliably; larger values may be ignored, record it"),
        PragmaRule("#pragma HLS UNROLL  (inside the loop body)", "#pragma hls_unroll yes", "on the line directly BEFORE the `for` statement"),
        PragmaRule("#pragma HLS UNROLL factor=n", "#pragma hls_unroll yes", "on the line directly BEFORE the `for` statement", note="xlscc has no partial unroll; record the factor as lost"),
        PragmaRule("#pragma HLS ARRAY_PARTITION ...", None, "removed from source", note="XLS has no partition; arrays become registers or memories by its own rules"),
        PragmaRule("#pragma HLS DATAFLOW", None, "removed from source", note="XLS procs are a different model; record as DROPPED"),
        PragmaRule("any other #pragma HLS ...", None, "removed from source", note="Remove and list as DROPPED with the original text."),
    ),
    forbidden_patterns=CATAPULT.forbidden_patterns,
    required_files=("testbench.cpp", "run_xlscc.sh", "translation_report.md"),
    driver_file="run_xlscc.sh",
    driver_required_tokens=("xlscc", "opt_main", "codegen_main"),
    top_marker="#pragma hls_top",
    extra_rules=(
        "Loops that are not pipelined must be fully unrolled for xlscc: add `#pragma hls_unroll yes` before any loop with no pragma and a constant bound; if the bound is not constant, report it.",
        "No dynamic memory, no recursion, no pointers into arrays; rewrite with indexing.",
        "The testbench is host-only: it includes `xls_emu.h` for the channel type and is never given to xlscc.",
    ),
)

TARGETS[XLSCC.name] = XLSCC
```

In `build_translate_prompt`, choose the driver example by target name: Catapult uses `render_catapult_run_tcl`, xlscc uses `render_xlscc_driver("<top_function>", ["<kernel>.cpp"])`, and the Step 7 text says the file must be executable. In `_render_missing_driver`, render the xlscc driver when `self.target.name == "xlscc"`.

In `rewrite.py`, make `HEADER_MAP` and the stream replacement target-dependent: pass `target` into `_rewrite_types`; for `xlscc`, `hls_stream.h` maps to `xls_emu.h` and `hls::stream<` to `__xls_channel<`. The pragma spellings are shared. `_insert_top_marker` already uses `target.top_marker`.

- [ ] **Step 4: Run all tests**

Run: `.venv/Scripts/python -m pytest tests -q`
Expected: all pass

- [ ] **Step 5: Live emulation check, Docker and key needed**

Run: `PYTHONPATH=. .venv/Scripts/python exp/run_translate/run.py --design exp/run_translate/designs/vitis_mac --target xlscc`
Expected: static checks pass, syntax and testbench pass under emulation, `run_xlscc.sh` present. Rung 3 is not run.

- [ ] **Step 6: Commit**

```bash
git add hlsfactory_agent/xls_emu_include hlsfactory_agent/translate.py hlsfactory_agent/rewrite.py tests
git commit -m "Add XLS (xlscc) as a second translation target, emulation rung"
```

---

## Follow-ups outside this plan

- XLS synthesis rung: add the XLS release binaries to `docker_images/Dockerfile` and a `run_xlscc_synth` container check. Needs an image rebuild.
- Lessons store keyed by scanner category, measured with and without.
- pass at k: `--samples k` in `run_repo.py` running independent fresh attempts per design.
- Spec rewrite of sections 4, 5.2, 5.3, 5.4, 9, 12 to the portable-layer wording.
- Dataset repository and an exporter from `runs/` into it.
