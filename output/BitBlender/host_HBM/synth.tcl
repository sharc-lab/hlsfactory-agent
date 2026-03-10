open_project host_HBM
set_top host_HBM
add_files {/output/BitBlender/host_HBM/host_HBM.cpp}
add_files -tb {/output/BitBlender/host_HBM/host_HBM_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
