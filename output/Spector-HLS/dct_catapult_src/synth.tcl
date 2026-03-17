open_project project
set_top dct
add_files {
  dct.cpp
  dct_test.cpp
}
add_files -tb {
  dct_test.cpp
}
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
