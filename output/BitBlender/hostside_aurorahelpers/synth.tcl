open_project hostside_aurorahelpers
set_top hostside_aurorahelpers
add_files {/output/BitBlender/hostside_aurorahelpers/hostside_aurorahelpers.cpp}
add_files -tb {/output/BitBlender/hostside_aurorahelpers/hostside_aurorahelpers_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
