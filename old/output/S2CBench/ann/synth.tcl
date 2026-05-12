open_project ann
set_top /output/S2CBench/ann/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/ann/ann.cpp
add_files /output/S2CBench/ann/ann_tb.cpp
add_files /output/S2CBench/ann/layer.cpp
add_files /output/S2CBench/ann/main.cpp
add_files -tb /output/S2CBench/ann/ann_tb.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
