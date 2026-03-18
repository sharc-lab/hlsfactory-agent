open_project synth
set_top intt_3_64k_modcrt
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
