open_project project
set_top matrix_mul
add_files {
  matrix_mul.cpp
  matrix_mul_test.cpp
}
add_files -tb {
  matrix_mul_test.cpp
}
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
