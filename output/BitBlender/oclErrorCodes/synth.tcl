open_project oclErrorCodes
set_top oclErrorCodes
add_files {/output/BitBlender/oclErrorCodes/oclErrorCodes.cpp}
add_files -tb {/output/BitBlender/oclErrorCodes/oclErrorCodes_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
