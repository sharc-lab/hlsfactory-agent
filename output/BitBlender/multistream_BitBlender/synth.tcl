open_project multistream_BitBlender
set_top multistream_BitBlender
add_files {/output/BitBlender/multistream_BitBlender/multistream_BitBlender.cpp}
add_files -tb {/output/BitBlender/multistream_BitBlender/multistream_BitBlender_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
