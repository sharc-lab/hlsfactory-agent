set_top main
open_solution -reset
add_files "/output/S2CBench/aes/aes_cipher/aes.cpp"
add_files "/output/S2CBench/aes/aes_cipher/aes_cipher.cpp"
add_files "/output/S2CBench/aes/aes_cipher/main.cpp"
add_files "/output/S2CBench/aes/aes_cipher/tb_aes.cpp"
add_files -tb "/output/S2CBench/aes/aes_cipher/tb_aes.cpp"
csynth_design
exit
