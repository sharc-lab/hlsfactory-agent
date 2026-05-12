# CNN HLS Design

This design implements a simple Convolutional Neural Network (CNN) for digit classification.
The top‑level function is `void cnn(float img_in[IMG_ROWS][IMG_COLS], float prediction[DIGITS])`.

Source files:
- `cnn.c`, `cnn.h` – top‑level function and orchestration.
- Supporting layers: `conv.c/h`, `pool.c/h`, `flat.c/h`, `dense.c/h`, `activ_fun.c/h`, `utils.c/h`.
- Header definitions in `03-Headers`.
- Testbench: `cnn_tb.c`.

The design targets Vitis HLS and can be synthesized using the provided TCL scripts.
