open_project sobel
set_top /output/S2CBench/sobel/main.cpp:int sc_main(int argc, char* argv[]){
add_files /output/S2CBench/sobel/main.cpp
add_files /output/S2CBench/sobel/sobel.cpp
add_files /output/S2CBench/sobel/tb_sobel.cpp
add_files -tb /output/S2CBench/sobel/tb_sobel.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
