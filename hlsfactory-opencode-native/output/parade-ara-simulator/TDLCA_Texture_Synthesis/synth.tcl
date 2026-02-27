open_project -reset TDLCA_Texture_Synthesis_proj
add_files *.cpp
add_files -tb *.h
set_top Texture_SynthesisLCacc
open_solution -reset "solution1"
set_part {xc7vx485tffg1761-2}
create_clock -period 10
csynth_design
export_design -format ip_catalog
exit
