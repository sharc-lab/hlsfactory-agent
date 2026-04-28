open_project fir
set_top /output/S2CBench/fir/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/fir/fir.cpp
add_files /output/S2CBench/fir/main.cpp
add_files /output/S2CBench/fir/tb_fir.cpp
add_files -tb /output/S2CBench/fir/tb_fir.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
