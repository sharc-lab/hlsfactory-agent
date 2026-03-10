open_project stencil3d
set_top stencil3d
add_files stencil3d.cpp
add_files -tb stencil3d_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
