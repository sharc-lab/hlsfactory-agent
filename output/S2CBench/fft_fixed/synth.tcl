open_project fft_fixed
set_top /output/S2CBench/fft_fixed/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/fft_fixed/fft.cpp
add_files /output/S2CBench/fft_fixed/main.cpp
add_files /output/S2CBench/fft_fixed/tb_fft.cpp
add_files -tb /output/S2CBench/fft_fixed/tb_fft.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
