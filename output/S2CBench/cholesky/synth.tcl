set_top main
open_solution -reset
add_files "/output/S2CBench/cholesky/cholesky.cpp"
add_files "/output/S2CBench/cholesky/main.cpp"
add_files "/output/S2CBench/cholesky/tb_cholesky.cpp"
add_files -tb "/output/S2CBench/cholesky/tb_cholesky.cpp"
csynth_design
exit
