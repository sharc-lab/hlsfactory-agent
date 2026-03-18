open_project proj
set_top sub_sum
set_part xcu250-figd2104-2L-e
add_files {
spmv.cpp
spmv_test.cpp
}
add_files -tb {
spmv_test.cpp
}
open_solution "solution1"
csynth_design
exit
