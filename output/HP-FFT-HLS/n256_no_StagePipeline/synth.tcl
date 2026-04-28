open_project /output/HP-FFT-HLS/n256_no_StagePipeline
set_top RADIX2_BFLY_double_buffer_quarter_CY
add_files [glob /output/HP-FFT-HLS/n256_no_StagePipeline/*.cpp /output/HP-FFT-HLS/n256_no_StagePipeline/*.h]
add_files -tb /output/HP-FFT-HLS/n256_no_StagePipeline/testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
