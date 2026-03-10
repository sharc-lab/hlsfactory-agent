open_project "dft"
set_top dft
add_files [glob "/output/HLSPilot/dft/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/dft/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
