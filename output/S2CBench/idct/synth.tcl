open_project project
set_top void idct::jpeg_idct_islow()
add_files idct.cpp tb_idct.cpp main.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
