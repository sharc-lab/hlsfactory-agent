# Vivado HLS/Vitis HLS Synthesis Script for FFT

# Open project
open_project hpcc_fft
set_top HPCC_fft235
add_files fft235.c
add_files hpccfft.h
add_files wrapfftw.h
add_files -tb testbench.cpp

# Open solution
open_solution "solution1"
set_part {xcu250-figd2104-2L-e}
create_clock -period 4 -name default

# Run synthesis
csynth_design

# Exit
exit
