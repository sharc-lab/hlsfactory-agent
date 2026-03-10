set_top main
open_solution -reset
add_files "/output/S2CBench/jpeg_decoder/decoder.cpp"
add_files "/output/S2CBench/jpeg_decoder/jpeg_decoder.cpp"
add_files "/output/S2CBench/jpeg_decoder/main.cpp"
add_files "/output/S2CBench/jpeg_decoder/tb_jpeg_decoder.cpp"
add_files -tb "/output/S2CBench/jpeg_decoder/tb_jpeg_decoder.cpp"
csynth_design
exit
