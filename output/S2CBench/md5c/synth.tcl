open_project md5c
set_top /output/S2CBench/md5c/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/md5c/main.cpp
add_files /output/S2CBench/md5c/md5c.cpp
add_files /output/S2CBench/md5c/tb_md5c.cpp
add_files -tb /output/S2CBench/md5c/tb_md5c.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
