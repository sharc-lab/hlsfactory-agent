open_project "spmv"
set_top spmv
add_files [glob "/output/HLSPilot/spmv/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/spmv/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
