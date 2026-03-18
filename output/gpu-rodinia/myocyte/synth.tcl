open_project myocyte
set_top if
if
if
if
if
if
if
if
if
if
if
if
if
if
if
add_files [glob *.cpp *.c *.cc]
add_files -tb myocyte_tb.cpp
open_solution "solution1"
set_part "xcu250-figd2104-2L-e"
create_clock -period 10 -name default
csynth_design
exit
