open_project project
set_top void aes::assign_key(U8 key_in[SIZE], U8 key_out[SIZE])
add_files aes.cpp tb_aes.cpp main.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
