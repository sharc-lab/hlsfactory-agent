set_top main
open_solution -reset
add_files "/output/S2CBench/idct/idct.cpp"
add_files "/output/S2CBench/idct/main.cpp"
add_files "/output/S2CBench/idct/tb_idct.cpp"
add_files -tb "/output/S2CBench/idct/tb_idct.cpp"
csynth_design
exit
