open_project oclHelper
set_top oclHelper
add_files {/output/BitBlender/oclHelper/oclHelper.cpp}
add_files -tb {/output/BitBlender/oclHelper/oclHelper_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
