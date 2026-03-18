open_project synth
set_top rmsnorm_fwd_two_scan_kernel
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
