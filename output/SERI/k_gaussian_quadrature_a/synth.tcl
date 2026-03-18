open_project k_gaussian_quadrature_a_proj
set_top /output/SERI/k_gaussian_quadrature_a/k_gaussian_quadrature_a.cpp:k_gaussian_quadrature_a
set_part xcu250-figd2104-2L-e
add_files {/output/SERI/k_gaussian_quadrature_a/*.cpp}
add_files -tb {/output/SERI/k_gaussian_quadrature_a/testbench.cpp}
open_solution "solution1"
csynth_design
exit
