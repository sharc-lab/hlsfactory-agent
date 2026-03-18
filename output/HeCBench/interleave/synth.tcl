open_project synth
set_top add_kernel_interleaved
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
