open_project cmdlineparser
set_top cmdlineparser
add_files {/output/BitBlender/cmdlineparser/cmdlineparser.cpp}
add_files -tb {/output/BitBlender/cmdlineparser/cmdlineparser_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
