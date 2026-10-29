from pathlib import Path

from hlsfactory_agent.extraction_repair import CloneIndex, oracle_sources, repair_data_files, repair_includes, tcl_files


def _design(tmp_path: Path) -> Path:
    d = tmp_path / "design"
    d.mkdir()
    (d / "synth.tcl").write_text(
        'set_top k\nadd_files k.cpp -cflags "-I[pwd] -DFOO=1"\nadd_files {helper.cpp}\nadd_files k.h\nadd_files missing.cpp\n'
        'add_files -tb "testbench.cpp"\nadd_files -tb in.dat\n', encoding="utf-8")
    (d / "k.cpp").write_text('#include "k.h"\n#include "../../include/align.h"\nint k(int x) { return x; }\n', encoding="utf-8")
    (d / "helper.cpp").write_text("int helper() { return 1; }\n", encoding="utf-8")
    (d / "k.h").write_text('#include "ap_int.h"\n#include "utils/deep.hpp"\nint k(int);\n', encoding="utf-8")
    (d / "align.h").write_text("// local copy\n", encoding="utf-8")
    (d / "host.cpp").write_text("int main() { return 1; }\n", encoding="utf-8")
    (d / "testbench.cpp").write_text('#include "helper.cpp"\n#define DATA_DIR "data"\nint main() { std::string(DATA_DIR)+"/input.dat"; "golden.txt"; return 0; }\n', encoding="utf-8")
    return d


def _clone(tmp_path: Path) -> Path:
    c = tmp_path / "clone"
    (c / "include").mkdir(parents=True)
    (c / "include" / "align.h").write_text("// local copy\n", encoding="utf-8")
    (c / "src" / "utils").mkdir(parents=True)
    (c / "src" / "utils" / "deep.hpp").write_text('#include "deeper.hpp"\n', encoding="utf-8")
    (c / "src" / "utils" / "deeper.hpp").write_text("// deeper\n", encoding="utf-8")
    (c / "test" / "design" / "data").mkdir(parents=True)
    (c / "test" / "design" / "data" / "input.dat").write_text("1\n", encoding="utf-8")
    (c / "test" / "other" / "data").mkdir(parents=True)
    (c / "test" / "other" / "data" / "input.dat").write_text("2\n", encoding="utf-8")
    return c


def test_tcl_files_and_oracle_sources(tmp_path):
    d = _design(tmp_path)
    assert [(e["file"], e["tb"]) for e in tcl_files(d)] == [("k.cpp", False), ("helper.cpp", False), ("k.h", False), ("missing.cpp", False), ("testbench.cpp", True), ("in.dat", True)]
    sel = oracle_sources(d)
    assert sel["sources"] == ["k.cpp", "testbench.cpp"]  # helper.cpp is included by the testbench; host.cpp is not in the script
    assert sel["dropped"] == {"missing.cpp": "listed in synth.tcl but missing", "helper.cpp": "included by another source"}
    assert sel["cflags"] == ["-I[pwd]", "-DFOO=1"]
    (d / "synth.tcl").unlink()  # no script: every top-level source, one main
    sel = oracle_sources(d)
    assert sel["sources"] == ["k.cpp", "testbench.cpp"] and "host.cpp" in sel["dropped"]


def test_repair_includes_local_and_clone(tmp_path):
    d = _design(tmp_path)
    log = repair_includes(d, None)
    assert log["rewritten"] == {"k.cpp: ../../include/align.h": "align.h"}
    assert log["unresolved"] == ["k.h: utils/deep.hpp"]
    assert '#include "align.h"' in (d / "k.cpp").read_text(encoding="utf-8")
    log = repair_includes(d, CloneIndex(_clone(tmp_path)))
    assert set(log["copied"]) == {"utils/deep.hpp", "utils/deeper.hpp"} and log["unresolved"] == []


def test_repair_data_files_prefers_design_name(tmp_path):
    d = _design(tmp_path)
    log = repair_data_files(d, CloneIndex(_clone(tmp_path)))
    assert (d / "data" / "input.dat").read_text(encoding="utf-8") == "1\n"
    assert "ambiguous" in log["copied"]["data/input.dat"] and log["missing"] == ["golden.txt"]
