open_project "fir"
set_top fir
add_files [glob "/output/HLSPilot/fir/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/fir/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
