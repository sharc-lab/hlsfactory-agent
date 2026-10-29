"""Mechanical repair of an extracted design so its original testbench builds and runs.

The extraction agent flattens a repository folder into one design directory and writes a `synth.tcl`.
Three things go wrong often enough to fix by rule rather than by hand:
- the script's source set is the truth about which files form the program, yet the oracle used to compile
  every source in the folder (testbenches that `#include` the kernel file, leftover host programs, second
  `main`s): `oracle_sources` picks the script's set and drops what another unit already includes;
- `#include "../../include/x.h"` paths escape the flattened folder although the file was copied next to the
  includer, or was not copied at all: `repair_includes` points them at the local copy, or recovers the file
  from a clone of the repository;
- data files the testbench opens were not copied: `repair_data_files` recovers them from the clone.
Everything is logged to `<design>/extraction_repair.json`."""

from __future__ import annotations

import json
import os
import re
import shlex
import shutil
from pathlib import Path

SOURCE_SUFFIXES = (".cpp", ".cc", ".c")
HEADER_SUFFIXES = (".h", ".hpp", ".hh", ".inc", ".tpp")
DATA_SUFFIXES = ("dat", "txt", "csv", "bin", "hex", "mem", "json")
DIR_VITIS_HLS_INCLUDE = Path(__file__).resolve().parent / "vitis_hls_include"
SYSTEM_INCLUDE_DIRS = (Path("/usr/include"), Path("/usr/local/include"))  # `#include "stdio.h"` is a system header too
REPAIR_LOG = "extraction_repair.json"
MAX_ROUNDS = 8  # a recovered header may include more missing headers

RE_ADD_FILES = re.compile(r"^[ \t]*add_files[ \t]+(.*?)[ \t]*$", re.M)
RE_INCLUDE = re.compile(r'(?m)^[ \t]*#[ \t]*include[ \t]*"([^"\n]+)"')
RE_COMMENT = re.compile(r"//[^\n]*|/\*.*?\*/", re.S)
RE_MAIN = re.compile(r"(?m)^[ \t]*(?:int|void)[ \t]+main[ \t]*\(")
RE_DATA_LITERAL = re.compile(r'"([^"\n\\]*\.(?:' + "|".join(DATA_SUFFIXES) + r'))"')
RE_DATA_DIR = re.compile(r'#[ \t]*define[ \t]+DATA_DIR[ \t]+"([^"\n]*)"')
KEPT_FLAGS = ("-I", "-D", "-U", "-std=")


def _read(p: Path) -> str:
    return p.read_text(encoding="utf-8", errors="replace")


def _defines_main(p: Path) -> bool:
    return bool(RE_MAIN.search(RE_COMMENT.sub("", _read(p))))


def tcl_files(dir_design: Path) -> list[dict]:
    """The `add_files` entries of synth.tcl in script order: {"file", "tb", "cflags"} each. Quoted and braced
    file arguments are accepted; `-cflags` and `-csimflags` both apply to the C simulation build."""
    tcl = Path(dir_design) / "synth.tcl"
    if not tcl.exists():
        return []
    out: list[dict] = []
    for m in RE_ADD_FILES.finditer(_read(tcl)):
        try:
            toks = shlex.split(m.group(1).replace("{", '"').replace("}", '"'))
        except ValueError:
            continue
        tb, cflags, files = False, [], []
        i = 0
        while i < len(toks):
            t = toks[i]
            if t == "-tb":
                tb = True
            elif t in ("-cflags", "-csimflags") and i + 1 < len(toks):
                cflags += toks[i + 1].split()
                i += 1
            elif not t.startswith("-"):
                files.append(t)
            i += 1
        out += [{"file": f, "tb": tb, "cflags": " ".join(cflags)} for f in files]
    return out


def oracle_sources(dir_design: Path) -> dict:
    """The translation units the oracle compiles, the unit that defines `main` last, with the script's compile
    flags (`[pwd]` left for the caller to substitute). Returns {"sources", "cflags", "dropped": {file: why}}."""
    d = Path(dir_design)
    top_level = sorted(p.name for p in d.iterdir() if p.is_file() and p.suffix.lower() in SOURCE_SUFFIXES)
    entries = tcl_files(d)
    listed = [e for e in entries if e["file"].lower().endswith(SOURCE_SUFFIXES)]
    dropped = {e["file"]: "listed in synth.tcl but missing" for e in listed if not (d / e["file"]).is_file()}
    sources = list(dict.fromkeys(e["file"] for e in listed if (d / e["file"]).is_file())) or list(top_level)
    tb_marked = {e["file"] for e in entries if e["tb"]}
    if not any(_defines_main(d / s) for s in sources):  # the script omits the testbench: take the top-level one
        extra = [s for s in top_level if s not in sources and _defines_main(d / s)]
        sources += [s for s in extra if s == "testbench.cpp"] or extra[:1]
    included = {Path(inc).name for s in sources for inc in RE_INCLUDE.findall(RE_COMMENT.sub("", _read(d / s)))}
    for s in list(sources):
        if Path(s).name in included and len(sources) > 1:
            sources.remove(s)
            dropped[s] = "included by another source"
    mains = [s for s in sources if _defines_main(d / s)]
    if len(mains) > 1:
        keep = ([m for m in mains if m in tb_marked] or [m for m in mains if Path(m).name == "testbench.cpp"]
                or [m for m in mains if re.search(r"test|_tb\b|bench", m, re.I)] or mains)[0]
        for m in mains:
            if m != keep:
                sources.remove(m)
                dropped[m] = f"defines main alongside {keep}"
        mains = [keep]
    if mains:
        sources = [s for s in sources if s != mains[0]] + [mains[0]]
    flags: list[str] = []
    for e in entries:
        if not (d / e["file"]).exists():
            continue
        for f in e["cflags"].split():
            if f.startswith(KEPT_FLAGS) and f not in flags:
                flags.append(f)
    return {"sources": sources, "cflags": flags, "dropped": dropped}


def _strip_dots(inc: str) -> str:
    parts = [x for x in Path(inc).parts if x not in ("..", ".")]
    return "/".join(parts)


class CloneIndex:
    """Files of a repository clone by path suffix, for recovering what the extraction left behind."""

    def __init__(self, dir_clone: Path):
        self.root = Path(dir_clone)
        self.files = [p for p in self.root.rglob("*") if p.is_file() and ".git" not in p.parts]

    def find(self, rel: str, hint: str = "") -> tuple[Path | None, str]:
        """The clone file whose path ends with `rel`; several hits are resolved by content, then by `hint`
        (the design name) in the path, then by path length. Returns (path, note)."""
        suffix = _strip_dots(rel)
        hits = [p for p in self.files if p.as_posix() == suffix or p.as_posix().endswith("/" + suffix)]
        if not hits:
            return None, "not in repo"
        if len(hits) == 1 or len({p.read_bytes() for p in hits}) == 1:
            return hits[0], ""
        named = [p for p in hits if hint and hint.lower() in p.as_posix().lower()]
        pick = sorted(named or hits, key=lambda p: (len(p.parts), p.as_posix()))[0]
        return pick, f"ambiguous: {len(hits)} candidates, took {pick.relative_to(self.root).as_posix()}"


def _design_files(d: Path) -> list[Path]:
    return [p for p in d.rglob("*") if p.is_file() and p.suffix.lower() in SOURCE_SUFFIXES + HEADER_SUFFIXES]


def repair_includes(dir_design: Path, clone: CloneIndex | None = None, include_dirs: tuple[Path, ...] = (DIR_VITIS_HLS_INCLUDE,)) -> dict:
    """Make every quoted include resolve: a path that escapes the design (`../`) is pointed at the copy the
    extraction placed in the design when one exists, otherwise the file is recovered from the clone and placed
    under the design at its path without the `..` segments; a plain missing include is recovered next to the
    includer. A file the design already holds under its bare name wins over the clone (the extraction placed it
    there on purpose); tool and system headers are left to the include path."""
    d = Path(dir_design).resolve()
    log: dict = {"rewritten": {}, "copied": {}, "unresolved": []}
    for _ in range(MAX_ROUNDS):
        copied = False
        files = _design_files(d)
        for p in files:
            text = _read(p)
            new = text
            for inc in dict.fromkeys(RE_INCLUDE.findall(RE_COMMENT.sub("", text))):
                if (p.parent / inc).exists() or (d / inc).exists() or any((i / inc).exists() for i in include_dirs + SYSTEM_INCLUDE_DIRS):
                    continue  # resolves next to the includer, from the design root, or from a tool or system header
                key = f"{p.relative_to(d).as_posix()}: {inc}"
                target: Path | None = None
                local = [q for q in files if q.name == Path(inc).name]
                if local and len({q.read_bytes() for q in local}) == 1:
                    target = local[0]  # the extraction copied the file next to the includer, under its bare name
                elif clone is not None:
                    src, note = clone.find(inc, d.name)
                    if src is not None:
                        target = d / _strip_dots(inc) if inc.startswith(".") else p.parent / inc
                        target.parent.mkdir(parents=True, exist_ok=True)
                        shutil.copy2(src, target)
                        copied = True
                        log["copied"][target.relative_to(d).as_posix()] = (src.relative_to(clone.root).as_posix() + (f" ({note})" if note else ""))
                if target is None:
                    log["unresolved"].append(key)
                    continue
                rel = Path(os.path.relpath(target, p.parent)).as_posix()
                if rel != inc:
                    new = re.sub(r'(#[ \t]*include[ \t]*)"' + re.escape(inc) + '"', lambda m: m.group(1) + f'"{rel}"', new)
                    log["rewritten"][key] = rel
            if new != text:
                p.write_text(new, encoding="utf-8")
        if not copied:
            break
    log["unresolved"] = sorted(set(log["unresolved"]))
    return log


def repair_data_files(dir_design: Path, clone: CloneIndex) -> dict:
    """Copy from the clone every data file a source names in a string literal and the design does not hold.
    A literal starting with `/` is joined to the design's `DATA_DIR` define when there is one."""
    d = Path(dir_design).resolve()
    log: dict = {"copied": {}, "missing": []}
    texts = {p: _read(p) for p in _design_files(d)}
    data_dir = next((m.group(1) for t in texts.values() for m in [RE_DATA_DIR.search(t)] if m), "")
    for p, text in texts.items():
        for lit in dict.fromkeys(RE_DATA_LITERAL.findall(RE_COMMENT.sub("", text))):
            rel = lit.lstrip("/")
            if not rel or (d / rel).exists() or (data_dir and (d / data_dir / rel).exists()) or (p.parent / rel).exists():
                continue
            src, note = clone.find(rel, d.name)
            if src is None:
                log["missing"].append(lit)
                continue
            dst = d / data_dir / rel if lit.startswith("/") and data_dir else d / rel
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, dst)
            log["copied"][dst.relative_to(d).as_posix()] = src.relative_to(clone.root).as_posix() + (f" ({note})" if note else "")
    log["missing"] = sorted(set(log["missing"]))
    return log


def repair_extraction(dir_design: Path, dir_clone: Path | None = None) -> dict:
    """Run every repair on a design in place and write the log next to it."""
    clone = CloneIndex(dir_clone) if dir_clone and Path(dir_clone).is_dir() else None
    log = {"clone": str(dir_clone) if clone else None, "includes": repair_includes(dir_design, clone)}
    if clone is not None:
        log["data"] = repair_data_files(dir_design, clone)
    log["sources"] = oracle_sources(dir_design)
    (Path(dir_design) / REPAIR_LOG).write_text(json.dumps(log, indent=2), encoding="utf-8")
    return log
