open_solution -reset solution1
set_top stencil
add_files stencil_stencil2d.c
add_files -tb stencil_stencil2d_tb.cpp
open_solution solution1
csynth_design
exit
