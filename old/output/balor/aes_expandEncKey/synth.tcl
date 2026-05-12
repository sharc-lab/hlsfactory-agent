open_project aes_expandEncKey
set_top aes_expandEncKey
add_files aes_expandEncKey.cpp
add_files -tb aes_expandEncKey_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
