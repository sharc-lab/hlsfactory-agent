open_project qsort
set_top /output/S2CBench/qsort/main.cpp:int sc_main(int argc, char** argv)
add_files /output/S2CBench/qsort/main.cpp
add_files /output/S2CBench/qsort/qsort.cpp
add_files /output/S2CBench/qsort/tb_qsort.cpp
add_files -tb /output/S2CBench/qsort/tb_qsort.cpp
open_solution solution1 -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
