# HLS Synthesis Script for aes
# Generated automatically by HLSFactory

open_project -reset aes
set_top aes256_encrypt_ecb
add_files aes_kernel.c
add_files -tb tb_aes.cpp

open_solution -reset "solution1"
set_part {xcvu9p-flga2104-2-i}
create_clock -period 10 -name default

# Run synthesis
csynth_design

# Export IP
export_design -format ip_catalog -description "aes HLS design"

exit
