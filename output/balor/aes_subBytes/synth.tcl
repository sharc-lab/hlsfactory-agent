open_project aes_subBytes
set_top aes_subBytes
add_files aes_subBytes.cpp
add_files -tb aes_subBytes_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
