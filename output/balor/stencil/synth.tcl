open_project stencil
set_top stencil
add_files stencil.cpp
add_files -tb stencil_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
