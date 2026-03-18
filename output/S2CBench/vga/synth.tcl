open_project /output/S2CBench/vga/project
set_top vga
add_files -tb /output/S2CBench/vga/tb/*.cpp
add_files /output/S2CBench/vga/src/define.h
add_files /output/S2CBench/vga/src/image.cpp
add_files /output/S2CBench/vga/src/image.h
add_files /output/S2CBench/vga/src/main.cpp
add_files /output/S2CBench/vga/src/top_vga.cpp
add_files /output/S2CBench/vga/src/top_vga.h
add_files /output/S2CBench/vga/src/vga.cpp
add_files /output/S2CBench/vga/src/vga.h
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
