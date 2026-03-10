set_top main
open_solution -reset
add_files "/output/S2CBench/uart/main.cpp"
add_files "/output/S2CBench/uart/tb_uart.cpp"
add_files "/output/S2CBench/uart/uart.cpp"
add_files -tb "/output/S2CBench/uart/tb_uart.cpp"
csynth_design
exit
