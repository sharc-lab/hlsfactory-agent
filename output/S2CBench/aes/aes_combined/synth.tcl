set_top main
open_solution -reset
add_files "/output/S2CBench/aes/aes_combined/aes.cpp"
add_files "/output/S2CBench/aes/aes_combined/main.cpp"
add_files "/output/S2CBench/aes/aes_combined/tb_aes.cpp"
add_files -tb "/output/S2CBench/aes/aes_combined/tb_aes.cpp"
csynth_design
exit
