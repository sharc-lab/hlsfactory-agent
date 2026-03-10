open_project "video_2dfilter"
set_top video_2dfilter
add_files [glob "/output/HLSPilot/video_2dfilter/**/*.cpp"]
add_files -tb [glob "/output/HLSPilot/video_2dfilter/**/*.cpp"]
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
