open_project synth
set_top KernelPool2DGrad
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
