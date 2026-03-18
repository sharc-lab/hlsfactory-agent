open_project k_gaussian_quadrature_b_proj
set_top /output/SERI/k_gaussian_quadrature_b/k_gaussian_quadrature_b.cpp:k_gaussian_quadrature_b
set_part xcu250-figd2104-2L-e
add_files {/output/SERI/k_gaussian_quadrature_b/*.cpp}
add_files -tb {/output/SERI/k_gaussian_quadrature_b/testbench.cpp}
open_solution "solution1"
csynth_design
exit
