"""Compilation Error Categories: classify failures from compile_log.txt."""
from __future__ import annotations

import re
from collections import Counter
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

# -- Shared academic style --
FONT = "serif"
ACCENT = "#2c5f8a"
GRAY = "#6b7280"

CATEGORIES = [
    ("Missing Header / Include", [
        r"fatal error:.*file not found",
        r"no such file or directory",
        r"fatal error:.*'.*\.h' file not found",
    ]),
    ("Undeclared Identifier / Type", [
        r"unknown type name",
        r"use of undeclared identifier",
        r"undeclared identifier",
    ]),
    ("Syntax Error", [
        r"expected .*;",
        r"expected expression",
        r"expected unqualified-id",
        r"extraneous closing brace",
        r"expected '\}'",
    ]),
    ("Missing Member / Function", [
        r"no member named",
        r"no matching function",
        r"no viable overloaded",
        r"too few arguments",
        r"too many arguments",
    ]),
    ("Type Mismatch", [
        r"cannot initialize",
        r"no viable conversion",
        r"incompatible",
        r"cannot convert",
    ]),
    ("Deprecated Keyword", [
        r"register.*storage class",
        r"ISO C\+\+17 does not allow.*register",
    ]),
    ("Redefinition", [
        r"redefinition of",
        r"multiple definition",
    ]),
]


def classify_log(content: str) -> list[str]:
    lower = content.lower()
    matched = []
    for label, patterns in CATEGORIES:
        for pat in patterns:
            if re.search(pat, lower):
                matched.append(label)
                break
    return matched


def main() -> None:
    repo_root = Path(__file__).resolve().parents[1]
    output_dir = repo_root / "output"
    figures_dir = repo_root / "figures"
    figures_dir.mkdir(parents=True, exist_ok=True)

    counts = Counter()
    total_logs = 0
    failed_logs = 0

    for log_path in output_dir.rglob("compile_log.txt"):
        total_logs += 1
        content = log_path.read_text(encoding="utf-8", errors="replace").strip()
        if not content or content.lower() in (
            "compilation successful", "pass", "success",
            "compilation passed", "no errors",
        ):
            continue
        if len(content) < 20 and "error" not in content.lower():
            continue
        failed_logs += 1
        categories = classify_log(content)
        if not categories:
            counts["Other / Unclassified"] += 1
        else:
            for cat in categories:
                counts[cat] += 1

    sorted_cats = counts.most_common()
    labels = [c[0] for c in sorted_cats]
    values = [c[1] for c in sorted_cats]

    plt.rcParams.update({
        "font.family": FONT,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.linewidth": 0.6,
        "axes.edgecolor": "#cccccc",
        "xtick.color": "#444444",
        "ytick.color": "#444444",
    })

    fig, ax = plt.subplots(figsize=(5.5, 3.2))
    ax.grid(True, axis="x", color="#e8e8e8", linewidth=0.5, zorder=0)

    # Gradient from dark to light
    n = len(labels)
    blues = plt.cm.Blues(np.linspace(0.75, 0.35, n))

    y_pos = np.arange(n)
    bars = ax.barh(y_pos, values, color=blues, edgecolor="white",
                   linewidth=0.5, height=0.6, zorder=3)

    for i, val in enumerate(values):
        ax.text(val + max(values) * 0.02, i, str(val),
                va="center", fontsize=7.5, color="#444444", fontweight="medium")

    ax.set_yticks(y_pos)
    ax.set_yticklabels(labels, fontsize=8)
    ax.invert_yaxis()
    ax.set_xlabel("Number of designs", fontsize=9, labelpad=6)
    ax.set_xlim(0, max(values) * 1.12)
    ax.tick_params(labelsize=7.5, length=2)

    # Subtitle with context
    ax.text(0.98, 0.12,
            f"Classified from {failed_logs} failed compile logs\n"
            f"({total_logs} total designs checked via clang with stub headers)",
            transform=ax.transAxes, ha="right", va="bottom",
            fontsize=6.5, color=GRAY, linespacing=1.4)

    fig.tight_layout(pad=0.8)
    fig.savefig(figures_dir / "error_categories.png", dpi=300, bbox_inches="tight",
                facecolor="white", edgecolor="none")
    fig.savefig(figures_dir / "error_categories.svg", bbox_inches="tight",
                facecolor="white", edgecolor="none")
    plt.close(fig)
    print(f"Wrote {figures_dir / 'error_categories.png'}")


if __name__ == "__main__":
    main()
