open_project bitmap
set_top bitmap
add_files {/output/BitBlender/bitmap/bitmap.cpp}
add_files -tb {/output/BitBlender/bitmap/bitmap_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
