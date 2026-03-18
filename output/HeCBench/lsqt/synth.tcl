open_project synth
set_top gpu_apply_hamiltonian
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
