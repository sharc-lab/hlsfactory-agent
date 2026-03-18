open_project synth
set_top reverse
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
