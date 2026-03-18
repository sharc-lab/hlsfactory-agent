open_project synth
set_top kernel_512_one_128
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
