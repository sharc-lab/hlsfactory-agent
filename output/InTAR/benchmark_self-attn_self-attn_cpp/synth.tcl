set_top matMul
set_part xcu250-figd2104-2L-e
add_files /output/InTAR/benchmark_self-attn_self-attn_cpp/self-attn.cpp
add_files -tb /output/InTAR/benchmark_self-attn_self-attn_cpp/self-attn-intrra-host.cpp
open_solution solution1
csynth_design
exit
