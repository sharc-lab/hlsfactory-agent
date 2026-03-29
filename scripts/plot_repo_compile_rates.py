from __future__ import annotations

import argparse
import csv
import json
import re
from dataclasses import dataclass
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.ticker import PercentFormatter


UUID_BENCH_RE = re.compile(r"_(?P<uuid>[0-9a-fA-F-]{36})_benchmark$")
STATUS_KEYS = ("pass", "fail", "skip", "unknown")
STATUS_COLORS = {
    "pass": "#2E8B57",
    "fail": "#C84C3A",
    "skip": "#8E9AAF",
    "unknown": "#D7DCE5",
}
STATUS_LABELS = {
    "pass": "Pass",
    "fail": "Fail",
    "skip": "Skip",
    "unknown": "Unknown",
}


@dataclass
class RepoStats:
    repo: str
    source: str
    design_dirs: int
    denominator: int
    pass_count: int
    fail_count: int
    skip_count: int
    unknown_count: int

    @property
    def accounted(self) -> int:
        return self.pass_count + self.fail_count + self.skip_count

    def rate(self, count: int) -> float:
        return (count / self.denominator) if self.denominator else 0.0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate a per-repo compile outcome bar chart from the current output/ snapshot."
    )
    parser.add_argument(
        "--output-dir",
        default="output",
        help="Directory containing extracted repo folders and benchmark JSON files (default: output)",
    )
    parser.add_argument(
        "--figures-dir",
        default="figures",
        help="Directory to write the chart and CSV into (default: figures)",
    )
    parser.add_argument(
        "--sort-by",
        choices=("pass_rate", "name", "absolute_total", "pass_count"),
        default="pass_rate",
        help="Sort repos by pass rate, total design count, pass count, or alphabetically (default: pass_rate)",
    )
    parser.add_argument(
        "--value-mode",
        choices=("counts", "rates"),
        default="counts",
        help="Plot absolute counts or normalized rates (default: counts)",
    )
    parser.add_argument(
        "--orientation",
        choices=("horizontal", "vertical"),
        default="horizontal",
        help="Plot horizontal or vertical stacked bars (default: horizontal)",
    )
    parser.add_argument(
        "--exclude-repo",
        action="append",
        default=[],
        help="Exclude a repo by name. Repeat to exclude multiple repos.",
    )
    parser.add_argument(
        "--output-stem",
        default="",
        help="Optional custom filename stem for the generated figure artifacts.",
    )
    return parser.parse_args()


def repo_name_from_benchmark(path: Path, data: dict) -> str:
    repo_url = str(data.get("repo_url", "")).rstrip("/")
    if repo_url:
        return repo_url.split("/")[-1].removesuffix(".git")
    stem = path.stem
    match = UUID_BENCH_RE.search(stem)
    if match:
        return stem[: match.start()]
    return stem.removesuffix("_benchmark")


def load_latest_benchmarks(output_dir: Path) -> dict[str, dict]:
    latest: dict[str, tuple[str, dict]] = {}
    for bench_path in output_dir.glob("*_benchmark.json"):
        try:
            data = json.loads(bench_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            continue
        repo = repo_name_from_benchmark(bench_path, data)
        timestamp = str(data.get("timestamp", ""))
        previous = latest.get(repo)
        if previous is None or timestamp > previous[0]:
            latest[repo] = (timestamp, data)
    return {repo: data for repo, (_, data) in latest.items()}


def manifest_counts(repo_dir: Path) -> tuple[int, int, int] | None:
    manifest_path = repo_dir / "manifest.json"
    if not manifest_path.is_file():
        return None
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None
    designs = manifest.get("designs")
    if not isinstance(designs, list) or not all(isinstance(item, dict) for item in designs):
        return None

    pass_count = fail_count = skip_count = 0
    for design in designs:
        status = design.get("compile_status", "skip")
        if status == "pass":
            pass_count += 1
        elif status == "fail":
            fail_count += 1
        else:
            skip_count += 1
    return pass_count, fail_count, skip_count


def benchmark_counts(output_dir: Path, repo_name: str, latest_benchmarks: dict[str, dict]) -> tuple[int, int, int, int] | None:
    data = latest_benchmarks.get(repo_name)
    if not data:
        return None
    quality = data.get("quality", {})
    try:
        design_count = int(quality.get("designs_found", 0) or 0)
        pass_count = int(quality.get("compile_pass", 0) or 0)
        fail_count = int(quality.get("compile_fail", 0) or 0)
        skip_count = int(quality.get("compile_skip", 0) or 0)
    except (TypeError, ValueError):
        return None
    if design_count == 0 and pass_count == 0 and fail_count == 0 and skip_count == 0:
        return None
    return design_count, pass_count, fail_count, skip_count


def collect_repo_stats(output_dir: Path) -> list[RepoStats]:
    latest_benchmarks = load_latest_benchmarks(output_dir)
    rows: list[RepoStats] = []

    for repo_dir in sorted(d for d in output_dir.iterdir() if d.is_dir()):
        repo_name = repo_dir.name
        design_dirs = len([d for d in repo_dir.iterdir() if d.is_dir()])

        manifest = manifest_counts(repo_dir)
        if manifest is not None:
            pass_count, fail_count, skip_count = manifest
            denominator = max(design_dirs, pass_count + fail_count + skip_count)
            unknown_count = max(0, denominator - (pass_count + fail_count + skip_count))
            rows.append(
                RepoStats(
                    repo=repo_name,
                    source="manifest",
                    design_dirs=design_dirs,
                    denominator=denominator,
                    pass_count=pass_count,
                    fail_count=fail_count,
                    skip_count=skip_count,
                    unknown_count=unknown_count,
                )
            )
            continue

        benchmark = benchmark_counts(output_dir, repo_name, latest_benchmarks)
        if benchmark is not None:
            benchmark_designs, pass_count, fail_count, skip_count = benchmark
            denominator = max(design_dirs, benchmark_designs, pass_count + fail_count + skip_count)
            unknown_count = max(0, denominator - (pass_count + fail_count + skip_count))
            rows.append(
                RepoStats(
                    repo=repo_name,
                    source="benchmark",
                    design_dirs=design_dirs,
                    denominator=denominator,
                    pass_count=pass_count,
                    fail_count=fail_count,
                    skip_count=skip_count,
                    unknown_count=unknown_count,
                )
            )
            continue

        rows.append(
            RepoStats(
                repo=repo_name,
                source="unknown",
                design_dirs=design_dirs,
                denominator=design_dirs,
                pass_count=0,
                fail_count=0,
                skip_count=0,
                unknown_count=design_dirs,
            )
        )

    return rows


def sort_rows(rows: list[RepoStats], sort_by: str) -> list[RepoStats]:
    if sort_by == "name":
        return sorted(rows, key=lambda row: row.repo.lower())
    if sort_by == "absolute_total":
        return sorted(
            rows,
            key=lambda row: (
                -row.denominator,
                -row.pass_count,
                row.fail_count,
                row.repo.lower(),
            ),
        )
    if sort_by == "pass_count":
        return sorted(
            rows,
            key=lambda row: (
                -row.pass_count,
                row.fail_count,
                -row.denominator,
                row.repo.lower(),
            ),
        )
    return sorted(
        rows,
        key=lambda row: (
            -row.rate(row.pass_count),
            row.rate(row.fail_count),
            -row.denominator,
            row.repo.lower(),
        ),
    )


def write_csv(rows: list[RepoStats], csv_path: Path) -> None:
    csv_path.parent.mkdir(parents=True, exist_ok=True)
    with csv_path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(
            [
                "repo",
                "source",
                "design_dirs",
                "denominator",
                "pass_count",
                "fail_count",
                "skip_count",
                "unknown_count",
                "pass_rate",
                "fail_rate",
                "skip_rate",
                "unknown_rate",
            ]
        )
        for row in rows:
            writer.writerow(
                [
                    row.repo,
                    row.source,
                    row.design_dirs,
                    row.denominator,
                    row.pass_count,
                    row.fail_count,
                    row.skip_count,
                    row.unknown_count,
                    row.rate(row.pass_count),
                    row.rate(row.fail_count),
                    row.rate(row.skip_count),
                    row.rate(row.unknown_count),
                ]
            )


def plot_rows(
    rows: list[RepoStats],
    png_path: Path,
    svg_path: Path,
    value_mode: str,
    orientation: str,
) -> None:
    labels = [f"{row.repo} (n={row.denominator})" for row in rows]
    if orientation == "vertical":
        fig_width = max(16, 0.48 * len(rows) + 4.0)
        fig, ax = plt.subplots(figsize=(fig_width, 8.5))
        positions = list(range(len(rows)))
        bottom = [0.0] * len(rows)
        for status in STATUS_KEYS:
            if value_mode == "counts":
                values = [float(getattr(row, f"{status}_count")) for row in rows]
            else:
                values = [row.rate(getattr(row, f"{status}_count")) for row in rows]
            if not any(values):
                continue
            ax.bar(
                positions,
                values,
                bottom=bottom,
                color=STATUS_COLORS[status],
                edgecolor="white",
                linewidth=0.6,
                label=STATUS_LABELS[status],
            )
            bottom = [b + v for b, v in zip(bottom, values)]

        ax.set_xticks(positions)
        ax.set_xticklabels(labels, rotation=75, ha="right", fontsize=8.5)
        if value_mode == "counts":
            max_total = max(
                row.pass_count + row.fail_count + row.skip_count + row.unknown_count for row in rows
            ) if rows else 0
            ax.set_ylim(0, max_total * 1.03 if max_total else 1)
            ax.set_ylabel("Design Count", fontsize=11)
        else:
            ax.set_ylim(0, 1)
            ax.yaxis.set_major_formatter(PercentFormatter(xmax=1.0))
            ax.set_ylabel("Rate", fontsize=11)
        ax.set_xlabel("Repository", fontsize=11)
        ax.grid(axis="y", alpha=0.2, linewidth=0.8)
    else:
        label_rows = labels
        fig_height = max(10, 0.38 * len(rows) + 2.8)
        fig, ax = plt.subplots(figsize=(15, fig_height))

        y_positions = list(range(len(rows)))
        left = [0.0] * len(rows)

        for status in STATUS_KEYS:
            if value_mode == "counts":
                values = [float(getattr(row, f"{status}_count")) for row in rows]
            else:
                values = [row.rate(getattr(row, f"{status}_count")) for row in rows]
            if not any(values):
                continue
            ax.barh(
                y_positions,
                values,
                left=left,
                color=STATUS_COLORS[status],
                edgecolor="white",
                linewidth=0.6,
                label=STATUS_LABELS[status],
            )
            left = [l + v for l, v in zip(left, values)]

        ax.set_yticks(y_positions)
        ax.set_yticklabels(label_rows, fontsize=9)
        ax.invert_yaxis()
        if value_mode == "counts":
            max_total = max(
                row.pass_count + row.fail_count + row.skip_count + row.unknown_count for row in rows
            ) if rows else 0
            ax.set_xlim(0, max_total * 1.02 if max_total else 1)
            ax.set_xlabel("Design Count", fontsize=11)
        else:
            ax.set_xlim(0, 1)
            ax.xaxis.set_major_formatter(PercentFormatter(xmax=1.0))
            ax.set_xlabel("Rate", fontsize=11)
        ax.set_ylabel("Repository", fontsize=11)
        ax.grid(axis="x", alpha=0.2, linewidth=0.8)

    if value_mode == "counts":
        fig.suptitle("Per-Repo Compile Outcome Counts", fontsize=16, fontweight="bold", y=0.99)
        orientation_text = "Vertical" if orientation == "vertical" else "Horizontal"
        ax.set_title(
            f"{orientation_text} stacked bars showing absolute pass/fail/skip counts for the current output snapshot.",
            fontsize=10,
            pad=14,
        )
    else:
        fig.suptitle("Per-Repo Compile Outcome Rates", fontsize=16, fontweight="bold", y=0.99)
        ax.set_title(
            "Pass/fail/skip rates for the current output snapshot. Unknown appears only when no trustworthy status accounting exists.",
            fontsize=10,
            pad=14,
        )
    ax.legend(
        loc="upper center",
        bbox_to_anchor=(0.5, 1.05),
        ncol=4,
        frameon=False,
        fontsize=10,
    )

    fig.tight_layout(rect=(0, 0, 1, 0.965))
    png_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(png_path, dpi=220, bbox_inches="tight")
    fig.savefig(svg_path, bbox_inches="tight")
    plt.close(fig)


def main() -> None:
    args = parse_args()
    repo_root = Path(__file__).resolve().parents[1]
    output_dir = repo_root / args.output_dir
    figures_dir = repo_root / args.figures_dir

    rows = collect_repo_stats(output_dir)
    excluded = {name.strip() for name in args.exclude_repo if name.strip()}
    if excluded:
        rows = [row for row in rows if row.repo not in excluded]
    rows = sort_rows(rows, args.sort_by)

    if args.output_stem:
        stem = args.output_stem
    else:
        suffix = "counts" if args.value_mode == "counts" else "rates"
        orientation_suffix = "vertical" if args.orientation == "vertical" else "horizontal"
        stem = f"repo_compile_outcome_{suffix}_{orientation_suffix}"

    csv_path = figures_dir / f"{stem}.csv"
    png_path = figures_dir / f"{stem}.png"
    svg_path = figures_dir / f"{stem}.svg"

    write_csv(rows, csv_path)
    plot_rows(rows, png_path, svg_path, args.value_mode, args.orientation)

    print(f"Wrote {csv_path}")
    print(f"Wrote {png_path}")
    print(f"Wrote {svg_path}")


if __name__ == "__main__":
    main()
