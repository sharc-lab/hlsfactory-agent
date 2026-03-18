open_project synth
set_top task1fun_GetZ
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
