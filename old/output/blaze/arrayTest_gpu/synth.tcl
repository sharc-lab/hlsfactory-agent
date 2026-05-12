open_project project
set_top ArrayTest
add_files ArrayTest.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
create_clock -period 5 -name default
csynth_design
exit
