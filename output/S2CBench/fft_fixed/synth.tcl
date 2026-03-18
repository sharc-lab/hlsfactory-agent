open_project project
set_top void fft::entry()
add_files tb_fft.cpp main.cpp fft.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
