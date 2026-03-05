# Auto-generated TCL script for vae-intrra
set design_name "vae-intrra"
set top_func "CC0_Encoder_Decoder_Conv1"

# Create project
open_project -reset $design_name
set_top $top_func

# Add source files
add_files "src/vae-intrra-kernel.cpp"
add_files "src/vae.cpp"

# Add testbench
add_files -tb "src/testbench.cpp"

# Open solution
open_solution -reset "solution1"
set_part {xcu280-fsvh2892-2L-e}
create_clock -period 300MHz -name default

# Run synthesis
csynth_design

# Export design
export_design -format ip_catalog

exit
