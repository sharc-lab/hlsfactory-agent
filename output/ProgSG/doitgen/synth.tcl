open_solution -reset solution1
set_top kernel_doitgen
add_files doitgen.c
add_files -tb doitgen_tb.cpp
open_solution solution1
csynth_design
exit
