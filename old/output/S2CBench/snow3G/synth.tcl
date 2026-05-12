open_project snow3G
set_top /output/S2CBench/snow3G/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/snow3G/main.cpp
add_files /output/S2CBench/snow3G/snow_3G.cpp
add_files /output/S2CBench/snow3G/tb_snow_3G.cpp
add_files -tb /output/S2CBench/snow3G/tb_snow_3G.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
