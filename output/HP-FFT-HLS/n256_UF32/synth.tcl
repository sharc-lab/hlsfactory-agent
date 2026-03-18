open_project n256_UF32_proj
set_top FFT_TOP
add_files -cflags "-I/workspace/stubs -I/output/HP-FFT-HLS/n256_UF32" /output/HP-FFT-HLS/n256_UF32/*.cpp
add_files -tb /output/HP-FFT-HLS/n256_UF32/testbench.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
create_clock -period 5 -name default
csynth_design
exit
