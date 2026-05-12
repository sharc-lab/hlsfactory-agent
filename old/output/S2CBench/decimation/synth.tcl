open_project decimation
set_top /output/S2CBench/decimation/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/decimation/filt_decim.cpp
add_files /output/S2CBench/decimation/main.cpp
add_files /output/S2CBench/decimation/tb_decim.cpp
add_files -tb /output/S2CBench/decimation/tb_decim.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
