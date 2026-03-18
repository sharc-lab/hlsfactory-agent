open_project synth
set_top zoom_in_reference
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
