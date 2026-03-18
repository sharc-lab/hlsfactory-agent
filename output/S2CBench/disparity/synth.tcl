open_project project
set_top void disparity::shift_diff( sc_uint<8> image_in_r[IN_BUFFER_DEPTH][WIN_SIZE],
add_files disparity.cpp main.cpp tb_disparity.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
