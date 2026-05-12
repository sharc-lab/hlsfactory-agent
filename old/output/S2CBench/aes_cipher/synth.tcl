open_project aes_cipher
set_top /output/S2CBench/aes_cipher/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/aes_cipher/aes.cpp
add_files /output/S2CBench/aes_cipher/aes_cipher.cpp
add_files /output/S2CBench/aes_cipher/main.cpp
add_files /output/S2CBench/aes_cipher/tb_aes.cpp
add_files -tb /output/S2CBench/aes_cipher/tb_aes.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
