set_top main
open_solution -reset
add_files "/output/S2CBench/sobel/main.cpp"
add_files "/output/S2CBench/sobel/sobel.cpp"
add_files "/output/S2CBench/sobel/tb_sobel.cpp"
add_files -tb "/output/S2CBench/sobel/tb_sobel.cpp"
csynth_design
exit
