"""Artifact Completeness Heatmap: per-repo artifact presence."""
from __future__ import annotations

import json
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap
import numpy as np

# -- Shared academic style --
FONT = "serif"
GRAY = "#6b7280"


def main() -> None:
    repo_root = Path(__file__).resolve().parents[1]
    output_dir = repo_root / "output"
    figures_dir = repo_root / "figures"
    figures_dir.mkdir(parents=True, exist_ok=True)

    artifact_types = ["Manifest", "Testbenches", "TCL Scripts", "Compile Logs", "READMEs"]

    rows: list[tuple[str, list[float]]] = []
    for repo_dir in sorted(d for d in output_dir.iterdir() if d.is_dir()):
        repo_name = repo_dir.name
        design_dirs = [d for d in repo_dir.iterdir() if d.is_dir()]
        if not design_dirs:
            continue
        n = len(design_dirs)

        manifest_path = repo_dir / "manifest.json"
        has_manifest = 1.0 if manifest_path.is_file() else 0.0
        if has_manifest:
            try:
                json.loads(manifest_path.read_text(encoding="utf-8"))
            except (json.JSONDecodeError, OSError):
                has_manifest = 0.5

        tb_count = tcl_count = log_count = readme_count = 0
        for dd in design_dirs:
            tb_patterns = ["*testbench*", "*tb_*", "*_tb.*", "*test_*", "*_test.*"]
            if any(dd.glob(p) for p in tb_patterns):
                tb_count += 1
            if list(dd.glob("*.tcl")):
                tcl_count += 1
            if list(dd.glob("compile_log*")):
                log_count += 1
            if list(dd.glob("README*")) or list(dd.glob("readme*")):
                readme_count += 1

        rows.append((repo_name, [
            has_manifest,
            tb_count / n if n else 0,
            tcl_count / n if n else 0,
            log_count / n if n else 0,
            readme_count / n if n else 0,
        ]))

    rows.sort(key=lambda r: sum(r[1]), reverse=True)
    repo_names = [r[0] for r in rows]
    data = np.array([r[1] for r in rows])

    plt.rcParams.update({
        "font.family": FONT,
        "axes.linewidth": 0.5,
    })

    fig_height = max(5.5, 0.28 * len(repo_names) + 1.8)
    fig, ax = plt.subplots(figsize=(4.5, fig_height))

    # Academic two-tone: white (0) -> light blue (0.5) -> deep blue (1)
    cmap = LinearSegmentedColormap.from_list(
        "academic", ["#ffffff", "#b8d4e8", "#2c5f8a"], N=256,
    )

    im = ax.imshow(data, aspect="auto", cmap=cmap, vmin=0, vmax=1, interpolation="nearest")

    # Thin cell borders
    for i in range(len(repo_names) + 1):
        ax.axhline(i - 0.5, color="#e0e0e0", linewidth=0.4)
    for j in range(len(artifact_types) + 1):
        ax.axvline(j - 0.5, color="#e0e0e0", linewidth=0.4)

    ax.set_xticks(range(len(artifact_types)))
    ax.set_xticklabels(artifact_types, fontsize=10, rotation=35, ha="right")
    ax.set_yticks(range(len(repo_names)))
    ax.set_yticklabels(repo_names, fontsize=10)

    # Annotate cells with simple text (avoids missing unicode glyphs)
    for i in range(len(repo_names)):
        for j in range(len(artifact_types)):
            val = data[i, j]
            if val >= 0.99:
                sym, col = "", "white"  # solid fill is sufficient
            elif val > 0:
                sym, col = f"{val:.0%}", "#333333"
            else:
                sym, col = "", "#bbbbbb"
            if sym:
                ax.text(j, i, sym, ha="center", va="center", fontsize=9,
                        color=col, fontweight="bold")

    cbar = fig.colorbar(im, ax=ax, shrink=0.5, pad=0.03, aspect=20)
    cbar.set_label("Coverage", fontsize=10, labelpad=4)
    cbar.set_ticks([0, 0.5, 1.0])
    cbar.set_ticklabels(["0%", "50%", "100%"])
    cbar.ax.tick_params(labelsize=10)
    cbar.outline.set_linewidth(0.4)

    ax.tick_params(length=0)

    fig.tight_layout(pad=0.8)
    fig.savefig(figures_dir / "artifact_completeness.png", dpi=300, bbox_inches="tight",
                facecolor="white", edgecolor="none")
    fig.savefig(figures_dir / "artifact_completeness.svg", bbox_inches="tight",
                facecolor="white", edgecolor="none")
    plt.close(fig)
    print(f"Wrote {figures_dir / 'artifact_completeness.png'}")


if __name__ == "__main__":
    main()
