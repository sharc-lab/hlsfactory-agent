open_project project
set_top void image::image_main(void){
add_files vga.cpp top_vga.cpp tb_vga.cpp main.cpp tb_top.cpp image.cpp 
open_solution solution1 -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
