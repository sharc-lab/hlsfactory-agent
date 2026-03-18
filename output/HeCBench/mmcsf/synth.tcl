open_project synth
set_top mttkrp_MIHCSR_kernel_slc_atomic_fbrLvlPar
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
