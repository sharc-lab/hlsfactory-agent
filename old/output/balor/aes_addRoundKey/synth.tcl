open_project aes_addRoundKey
set_top aes_addRoundKey
add_files aes_addRoundKey.cpp
add_files -tb aes_addRoundKey_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
