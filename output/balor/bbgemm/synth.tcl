open_project bbgemm
set_top bbgemm
add_files bbgemm.cpp
add_files -tb bbgemm_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
