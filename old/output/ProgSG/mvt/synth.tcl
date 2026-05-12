open_solution -reset solution1
set_top kernel_mvt
add_files mvt.c
add_files -tb mvt_tb.cpp
open_solution solution1
csynth_design
exit
