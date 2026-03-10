open_project "face_detection"
set_top main
add_files [glob "/output/HLSPilot/face_detection/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/face_detection/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
