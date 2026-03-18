open_project synth
set_top k_forward_rush_larsen
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
