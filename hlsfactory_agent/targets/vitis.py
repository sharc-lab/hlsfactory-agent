from __future__ import annotations

import re
from pathlib import Path

from hlsfactory_agent.spec import SynthResult, TargetSpec

DIR_PKG = Path(__file__).resolve().parent.parent


def parse_vitis_report(dir_run: Path) -> SynthResult:
    """Top-level row of the csynth summary: latency, interval, and resource counts."""
    logs = sorted(dir_run.glob("vitis_hls.log"))
    log = logs[0].read_text(encoding="utf-8", errors="replace") if logs else ""
    errors = [l for l in log.splitlines() if l.startswith("ERROR:")]
    rpts = sorted(dir_run.glob("*/*/syn/report/csynth.rpt"))
    if not rpts:
        return SynthResult("FAILED", first_error=errors[0][:300] if errors else "", failure_class="other" if errors else "no-report")
    for line in rpts[0].read_text(encoding="utf-8", errors="replace").splitlines():
        cells = [c.strip() for c in line.split("|")]
        if len(cells) > 13 and cells[1].startswith("+ "):
            def num(x: str) -> int | None:
                x = x.split("(")[0].strip()
                return int(x) if x.isdigit() else (0 if x == "-" else None)

            return SynthResult(
                "PASS",
                latency=num(cells[4]),
                throughput=num(cells[7]),
                resources={k: num(cells[i]) for k, i in (("BRAM", 10), ("DSP", 11), ("FF", 12), ("LUT", 13))},
            )
    return SynthResult("FAILED", first_error="csynth.rpt has no module row", failure_class="no-report")


# Source-side only: Vitis is never a translation target. Only the synthesis fields are meaningful.
VITIS = TargetSpec(
    name="vitis",
    display_name="AMD Vitis HLS",
    include_dir=DIR_PKG / "vitis_hls_include",
    include_dir_name="vitis_hls_include",
    cxx_std_check="c++17",
    header_map={},
    type_map={},
    pragma_rules=(),
    forbidden_patterns=(),
    required_files=("synth.tcl",),
    driver_file="synth.tcl",
    driver_required_tokens=("csynth_design",),
    top_marker="",
    tool="vitis",
    synth_command=("vitis_hls", "-f", "synth.tcl"),
    synth_timeout_s=600,
    synth_success_marker="*/*/syn/report/csynth.rpt",
    parse_report=parse_vitis_report,
)
