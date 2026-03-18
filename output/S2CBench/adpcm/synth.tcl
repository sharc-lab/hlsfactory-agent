open_project project
set_top void adpcm::run() {
add_files main.cpp adpcm_encoder.cpp tb_adpcm_encoder.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
