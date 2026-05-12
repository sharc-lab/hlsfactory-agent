import datetime
import json

from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator

DIR_RUNS = Path(__file__).resolve().parent / "runs"
TS_FMT = "%Y-%m-%dT%H:%M:%S.%fZ"
UTC = datetime.timezone.utc
COST_TYPES_STACKED = ["input", "output", "cacheRead"]
COST_TYPE_COLORS = {
    "input": "#1f77b4",
    "output": "#ff7f0e",
    "cacheRead": "#2ca02c",
}


def parse_ts(timestamp_str):
    return datetime.datetime.strptime(timestamp_str, TS_FMT).replace(tzinfo=UTC)


def parse_unix_ms(timestamp_ms):
    return datetime.datetime.fromtimestamp(timestamp_ms / 1000, tz=UTC)


def format_ts(timestamp):
    timestamp = timestamp.astimezone(UTC)
    return timestamp.strftime("%Y-%m-%dT%H:%M:%S.%f")[:-3] + "Z"


def normalize_content(content):
    if isinstance(content, str):
        return [{"type": "text", "text": content}]
    return content or []


AGENT_TEXT_LANE = "Agent: Text Output"
AGENT_THINKING_LANE = "Agent: Thinking"
AGENT_TOOL_CALL_LANE = "Agent: Tool Call"
TOOL_EXEC_PREFIX = "Tool Exec: "


def lane_sort_key(lane):
    if lane == AGENT_TEXT_LANE:
        return (0, lane)
    if lane == AGENT_THINKING_LANE:
        return (1, lane)
    if lane == AGENT_TOOL_CALL_LANE:
        return (2, lane)
    if lane.startswith(TOOL_EXEC_PREFIX):
        return (3, lane[len(TOOL_EXEC_PREFIX) :].lower())
    return (4, lane.lower())


def order_lanes(lane_set):
    return sorted(lane_set, key=lane_sort_key)


def lane_color(lane):
    if lane == AGENT_TEXT_LANE:
        return "#2ca02c"
    if lane == AGENT_THINKING_LANE:
        return "#1f77b4"
    if lane == AGENT_TOOL_CALL_LANE:
        return "#d62728"
    if lane.startswith(TOOL_EXEC_PREFIX):
        return "#ff7f0e"
    return "#7f7f7f"


def make_step(idx, event_type, role, lanes, start_time, stop_time, session_start_time, cost=None, **extra):
    dt_seconds = (stop_time - start_time).total_seconds() if start_time and stop_time else None
    step = {
        "idx": idx,
        "event_type": event_type,
        "role": role,
        "lanes": lanes,
        "start": format_ts(start_time) if start_time else None,
        "stop": format_ts(stop_time) if stop_time else None,
        "start_seconds": (start_time - session_start_time).total_seconds() if start_time else None,
        "stop_seconds": (stop_time - session_start_time).total_seconds() if stop_time else None,
        "dt_seconds": dt_seconds,
        "cost_total": cost.get("total") if cost else None,
        "cost": cost or {},
    }
    step.update(extra)
    return step

jsonl_files = sorted(DIR_RUNS.rglob("*.jsonl"))

data = []
for jsonl_file in jsonl_files:
    dir_run = jsonl_file.parent.parent.parent.parent
    fp_run_data = dir_run / "run_data.json"
    run_data = json.loads(fp_run_data.read_text())
    data.append(run_data)

data_computed = []
for run_data in data:
    computed = {}
    computed["run_id"] = run_data["run_id"]
    computed["repo_id"] = run_data["repo_id"]

    # Compute total runtime from the session entry timestamps. For messages,
    # the inner message.timestamp is the message start, while the outer
    # entry timestamp is when that entry was appended.
    session_first_time_str = run_data["session_data"][0]["timestamp"]
    session_last_time_str = run_data["session_data"][-1]["timestamp"]
    session_first_time_stamp = parse_ts(session_first_time_str)
    session_last_time_stamp = parse_ts(session_last_time_str)
    computed["total_runtime"] = (session_last_time_stamp - session_first_time_stamp).total_seconds()

    steps_simplified = []
    session_data = run_data["session_data"]
    pending_tool_calls = {}

    for idx, step in enumerate(session_data):
        message = step.get("message", {})
        entry_time_str = step.get("timestamp")
        entry_time_stamp = parse_ts(entry_time_str) if entry_time_str else None

        tool_names = []
        tool_call_ids = []
        lanes = []
        has_tool_call = False

        for content_item in normalize_content(message.get("content", [])):
            content_type = content_item.get("type")
            if content_type == "toolCall":
                has_tool_call = True
                tool_name = content_item.get("name")
                tool_call_id = content_item.get("id")
                tool_names.append(tool_name)
                tool_call_ids.append(tool_call_id)
                pending_tool_calls[tool_call_id] = {
                    "idx": idx,
                    "id": tool_call_id,
                    "name": tool_name,
                    "emitted_time_stamp": entry_time_stamp,
                    "emitted_time": entry_time_str,
                }
            elif message.get("role") == "assistant" and content_type == "text":
                lanes.append(AGENT_TEXT_LANE)
            elif message.get("role") == "assistant" and content_type == "thinking":
                lanes.append(AGENT_THINKING_LANE)

        if message.get("role") == "assistant" and has_tool_call:
            lanes.append(AGENT_TOOL_CALL_LANE)

        usage = message.get("usage", {})
        cost = usage.get("cost", {})

        if step.get("type") == "message" and message.get("timestamp") is not None and message.get("role") != "toolResult":
            message_start_time = parse_unix_ms(message["timestamp"])
        else:
            message_start_time = entry_time_stamp

        steps_simplified.append(
            make_step(
                idx=idx,
                event_type=step.get("type"),
                role=message.get("role"),
                lanes=list(dict.fromkeys(lanes)),
                start_time=message_start_time,
                stop_time=entry_time_stamp,
                session_start_time=session_first_time_stamp,
                cost=cost,
                tool_names=tool_names,
                tool_call_ids=tool_call_ids,
                tool_call_id=message.get("toolCallId"),
            )
        )

        if message.get("role") == "toolResult":
            tool_call = pending_tool_calls.pop(message.get("toolCallId"), None)
            if tool_call and tool_call["emitted_time_stamp"] and entry_time_stamp:
                exec_start_time_stamp = tool_call["emitted_time_stamp"]
                exec_stop_time_stamp = entry_time_stamp
                steps_simplified.append(
                    make_step(
                        idx=idx,
                        event_type="tool_execution",
                        role="toolExecution",
                        lanes=[f"{TOOL_EXEC_PREFIX}{tool_call['name']}"],
                        start_time=exec_start_time_stamp,
                        stop_time=exec_stop_time_stamp,
                        session_start_time=session_first_time_stamp,
                        tool_names=[tool_call["name"]],
                        tool_call_ids=[tool_call["id"]],
                        tool_call_id=tool_call["id"],
                    )
                )

    computed["steps"] = steps_simplified
    computed["total_cost"] = sum(step["cost_total"] or 0 for step in steps_simplified)

    data_computed.append(computed)

DIR_FIGURES = Path(__file__).resolve().parent / "figures"
DIR_FIGURES.mkdir(parents=True, exist_ok=True)

DIR_FIGURES_RUNS = DIR_FIGURES / "runs"
DIR_FIGURES_RUNS.mkdir(parents=True, exist_ok=True)

for run_data in data_computed:

    run_id = run_data["run_id"]
    print(f"Plotting timeline for run {run_id}")

    steps = run_data["steps"]

    lanes = order_lanes({lane for step in steps for lane in step["lanes"]})
    agent_lanes = [lane for lane in lanes if not lane.startswith(TOOL_EXEC_PREFIX)]
    exec_lanes = [lane for lane in lanes if lane.startswith(TOOL_EXEC_PREFIX)]

    # Same center-to-center spacing for agent rows and tool-exec rows.
    ROW_STEP = 0.44
    bar_height = min(0.32, ROW_STEP * 0.72)
    # Match: (top bar top → axes top) == (last agent bar bottom → dashed line).
    PAD_MARGIN = 0.2

    lane_to_y = {}
    for i, lane in enumerate(agent_lanes):
        lane_to_y[lane] = i * ROW_STEP

    first_exec_y = None
    y_dash = None
    if exec_lanes:
        if agent_lanes:
            last_agent_y = (len(agent_lanes) - 1) * ROW_STEP
            y_dash = last_agent_y + bar_height / 2 + PAD_MARGIN
            first_exec_y = 2 * y_dash - last_agent_y
        else:
            first_exec_y = 0.0
        for j, lane in enumerate(exec_lanes):
            lane_to_y[lane] = first_exec_y + j * ROW_STEP
    else:
        first_exec_y = None

    y_positions = sorted(lane_to_y.items(), key=lambda kv: kv[1])
    ytick_vals = [y for _, y in y_positions]
    ytick_labs = [lab for lab, _ in y_positions]

    # Timeline (top) + cumulative cost vs time (bottom); share x-axis only.
    n_y_lanes = len(agent_lanes) + len(exec_lanes)
    timeline_h = max(2.5, 0.28 * max(1, n_y_lanes) + 1.0)
    fig, (ax, ax_cost) = plt.subplots(
        2,
        1,
        figsize=(10, 5),
        # sharex=True,
        gridspec_kw={"height_ratios": [2, 1.5]},
        constrained_layout=False,
    )
    min_bar_width_seconds = max(run_data["total_runtime"] * 0.001, 0.05)
    x_max = max(run_data["total_runtime"], min_bar_width_seconds)

    if agent_lanes and exec_lanes and y_dash is not None:
        ax.axhline(
            y=y_dash,
            color="0.45",
            linestyle=(0, (4, 4)),
            linewidth=1.0,
            zorder=0,
            clip_on=False,
        )

    for step in steps:
        start_seconds = step["start_seconds"]
        if start_seconds is None:
            continue

        dt_seconds = step["dt_seconds"]
        width_seconds = dt_seconds if dt_seconds and dt_seconds > 0 else min_bar_width_seconds
        width_seconds = max(width_seconds, min_bar_width_seconds)

        for lane in step["lanes"]:
            y = lane_to_y[lane]
            color = lane_color(lane)
            ax.broken_barh(
                [(start_seconds, width_seconds)],
                (y - bar_height / 2, bar_height),
                facecolors=color,
                edgecolors="black",
                linewidth=0.2,
                linestyle="dashed",
                alpha=1.0,
            )

    # ax.set_title(f"Timeline: {run_id}")
    ax.set_xlim(0, x_max)
    ax.tick_params(axis="x", labelbottom=False)
    y_min_vals = list(lane_to_y.values()) or [0]
    y_low = min(y_min_vals) - bar_height / 2 - PAD_MARGIN
    y_high = max(y_min_vals) + bar_height / 2 + PAD_MARGIN
    ax.set_ylim(y_low, y_high)
    ax.invert_yaxis()
    ax.xaxis.set_major_locator(MultipleLocator(100))
    ax.grid(axis="x", alpha=0.3)
    ax.set_axisbelow(True)
    ax.set_yticks(ytick_vals)
    ax.set_yticklabels(ytick_labs)

    cost_events = []
    cost_type_events = {cost_type: [] for cost_type in COST_TYPES_STACKED}
    for step in steps:
        t = step.get("stop_seconds")
        if t is None:
            t = step.get("start_seconds")
        if t is None:
            continue

        c = step.get("cost_total")
        if c is not None and c != 0:
            cost_events.append((t, float(c)))

        for cost_type in COST_TYPES_STACKED:
            cost_value = (step.get("cost") or {}).get(cost_type)
            if cost_value is None or cost_value == 0:
                continue
            try:
                cost_val = float(cost_value)
            except (TypeError, ValueError):
                continue
            cost_type_events[cost_type].append((t, cost_val))

    # Build a shared time axis so per-type cumulative areas stack correctly.
    total_inc_by_t = {}
    for t, c in cost_events:
        total_inc_by_t[t] = total_inc_by_t.get(t, 0.0) + c

    inc_by_type = {}
    for cost_type in COST_TYPES_STACKED:
        inc_map = {}
        for t, c in cost_type_events.get(cost_type, []):
            inc_map[t] = inc_map.get(t, 0.0) + c
        inc_by_type[cost_type] = inc_map

    time_points = {0.0, x_max}
    time_points.update(total_inc_by_t.keys())
    for inc_map in inc_by_type.values():
        time_points.update(inc_map.keys())
    time_points = sorted(time_points)

    times_cum = []
    cum = []
    running_total = 0.0
    for t in time_points:
        running_total += total_inc_by_t.get(t, 0.0)
        times_cum.append(t)
        cum.append(running_total)

    stacked_bottom = [0.0] * len(time_points)
    for cost_type in COST_TYPES_STACKED:
        running_type = 0.0
        type_cum = []
        inc_map = inc_by_type[cost_type]
        for t in time_points:
            running_type += inc_map.get(t, 0.0)
            type_cum.append(running_type)
        stacked_top = [b + v for b, v in zip(stacked_bottom, type_cum)]
        ax_cost.fill_between(
            time_points,
            stacked_bottom,
            stacked_top,
            step="post",
            color=COST_TYPE_COLORS[cost_type],
            alpha=0.28,
            label=cost_type,
        )
        stacked_bottom = stacked_top

    if len(cum) > 1:
        ax_cost.step(times_cum, cum, where="post", color="black", linewidth=1.5, label="total")

    all_cum_max = max(max(cum) if cum else 0.0, max(stacked_bottom) if stacked_bottom else 0.0)
    ax_cost.set_ylabel("Cumulative cost ($)")
    ax_cost.set_xlabel("Time since run start (seconds)")
    ymax = all_cum_max
    ax_cost.set_ylim(0, ymax * 1.08 if ymax > 0 else 1.0)
    ax_cost.set_xlim(0, x_max)
    ax_cost.xaxis.set_major_locator(MultipleLocator(100))
    ax_cost.grid(axis="both", alpha=0.3)
    ax_cost.set_axisbelow(True)
    if COST_TYPES_STACKED or len(ax_cost.lines) > 1:
        labels = [
            "Input",
            "Output",
            "Cache Read",
            "Total",
        ]
        ax_cost.legend(loc="upper left", fontsize=8, framealpha=0.9, ncol=2, labels=labels)

    repo_id = run_data["repo_id"]
    fig.suptitle(f"HLSFactory Agent Run: {repo_id}")
    fig.tight_layout()

    fig.savefig(DIR_FIGURES_RUNS / f"timeline__{run_id}.png", dpi=300)
    plt.close(fig)