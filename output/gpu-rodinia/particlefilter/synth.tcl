open_project particlefilter
set_top cdfCalc
d_randu
d_randn
updateWeights
findIndexBin
if
dev_round_double
find_index_kernel
normalize_weights_kernel
sum_kernel
likelihood_kernel
kernel
add_files [glob *.cpp *.c *.cc]
add_files -tb particlefilter_tb.cpp
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
create_clock -period 10 -name default
csynth_design
exit
