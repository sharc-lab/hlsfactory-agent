"""Agent Efficiency: cost and API calls vs. designs extracted."""
from __future__ import annotations

import json
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from adjustText import adjust_text

# -- Shared academic style --
FONT = "serif"
ACCENT = "#2c5f8a"
GRAY = "#6b7280"


def load_latest_benchmarks(output_dir: Path) -> list[dict]:
    seen: dict[str, tuple[str, dict]] = {}
    for f in output_dir.glob("*_benchmark.json"):
        try:
            data = json.loads(f.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            continue
        repo_url = str(data.get("repo_url", "")).rstrip("/")
        repo_name = repo_url.split("/")[-1].removesuffix(".git") if repo_url else f.stem
        ts = str(data.get("timestamp", ""))
        prev = seen.get(repo_name)
        if prev is None or ts > prev[0]:
            data["_repo_name"] = repo_name
            seen[repo_name] = (ts, data)
    return [d for _, d in seen.values()]


def main() -> None:
    repo_root = Path(__file__).resolve().parents[1]
    output_dir = repo_root / "output"
    figures_dir = repo_root / "figures"
    figures_dir.mkdir(parents=True, exist_ok=True)

    benchmarks = load_latest_benchmarks(output_dir)
    benchmarks = [b for b in benchmarks if b["_repo_name"] != "HeCBench"]

    names = [b["_repo_name"] for b in benchmarks]
    designs = np.array([b.get("quality", {}).get("designs_found", 0) for b in benchmarks])
    costs = np.array([b.get("total_cost_usd", 0) for b in benchmarks])
    calls = np.array([b.get("total_api_calls", 0) for b in benchmarks], dtype=float)

    plt.rcParams.update({
        "font.family": FONT,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.linewidth": 0.6,
        "axes.edgecolor": "#cccccc",
        "xtick.color": "#444444",
        "ytick.color": "#444444",
        "xtick.major.width": 0.5,
        "ytick.major.width": 0.5,
    })

    fig, ax = plt.subplots(figsize=(6.0, 4.2))

    # Light grid
    ax.grid(True, axis="both", color="#e8e8e8", linewidth=0.5, zorder=0)

    # Bubble sizes
    min_s, max_s = 30, 280
    if calls.max() > calls.min():
        sizes = min_s + (max_s - min_s) * (calls - calls.min()) / (calls.max() - calls.min())
    else:
        sizes = np.full_like(calls, (min_s + max_s) / 2)

    scatter = ax.scatter(
        designs, costs, s=sizes, c=calls,
        cmap="Blues", edgecolors="#4a6fa5", linewidth=0.5,
        alpha=0.8, zorder=3, vmin=0,
    )

    # Label only clear outliers that separate from the main cluster
    texts = []
    for i, name in enumerate(names):
        is_outlier = (
            costs[i] >= 0.05                   # high cost
            or designs[i] >= 35                 # many designs
            or (calls[i] >= 55 and designs[i] <= 5)  # high effort, few designs
            or (designs[i] >= 25 and costs[i] >= 0.02)  # large + moderately expensive
        )
        if is_outlier:
            texts.append(ax.text(
                designs[i], costs[i], name,
                fontsize=6.5, color=GRAY,
            ))

    # Use adjustText to push labels apart and away from data points
    adjust_text(
        texts, ax=ax,
        x=designs.tolist(), y=costs.tolist(),
        arrowprops=dict(arrowstyle="-", color="#cccccc", lw=0.4),
        expand=(2.0, 2.5),
        force_text=(1.2, 1.5),
        force_points=(0.8, 0.8),
    )

    cbar = fig.colorbar(scatter, ax=ax, shrink=0.75, pad=0.03, aspect=25)
    cbar.set_label("API calls", fontsize=8.5, labelpad=6)
    cbar.ax.tick_params(labelsize=7.5)
    cbar.outline.set_linewidth(0.4)

    ax.set_xlabel("Designs extracted per repository", fontsize=9.5, labelpad=6)
    ax.set_ylabel("LLM API cost (USD)", fontsize=9.5, labelpad=6)
    ax.tick_params(labelsize=8)

    # Summary box — top-right, clear of data
    total_d = int(designs.sum())
    total_c = costs.sum()
    avg_c = total_c / total_d if total_d else 0
    ax.text(
        0.97, 0.97,
        f"{total_d} designs  |  \${total_c:.2f} total  |  \${avg_c:.4f}/design",
        transform=ax.transAxes, ha="right", va="top",
        fontsize=7.5, color="#444444",
        bbox=dict(boxstyle="round,pad=0.35", facecolor="#f7f7f7",
                  edgecolor="#dddddd", linewidth=0.5),
    )

    # Encoding legend — below summary box in top-right (clear of data)
    ax.text(
        0.97, 0.85,
        "Each point = one repository\nBubble size = agent reasoning iterations",
        transform=ax.transAxes, ha="right", va="top",
        fontsize=6.5, color=GRAY, linespacing=1.4,
    )

    fig.tight_layout(pad=1.0)
    fig.savefig(figures_dir / "agent_efficiency.png", dpi=300, bbox_inches="tight",
                facecolor="white", edgecolor="none")
    fig.savefig(figures_dir / "agent_efficiency.svg", bbox_inches="tight",
                facecolor="white", edgecolor="none")
    plt.close(fig)
    print(f"Wrote {figures_dir / 'agent_efficiency.png'}")


if __name__ == "__main__":
    main()
