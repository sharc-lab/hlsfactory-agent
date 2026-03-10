open_project xcl2
set_top xcl2
add_files {/output/BitBlender/xcl2/xcl2.cpp}
add_files -tb {/output/BitBlender/xcl2/xcl2_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
