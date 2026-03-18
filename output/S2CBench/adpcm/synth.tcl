open_project /output/S2CBench/adpcm/project
set_top main
add_files -tb /output/S2CBench/adpcm/tb/*.cpp
add_files /output/S2CBench/adpcm/src/adpcm_encoder.cpp
add_files /output/S2CBench/adpcm/src/adpcm_encoder.h
add_files /output/S2CBench/adpcm/src/define.h
add_files /output/S2CBench/adpcm/src/main.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
