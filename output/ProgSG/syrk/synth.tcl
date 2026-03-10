open_solution -reset solution1
set_top kernel_syrk
add_files syrk.c
add_files -tb syrk_tb.cpp
open_solution solution1
csynth_design
exit
