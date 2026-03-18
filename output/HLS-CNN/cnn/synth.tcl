open_project Project
set_top cnn
add_files cnn.c
add_files conv.c
add_files dense.c
add_files flat.c
add_files pool.c
add_files utils.c
add_files -tb cnn_tb.c
open_solution "solution1" -flow_target vivado
set_part {xc7a200tfbg484-1}
create_clock -period 10 -name default
source "directives.tcl"
csynth_design
exit
