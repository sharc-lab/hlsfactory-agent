open_project aes_combined
set_top /output/S2CBench/aes_combined/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/aes_combined/aes.cpp
add_files /output/S2CBench/aes_combined/main.cpp
add_files /output/S2CBench/aes_combined/tb_aes.cpp
add_files -tb /output/S2CBench/aes_combined/tb_aes.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
