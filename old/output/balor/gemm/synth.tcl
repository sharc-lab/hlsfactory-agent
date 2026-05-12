open_project gemm
set_top gemm
add_files gemm.cpp
add_files -tb gemm_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
