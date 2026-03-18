open_project knn_proj
set_top Knn
add_files knn.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
