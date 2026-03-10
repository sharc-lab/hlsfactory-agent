open_project "cc"
set_top bfs
add_files [glob "/output/HLSPilot/cc/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/cc/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
