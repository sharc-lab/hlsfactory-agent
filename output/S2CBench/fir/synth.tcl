set_top main
open_solution -reset
add_files "/output/S2CBench/fir/fir.cpp"
add_files "/output/S2CBench/fir/main.cpp"
add_files "/output/S2CBench/fir/tb_fir.cpp"
add_files -tb "/output/S2CBench/fir/tb_fir.cpp"
csynth_design
exit
