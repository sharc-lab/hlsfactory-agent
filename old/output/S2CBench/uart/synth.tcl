open_project uart
set_top /output/S2CBench/uart/main.cpp:int sc_main (int argc, char* argv[])
add_files /output/S2CBench/uart/main.cpp
add_files /output/S2CBench/uart/tb_uart.cpp
add_files /output/S2CBench/uart/uart.cpp
add_files -tb /output/S2CBench/uart/tb_uart.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
