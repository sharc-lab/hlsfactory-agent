set_top main
open_solution -reset
add_files "/output/S2CBench/fft_fixed/fft.cpp"
add_files "/output/S2CBench/fft_fixed/main.cpp"
add_files "/output/S2CBench/fft_fixed/tb_fft.cpp"
add_files -tb "/output/S2CBench/fft_fixed/tb_fft.cpp"
csynth_design
exit
