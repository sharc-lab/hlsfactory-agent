"""Before vs After: heterogeneous repos -> standardized output."""
from __future__ import annotations

import json
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

# -- Shared academic style --
FONT = "serif"
BLUE = "#2c5f8a"
GREEN = "#3a8c5c"
GRAY = "#6b7280"
LIGHT_BLUE = "#a8c4db"
LIGHT_GREEN = "#a3d4b3"


def main() -> None:
    repo_root = Path(__file__).resolve().parents[1]
    output_dir = repo_root / "output"
    figures_dir = repo_root / "figures"
    figures_dir.mkdir(parents=True, exist_ok=True)

    # Load GitHub stats (before)
    github_stats = json.loads((figures_dir / "repo_github_stats.json").read_text())
    github_by_name = {s["name"]: s for s in github_stats}

    # Compute output stats (after) for each repo
    output_stats = {}
    for repo_dir in sorted(d for d in output_dir.iterdir() if d.is_dir()):
        name = repo_dir.name
        design_dirs = [d for d in repo_dir.iterdir() if d.is_dir()]
        if not design_dirs:
            continue

        # Files per design
        files_per_design = []
        all_exts = set()
        max_depth = 0
        for dd in design_dirs:
            files = list(dd.rglob("*"))
            files = [f for f in files if f.is_file()]
            files_per_design.append(len(files))
            for f in files:
                if f.suffix:
                    all_exts.add(f.suffix)
                # Depth relative to design dir
                rel = f.relative_to(dd)
                depth = len(rel.parts)
                max_depth = max(max_depth, depth)

        output_stats[name] = {
            "designs": len(design_dirs),
            "total_files": sum(files_per_design),
            "avg_files_per_design": np.mean(files_per_design) if files_per_design else 0,
            "std_files_per_design": np.std(files_per_design) if files_per_design else 0,
            "ext_count": len(all_exts),
            "max_depth": max_depth + 1,  # +1 for design dir level
        }

    # Match repos present in both datasets
    common = sorted(set(github_by_name) & set(output_stats))

    before_files = [github_by_name[n]["files"] for n in common]
    before_depth = [github_by_name[n]["max_depth"] for n in common]
    before_exts = [github_by_name[n]["ext_count"] for n in common]

    after_files = [output_stats[n]["total_files"] for n in common]
    after_depth = [output_stats[n]["max_depth"] for n in common]
    after_exts = [output_stats[n]["ext_count"] for n in common]

    after_fpd_mean = [output_stats[n]["avg_files_per_design"] for n in common]
    after_fpd_std = [output_stats[n]["std_files_per_design"] for n in common]

    # --- Style ---
    plt.rcParams.update({
        "font.family": FONT,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.linewidth": 0.6,
        "axes.edgecolor": "#cccccc",
        "xtick.color": "#444444",
        "ytick.color": "#444444",
    })

    fig, axes = plt.subplots(1, 3, figsize=(7.5, 3.5))

    # --- Panel 1: File Count ---
    ax = axes[0]
    x = np.arange(len(common))
    w = 0.35
    ax.bar(x - w / 2, before_files, w, color=LIGHT_BLUE, edgecolor=BLUE,
           linewidth=0.5, label="Original repo", zorder=3)
    ax.bar(x + w / 2, after_files, w, color=LIGHT_GREEN, edgecolor=GREEN,
           linewidth=0.5, label="Extracted output", zorder=3)
    ax.set_ylabel("Total files", fontsize=11)
    ax.set_yscale("log")
    ax.set_xticks(x)
    ax.set_xticklabels(common, rotation=75, ha="right", fontsize=8)
    ax.tick_params(labelsize=10, length=2)
    ax.grid(axis="y", color="#e8e8e8", linewidth=0.4, zorder=0)
    ax.legend(fontsize=9, frameon=True, edgecolor="#dddddd", fancybox=False,
              loc="upper right")
    ax.set_title("File count", fontsize=12, pad=6)

    # --- Panel 2: Max Directory Depth ---
    ax = axes[1]
    ax.bar(x - w / 2, before_depth, w, color=LIGHT_BLUE, edgecolor=BLUE,
           linewidth=0.5, label="Original repo", zorder=3)
    ax.bar(x + w / 2, after_depth, w, color=LIGHT_GREEN, edgecolor=GREEN,
           linewidth=0.5, label="Extracted output", zorder=3)
    ax.set_ylabel("Max directory depth", fontsize=11)
    ax.set_xticks(x)
    ax.set_xticklabels(common, rotation=75, ha="right", fontsize=8)
    ax.tick_params(labelsize=10, length=2)
    ax.grid(axis="y", color="#e8e8e8", linewidth=0.4, zorder=0)
    ax.set_title("Directory depth", fontsize=12, pad=6)

    # --- Panel 3: File Extension Diversity ---
    ax = axes[2]
    ax.bar(x - w / 2, before_exts, w, color=LIGHT_BLUE, edgecolor=BLUE,
           linewidth=0.5, label="Original repo", zorder=3)
    ax.bar(x + w / 2, after_exts, w, color=LIGHT_GREEN, edgecolor=GREEN,
           linewidth=0.5, label="Extracted output", zorder=3)
    ax.set_ylabel("Unique file extensions", fontsize=11)
    ax.set_xticks(x)
    ax.set_xticklabels(common, rotation=75, ha="right", fontsize=8)
    ax.tick_params(labelsize=10, length=2)
    ax.grid(axis="y", color="#e8e8e8", linewidth=0.4, zorder=0)
    ax.set_title("File type diversity", fontsize=12, pad=6)

    fig.tight_layout(pad=0.8, w_pad=1.5)
    fig.savefig(figures_dir / "standardization_comparison.png", dpi=300,
                bbox_inches="tight", facecolor="white", edgecolor="none")
    fig.savefig(figures_dir / "standardization_comparison.svg",
                bbox_inches="tight", facecolor="white", edgecolor="none")
    plt.close(fig)

    # --- Also make a summary box-plot style figure ---
    fig2, axes2 = plt.subplots(1, 3, figsize=(7, 3.2))

    # Overall title
    fig2.suptitle(
        "Structural comparison: original repositories vs. standardized output",
        fontsize=13, y=0.99, color="#333333",
    )

    metrics = [
        ("Total files", before_files, after_files),
        ("Max directory depth", before_depth, after_depth),
        ("Unique file extensions", before_exts, after_exts),
    ]

    for ax, (title, before, after) in zip(axes2, metrics):
        bp = ax.boxplot(
            [before, after],
            tick_labels=["Original\nrepo", "Extracted\noutput"],
            widths=0.5,
            patch_artist=True,
            medianprops=dict(color="#333333", linewidth=1.2),
            flierprops=dict(marker="o", markersize=3, alpha=0.5),
        )
        bp["boxes"][0].set_facecolor(LIGHT_BLUE)
        bp["boxes"][0].set_edgecolor(BLUE)
        bp["boxes"][1].set_facecolor(LIGHT_GREEN)
        bp["boxes"][1].set_edgecolor(GREEN)

        ax.set_title(title, fontsize=12, pad=6)
        ax.tick_params(labelsize=10, length=2)
        ax.grid(axis="y", color="#e8e8e8", linewidth=0.4, zorder=0)
        ax.spines["top"].set_visible(False)
        ax.spines["right"].set_visible(False)
        ax.spines["left"].set_linewidth(0.5)
        ax.spines["bottom"].set_linewidth(0.5)

        # Annotate reduction
        med_b = np.median(before)
        med_a = np.median(after)
        if med_b > 0:
            reduction = (1 - med_a / med_b) * 100
            ax.text(0.5, 0.95, f"{reduction:.0f}% reduction (median)",
                    transform=ax.transAxes, ha="center", va="top",
                    fontsize=10, color=GRAY)

    fig2.tight_layout(pad=0.8, w_pad=1.2, rect=(0, 0, 1, 0.94))
    fig2.savefig(figures_dir / "standardization_boxplot.png", dpi=300,
                 bbox_inches="tight", facecolor="white", edgecolor="none")
    fig2.savefig(figures_dir / "standardization_boxplot.svg",
                 bbox_inches="tight", facecolor="white", edgecolor="none")
    plt.close(fig2)

    # Print summary stats
    print(f"Repos compared: {len(common)}")
    print(f"\nFile count     — before: median={np.median(before_files):.0f}, range=[{min(before_files)}, {max(before_files)}]")
    print(f"                  after:  median={np.median(after_files):.0f}, range=[{min(after_files)}, {max(after_files)}]")
    print(f"Max depth      — before: median={np.median(before_depth):.0f}, range=[{min(before_depth)}, {max(before_depth)}]")
    print(f"                  after:  median={np.median(after_depth):.0f}, range=[{min(after_depth)}, {max(after_depth)}]")
    print(f"Extensions     — before: median={np.median(before_exts):.0f}, range=[{min(before_exts)}, {max(before_exts)}]")
    print(f"                  after:  median={np.median(after_exts):.0f}, range=[{min(after_exts)}, {max(after_exts)}]")
    print(f"\nWrote {figures_dir / 'standardization_comparison.png'}")
    print(f"Wrote {figures_dir / 'standardization_boxplot.png'}")


if __name__ == "__main__":
    main()
