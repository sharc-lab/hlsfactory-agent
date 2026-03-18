open_project VGG13_proj
set_top 
add_files {src.cpp}
add_files -tb {tb.cpp}
open_solution "solution1" -flow_target vivado
set_part {xcu250-figd2104-2L-e}
csynth_design
exit
