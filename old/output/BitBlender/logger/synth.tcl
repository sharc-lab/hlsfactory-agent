open_project logger
set_top logger
add_files {/output/BitBlender/logger/logger.cpp}
add_files -tb {/output/BitBlender/logger/logger_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
