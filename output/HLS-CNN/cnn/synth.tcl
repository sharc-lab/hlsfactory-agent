open_project /output/HLS-CNN/cnn/project
set_top cnn
add_files {
/output/HLS-CNN/cnn/activ_fun.c /output/HLS-CNN/cnn/cnn.c /output/HLS-CNN/cnn/cnn_tb.c /output/HLS-CNN/cnn/conv.c /output/HLS-CNN/cnn/dense.c /output/HLS-CNN/cnn/flat.c /output/HLS-CNN/cnn/pool.c /output/HLS-CNN/cnn/utils.c 
}
add_files -tb {
/output/HLS-CNN/cnn/cnn_tb.c
}
open_solution "solution1" -flow_target vivado
set_part xcu250-figd2104-2L-e
csynth_design
exit
