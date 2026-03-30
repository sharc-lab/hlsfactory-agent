"""Generate aggregate pipeline summary table as figure and LaTeX."""
from __future__ import annotations

import json
from pathlib import Path

import matplotlib.pyplot as plt

# -- Shared academic style --
FONT = "serif"
HEADER_BG = "#2c5f8a"


def load_latest_benchmarks(output_dir: Path) -> dict[str, dict]:
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
            seen[repo_name] = (ts, data)
    return {name: d for name, (_, d) in seen.items()}


def main() -> None:
    repo_root = Path(__file__).resolve().parents[1]
    output_dir = repo_root / "output"
    figures_dir = repo_root / "figures"
    figures_dir.mkdir(parents=True, exist_ok=True)

    benchmarks = load_latest_benchmarks(output_dir)
    benchmarks = {k: v for k, v in benchmarks.items() if k != "HeCBench"}

    n_repos = len(benchmarks)
    total_designs = sum(b.get("quality", {}).get("designs_found", 0) for b in benchmarks.values())
    total_pass = sum(b.get("quality", {}).get("compile_pass", 0) for b in benchmarks.values())
    total_cost = sum(b.get("total_cost_usd", 0) for b in benchmarks.values())
    total_calls = sum(b.get("total_api_calls", 0) for b in benchmarks.values())
    total_tb = sum(b.get("quality", {}).get("designs_with_testbench", 0) for b in benchmarks.values())
    total_tcl = sum(b.get("quality", {}).get("designs_with_tcl", 0) for b in benchmarks.values())

    perfect_repos = sum(
        1 for b in benchmarks.values()
        if b.get("quality", {}).get("compile_pass", 0) > 0
        and b.get("quality", {}).get("compile_pass", 0) == b.get("quality", {}).get("designs_found", 0)
    )

    pass_rate = (total_pass / total_designs * 100) if total_designs else 0
    avg_cost = total_cost / total_designs if total_designs else 0
    avg_calls = total_calls / n_repos if n_repos else 0

    rows = [
        ("Repositories processed", str(n_repos)),
        ("Total designs extracted", str(total_designs)),
        ("Compilation pass rate", f"{pass_rate:.1f}%"),
        ("Repos with 100% pass rate", f"{perfect_repos}/{n_repos}"),
        ("Designs with testbenches", f"{total_tb}/{total_designs}"),
        ("Designs with TCL scripts", f"{total_tcl}/{total_designs}"),
        ("Total pipeline cost", f"${total_cost:.2f}"),
        ("Avg. cost per design", f"${avg_cost:.4f}"),
        ("Avg. API calls per repo", f"{avg_calls:.1f}"),
    ]

    # --- Figure ---
    plt.rcParams.update({"font.family": FONT})

    fig, ax = plt.subplots(figsize=(4.5, 3.5))
    ax.axis("off")

    table = ax.table(
        cellText=[[r[0], r[1]] for r in rows],
        colLabels=["Metric", "Value"],
        cellLoc="left",
        colLoc="left",
        loc="center",
    )
    table.auto_set_font_size(False)
    table.set_fontsize(9)
    table.scale(1.0, 1.4)

    # Header
    for j in range(2):
        cell = table[0, j]
        cell.set_facecolor(HEADER_BG)
        cell.set_text_props(color="white", fontweight="bold", fontsize=9.5)
        cell.set_edgecolor("#1a3f5c")

    # Rows
    for i in range(1, len(rows) + 1):
        for j in range(2):
            cell = table[i, j]
            cell.set_facecolor("#f8f9fb" if i % 2 == 0 else "white")
            cell.set_edgecolor("#e0e4e8")
            cell.set_text_props(fontsize=9)

    fig.tight_layout(pad=0.3)
    fig.savefig(figures_dir / "summary_table.png", dpi=300, bbox_inches="tight",
                facecolor="white", edgecolor="none")
    fig.savefig(figures_dir / "summary_table.svg", bbox_inches="tight",
                facecolor="white", edgecolor="none")
    plt.close(fig)

    # --- LaTeX ---
    tex_lines = [
        r"\begin{table}[t]",
        r"\centering",
        r"\caption{Aggregate pipeline statistics (excluding HeCBench).}",
        r"\label{tab:summary}",
        r"\begin{tabular}{lr}",
        r"\toprule",
        r"\textbf{Metric} & \textbf{Value} \\",
        r"\midrule",
    ]
    for metric, value in rows:
        tex_value = value.replace("$", r"\$")
        tex_lines.append(f"{metric} & {tex_value} \\\\")
    tex_lines += [r"\bottomrule", r"\end{tabular}", r"\end{table}"]
    tex_path = figures_dir / "summary_table.tex"
    tex_path.write_text("\n".join(tex_lines), encoding="utf-8")

    print(f"Wrote {figures_dir / 'summary_table.png'}")
    print(f"Wrote {tex_path}")


if __name__ == "__main__":
    main()
