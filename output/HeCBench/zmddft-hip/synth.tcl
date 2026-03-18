open_project synth
set_top ker_zmddft_fwd_256x256x256_cu0
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
