open_project project
set_top sobel_test_subdimx
add_files {
  sobel_test_subdimx.cpp
  sobel_subdimx.cpp
}
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
