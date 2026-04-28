open_project proj
set_top matrix_mul
set_part xcu250-figd2104-2L-e
add_files {
matrix_mul.cpp
matrix_mul_test.cpp
}
add_files -tb {
matrix_mul_test.cpp
}
open_solution "solution1"
csynth_design
exit
