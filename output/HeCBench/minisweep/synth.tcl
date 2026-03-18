open_project synth
set_top Quantities_scalefactor_space_acceldir
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
