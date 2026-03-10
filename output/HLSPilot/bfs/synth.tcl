open_project "bfs"
set_top align
add_files [glob "/output/HLSPilot/bfs/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/bfs/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
