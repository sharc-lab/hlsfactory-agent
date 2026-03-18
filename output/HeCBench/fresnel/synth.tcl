open_project synth
set_top xFresnel_Auxiliary_Cosine_Integral
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
