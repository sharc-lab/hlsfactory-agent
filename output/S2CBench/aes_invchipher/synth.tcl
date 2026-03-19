open_project aes_invchipher
set_top /output/S2CBench/aes_invchipher/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/aes_invchipher/aes.cpp
add_files /output/S2CBench/aes_invchipher/aes_inv_cipher.cpp
add_files /output/S2CBench/aes_invchipher/main.cpp
add_files /output/S2CBench/aes_invchipher/tb_aes.cpp
add_files -tb /output/S2CBench/aes_invchipher/tb_aes.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
