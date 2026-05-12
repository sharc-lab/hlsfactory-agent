open_project interpolation
set_top /output/S2CBench/interpolation/main.cpp:int sc_main(int argc, char* argv[]){
add_files /output/S2CBench/interpolation/filter_interp.cpp
add_files /output/S2CBench/interpolation/main.cpp
add_files /output/S2CBench/interpolation/tb_interp.cpp
add_files -tb /output/S2CBench/interpolation/tb_interp.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
