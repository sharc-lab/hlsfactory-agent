open_project synth
set_top conv_depthwise2d_forward_kernel
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
