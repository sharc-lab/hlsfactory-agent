"""Turn HLSFactory dataset designs (dataset_hls.tcl) into the standalone layout run.py expects:
sources + testbench.cpp + data files + synth.tcl. Designs whose testbench spans several source files are skipped."""
from __future__ import annotations

import argparse
import json
import re
import shutil
from pathlib import Path

RE_ADD = re.compile(r"^\s*add_files\s+(-tb\s+)?(\S+)", re.M)
RE_TOP = re.compile(r"^\s*set_top\s+(\S+)", re.M)
RE_CLOCK = re.compile(r"^\s*create_clock\s+-period\s+(\S+)", re.M)
SOURCE_SUFFIXES = (".cpp", ".cc", ".c")


def import_design(dir_src: Path, dir_dst: Path) -> str:
    tcl = (dir_src / "dataset_hls.tcl").read_text(encoding="utf-8", errors="replace")
    top = RE_TOP.search(tcl)
    if not top:
        return "no set_top"
    files = [(bool(tb), dir_src / name) for tb, name in RE_ADD.findall(tcl)]
    if any(not p.exists() for _, p in files):
        return "missing file " + ", ".join(p.name for _, p in files if not p.exists())
    tb_sources = [p for tb, p in files if tb and p.suffix.lower() in SOURCE_SUFFIXES]
    if len(tb_sources) != 1:
        return f"{len(tb_sources)} testbench sources"
    if dir_dst.exists():
        shutil.rmtree(dir_dst)
    dir_dst.mkdir(parents=True)
    for p in sorted(dir_src.rglob("*")):  # headers and data alongside; the tcl scripts stay behind
        if p.is_file() and p.suffix.lower() != ".tcl":
            rel = p.relative_to(dir_src)
            (dir_dst / rel).parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(p, dir_dst / rel)
    tb = tb_sources[0].relative_to(dir_src)
    (dir_dst / tb).rename(dir_dst / "testbench.cpp")
    period = RE_CLOCK.search(tcl)
    lines = ["open_project prj", f"set_top {top.group(1)}"]
    lines += [f"add_files {p.relative_to(dir_src).as_posix()}" for tb_, p in files if not tb_]
    lines += ["add_files -tb testbench.cpp"]
    lines += [f"add_files -tb {p.relative_to(dir_src).as_posix()}" for tb_, p in files if tb_ and p != tb_sources[0]]
    lines += ["open_solution solution", "set_part xczu9eg-ffvb1156-2-i", f"create_clock -period {period.group(1) if period else 5}", "csynth_design", "exit"]
    (dir_dst / "synth.tcl").write_text("\n".join(lines) + "\n", encoding="utf-8")
    return "ok"


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--sources", type=Path, required=True, help="HLSFactory hls_dataset_sources dir")
    parser.add_argument("--sets", nargs="+", default=["vitis_examples", "pp4fpgas"])
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    report = {}
    for s in args.sets:
        for d in sorted(p for p in (args.sources / s).iterdir() if (p / "dataset_hls.tcl").exists()):
            report[f"{s}__{d.name}"] = import_design(d, args.out / f"{s}__{d.name}")
    (args.out / "import.json").write_text(json.dumps(report, indent=1), encoding="utf-8")
    ok = sum(v == "ok" for v in report.values())
    print(f"{ok}/{len(report)} imported")
    for k, v in report.items():
        if v != "ok":
            print(f"  {k}: {v}")


if __name__ == "__main__":
    main()
