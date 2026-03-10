open_project "matrix_multiplication"
set_top matrix_mul
add_files [glob "/output/HLSPilot/matrix_multiplication/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/matrix_multiplication/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
