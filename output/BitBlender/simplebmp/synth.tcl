open_project simplebmp
set_top simplebmp
add_files {/output/BitBlender/simplebmp/simplebmp.cpp}
add_files -tb {/output/BitBlender/simplebmp/simplebmp_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
