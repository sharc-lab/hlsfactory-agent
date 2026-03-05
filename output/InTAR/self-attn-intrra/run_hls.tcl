# Auto-generated TCL script for self-attn-intrra
set design_name "self-attn-intrra"
set top_func "send_inst_cc0"

# Create project
open_project -reset $design_name
set_top $top_func

# Add source files
add_files "src/self-attn.cpp"
add_files "src/self-attn-intrra-kernel.cpp"

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
