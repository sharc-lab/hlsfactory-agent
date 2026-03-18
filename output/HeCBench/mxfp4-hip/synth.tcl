open_project synth
set_top fp16_to_fp4_simulate
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
