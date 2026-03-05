# Auto-generated TCL script for mlp-spatial
set design_name "mlp-spatial"
set top_func "black_hole_int16_v16"

# Create project
open_project -reset $design_name
set_top $top_func

# Add source files
add_files "src/mlp-spatial-kernel.cpp"

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
