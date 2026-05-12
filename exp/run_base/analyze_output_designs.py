import json
from pathlib import Path
from typing import Any

import matplotlib.pyplot as plt
from matplotlib.lines import Line2D


DIR_CURRENT = Path(__file__).resolve().parent
DIR_BUILDS = DIR_CURRENT / "builds"

# build_check_data.json

build_check_data_fps = sorted(DIR_BUILDS.rglob("build_check_data.json"))

build_check_data_dicts = [json.loads(fp.read_text()) for fp in build_check_data_fps]

runs_with_no_designs: list[str] = []
runs_with_designs: list[str] = []
runs_num_designs_dict: dict[str, int] = {}
runs_repo_name_dict: dict[str, str] = {}
runs_designs_dict: dict[str, list[dict[str, Any]]] = {}
for build_check_data_dict in build_check_data_dicts:
    run_id = build_check_data_dict["run_id"]
    repo_id = build_check_data_dict["repo_id"]
    repo_name = repo_id.split("/")[-1]
    runs_repo_name_dict[run_id] = repo_name

    num_designs = len(build_check_data_dict["check__designs"])
    runs_num_designs_dict[run_id] = num_designs
    if num_designs == 0:
        runs_with_no_designs.append(run_id)
    else:
        runs_with_designs.append(run_id)

    # for each design, check if it has the key check__csynth_ran_successfully and if it is True
    runs_designs_dict[run_id] = []
    for design in build_check_data_dict["check__designs"]:
        data = {}
        design_name = design["design_name"]
        data["design_name"] = design_name


        if "check__csynth_ran_successfully" in design and design["check__csynth_ran_successfully"]:
            data["working_design"] = True
        else:
            data["working_design"] = False

        runs_designs_dict[run_id].append(data)
    
print(f"Runs with no designs: {len(runs_with_no_designs)}")
print(f"Runs with designs: {len(runs_with_designs)}")

# pp(sorted(runs_num_designs_dict.items(), key=lambda x: x[1], reverse=True))

# for each run, print the number of working designs
# if it has none say "No generated designs"
# it it has some print each deisgn name and if it is working or not
# make this like a report to print out to console
# print("--------------------------------")
# print("Report on generated designs")
# print("--------------------------------")
# for run_id, designs in runs_designs_dict.items():
#     print(f"{run_id}:")
#     if len(designs) == 0:
#         print("No generated designs")
#     else:
#         for design in designs:
#             # if working do PASS: ...
#             # if not working do FAIL: ..
#             if design["working_design"]:
#                 print(f"PASS: {design['design_name']}")
#             else:
#                 print(f"FAIL: {design['design_name']}")
#     print("--------------------------------")

DIR_FIGURES = DIR_CURRENT / "figures"
DIR_FIGURES.mkdir(parents=True, exist_ok=True)


    # "AlexMontgomerie/fpgaconvnet-hls",
    # "DARClab-UTD/S2CBench",
    # "ETHZ-DYNAMO/balor",
    # "FedericoSerafini/HLS-CNN",
    # "KastnerRG/Spector-HLS",
    # "OswaldHe/InTAR",
    # "SFU-HiAccel/AutoNTT",
    # "SFU-HiAccel/BitBlender",
    # "SFU-HiAccel/CHIP-KNN",
    # "SFU-HiAccel/FORC",
    # "SFU-HiAccel/HiSpMV",
    # "SFU-HiAccel/SyncNN",
    # "SFU-HiAccel/blaze",
    # "SFU-HiAccel/pasta",
    # "SFU-HiAccel/SERI",
    # "TurakhiaLab/DP-HLS",
    # "UCLA-VAST/CLINK",
    # "UCLA-VAST/HP-FFT-HLS",
    # "UIUC-ChenLab/ScaleHLS-HIDA",
    # "Xtra-Computing/ThunderGP",
    # "ZongyueQin/ProgSG",
    # "bsc-loca/PQC-Crystals-HLS-Accelerators",
    # "icl-utk-edu/hpcc",
    # "robertoBosio/NN2FPGA",
    # "spcl/gemm_hls",
    # "ECASLab/hls-fpga-accelerators",
MAP_REPO_NAME_CLEAN ={
    "fpgaconvnet-hls": "AlexMontgomerie\nfpgaconvnet-hls",
    "S2CBench": "DARClab-UTD\nS2CBench",
    "balor": "ETHZ-DYNAMO\nbalor",
    "HLS-CNN": "FedericoSerafini\nHLS-CNN",
    "Spector-HLS": "KastnerRG\nSpector-HLS",
    "InTAR": "OswaldHe\nInTAR",
    "AutoNTT": "SFU-HiAccel\nAutoNTT",
    "ThunderGP": "Xtra-Computing\nThunderGP",
    "ProgSG": "ZongyueQin\nProgSG",
    "PQC-Crystals-HLS-Accelerators": "bsc-loca\nPQC-Crystals-Acc",
    "hpcc": "icl-utk-edu\nhpcc",
    "NN2FPGA": "robertoBosio\nNN2FPGA",
    "gemm_hls": "spcl\ngemm_hls",
    "hls-fpga-accelerators": "ECASLab\nhls-fpga-acc",
    "CLINK": "UCLA-VAST\nCLINK",
    "HP-FFT-HLS": "UCLA-VAST\nHP-FFT-HLS",
    "ScaleHLS-HIDA": "UIUC-ChenLab\nScaleHLS-HIDA",
    "blaze": "SFU-HiAccel\nblaze",
    "pasta": "SFU-HiAccel\npasta",
    "SERI": "SFU-HiAccel\nSERI",
    "DP-HLS": "TurakhiaLab\nDP-HLS",
    "SyncNN": "SFU-HiAccel\nSyncNN",
    "CHIP-KNN": "SFU-HiAccel\nCHIP-KNN",
    "FORC": "SFU-HiAccel\nFORC",
    "HiSpMV": "SFU-HiAccel\nHiSpMV",
    "BitBlender": "SFU-HiAccel\nBitBlender",
}

# Build per-source pass/fail counts and include sources with zero designs.
source_counts: list[tuple[str, int, int, int]] = []
for run_id in runs_num_designs_dict:
    designs = runs_designs_dict[run_id]
    pass_count = sum(1 for design in designs if design["working_design"])
    fail_count = len(designs) - pass_count
    total_count = pass_count + fail_count
    source_counts.append((run_id, pass_count, fail_count, total_count))

# Sort by passing designs first, then failing designs (both high to low).
source_counts.sort(key=lambda item: (item[1], item[2]), reverse=True)

sources = [MAP_REPO_NAME_CLEAN[runs_repo_name_dict[item[0]]] for item in source_counts]
pass_counts = [item[1] for item in source_counts]
fail_counts = [item[2] for item in source_counts]
total_counts = [item[3] for item in source_counts]

fig, ax = plt.subplots(figsize=(12, 4))
ax.set_axisbelow(True)
pass_bars = ax.bar(sources, pass_counts, width=0.72, color="green", label="Passing designs")
ax.bar(sources, fail_counts, width=0.72, bottom=pass_counts, color="red", label="Failing designs")
bar_centers = [bar.get_x() + (bar.get_width() / 2) for bar in pass_bars]
ax.set_xlim(-0.5, len(sources) - 0.5)
ax.yaxis.grid(True, linestyle="--", linewidth=0.8, alpha=0.5)

y_max = max(total_counts, default=0)
if y_max == 0:
    ax.set_ylim(0, 1)
else:
    ax.set_ylim(0, y_max * 1.15)

for i, total_count in enumerate(total_counts):
    if total_count == 0:
        x_center = pass_bars[i].get_x() + (pass_bars[i].get_width() / 2)
        ax.text(
            x_center,
            ax.get_ylim()[1] * 0.02,
            "✗",
            ha="center",
            va="bottom",
            fontsize=18,
            fontweight="bold",
            color="red",
        )

# Dashed separators between:
# 1) sources with some passing designs and sources with only failing designs
# 2) sources with only failing designs and sources with no designs
first_only_failing_idx = next(
    (i for i, (pass_count, fail_count) in enumerate(zip(pass_counts, fail_counts)) if pass_count == 0 and fail_count > 0),
    None,
)
first_no_designs_idx = next((i for i, total_count in enumerate(total_counts) if total_count == 0), None)

if first_only_failing_idx is not None and first_only_failing_idx > 0:
    if pass_counts[first_only_failing_idx - 1] > 0:
        ax.axvline(
            x=(bar_centers[first_only_failing_idx - 1] + bar_centers[first_only_failing_idx]) / 2,
            color="gray",
            linestyle="-",
            linewidth=1.2,
            alpha=0.7,
        )

if first_no_designs_idx is not None and first_no_designs_idx > 0:
    prev_idx = first_no_designs_idx - 1
    if pass_counts[prev_idx] == 0 and fail_counts[prev_idx] > 0:
        ax.axvline(
            x=(bar_centers[prev_idx] + bar_centers[first_no_designs_idx]) / 2,
            color="gray",
            linestyle="-",
            linewidth=1.2,
            alpha=0.7,
        )

# Group labels centered above each contiguous section.
if sources:
    section_title_fontsize = 12
    first_non_passing_idx = next((i for i, pass_count in enumerate(pass_counts) if pass_count == 0), len(sources))
    if first_non_passing_idx > 0:
        ax.text(
            (bar_centers[0] + bar_centers[first_non_passing_idx - 1]) / 2,
            0.95,
            "Some Valid Designs Extracted",
            transform=ax.get_xaxis_transform(),
            ha="center",
            va="top",
            fontsize=section_title_fontsize,
            fontweight="bold",
        )

    if first_only_failing_idx is not None:
        end_only_failing_idx = (first_no_designs_idx - 1) if first_no_designs_idx is not None else (len(sources) - 1)
        if end_only_failing_idx >= first_only_failing_idx:
            ax.text(
                (bar_centers[first_only_failing_idx] + bar_centers[end_only_failing_idx]) / 2,
                0.95,
                "Designs Extracted But None Valid",
                transform=ax.get_xaxis_transform(),
                ha="center",
                va="top",
                fontsize=section_title_fontsize,
                fontweight="bold",
            )

    if first_no_designs_idx is not None:
        ax.text(
            (bar_centers[first_no_designs_idx] + bar_centers[-1]) / 2,
            0.95,
            "No Designs Extracted",
            transform=ax.get_xaxis_transform(),
            ha="center",
            va="top",
            fontsize=section_title_fontsize,
            fontweight="bold",
        )

total_passing = sum(pass_counts)
total_failing = sum(fail_counts)
fig.suptitle(
    "Passing vs Failing Designs by Design Source\n"
    f"({total_passing} passing, {total_failing} failing)",
    y=0.98,
    fontsize=12,
    fontweight="bold",
)
ax.set_xlabel("")
ax.set_ylabel("Number of Designs", fontweight="bold")
legend_handles, legend_labels = ax.get_legend_handles_labels()
legend_handles.append(
    Line2D(
        [0],
        [0],
        linestyle="None",
        marker="$✗$",
        color="red",
        markersize=12,
        label="No designs extracted",
    )
)
legend_labels.append("No designs extracted")
fig.legend(
    legend_handles,
    legend_labels,
    loc="upper center",
    bbox_to_anchor=(0.5, 0.895),
    ncol=3,
    # frameon=False,
)
ax.tick_params(axis="x", rotation=75)
for tick_label in ax.get_xticklabels():
    tick_label.set_ha("right")
    tick_label.set_rotation_mode("anchor")
fig.tight_layout(rect=[0, 0, 1, 0.94])

output_fp = DIR_FIGURES / "design_source_pass_fail_stacked_bar.png"
fig.savefig(output_fp, dpi=300)
print(f"Saved plot to: {output_fp}")
