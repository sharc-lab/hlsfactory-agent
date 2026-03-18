open_project project
set_top void dct::jpeg_dct(void)
add_files tb_jpeg_encoder.cpp dct.cpp rle.cpp main.cpp jpeg_encoder.cpp huffman.cpp quantization.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
