open_solution -reset solution1
set_top stencil3d
add_files stencil-3d.c
add_files -tb stencil-3d_tb.cpp
open_solution solution1
csynth_design
exit
