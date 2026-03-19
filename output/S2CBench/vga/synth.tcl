open_project vga
set_top /output/S2CBench/vga/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/vga/image.cpp
add_files /output/S2CBench/vga/main.cpp
add_files /output/S2CBench/vga/tb_top.cpp
add_files /output/S2CBench/vga/tb_vga.cpp
add_files /output/S2CBench/vga/top_vga.cpp
add_files /output/S2CBench/vga/vga.cpp
add_files -tb /output/S2CBench/vga/tb_top.cpp
add_files -tb /output/S2CBench/vga/tb_vga.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
