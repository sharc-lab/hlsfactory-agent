open_project project
set_top sobel_test_subdimy
add_files {
  sobel_test_subdimy.cpp
  sobel_subdimy.cpp
}
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
