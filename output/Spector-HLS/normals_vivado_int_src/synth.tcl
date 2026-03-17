open_project project
set_top normals
add_files {
  normals.cpp
  normals_test.cpp
}
add_files -tb {
  normals_test.cpp
}
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
