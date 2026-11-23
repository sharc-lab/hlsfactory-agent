from hlsfactory_agent.paper_filter.scan import scan_dir, scan_paths


def write(root, rel, text):
    p = root / rel
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(text)


def test_single_vitis_kernel_counts_as_hls_found(tmp_path):
    write(tmp_path, "src/k.cpp", '#include "ap_int.h"\nvoid top(){\n#pragma HLS PIPELINE\n}\n')
    write(tmp_path, "src/util.cpp", "int main(){}\n")
    out = scan_dir(tmp_path)
    assert out["classification"] == "hls_found"
    assert out["hls_files"] == 1 and out["by_dialect"]["vitis"] == 1
    assert out["pragma_hls_count"] == 1
    assert out["examples"] == ["src/k.cpp"]


def test_no_hls_markers(tmp_path):
    write(tmp_path, "a.cpp", "#include <vector>\nint main(){}\n")
    write(tmp_path, "README.md", "#pragma HLS in a readme is ignored\n")
    assert scan_dir(tmp_path)["classification"] == "repo_no_hls"


def test_tapa_only_is_flagged_not_hls(tmp_path):
    write(tmp_path, "k.cpp", '#include <tapa.h>\nvoid f(tapa::mmap<int> m){}\n#pragma HLS INLINE\n')
    out = scan_dir(tmp_path)
    assert out["classification"] == "tapa_only"
    assert out["tapa_files"] == 1 and out["hls_files"] == 0


def test_mixed_tapa_and_vitis_is_hls_found_with_flag(tmp_path):
    write(tmp_path, "t.cpp", "#include <tapa.h>\n")
    write(tmp_path, "v.cpp", "#pragma HLS UNROLL\n")
    out = scan_dir(tmp_path)
    assert out["classification"] == "hls_found" and out["mixed_tapa"]


def test_other_dialects_are_recognised(tmp_path):
    write(tmp_path, "i.cpp", '#include "HLS/hls.h"\ncomponent int f(int x){return x;}\n')
    write(tmp_path, "c.cpp", '#include "ac_int.h"\nac_channel<int> ch;\n')
    out = scan_dir(tmp_path)
    assert out["by_dialect"]["intel"] == 1 and out["by_dialect"]["catapult"] == 1
    assert out["classification"] == "hls_found"


def test_tcl_hints_alone_do_not_make_hls_found(tmp_path):
    write(tmp_path, "run.tcl", "open_project p\nset_top my_top\ncsynth_design\n")
    out = scan_dir(tmp_path)
    assert out["classification"] == "repo_no_hls"
    assert out["tcl_set_top"] == ["my_top"] and out["tcl_csynth_count"] == 1


def test_git_dir_is_skipped(tmp_path):
    write(tmp_path, ".git/x.cpp", "#pragma HLS PIPELINE\n")
    assert scan_dir(tmp_path)["classification"] == "repo_no_hls"


def test_tree_mode_never_claims_hls_found():
    out = scan_paths(["hls/kernel.cpp", "hls/run.tcl", "data/big.bin"])
    assert out["classification"] == "unscanned" and out["partial_scan"]
    assert out["source_files"] == 1 and out["tcl_files"] == 1


def test_source_only_clone_skips_data_files(tmp_path):
    import subprocess

    from hlsfactory_agent.paper_filter.scan import clone_source_only, scan_dir, source_bytes

    src = tmp_path / "origin"
    src.mkdir()
    git = lambda *a, cwd=src: subprocess.run(["git", *a], cwd=cwd, check=True, capture_output=True)
    git("init", "-q", "-b", "main")
    git("config", "user.email", "t@t")
    git("config", "user.name", "t")
    git("config", "uploadpack.allowFilter", "true")
    git("config", "uploadpack.allowAnySHA1InWant", "true")
    write(src, "hls/k.cpp", "#pragma HLS PIPELINE\n")
    write(src, "run.tcl", "set_top k\n")
    (src / "data").mkdir()
    (src / "data" / "big.bin").write_bytes(b"\0" * 200_000)
    git("add", ".")
    git("commit", "-q", "-m", "x")

    dest = tmp_path / "clone"
    assert clone_source_only(f"file://{src}", dest) is None
    assert (dest / "hls" / "k.cpp").exists()
    assert not (dest / "data" / "big.bin").exists()  # data blob never checked out
    assert scan_dir(dest)["classification"] == "hls_found"

    tree = [{"path": "hls/k.cpp", "type": "blob", "size": 21},
            {"path": "data/big.bin", "type": "blob", "size": 200_000}]
    assert source_bytes(tree) == 21


def test_source_only_clone_reports_errors(tmp_path):
    from hlsfactory_agent.paper_filter.scan import clone_source_only

    err = clone_source_only(f"file://{tmp_path}/does-not-exist", tmp_path / "c")
    assert err and "git clone failed" in err


def test_ap_int_include_alone_is_only_a_weak_marker(tmp_path):
    write(tmp_path, "host.cpp", '#include "ap_int.h"\nint main(){}\n')
    out = scan_dir(tmp_path)
    assert out["classification"] == "weak_hls_only"
    assert out["hls_files"] == 0 and out["weak_files"] == 1


def test_host_file_with_ap_int_does_not_rescue_a_tapa_repo(tmp_path):
    # the real InTAR shape: many TAPA kernels plus a host file including ap_int.h
    for i in range(3):
        write(tmp_path, f"k{i}.cpp", "#include <tapa.h>\n#pragma HLS INLINE\n")
    write(tmp_path, "host.cpp", '#include "ap_int.h"\n')
    assert scan_dir(tmp_path)["classification"] == "tapa_only"


def test_tapa_dominant_when_tapa_files_outnumber_hls_files(tmp_path):
    for i in range(3):
        write(tmp_path, f"k{i}.cpp", "#include <tapa.h>\n")
    write(tmp_path, "v.cpp", "#pragma HLS PIPELINE\n")
    out = scan_dir(tmp_path)
    assert out["classification"] == "hls_found" and out["tapa_dominant"]


def test_tcl_project_with_c_source_counts_as_hls(tmp_path):
    # MachSuite layout: plain C kernel, directives in a TCL-sourced file, no #pragma HLS.
    write(tmp_path, "aes/aes.c", "void aes256_encrypt_ecb(int *k) { k[0] = 1; }\n")
    write(tmp_path, "aes/hls.tcl", "open_project p\nadd_files aes.c\nset_top aes256_encrypt_ecb\nsource ./aes_dir\ncsynth_design\n")
    out = scan_dir(tmp_path)
    assert out["classification"] == "hls_tcl_only" and out["source_files"] == 1


def test_tcl_project_does_not_override_tapa(tmp_path):
    write(tmp_path, "k.cpp", '#include <tapa.h>\n')
    write(tmp_path, "run.tcl", "set_top k\ncsynth_design\n")
    assert scan_dir(tmp_path)["classification"] == "tapa_only"


def test_archive_only_repo_goes_to_review(tmp_path):
    write(tmp_path, "train.py", "print(1)\n")
    write(tmp_path, "data/HLSyn_data.tar.gz", "x")
    out = scan_dir(tmp_path)
    assert out["classification"] == "archive_only"
    assert out["archives"] == ["data/HLSyn_data.tar.gz"]


def test_archives_do_not_override_real_hls(tmp_path):
    write(tmp_path, "k.cpp", "#pragma HLS PIPELINE\n")
    write(tmp_path, "data.zip", "x")
    assert scan_dir(tmp_path)["classification"] == "hls_found"


def test_archive_next_to_plain_source_stays_no_hls(tmp_path):
    write(tmp_path, "rtl/top.c", "int main(void) { return 0; }\n")
    write(tmp_path, "data/results.tar.gz", "x")
    assert scan_dir(tmp_path)["classification"] == "repo_no_hls"
