open_project synth
set_top hessian_matrix_det
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
