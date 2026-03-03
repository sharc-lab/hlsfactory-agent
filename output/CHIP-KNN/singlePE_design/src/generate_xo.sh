tapac \
      --work-dir run \
      --top Knn \
      --part-num xcu280-fsvh2892-2L-e \
      --clock-period 4.44 \
      -o knn.xo \
      --enable-synth-util \
      --max-parallel-synth-jobs 12 \
      --run-tapacc \
      --run-hls \
      --generate-task-rtl \
      --run-floorplanning \
      --constraint knn_floorplan.tcl \
      --generate-top-rtl \
      --pack-xo \
      --enable-hbm-binding-adjustment \
      --connectivity ../src/knn.ini \
      --read-only-args in_.* \
      --write-only-args final_out\
      ../src/knn.cpp
