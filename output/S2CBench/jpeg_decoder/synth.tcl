open_project /output/S2CBench/jpeg_decoder/project
set_top decoder
add_files -tb /output/S2CBench/jpeg_decoder/tb/*.cpp
add_files /output/S2CBench/jpeg_decoder/src/bmp.h
add_files /output/S2CBench/jpeg_decoder/src/common.h
add_files /output/S2CBench/jpeg_decoder/src/config_tb.h
add_files /output/S2CBench/jpeg_decoder/src/decoder.cpp
add_files /output/S2CBench/jpeg_decoder/src/decoder.h
add_files /output/S2CBench/jpeg_decoder/src/jpeg_decoder.cpp
add_files /output/S2CBench/jpeg_decoder/src/jpeg_decoder.h
add_files /output/S2CBench/jpeg_decoder/src/main.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
