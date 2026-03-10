set_top main
open_solution -reset
add_files "/output/S2CBench/aes/aes_invchipher/aes.cpp"
add_files "/output/S2CBench/aes/aes_invchipher/aes_inv_cipher.cpp"
add_files "/output/S2CBench/aes/aes_invchipher/main.cpp"
add_files "/output/S2CBench/aes/aes_invchipher/tb_aes.cpp"
add_files -tb "/output/S2CBench/aes/aes_invchipher/tb_aes.cpp"
csynth_design
exit
