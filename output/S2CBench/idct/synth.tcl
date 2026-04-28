open_project idct
set_top /output/S2CBench/idct/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/idct/idct.cpp
add_files /output/S2CBench/idct/main.cpp
add_files /output/S2CBench/idct/tb_idct.cpp
add_files -tb /output/S2CBench/idct/tb_idct.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
