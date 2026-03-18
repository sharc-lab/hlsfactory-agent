open_project project
set_top void aes::assign_key(U8 key_in[SIZE], U8 key_out[SIZE])
add_files tb_aes_5.cpp tb_aes_2.cpp aes.cpp aes_inv_cipher.cpp aes_cipher.cpp aes_combined/aes.cpp aes_combined/tb_aes.cpp aes_combined/main.cpp tb_aes_1.cpp main_2.cpp aes_cipher/aes.cpp aes_cipher/aes_cipher.cpp aes_cipher/tb_aes.cpp aes_cipher/main.cpp tb_aes.cpp tb_aes_3.cpp aes_invchipher/aes.cpp aes_invchipher/aes_inv_cipher.cpp aes_invchipher/tb_aes.cpp aes_invchipher/main.cpp aes_2.cpp main.cpp tb_aes_4.cpp main_1.cpp aes_1.cpp 
add_files -tb tb_aes_5.cpp tb_aes_2.cpp aes_combined/tb_aes.cpp tb_aes_1.cpp aes_cipher/tb_aes.cpp tb_aes.cpp tb_aes_3.cpp aes_invchipher/tb_aes.cpp tb_aes_4.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
