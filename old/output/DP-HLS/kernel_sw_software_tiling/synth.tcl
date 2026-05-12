open_project kernel_sw_software_tiling
set_top unknown_top
add_files {kernel_sw_software_tiling.cpp}
add_files -tb {testbench.cpp}
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
