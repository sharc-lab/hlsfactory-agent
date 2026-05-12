open_project proj
set_top DCT8_auto
set_part xcu250-figd2104-2L-e
add_files {
dct.cpp
dct_test.cpp
}
add_files -tb {
dct_test.cpp
}
open_solution "solution1"
csynth_design
exit
