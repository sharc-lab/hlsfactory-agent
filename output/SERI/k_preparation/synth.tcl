open_project k_preparation_proj
set_top /output/SERI/k_preparation/k_preparation.cpp:k_preparation
set_part xcu250-figd2104-2L-e
add_files {/output/SERI/k_preparation/*.cpp}
add_files -tb {/output/SERI/k_preparation/testbench.cpp}
open_solution "solution1"
csynth_design
exit
