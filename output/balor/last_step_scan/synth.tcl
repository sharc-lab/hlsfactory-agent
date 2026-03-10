open_project last_step_scan
set_top last_step_scan
add_files last_step_scan.cpp
add_files -tb last_step_scan_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
