open_project "fft"
set_top fft
add_files [glob "/output/HLSPilot/fft/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/fft/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
