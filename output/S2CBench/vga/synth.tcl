set_top main
open_solution -reset
add_files "/output/S2CBench/vga/image.cpp"
add_files "/output/S2CBench/vga/main.cpp"
add_files "/output/S2CBench/vga/tb_top.cpp"
add_files "/output/S2CBench/vga/tb_vga.cpp"
add_files "/output/S2CBench/vga/top_vga.cpp"
add_files "/output/S2CBench/vga/vga.cpp"
add_files -tb "/output/S2CBench/vga/tb_top.cpp"
add_files -tb "/output/S2CBench/vga/tb_vga.cpp"
csynth_design
exit
