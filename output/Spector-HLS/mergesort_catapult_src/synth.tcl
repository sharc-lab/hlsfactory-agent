open_project project
set_top mergesort_test
add_files {
  mergesort_test.cpp
  mergesort.cpp
}
add_files -tb {
  mergesort_test.cpp
}
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
