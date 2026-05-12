import json
from pathlib import Path
from pprint import pprint
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.ticker import MaxNLocator

DIR_CURRENT = Path(__file__).parent
DIR_DATA = DIR_CURRENT / "output"

bench_files = list(DIR_DATA.glob("*_benchmark.json"))

data = []
for bench_file in bench_files:
    with open(bench_file, "r") as f:
        bench_data = json.load(f)
    if "quality" not in bench_data:
        continue
    if "HeCBench" in bench_file.stem:
        continue
    name = "_".join(bench_file.stem.split("_")[:2])
    c_pass = bench_data["quality"]["compile_pass"]
    c_fail = bench_data["quality"]["compile_fail"]
    c_skip = bench_data["quality"]["compile_skip"]
    d_found = c_pass + c_fail + c_skip
    data.append({
        "name": name,
        "compile_pass": c_pass,
        "compile_fail": c_fail,
        "compile_skip": c_skip,
        "designs_found": d_found,
    })

df = pd.DataFrame(data).sort_values(by="designs_found").reset_index(drop=True)

# ── Style ────────────────────────────────────────────────────────────────────
plt.rcParams.update({
    "font.family": "DejaVu Sans",
    "axes.spines.top": False,
    "axes.spines.right": False,
    "axes.grid": True,
    "axes.grid.axis": "y",
    "grid.color": "#e0e0e0",
    "grid.linewidth": 0.8,
    "axes.axisbelow": True,
})

COLORS = {
    "compile_pass": "#2ecc71",   # green
    "compile_fail": "#e74c3c",   # red
    "compile_skip": "#95a5a6",   # neutral grey
}

fig, ax = plt.subplots(figsize=(8, 4))

x = range(len(df))
bar_width = 0.65

bottoms_fail = df["compile_pass"].values
bottoms_skip = (df["compile_pass"] + df["compile_fail"]).values

bars_pass = ax.bar(x, df["compile_pass"],  width=bar_width, color=COLORS["compile_pass"],  label="Compile Pass",  zorder=3)
bars_fail = ax.bar(x, df["compile_fail"],  width=bar_width, color=COLORS["compile_fail"],  label="Compile Fail",  bottom=bottoms_fail,  zorder=3)
bars_skip = ax.bar(x, df["compile_skip"],  width=bar_width, color=COLORS["compile_skip"],  label="Compile Skip",  bottom=bottoms_skip, zorder=3)

# ── Annotate total above each bar ────────────────────────────────────────────
for i, total in enumerate(df["designs_found"]):
    ax.text(i, total + 0.3, str(total), ha="center", va="bottom",
            fontsize=8, color="#333333", fontweight="bold")

# ── Axes & labels ─────────────────────────────────────────────────────────────
ax.set_xticks(list(x))
ax.set_xticklabels(
    [f"{i}" for i, name in enumerate(df["name"])],
    fontsize=7.5, rotation=35, ha="right",
)
ax.set_ylabel("Design Count", fontsize=11, labelpad=8)
ax.set_xlabel("Repository (index · name)", fontsize=11, labelpad=10)
ax.set_title("HLS Compilation Outcomes by Repository", fontsize=13, fontweight="bold", pad=14)
ax.yaxis.set_major_locator(MaxNLocator(integer=True))
ax.set_ylim(0, df["designs_found"].max() * 1.12)

# ── Legend ───────────────────────────────────────────────────────────────────
legend_handles = [
    mpatches.Patch(color=COLORS["compile_pass"], label="Compile Pass"),
    mpatches.Patch(color=COLORS["compile_fail"], label="Compile Fail"),
    mpatches.Patch(color=COLORS["compile_skip"], label="Compile Skip"),
]
ax.legend(handles=legend_handles, loc="upper left",
          frameon=True, framealpha=0.9, fontsize=9)

fig.tight_layout()
fig.savefig("designs_found_by_model.png", dpi=300, bbox_inches="tight")
plt.show()