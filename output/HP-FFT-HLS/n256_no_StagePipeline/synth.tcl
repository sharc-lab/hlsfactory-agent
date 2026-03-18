open_project n256_no_StagePipeline_proj
set_top FFT_TOP
add_files -cflags "-I/workspace/stubs -I/output/HP-FFT-HLS/n256_no_StagePipeline" /output/HP-FFT-HLS/n256_no_StagePipeline/*.cpp
add_files -tb /output/HP-FFT-HLS/n256_no_StagePipeline/testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
create_clock -period 5 -name default
csynth_design
exit
