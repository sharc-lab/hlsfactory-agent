import json
import os
import threading
import traceback
from pathlib import Path
import shutil
from typing import Any
from concurrent.futures import ThreadPoolExecutor, as_completed

from tools import VitisHLSSynthTool

DIR_CURRENT = Path(__file__).resolve().parent
DIR_RUNS = DIR_CURRENT / "runs"

DIR_BUILDS = DIR_CURRENT / "builds"
DIR_BUILDS.mkdir(parents=True, exist_ok=True)

N_JOBS_RUNS = 16
N_JOBS_DESIGNS = 8

jsonl_files = sorted(DIR_RUNS.rglob("*.jsonl"))
run_dirs = list(
    dict.fromkeys(
        jsonl_file.parent.parent.parent.parent for jsonl_file in jsonl_files
    )
)


def write_build_check_data(build_dir: Path, build_check_data: dict[str, Any]) -> None:
    fp_build_check_data = build_dir / "build_check_data.json"
    fp_build_check_data.write_text(json.dumps(build_check_data, indent=4))


def find_single_file_in_dir_by_glob(dir_path: Path, glob_pattern: str) -> Path | None:
    files = list(dir_path.glob(glob_pattern))
    if len(files) != 1:
        return None
    return files[0]


def process_run_dir(run_dir: Path) -> dict[str, Any]:
    fp_run_data = run_dir / "run_data.json"
    run_data = json.loads(fp_run_data.read_text())
    
    run_id = run_data["run_id"]
    build_dir = DIR_BUILDS / run_id
    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)

    repo_id = run_data["repo_id"]
    repo_name = repo_id.split("/")[-1]

    build_check_data = {}
    build_check_data["run_id"] = run_id
    build_check_data["repo_id"] = repo_id
    build_check_data["check__designs"] = []

    def persist_run_state() -> None:
        write_build_check_data(build_dir, build_check_data)

    try:
        run_designs_dir = run_dir / "run_area" / "output_hls_designs"
        check__has_run_designs_dir = run_designs_dir.exists()
        build_check_data["check__has_run_designs_dir"] = check__has_run_designs_dir
        if not check__has_run_designs_dir:
            return build_check_data

        repo_designs_root = run_designs_dir / repo_name
        if not repo_designs_root.exists():
            build_check_data["check__has_repo_designs_dir"] = False
            build_check_data["check__repo_designs_path"] = str(repo_designs_root)
            num_designs = 0
            design_dirs: list[Path] = []
        else:
            build_check_data["check__has_repo_designs_dir"] = True
            design_dirs = sorted(
                [d for d in repo_designs_root.iterdir() if d.is_dir()],
                key=lambda p: p.name,
            )
            num_designs = len(design_dirs)

        build_check_data["check__has_nonzero_designs"] = num_designs > 0
        build_check_data["check__num_designs"] = num_designs

        build_dir_designs = build_dir / "designs"
        build_dir_designs.mkdir(parents=True, exist_ok=True)

        def process_design_dir(design_dir: Path) -> dict[str, Any]:
            design_name = design_dir.name
            build_check_data__design: dict[str, Any] = {"design_name": design_name}
            try:
                design_dir_copy = build_dir_designs / design_name
                shutil.copytree(design_dir, design_dir_copy, dirs_exist_ok=True)

                synth_tcl_file = find_single_file_in_dir_by_glob(design_dir_copy, "synth.tcl")
                check__has_synth_tcl = synth_tcl_file is not None
                build_check_data__design["check__has_synth_tcl"] = check__has_synth_tcl
                if not check__has_synth_tcl:
                    return build_check_data__design

                assert synth_tcl_file is not None
                tcl_content = synth_tcl_file.read_text()
                tcl_content_lines = [line.strip() for line in tcl_content.splitlines()]
                tcl_content_lines = [
                    line for line in tcl_content_lines if not line.startswith("#")
                ]

                check__has_csynth_design = any(
                    line.startswith("csynth_design") for line in tcl_content_lines
                )
                build_check_data__design["check__has_csynth_design"] = check__has_csynth_design
                if not check__has_csynth_design:
                    return build_check_data__design

                csynth_tool = VitisHLSSynthTool()
                csynth_tool_data = csynth_tool.run(
                    synth_tcl_file, work_dir=design_dir_copy
                )
                execution_data = csynth_tool_data.data_execution
                build_check_data__design["execution_data"] = execution_data.to_dict()

                check__csynth_ran_successfully = execution_data.return_code == 0
                build_check_data__design["check__csynth_ran_successfully"] = (
                    check__csynth_ran_successfully
                )
                return build_check_data__design
            except Exception as exc:
                build_check_data__design["check__error"] = repr(exc)
                build_check_data__design["check__traceback"] = traceback.format_exc()
                return build_check_data__design

        # Workers only return dicts; parent thread assigns aggregate once.
        if num_designs:
            n_design_workers = min(N_JOBS_DESIGNS, num_designs)
            with ThreadPoolExecutor(max_workers=n_design_workers) as design_executor:
                build_check_data["check__designs"] = list(
                    design_executor.map(process_design_dir, design_dirs)
                )
            for dd in build_check_data["check__designs"]:
                print(f"Processed design: {dd['design_name']}")
    finally:
        persist_run_state()

    return build_check_data


if __name__ == "__main__":

    # 2. Use ThreadPoolExecutor instead of ProcessPoolExecutor
    with ThreadPoolExecutor(max_workers=N_JOBS_RUNS) as executor:
        futures = {executor.submit(process_run_dir, run_dir): run_dir for run_dir in run_dirs}
        for future in as_completed(futures):
            run_dir = futures[future]
            try:
                build_check_data = future.result()
                print(f"Processed run: {build_check_data['run_id']}")
            except Exception:
                print(f"Failed run_dir: {run_dir}")
                print(traceback.format_exc())