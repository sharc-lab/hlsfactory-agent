open_project jpeg_encoder
set_top /output/S2CBench/jpeg_encoder/main.cpp:int sc_main(int argc, char* argv[]){
add_files /output/S2CBench/jpeg_encoder/dct.cpp
add_files /output/S2CBench/jpeg_encoder/huffman.cpp
add_files /output/S2CBench/jpeg_encoder/jpeg_encoder.cpp
add_files /output/S2CBench/jpeg_encoder/main.cpp
add_files /output/S2CBench/jpeg_encoder/quantization.cpp
add_files /output/S2CBench/jpeg_encoder/rle.cpp
add_files /output/S2CBench/jpeg_encoder/tb_jpeg_encoder.cpp
add_files -tb /output/S2CBench/jpeg_encoder/tb_jpeg_encoder.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
