open_project kasumi
set_top /output/S2CBench/kasumi/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/kasumi/kasumi.cpp
add_files /output/S2CBench/kasumi/main.cpp
add_files /output/S2CBench/kasumi/tb_kasumi.cpp
add_files -tb /output/S2CBench/kasumi/tb_kasumi.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
