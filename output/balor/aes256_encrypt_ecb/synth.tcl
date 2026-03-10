open_project aes256_encrypt_ecb
set_top aes256_encrypt_ecb
add_files aes256_encrypt_ecb.cpp
add_files -tb aes256_encrypt_ecb_tb.cpp
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
