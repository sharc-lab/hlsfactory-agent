open_project k_recurrence_relations_proj
set_top /output/SERI/k_recurrence_relations/k_recurrence_relations.cpp:k_recurrence_relations
set_part xcu250-figd2104-2L-e
add_files {/output/SERI/k_recurrence_relations/*.cpp}
add_files -tb {/output/SERI/k_recurrence_relations/testbench.cpp}
open_solution "solution1"
csynth_design
exit
