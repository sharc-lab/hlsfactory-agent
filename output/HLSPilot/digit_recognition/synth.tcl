open_project "digit_recognition"
set_top check_results
add_files [glob "/output/HLSPilot/digit_recognition/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/digit_recognition/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
