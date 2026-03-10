set_top main
open_solution -reset
add_files "/output/S2CBench/md5c/main.cpp"
add_files "/output/S2CBench/md5c/md5c.cpp"
add_files "/output/S2CBench/md5c/tb_md5c.cpp"
add_files -tb "/output/S2CBench/md5c/tb_md5c.cpp"
csynth_design
exit
