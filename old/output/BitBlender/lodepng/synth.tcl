open_project lodepng
set_top lodepng
add_files {/output/BitBlender/lodepng/lodepng.cpp}
add_files -tb {/output/BitBlender/lodepng/lodepng_tb.cpp}
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
csynth_design
exit
