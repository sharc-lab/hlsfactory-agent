open_project "pagerank"
set_top pagerank
add_files [glob "/output/HLSPilot/pagerank/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/pagerank/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
