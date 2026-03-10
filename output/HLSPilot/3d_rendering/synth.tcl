open_project "3d_rendering"
set_top check_results
add_files [glob "/output/HLSPilot/3d_rendering/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/3d_rendering/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
