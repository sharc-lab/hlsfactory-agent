open_project k_recurrence_relations_proj
set_top k_recurrence_relations
add_files k_recurrence_relations.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
create_clock -period 3.33 -name default
csynth_design
exit
