open_project proj
set_top sobel_x
set_part xcu250-figd2104-2L-e
add_files {
sobel_subdimx.cpp
sobel_test_subdimx.cpp
}
add_files -tb {
sobel_test_subdimx.cpp
}
open_solution "solution1"
csynth_design
exit
