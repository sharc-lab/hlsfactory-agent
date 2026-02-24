# TCL script for HLS synthesis
# Design: k_compress_store

open_project k_compress_store_proj
set_top k_compress_store

add_files k_compress_store.cpp -cflags "-I./stubs -I./src/common -I./src/device -DAP_INT_MAX_W=8192 -DAM_ABCD=3131"
add_files -tb k_compress_store_tb.cpp -cflags "-I./stubs -I./src/common -I./src/device"

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
