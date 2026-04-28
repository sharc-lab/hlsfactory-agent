open_project proj
set_top sobel_y
set_part xcu250-figd2104-2L-e
add_files {
sobel_subdimy.cpp
sobel_test_subdimy.cpp
}
add_files -tb {
sobel_test_subdimy.cpp
}
open_solution "solution1"
csynth_design
exit
