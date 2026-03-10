set_top main
open_solution -reset
add_files "/output/S2CBench/ave8/ave8.cpp"
add_files "/output/S2CBench/ave8/main.cpp"
add_files "/output/S2CBench/ave8/tb_ave8.cpp"
add_files -tb "/output/S2CBench/ave8/tb_ave8.cpp"
csynth_design
exit
