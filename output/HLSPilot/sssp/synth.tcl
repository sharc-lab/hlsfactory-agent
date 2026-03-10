open_project "sssp"
set_top sssp
add_files [glob "/output/HLSPilot/sssp/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/sssp/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
