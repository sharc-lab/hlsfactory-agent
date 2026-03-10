set_top main
open_solution -reset
add_files "/output/S2CBench/adpcm/adpcm_encoder.cpp"
add_files "/output/S2CBench/adpcm/main.cpp"
add_files "/output/S2CBench/adpcm/tb_adpcm_encoder.cpp"
add_files -tb "/output/S2CBench/adpcm/tb_adpcm_encoder.cpp"
csynth_design
exit
