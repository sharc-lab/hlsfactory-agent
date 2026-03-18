open_project /output/S2CBench/jpeg_encoder/project
set_top dct
add_files -tb /output/S2CBench/jpeg_encoder/tb/*.cpp
add_files /output/S2CBench/jpeg_encoder/src/dct.cpp
add_files /output/S2CBench/jpeg_encoder/src/dct.h
add_files /output/S2CBench/jpeg_encoder/src/define.h
add_files /output/S2CBench/jpeg_encoder/src/huffman.cpp
add_files /output/S2CBench/jpeg_encoder/src/huffman.h
add_files /output/S2CBench/jpeg_encoder/src/jpeg_encoder.cpp
add_files /output/S2CBench/jpeg_encoder/src/jpeg_encoder.h
add_files /output/S2CBench/jpeg_encoder/src/main.cpp
add_files /output/S2CBench/jpeg_encoder/src/quantization.cpp
add_files /output/S2CBench/jpeg_encoder/src/quantization.h
add_files /output/S2CBench/jpeg_encoder/src/rle.cpp
add_files /output/S2CBench/jpeg_encoder/src/rle.h
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
