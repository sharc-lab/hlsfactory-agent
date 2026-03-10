open_project aes_shiftRows
set_top aes_shiftRows
add_files aes_shiftRows.cpp
add_files -tb aes_shiftRows_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
