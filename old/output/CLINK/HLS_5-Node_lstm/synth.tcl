open_project proj
set_top lstm_n5_16s_16b
add_files {lstm_n5_16s_16b.cpp lstm_n5_16s_16b.h lstm_n5_16s_16b_mmb6.h lstm_n5_16s_16b_mlbW.h lstm_n5_16s_16b_mkbM.h lstm_n5_16s_16b_mjbC.h lstm_n5_16s_16b_mlbW.h lstm_n5_16s_16b_ifYi.h lstm_n5_16s_16b_ceOg.h lstm_n5_16s_16b_hbkb.h lstm_n5_16s_16b_ldEe.h lstm_n5_16s_16b_mkbM.h lstm_n5_16s_16b_mlbW.h}
add_files -tb {testbench.cpp}
open_solution "solution1"
set_part xcu250-figd2104-2L-e
csynth_design
exit
