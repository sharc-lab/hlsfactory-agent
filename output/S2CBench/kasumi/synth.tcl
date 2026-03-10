set_top main
open_solution -reset
add_files "/output/S2CBench/kasumi/kasumi.cpp"
add_files "/output/S2CBench/kasumi/main.cpp"
add_files "/output/S2CBench/kasumi/tb_kasumi.cpp"
add_files -tb "/output/S2CBench/kasumi/tb_kasumi.cpp"
csynth_design
exit
