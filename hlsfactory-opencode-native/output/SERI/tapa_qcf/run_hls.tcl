# TCL script for HLS synthesis
# Design: tapa_qcf

open_project tapa_qcf_proj
set_top tapa_qcf

add_files tapa_qcf.cpp -cflags "-I./stubs -I./src/common -I./src/device -DAP_INT_MAX_W=8192 -DAM_ABCD=3131"
add_files -tb tapa_qcf_tb.cpp -cflags "-I./stubs -I./src/common -I./src/device"

open_solution "solution1" -flow_target vitis
set_part {xcu280-fsvh2892-2L-e}
create_clock -period 300MHz -name default

# Run C simulation
csim_design

# Run C synthesis
csynth_design

# Run C/RTL cosimulation
cosim_design

# Export design
export_design -format xo

exit
