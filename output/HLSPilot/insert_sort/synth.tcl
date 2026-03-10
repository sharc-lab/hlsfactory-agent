open_project "insert_sort"
set_top insert_sort
add_files [glob "/output/HLSPilot/insert_sort/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/insert_sort/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
