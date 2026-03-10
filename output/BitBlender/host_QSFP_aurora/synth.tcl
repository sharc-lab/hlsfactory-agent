open_project host_QSFP_aurora
set_top host_QSFP_aurora
add_files {/output/BitBlender/host_QSFP_aurora/host_QSFP_aurora.cpp}
add_files -tb {/output/BitBlender/host_QSFP_aurora/host_QSFP_aurora_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
