set_top main
open_solution -reset
add_files "/output/S2CBench/snow3G/main.cpp"
add_files "/output/S2CBench/snow3G/snow_3G.cpp"
add_files "/output/S2CBench/snow3G/tb_snow_3G.cpp"
add_files -tb "/output/S2CBench/snow3G/tb_snow_3G.cpp"
csynth_design
exit
