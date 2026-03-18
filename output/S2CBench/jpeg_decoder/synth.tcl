open_project project
set_top de_quantization
add_files decoder.cpp tb_jpeg_decoder.cpp main.cpp jpeg_decoder.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
