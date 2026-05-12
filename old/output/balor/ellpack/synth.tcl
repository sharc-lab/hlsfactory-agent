open_project ellpack
set_top ellpack
add_files ellpack.cpp
add_files -tb ellpack_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
