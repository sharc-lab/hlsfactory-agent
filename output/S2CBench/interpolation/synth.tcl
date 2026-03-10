set_top main
open_solution -reset
add_files "/output/S2CBench/interpolation/filter_interp.cpp"
add_files "/output/S2CBench/interpolation/main.cpp"
add_files "/output/S2CBench/interpolation/tb_interp.cpp"
add_files -tb "/output/S2CBench/interpolation/tb_interp.cpp"
csynth_design
exit
