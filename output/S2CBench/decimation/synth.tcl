set_top main
open_solution -reset
add_files "/output/S2CBench/decimation/filt_decim.cpp"
add_files "/output/S2CBench/decimation/main.cpp"
add_files "/output/S2CBench/decimation/tb_decim.cpp"
add_files -tb "/output/S2CBench/decimation/tb_decim.cpp"
csynth_design
exit
