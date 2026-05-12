open_project hostside_bitblender_dataprep
set_top hostside_bitblender_dataprep
add_files {/output/BitBlender/hostside_bitblender_dataprep/hostside_bitblender_dataprep.cpp}
add_files -tb {/output/BitBlender/hostside_bitblender_dataprep/hostside_bitblender_dataprep_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
