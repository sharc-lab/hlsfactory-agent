open_solution -reset solution1
set_top spmv
add_files spmv-crs.c
add_files -tb spmv-crs_tb.cpp
open_solution solution1
csynth_design
exit
