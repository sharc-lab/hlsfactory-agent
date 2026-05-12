open_project /output/HiSpMV/automation_tool
set_top spmv
add_files /output/HiSpMV/automation_tool/base_functions.cpp
add_files -tb /output/HiSpMV/automation_tool/testbench.cpp
csynth_design
exit
