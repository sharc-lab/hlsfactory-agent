open_project project
set_top fir_hls
add_files {
  fir_hls.cpp
  main_test.cpp
}
add_files -tb {
  main_test.cpp
}
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
