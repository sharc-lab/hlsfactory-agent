open_project "optical_flow"
set_top ReadFlowFile
add_files [glob "/output/HLSPilot/optical_flow/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/optical_flow/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
