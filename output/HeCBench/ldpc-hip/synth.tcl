open_project synth
set_top ldpc_cnp_kernel_1st_iter
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
