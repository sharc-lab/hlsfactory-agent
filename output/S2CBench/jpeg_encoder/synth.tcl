set_top main
open_solution -reset
add_files "/output/S2CBench/jpeg_encoder/dct.cpp"
add_files "/output/S2CBench/jpeg_encoder/huffman.cpp"
add_files "/output/S2CBench/jpeg_encoder/jpeg_encoder.cpp"
add_files "/output/S2CBench/jpeg_encoder/main.cpp"
add_files "/output/S2CBench/jpeg_encoder/quantization.cpp"
add_files "/output/S2CBench/jpeg_encoder/rle.cpp"
add_files "/output/S2CBench/jpeg_encoder/tb_jpeg_encoder.cpp"
add_files -tb "/output/S2CBench/jpeg_encoder/tb_jpeg_encoder.cpp"
csynth_design
exit
