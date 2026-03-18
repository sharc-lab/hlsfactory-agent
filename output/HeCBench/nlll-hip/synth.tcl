open_project synth
set_top nll_loss_forward_reduce2d_kernel
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
