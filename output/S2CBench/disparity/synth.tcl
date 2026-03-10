set_top main
open_solution -reset
add_files "/output/S2CBench/disparity/disparity.cpp"
add_files "/output/S2CBench/disparity/main.cpp"
add_files "/output/S2CBench/disparity/tb_disparity.cpp"
add_files -tb "/output/S2CBench/disparity/tb_disparity.cpp"
csynth_design
exit
