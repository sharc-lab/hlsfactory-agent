# Vitis HLS Synthesis Script for CHIP-KNN Single PE Design
# Target: Xilinx U280 FPGA

open_project chip_knn_singlePE
set_top Knn
add_files src/knn.cpp
add_files src/knn.h

open_solution "solution1"
set_part {xcu280-fsvh2892-2L-e}
create_clock -period 4.44 -name default

# HLS Directives
config_compile -pragma_strict_mode
config_interface -m_axi_addr64

# Run synthesis
csynth_design

# Export design
export_design -rtl verilog -format ip_catalog

close_project
