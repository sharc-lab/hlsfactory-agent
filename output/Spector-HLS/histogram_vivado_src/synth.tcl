open_project project
set_top main_test
add_files {
  main_test.cpp
  histogram_hls.cpp
  histogram_main.cpp
}
add_files -tb {
  main_test.cpp
}
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
