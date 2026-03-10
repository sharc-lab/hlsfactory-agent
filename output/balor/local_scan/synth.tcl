open_project local_scan
set_top local_scan
add_files local_scan.cpp
add_files -tb local_scan_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
