open_project disparity
set_top /output/S2CBench/disparity/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/disparity/disparity.cpp
add_files /output/S2CBench/disparity/main.cpp
add_files /output/S2CBench/disparity/tb_disparity.cpp
add_files -tb /output/S2CBench/disparity/tb_disparity.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
