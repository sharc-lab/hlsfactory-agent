open_project "merge_sort"
set_top merge_arrays
add_files [glob "/output/HLSPilot/merge_sort/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/merge_sort/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
