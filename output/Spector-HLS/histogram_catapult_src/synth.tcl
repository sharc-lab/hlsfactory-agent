open_project project
set_top hist_test
add_files {
  hist_test.cpp
  histogram_hls.cpp
}
add_files -tb {
  hist_test.cpp
}
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
