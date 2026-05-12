open_project k_compress_store_proj
set_top k_compress_store
add_files k_compress_store.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
create_clock -period 3.33 -name default
csynth_design
exit
