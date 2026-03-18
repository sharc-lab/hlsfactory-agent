open_project synth
set_top secp256k1_fe_from_storage
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
