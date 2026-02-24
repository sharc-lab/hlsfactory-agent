# TCL script for HLS synthesis
# Design: k_rr_a

open_project k_rr_a_proj
set_top k_rr_a

add_files k_rr_a.cpp -cflags "-I./stubs -I./src/common -I./src/device -DAP_INT_MAX_W=8192 -DAM_ABCD=3131"
add_files -tb k_rr_a_tb.cpp -cflags "-I./stubs -I./src/common -I./src/device"

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
