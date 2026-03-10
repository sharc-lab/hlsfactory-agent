open_solution -reset solution1
set_top ellpack
add_files spmv-ellpack.c
add_files -tb spmv-ellpack_tb.cpp
open_solution solution1
csynth_design
exit
