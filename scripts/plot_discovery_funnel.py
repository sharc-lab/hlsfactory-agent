"""Stage I Discovery Funnel: 2,518 papers -> 30 repos processed."""
from __future__ import annotations

from pathlib import Path

import matplotlib.pyplot as plt
import matplotlib.patches as FancyBboxPatch
import numpy as np


# -- Shared academic style --
FONT = "serif"
ACCENT = "#2c5f8a"
ACCENT2 = "#3a8c5c"
GRAY = "#6b7280"
LIGHT_GRAY = "#f3f4f6"


def main() -> None:
    repo_root = Path(__file__).resolve().parents[1]
    figures_dir = repo_root / "figures"
    figures_dir.mkdir(parents=True, exist_ok=True)

    plt.rcParams.update({
        "font.family": FONT,
        "text.usetex": False,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.spines.left": False,
        "axes.spines.bottom": False,
    })

    fig, ax = plt.subplots(figsize=(5.5, 3.2))
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 6)
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_aspect("equal")

    # -- Funnel: two trapezoids narrowing downward --
    # Top box: 2,518 papers
    top_box = plt.Polygon(
        [[1.5, 4.0], [8.5, 4.0], [8.5, 5.5], [1.5, 5.5]],
        closed=True, facecolor=ACCENT, edgecolor="white", linewidth=1.5, alpha=0.9,
    )
    ax.add_patch(top_box)
    ax.text(5.0, 4.75, "2,518", fontsize=22, fontweight="bold", color="white",
            ha="center", va="center")
    ax.text(5.0, 4.25, "papers mentioning HLS", fontsize=9, color="white",
            ha="center", va="center", alpha=0.9)

    # Arrow + label
    ax.annotate(
        "", xy=(5.0, 2.6), xytext=(5.0, 3.85),
        arrowprops=dict(arrowstyle="-|>", color=GRAY, lw=1.5, mutation_scale=14),
    )
    ax.text(6.8, 3.22, "manual curation\n& filtering", fontsize=7.5,
            color=GRAY, ha="left", va="center", style="italic")

    # Bottom box: 30 repos
    bot_box = plt.Polygon(
        [[3.2, 1.0], [6.8, 1.0], [6.8, 2.5], [3.2, 2.5]],
        closed=True, facecolor=ACCENT2, edgecolor="white", linewidth=1.5, alpha=0.9,
    )
    ax.add_patch(bot_box)
    ax.text(5.0, 1.75, "30", fontsize=22, fontweight="bold", color="white",
            ha="center", va="center")
    ax.text(5.0, 1.22, "repos with HLS designs", fontsize=9, color="white",
            ha="center", va="center", alpha=0.9)

    # Taper lines connecting the two boxes
    ax.plot([1.5, 3.2], [4.0, 2.5], color=GRAY, lw=0.8, ls="--", alpha=0.4)
    ax.plot([8.5, 6.8], [4.0, 2.5], color=GRAY, lw=0.8, ls="--", alpha=0.4)

    # Source annotation
    ax.text(5.0, 5.75, "IEEE Xplore  |  ACM DL  |  arXiv  |  DBLP",
            fontsize=7.5, color=GRAY, ha="center", va="center")

    # Yield annotation
    ax.text(5.0, 0.55, "1.2% yield", fontsize=9, color=GRAY,
            ha="center", va="center", fontweight="bold")

    fig.tight_layout(pad=0.5)
    fig.savefig(figures_dir / "discovery_funnel.png", dpi=300, bbox_inches="tight",
                facecolor="white", edgecolor="none")
    fig.savefig(figures_dir / "discovery_funnel.svg", bbox_inches="tight",
                facecolor="white", edgecolor="none")
    plt.close(fig)
    print(f"Wrote {figures_dir / 'discovery_funnel.png'}")


if __name__ == "__main__":
    main()
