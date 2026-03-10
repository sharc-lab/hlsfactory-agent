open_project fft
set_top fft
add_files fft.cpp
add_files -tb fft_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
