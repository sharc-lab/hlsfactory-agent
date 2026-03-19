open_project /output/HP-FFT-HLS/n1024_original_C_style
set_top RADIX2_BFLY_double_buffer_quarter_CY
add_files [glob /output/HP-FFT-HLS/n1024_original_C_style/*.cpp /output/HP-FFT-HLS/n1024_original_C_style/*.h]
add_files -tb /output/HP-FFT-HLS/n1024_original_C_style/testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
