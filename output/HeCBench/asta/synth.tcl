open_project synth
set_top PTTWAC_soa_asta
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
