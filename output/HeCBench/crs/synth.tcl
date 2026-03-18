open_project synth
set_top gcrs_m_1_w_4_coding_dotprod
add_files [glob *.cpp]
open_solution "solution1" -flow_target "vivado"
csynth_design
exit
