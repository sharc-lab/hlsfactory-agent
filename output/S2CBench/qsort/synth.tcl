set_top main
open_solution -reset
add_files "/output/S2CBench/qsort/main.cpp"
add_files "/output/S2CBench/qsort/qsort.cpp"
add_files "/output/S2CBench/qsort/tb_qsort.cpp"
add_files -tb "/output/S2CBench/qsort/tb_qsort.cpp"
csynth_design
exit
