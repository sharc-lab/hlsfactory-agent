open_project /output/S2CBench/ann/project
set_top layer
add_files -tb /output/S2CBench/ann/tb/*.cpp
add_files /output/S2CBench/ann/src/ann.cpp
add_files /output/S2CBench/ann/src/ann.h
add_files /output/S2CBench/ann/src/ann_tb.cpp
add_files /output/S2CBench/ann/src/ann_tb.h
add_files /output/S2CBench/ann/src/config.h
add_files /output/S2CBench/ann/src/image_parameters.h
add_files /output/S2CBench/ann/src/layer.cpp
add_files /output/S2CBench/ann/src/layer.h
add_files /output/S2CBench/ann/src/main.cpp
add_files /output/S2CBench/ann/src/synth_param.h
add_files /output/S2CBench/ann/src/train_parameters.h
open_solution "solution1" -flow_target vitis
set_part xcu250-figd2104-2L-e
csynth_design
exit
