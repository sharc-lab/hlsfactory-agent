open_project "prefix_sum"
set_top prefix_sum
add_files [glob "/output/HLSPilot/prefix_sum/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/prefix_sum/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
