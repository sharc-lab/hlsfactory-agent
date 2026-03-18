open_project project
set_top fpga_temp_match
add_files {
  fpga_temp_match.cpp
  testbench.cpp
}
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
