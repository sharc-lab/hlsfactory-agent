import sys
import os
import re

#######################################
## RUNTIME ESTIMATION:
#######################################

def Get_HLS_Max_Latency(_fname):
    latencies_line_num = 32         ## The HLS latencies show up on this line # of the report.

    the_file = open(_fname, "r")
    latency_str = the_file.readlines()[latencies_line_num - 1]
    latency_str = latency_str.split("|")[2].strip()

    return int(latency_str)


"""
### This function parses the HLS-reported latencies from the multi-PE design, and estimates the runtime of the design.
"""
def Estimate_Runtime_from_HLS_Latencies(_achieved_memsys_freq, _achieved_kern_freq, _approx_num_hiermerge_stages):

    report_dir_relpath = "run/report/"

    load_rpt_fname = "load_KNN_csynth.rpt"
    compute_rpt_fname = "compute_KNN_csynth.rpt"
    sort_rpt_fname = "para_partial_sort_csynth.rpt"
    merge_rpt_fname = "merge_dual_streams_csynth.rpt"
    load_HLS_cycles = 0
    compute_HLS_cycles = 0
    sort_HLS_cycles = 0
    merge_HLS_cycles = 0

    #########################
    ## Grab the HLS latencies

    load_HLS_cycles     = Get_HLS_Max_Latency(report_dir_relpath + load_rpt_fname)
    compute_HLS_cycles  = Get_HLS_Max_Latency(report_dir_relpath + compute_rpt_fname)
    sort_HLS_cycles     = Get_HLS_Max_Latency(report_dir_relpath + sort_rpt_fname)
    merge_HLS_cycles    = Get_HLS_Max_Latency(report_dir_relpath + merge_rpt_fname)

    #########################
    ## Estimate the runtime


    max_kern_freq = 225
    max_memsys_freq = 450
    MEMSYS_SCALAR = 0.93

    ### Modify the cycle estimates, to correct for frequency.
    ### For load, divide by a correction factor to account for frequency disparities,
    ### and the theoretical vs practical peak bandwidth.
    load_cycles_CORRECTED       = load_HLS_cycles / (MEMSYS_SCALAR * min(_achieved_memsys_freq/max_memsys_freq, _achieved_kern_freq/max_kern_freq))
    compute_cycles_CORRECTED    = compute_HLS_cycles / (_achieved_kern_freq/max_kern_freq)
    sort_cycles_CORRECTED       = sort_HLS_cycles / (_achieved_kern_freq/max_kern_freq)
    merge_cycles_CORRECTED      = merge_HLS_cycles / (_achieved_kern_freq/max_kern_freq)

    total_cycles = max(load_cycles_CORRECTED, compute_cycles_CORRECTED, sort_cycles_CORRECTED) + _approx_num_hiermerge_stages * merge_cycles_CORRECTED

    estimated_runtime = float(total_cycles) / (max_kern_freq*1000*1000)

    return estimated_runtime


def Log_Runtime_Estimate(_out_file_name, _achieved_memsys_freq, _achieved_kern_freq, _num_hiermerge_stages=2):
    out_file_contents = []

    out_file_contents.append("  RUNTIME ESTIMATION\n")
    out_file_contents.append("------------------------------------------------------------\n")
    out_file_contents.append("Runtime estimation script is using:\n")
    out_file_contents.append("  achieved_memsys_freq = {}\n". format(_achieved_memsys_freq))
    out_file_contents.append("  achieved_kern_freq = {}\n".   format(_achieved_kern_freq))
    out_file_contents.append("  num_hiermerge_stages = {}\n". format(_num_hiermerge_stages))

    estimated_runtime = Estimate_Runtime_from_HLS_Latencies(_achieved_memsys_freq, _achieved_kern_freq, _num_hiermerge_stages)
    out_file_contents.append("Estimated runtime is: {}\n ".format(estimated_runtime))
    out_file_contents.append("\n\n\n")

    with open(_out_file_name, "w") as f:
        # go to start of file
        f.seek(0)
        # actually write the lines
        f.writelines(out_file_contents)

#######################################
## DATA GRABBING:
#######################################


def _Get_HW_Resource_Total_CLB_LUT(util_report_lines):
    for line in util_report_lines:
        if ("CLB LUTs" in line):
            total_resources = line.split("|")[5]
            total_resources = total_resources.strip()
            break

    return float(total_resources)


def _Get_HW_Resource_Total_CLB_REG(util_report_lines):
    for line in util_report_lines:
        if ("CLB Registers" in line):
            reg_resources = line.split("|")[5]
            reg_resources = reg_resources.strip()
            break

    return float(reg_resources)


def _Get_HW_Resource_Total_FF(util_report_lines):
    for line in util_report_lines:
        if ("Register as Flip Flop" in line):
            ff_resources = line.split("|")[5]
            ff_resources = ff_resources.strip()
            break

    return float(ff_resources)


def _Get_HW_Resource_Total_CLB(util_report_lines):
    section = 0
    for line in util_report_lines:
        if ("2. CLB Logic Distribution" in line):
            section += 1
        elif ("CLB      " in line) and (section == 2):
            total_resources = line.split("|")[5]
            total_resources = total_resources.strip()
            break

    return float(total_resources)


def _Get_HW_Resource_Total_BRAM(util_report_lines):
    section = 0
    for line in util_report_lines:
        if ("3. BLOCKRAM" in line):
            section += 1
        elif ("Block RAM Tile" in line) and (section == 2):
            total_resources = line.split("|")[5]
            total_resources = total_resources.strip()
            break

    return float(total_resources)

def _Get_HW_Resource_Total_URAM(util_report_lines):
    section = 0
    for line in util_report_lines:
        if ("3. BLOCKRAM" in line):
            section += 1
        elif ("URAM" in line) and (section == 2):
            total_resources = line.split("|")[5]
            total_resources = total_resources.strip()
            break

    return float(total_resources)


def _Get_HW_Resource_Total_DSP(util_report_lines):
    section = 0
    for line in util_report_lines:
        if ("4. ARITHMETIC" in line):
            section += 1
        elif ("DSPs" in line) and (section == 2):
            total_resources = line.split("|")[5]
            total_resources = total_resources.strip()
            break

    return float(total_resources)


def _Get_HW_Resource_CLB_per_SLR(util_report_lines):
    clb_per_slr = [0, 0, 0]
    section = 0
    for line in util_report_lines:
        if ("14. SLR CLB Logic and Dedicated Block Utilization" in line):
            section += 1
        elif ("CLB    " in line) and (section == 2):
            for i in range(0, 3):
                clb_per_slr[i] = line.split("|")[5+i]
                clb_per_slr[i] = clb_per_slr[i].strip()
                clb_per_slr[i] = float(clb_per_slr[i])
            break

    return clb_per_slr



def Log_HW_Resource_Usages(_out_file_name):
    out_file_contents = []
    out_file_contents.append("  UTILIZATION REPORTS" + "\n")
    out_file_contents.append("------------------------------------------------------------" + "\n")

    report_dir_relpath = "vitis_run_hw/knn_xilinx_u280_xdma_201920_3.temp/reports/link/imp/"
    util_report_fname = "impl_1_full_util_routed.rpt"

    util_report_file = open(report_dir_relpath + util_report_fname, "r")
    util_report_lines = util_report_file.readlines()

    total_LUT_usage     = _Get_HW_Resource_Total_CLB_LUT    (util_report_lines)
    total_CLB_usage     = _Get_HW_Resource_Total_CLB        (util_report_lines)
    total_CLB_REG_usage = _Get_HW_Resource_Total_CLB_REG    (util_report_lines)
    total_FF_usage      = _Get_HW_Resource_Total_FF         (util_report_lines)
    total_BRAM_usage    = _Get_HW_Resource_Total_BRAM       (util_report_lines)
    total_URAM_usage    = _Get_HW_Resource_Total_URAM       (util_report_lines)
    total_DSP_usage     = _Get_HW_Resource_Total_DSP        (util_report_lines)
    CLBs_per_SLR_usage  = _Get_HW_Resource_CLB_per_SLR      (util_report_lines)

    for i in range(0, 3):
        out_file_contents.append("CLBs in SLR {}: {}".format(i, CLBs_per_SLR_usage[i]) + "\n")
    out_file_contents.append("Total LUT Usage:      {}".format(total_LUT_usage) + "\n")
    out_file_contents.append("Total CLB_REG Usage:  {}".format(total_CLB_REG_usage) + "\n")
    out_file_contents.append(" NOTE: Total FF Usage:      {}".format(total_FF_usage) + "\n")
    out_file_contents.append("Total BRAM Usage:     {}".format(total_BRAM_usage) + "\n")
    out_file_contents.append("Total URAM Usage:     {}".format(total_URAM_usage) + "\n")
    out_file_contents.append("Total DSP Usage:      {}".format(total_DSP_usage) + "\n")

    out_file_contents.append("Total CLB Usage:  {}".format(total_CLB_usage) + "\n")

    with open(_out_file_name, "a") as f:
        f.writelines(out_file_contents)


def Get_frequencies():
    report_dir_relpath = "vitis_run_hw/"
    freq_report_fname = "knn_xilinx_u280_xdma_201920_3.xclbin.info"

    freq_report_file = open(report_dir_relpath + freq_report_fname, "r")
    freq_report_lines = freq_report_file.readlines()

    section = 0
    for line in freq_report_lines:
        if ("DATA_CLK" in line):
            section = 1
        elif ("hbm_aclk" in line):
            section = 2
        elif ("KERNEL_CLK" in line):
            break
        elif ("Frequency" in line) and (section == 1):
            kern_freq = line.split(":")[1]
            kern_freq = kern_freq.split("MHz")[0].strip()
            kern_freq = int(kern_freq)
        elif ("Frequency" in line) and (section == 2):
            mem_freq = line.split(":")[1]
            mem_freq = mem_freq.split("MHz")[0].strip()
            mem_freq = int(mem_freq)

    return (mem_freq, kern_freq)


if __name__ == "__main__":
    if ((len(sys.argv) > 1)):
        print("")
        print("ERROR: Incorrect arguments.")
        print("     This program does not take any CLI arguments.")
        print("")
        sys.exit(-1)

    output_filename = "RUNTIME_AND_RESOURCES.log"

    (mem_freq, kern_freq) = Get_frequencies()
    Log_Runtime_Estimate(output_filename, mem_freq, kern_freq)

    Log_HW_Resource_Usages(output_filename)
