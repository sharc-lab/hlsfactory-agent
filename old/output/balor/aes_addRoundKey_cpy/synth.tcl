open_project aes_addRoundKey_cpy
set_top aes_addRoundKey_cpy
add_files aes_addRoundKey_cpy.cpp
add_files -tb aes_addRoundKey_cpy_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
