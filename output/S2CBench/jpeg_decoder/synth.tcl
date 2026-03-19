open_project jpeg_decoder
set_top /output/S2CBench/jpeg_decoder/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/jpeg_decoder/decoder.cpp
add_files /output/S2CBench/jpeg_decoder/jpeg_decoder.cpp
add_files /output/S2CBench/jpeg_decoder/main.cpp
add_files /output/S2CBench/jpeg_decoder/tb_jpeg_decoder.cpp
add_files -tb /output/S2CBench/jpeg_decoder/tb_jpeg_decoder.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
