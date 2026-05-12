open_project cholesky
set_top /output/S2CBench/cholesky/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/cholesky/cholesky.cpp
add_files /output/S2CBench/cholesky/main.cpp
add_files /output/S2CBench/cholesky/tb_cholesky.cpp
add_files -tb /output/S2CBench/cholesky/tb_cholesky.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
