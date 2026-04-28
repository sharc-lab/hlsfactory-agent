open_project ave8
set_top /output/S2CBench/ave8/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/ave8/ave8.cpp
add_files /output/S2CBench/ave8/main.cpp
add_files /output/S2CBench/ave8/tb_ave8.cpp
add_files -tb /output/S2CBench/ave8/tb_ave8.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
