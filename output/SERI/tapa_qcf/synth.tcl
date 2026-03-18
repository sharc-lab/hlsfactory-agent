open_project tapa_qcf_proj
set_top /output/SERI/tapa_qcf/tapa_qcf.cpp:t_preparation
set_part xcu250-figd2104-2L-e
add_files {/output/SERI/tapa_qcf/*.cpp}
add_files -tb {/output/SERI/tapa_qcf/testbench.cpp}
open_solution "solution1"
csynth_design
exit
