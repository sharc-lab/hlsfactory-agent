open_project "spam_filter"
set_top dotProduct
add_files [glob "/output/HLSPilot/spam_filter/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/spam_filter/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
