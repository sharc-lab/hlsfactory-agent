open_project /output/S2CBench/aes_cipher/project
set_top aes
add_files -tb /output/S2CBench/aes_cipher/tb/*.cpp
add_files /output/S2CBench/aes_cipher/src/aes.cpp
add_files /output/S2CBench/aes_cipher/src/aes.h
add_files /output/S2CBench/aes_cipher/src/define.h
add_files /output/S2CBench/aes_cipher/src/main.cpp
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
