open_project project
set_top spmv
add_files {
  spmv.cpp
  spmv_test.cpp
}
add_files -tb {
  spmv_test.cpp
}
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
