open_project synth
set_top compute_haversine_distance
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
