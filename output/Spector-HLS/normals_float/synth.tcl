open_project proj
set_top normalized
set_part xcu250-figd2104-2L-e
add_files {
normals.cpp
normals_test.cpp
}
add_files -tb {
normals_test.cpp
}
open_solution "solution1"
csynth_design
exit
