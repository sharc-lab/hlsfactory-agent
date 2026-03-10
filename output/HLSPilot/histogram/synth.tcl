open_project "histogram"
set_top histogram
add_files [glob "/output/HLSPilot/histogram/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/histogram/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
