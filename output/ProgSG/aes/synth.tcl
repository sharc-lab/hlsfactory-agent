open_solution -reset solution1
set_top aes256_encrypt_ecb
add_files aes.c
add_files -tb aes_tb.cpp
open_solution solution1
csynth_design
exit
