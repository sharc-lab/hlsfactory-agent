open_project adpcm
set_top /output/S2CBench/adpcm/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/adpcm/adpcm_encoder.cpp
add_files /output/S2CBench/adpcm/main.cpp
add_files /output/S2CBench/adpcm/tb_adpcm_encoder.cpp
add_files -tb /output/S2CBench/adpcm/tb_adpcm_encoder.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
