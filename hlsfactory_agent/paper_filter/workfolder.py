"""Build the per-paper work folder that is bind-mounted into the agent container."""

import json
import shutil
from pathlib import Path

from hlsfactory_agent.paper_filter.manifest import PaperMeta
from hlsfactory_agent.paper_filter.pdf import PdfDoc

DIR_CURRENT = Path(__file__).resolve().parent
DIR_TOOLS_SRC = DIR_CURRENT / "container_tools"
SCAN_SRC = DIR_CURRENT / "scan.py"


def prepare_work_folder(
    dir_run_area: Path,
    meta: PaperMeta,
    prepass: dict,
    doc: PdfDoc,
    snapshot: dict,
) -> Path:
    """Layout (all under dir_run_area, mounted at /workspace/run_area):

    paper.pdf  pages/pNN.txt  meta.json  prepass.json  known_repos.json  tools/  output/
    """
    if dir_run_area.exists():
        shutil.rmtree(dir_run_area)
    (dir_run_area / "pages").mkdir(parents=True)
    (dir_run_area / "output").mkdir()
    (dir_run_area / "tools").mkdir()

    shutil.copy(meta.pdf, dir_run_area / "paper.pdf")
    for i, text in enumerate(doc.pages, start=1):
        (dir_run_area / "pages" / f"p{i:02d}.txt").write_text(text)
    (dir_run_area / "meta.json").write_text(json.dumps(meta.to_json(), indent=2))
    (dir_run_area / "prepass.json").write_text(json.dumps(prepass, indent=2))
    (dir_run_area / "known_repos.json").write_text(
        json.dumps(sorted(v["display_id"] for v in snapshot.values()), indent=2)
    )

    for src in DIR_TOOLS_SRC.glob("*.sh"):
        dst = dir_run_area / "tools" / src.name
        shutil.copy(src, dst)
        dst.chmod(0o755)
    scan_dst = dir_run_area / "tools" / "hls_scan.py"
    shutil.copy(SCAN_SRC, scan_dst)
    scan_dst.chmod(0o755)
    return dir_run_area
