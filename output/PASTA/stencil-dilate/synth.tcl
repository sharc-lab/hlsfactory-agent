open_project /output/pasta/stencil-dilate//output/pasta/stencil-dilate/tapa/src/unikernel.cpp:void DILATE(tapa::istream<INTERFACE_WIDTH>& s, tapa::ostream<INTERFACE_WIDTH>& y,// int useless, 
/output/pasta/stencil-dilate/tapa/src/main.cpp:void unikernel(tapa::mmap<INTERFACE_WIDTH> in_0, tapa::mmap<INTERFACE_WIDTH> out_0, //HBM 0 1_proj
set_top /output/pasta/stencil-dilate/tapa/src/unikernel.cpp:void DILATE(tapa::istream<INTERFACE_WIDTH>& s, tapa::ostream<INTERFACE_WIDTH>& y,// int useless, 
/output/pasta/stencil-dilate/tapa/src/main.cpp:void unikernel(tapa::mmap<INTERFACE_WIDTH> in_0, tapa::mmap<INTERFACE_WIDTH> out_0, //HBM 0 1
add_files -cflags "-I/workspace/stubs"
add_files "/output/pasta/stencil-dilate/tapa/src/unikernel.cpp"
add_files "/output/pasta/stencil-dilate/tapa/src/main.cpp"
open_solution "solution1" -flow_target vitis
csynth_design
exit
