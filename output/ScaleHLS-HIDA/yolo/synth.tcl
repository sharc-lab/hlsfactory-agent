open_solution -flow_target vivado
set_top yolo
add_files yolo.cpp
add_files -tb yolo_tb.cpp
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
