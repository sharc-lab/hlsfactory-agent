import datetime
import json
from pathlib import Path

import matplotlib.pyplot as plt

from adjustText import adjust_text


DIR_CURRENT = Path(__file__).resolve().parent
DIR_BUILDS = DIR_CURRENT / "builds"
DIR_RUNS = DIR_CURRENT / "runs"
DIR_FIGURES = DIR_CURRENT / "figures"

TS_FMT = "%Y-%m-%dT%H:%M:%S.%fZ"
UTC = datetime.timezone.utc


def parse_ts(timestamp_str: str) -> datetime.datetime:
    return datetime.datetime.strptime(timestamp_str, TS_FMT).replace(tzinfo=UTC)


def parse_unix_ms(timestamp_ms: int) -> datetime.datetime:
    return datetime.datetime.fromtimestamp(timestamp_ms / 1000, tz=UTC)


def normalize_content(content):
    if isinstance(content, str):
        return [{"type": "text", "text": content}]
    return content or []


def compute_run_metrics(run_data: dict) -> tuple[float, float]:
    session_data = run_data.get("session_data", [])
    if not session_data:
        return 0.0, 0.0

    first_ts = parse_ts(session_data[0]["timestamp"])
    last_ts = parse_ts(session_data[-1]["timestamp"])
    total_runtime_seconds = (last_ts - first_ts).total_seconds()

    total_cost = 0.0
    for step in session_data:
        message = step.get("message", {})
        for _ in normalize_content(message.get("content", [])):
            pass
        usage = message.get("usage", {})
        cost = usage.get("cost", {})
        cost_total = cost.get("total")
        if cost_total is None:
            continue
        try:
            total_cost += float(cost_total)
        except (TypeError, ValueError):
            continue

        # Mirror time handling style used elsewhere; these fields are optional.
        if step.get("type") == "message" and message.get("timestamp") is not None and message.get("role") != "toolResult":
            _ = parse_unix_ms(message["timestamp"])

    return total_runtime_seconds, total_cost


def plot_scatter(
    x_values: list[float],
    y_values: list[int],
    labels: list[str],
    x_label: str,
    title: str,
    output_fp: Path,
) -> None:
    fig, ax = plt.subplots(figsize=(8, 6))
    draw_scatter_on_ax(
        ax=ax,
        x_values=x_values,
        y_values=y_values,
        labels=labels,
        x_label=x_label,
        title=title,
    )
    fig.tight_layout()
    fig.savefig(output_fp, dpi=300)
    plt.close(fig)
    print(f"Saved plot to: {output_fp}")


def draw_scatter_on_ax(
    ax,
    x_values: list[float],
    y_values: list[int],
    labels: list[str],
    x_label: str,
    title: str,
    point_color: str = "#1f77b4",
    title_bold: bool = True,
) -> None:
    ax.scatter(
        x_values,
        y_values,
        s=80,
        alpha=0.85,
        color=point_color,
        edgecolors="black",
        linewidths=0.5,
    )

    texts = []
    for x, y, label in zip(x_values, y_values, labels):
        text = ax.text(
            x,
            y,
            label,
            ha="center",
            va="center",
            fontsize=8,
            fontweight="bold",
            alpha=0.9,
        )
        texts.append(text)

    adjust_text(
        texts,
        x=x_values,
        y=y_values,
        ax=ax,
        ensure_inside_axes=True,
        expand_axes=False,
        force_text=(0.2, 0.35),
        force_static=(0.3, 0.45),
        force_pull=(0.005, 0.008),
        expand=(1.2, 1.4),
        max_move=(14, 14),
        iter_lim=500,
    )

    # Draw connector lines for all points to their final label locations.
    for x, y, text in zip(x_values, y_values, texts):
        tx, ty = text.get_position()
        ax.plot([x, tx], [y, ty], color="0.45", linewidth=0.6, alpha=0.7, zorder=1)

    ax.set_xlabel(x_label, fontweight="bold")
    ax.set_ylabel("Number of Valid Designs Extracted", fontweight="bold")
    ax.set_title(title, fontweight="bold" if title_bold else None)
    ax.grid(True, linestyle="--", linewidth=0.8, alpha=0.4)

def plot_stacked_scatters(
    *,
    x_total_cost: list[float],
    x_total_runtime: list[float],
    y_num_designs: list[int],
    labels: list[str],
    common_title: str,
    cost_subtitle: str,
    runtime_subtitle: str,
    output_fp: Path,
) -> None:
    fig, (ax_cost, ax_runtime) = plt.subplots(2, 1, figsize=(7, 9))

    draw_scatter_on_ax(
        ax=ax_cost,
        x_values=x_total_cost,
        y_values=y_num_designs,
        labels=labels,
        x_label="Total Cost ($)",
        title=cost_subtitle,
        point_color="green",
        title_bold=True,
    )

    draw_scatter_on_ax(
        ax=ax_runtime,
        x_values=x_total_runtime,
        y_values=y_num_designs,
        labels=labels,
        x_label="Agent Total Runtime (Seconds)",
        title=runtime_subtitle,
        point_color="#1f77b4",
        title_bold=True,
    )

    fig.suptitle(common_title, fontweight="bold", fontsize=14)
    # fig.tight_layout(rect=[0, 0, 1, 0.97])
    fig.tight_layout()
    fig.savefig(output_fp, dpi=300)
    plt.close(fig)
    print(f"Saved plot to: {output_fp}")


def main() -> None:
    DIR_FIGURES.mkdir(parents=True, exist_ok=True)

    build_check_data_fps = sorted(DIR_BUILDS.rglob("build_check_data.json"))
    build_check_data_dicts = [json.loads(fp.read_text()) for fp in build_check_data_fps]

    run_design_count: dict[str, int] = {}
    run_repo_name: dict[str, str] = {}
    for build_check_data_dict in build_check_data_dicts:
        run_id = build_check_data_dict["run_id"]
        repo_id = build_check_data_dict["repo_id"]
        run_repo_name[run_id] = repo_id.split("/")[-1]
        run_design_count[run_id] = len(build_check_data_dict.get("check__designs", []))

    run_data_files = []
    for jsonl_file in sorted(DIR_RUNS.rglob("*.jsonl")):
        dir_run = jsonl_file.parent.parent.parent.parent
        fp_run_data = dir_run / "run_data.json"
        run_data_files.append(fp_run_data)

    run_runtime_cost: dict[str, tuple[float, float]] = {}
    for fp_run_data in run_data_files:
        if not fp_run_data.exists():
            continue
        run_data = json.loads(fp_run_data.read_text())
        run_id = run_data["run_id"]
        run_runtime_cost[run_id] = compute_run_metrics(run_data)

    labels = []
    y_num_designs = []
    x_total_cost = []
    x_total_runtime = []
    for run_id, num_designs in sorted(run_design_count.items()):
        if run_id not in run_runtime_cost:
            continue
        total_runtime_seconds, total_cost = run_runtime_cost[run_id]
        labels.append(run_repo_name.get(run_id, run_id))
        y_num_designs.append(num_designs)
        x_total_cost.append(total_cost)
        x_total_runtime.append(total_runtime_seconds)

    common_title = "Design Extraction Effort per Design Source"
    cost_subtitle = "Number of Valid Designs Extracted vs. Total Agent Run Cost"
    runtime_subtitle = "Number of Valid Designs Extracted vs. Total Agent Run Runtime"

    plot_scatter(
        x_values=x_total_cost,
        y_values=y_num_designs,
        labels=labels,
        x_label="Total Cost ($)",
        title=f"{common_title}\n{cost_subtitle}",
        output_fp=DIR_FIGURES / "design_source_num_designs_vs_cost_scatter.png",
    )

    plot_scatter(
        x_values=x_total_runtime,
        y_values=y_num_designs,
        labels=labels,
        x_label="Agent Total Runtime (Seconds)",
        title=f"{common_title}\n{runtime_subtitle}",
        output_fp=DIR_FIGURES / "design_source_num_designs_vs_runtime_scatter.png",
    )

    plot_stacked_scatters(
        x_total_cost=x_total_cost,
        x_total_runtime=x_total_runtime,
        y_num_designs=y_num_designs,
        labels=labels,
        common_title=common_title,
        cost_subtitle=cost_subtitle,
        runtime_subtitle=runtime_subtitle,
        output_fp=DIR_FIGURES / "design_source_num_designs_scatter_stacked.png",
    )


if __name__ == "__main__":
    main()