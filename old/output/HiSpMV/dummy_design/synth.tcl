open_project /output/HiSpMV/dummy_design
set_top dummy_top
add_files /output/HiSpMV/dummy_design/dummy.cpp
add_files -tb /output/HiSpMV/dummy_design/testbench.cpp
csynth_design
exit
