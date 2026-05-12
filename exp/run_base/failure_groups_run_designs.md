# Failure Groups: run + designs

## TCL_SCRIPT_ORDERING_ERROR (57 failing designs across 4 runs)
- `hlsfactory-agent-UCLA-VAST__HP-FFT-HLS` (16): `n1024_UF1`, `n1024_UF16`, `n1024_UF2`, `n1024_UF32`, `n1024_UF4`, `n1024_UF8`, `n1024_no_StagePipeline`, `n1024_original_C_style`, `n256_UF1`, `n256_UF16`, `n256_UF2`, `n256_UF32`, `n256_UF4`, `n256_UF8`, `n256_no_StagePipeline`, `n256_original_C_style`
- `hlsfactory-agent-Xtra-Computing__ThunderGP` (10): `ar`, `bfs`, `casair`, `casir`, `cc`, `pr`, `spmv`, `sssp`, `template`, `wcc`
- `hlsfactory-agent-robertoBosio__NN2FPGA` (30): `BandwidthAdjust`, `DequantQuant`, `NHWCToStream`, `StreamToNHWC`, `StreamingAdd`, `StreamingAveragePool`, `StreamingConcat`, `StreamingConstMul`, `StreamingConv`, `StreamingDepthwiseConv`, `StreamingFusedSoftmaxMatmul`, `StreamingGlobalAveragePool`, `StreamingLUT`, `StreamingMaxPool`, `StreamingMemory`, `StreamingMul`, `StreamingPad`, `StreamingReLU`, `StreamingSoftmax`, `StreamingSplit`, `StreamingUpsample`, `StreamingWindowBuffer`, `StreamingWindowSelector`, `TensorDuplicator`, `YoloAttention_Attention`, `YoloAttention_QKMatMul`, `YoloAttention_ReshapeV`, `YoloAttention_SplitReshapeQKV`, `YoloAttention_Transpose`, `YoloAttention_VPMatMul`
- `hlsfactory-agent-spcl__gemm_hls` (1): `matrix_multiplication`
  - Top signatures: `set_part before open_solution` x57

## TCL_SCRIPT_API_MISUSE (39 failing designs across 2 runs)
- `hlsfactory-agent-AlexMontgomerie__fpgaconvnet-hls` (26): `accum`, `avg_pool`, `avg_pooling_layer`, `batch_norm_layer`, `bias`, `conv`, `convolution_layer`, `elementwise_add`, `elementwise_add_layer`, `elementwise_mul`, `elementwise_mul_layer`, `fork`, `global_pool`, `global_pooling_layer`, `glue`, `inner_product_layer`, `mem_read`, `mem_write`, `pool`, `pooling_layer`, `relu`, `relu_layer`, `sliding_window`, `split_layer`, `squeeze`, `squeeze_layer`
- `hlsfactory-agent-KastnerRG__Spector-HLS` (13): `dct_float`, `dct_int`, `fir_filter`, `histogram`, `matrix_mul_float`, `matrix_mul_int`, `normals_float`, `normals_int`, `sobel_subdimx`, `sobel_subdimy`, `spmv_float`, `spmv_int`, `template_matching`
  - Top signatures: `add_files misuse (positional args or repeated -tb)` x39

## TIMEOUT (30 failing designs across 4 runs)
- `hlsfactory-agent-SFU-HiAccel__SyncNN` (1): `VGG13`
- `hlsfactory-agent-SFU-HiAccel__pasta` (3): `kernel0`, `kernel3`, `serpens`
- `hlsfactory-agent-UIUC-ChenLab__ScaleHLS-HIDA` (5): `mobilenet`, `resnet18`, `vgg16`, `yolo`, `zfnet`
- `hlsfactory-agent-ZongyueQin__ProgSG` (21): `atax`, `atax-medium`, `bicg`, `bicg-large`, `bicg-medium`, `correlation`, `covariance`, `doitgen`, `doitgen-red`, `gemver`, `gemver-medium`, `gesummv`, `gesummv-medium`, `heat-3d`, `md`, `mvt`, `mvt-medium`, `nw`, `spmv-crs`, `spmv-ellpack`, `stencil-3d`
  - Top signatures: `execution timeout` x30

## MISSING_INCLUDE_OR_SOURCE_FILE (25 failing designs across 2 runs)
- `hlsfactory-agent-SFU-HiAccel__pasta` (1): `spmv`
- `hlsfactory-agent-TurakhiaLab__DP-HLS` (24): `banding_global_linear`, `banding_global_two_piece_affine`, `banding_local_affine`, `banding_local_affine_scored`, `dtw`, `global_affine`, `global_affine_int`, `global_linear`, `global_two_piece_affine`, `local_affine`, `local_linear`, `local_linear_notb`, `local_linear_shifting`, `local_two_piece_affine`, `overlap_linear_prefix_suffix`, `overlap_linear_suffix_prefix`, `profile_alignment`, `protein_local_affine`, `sdtw`, `semi_global_linear_long_short`, `semi_global_linear_short_long`, `sw_software_tiling`, `sw_tiling`, `viterbi`
  - Top signatures: `'../include/pe.h' file not found (pe.cpp:5:10)` x24; `'common.h' file not found (spmv.cpp:5:10)` x1

## PRAGMA_CONFLICT (22 failing designs across 1 runs)
- `hlsfactory-agent-ZongyueQin__ProgSG` (22): `2mm`, `3mm`, `adi`, `fdtd-2d`, `fdtd-2d-large`, `gemm-blocked`, `gemm-blocked-large`, `gemm-ncubed`, `gemm-ncubed-large`, `gemm-p`, `gemm-p-large`, `jacobi-2d`, `seidel-2d`, `stencil-large`, `stencil_stencil2d`, `symm`, `symm-opt`, `symm-opt-medium`, `syr2k`, `syrk`, `trmm`, `trmm-opt`
  - Top signatures: `in 'vitis_loop_38_4', pragma conflict happens on 'unroll' and 'pipeline' pragmas: complete unroll will break the target of other loop pragmas (2mm.cpp:38:19)` x1; `in 'vitis_loop_60_7', pragma conflict happens on 'unroll' and 'pipeline' pragmas: complete unroll will break the target of other loop pragmas (3mm.cpp:60:19)` x1; `in 'vitis_loop_43_1', pragma conflict happens on 'unroll' and 'pipeline' pragmas: complete unroll will break the target of other loop pragmas (adi.cpp:43:19)` x1

## UNKNOWN_NONZERO_RETURN (13 failing designs across 1 runs)
- `hlsfactory-agent-DARClab-UTD__S2CBench` (13): `ann`, `decimation`, `disparity`, `idct`, `interpolation`, `jpeg_decoder`, `jpeg_encoder`, `kasumi`, `md5c`, `snow3g`, `sobel`, `uart`, `vga`
  - Top signatures: `return_code=1 without parseable HLS error line` x13

## C_CPP_COMPILE_ERROR (6 failing designs across 2 runs)
- `hlsfactory-agent-DARClab-UTD__S2CBench` (4): `adpcm`, `aes_cipher`, `aes_combined`, `aes_invcipher`
- `hlsfactory-agent-SFU-HiAccel__SyncNN` (2): `LeNet`, `NiN`
  - Top signatures: `unknown type name 'aes' (./aes.h:24:1)` x2; `unknown type name 'aes' (./aes.h:22:1)` x1; `conditional expression is ambiguous; 'typename ap_int_base<33, true>::rtype<33, true>::arg1' (aka 'ap_int<33>') can be converted to 'typename ap_int_base<17, false>::rtype<17, false>::arg1' (aka 'ap_uint<17>') and vice versa (adpcm.cpp:20:20)` x1

## UNSUPPORTED_HLS_CONSTRUCT (3 failing designs across 2 runs)
- `hlsfactory-agent-DARClab-UTD__S2CBench` (1): `qsort`
- `hlsfactory-agent-ETHZ-DYNAMO__balor` (2): `aes_expandEncKey`, `aes_shiftRows`
  - Top signatures: `recursive function calls are not supported: qs(ap_uint<8>*, int, int) -> qs(ap_uint<8>*, int, int) -> qs(ap_uint<8>*, int, int) -> qs(ap_uint<8>*, int, int)` x1; `address computation on scalar port 'k' is not supported (aes_expandenckey.cpp:47:0)` x1; `address computation on scalar port 'buf_r' is not supported (aes_shiftrows.cpp:18:0)` x1

## TARGET_PART_NOT_INSTALLED (2 failing designs across 1 runs)
- `hlsfactory-agent-SFU-HiAccel__CHIP-KNN` (2): `knn_single_pe`, `knn_single_pe_euclidean`
  - Top signatures: `part 'xcu280-fsvh2892-2l-e' is not installed.` x2

## INVALID_SET_PART_OPTION (2 failing designs across 1 runs)
- `hlsfactory-agent-UCLA-VAST__CLINK` (2): `lstm_n`, `lstm_n5_16s_16b`
  - Top signatures: `set_part: unknown option '-tool'.` x2

## NO_DESIGNS_GENERATED (5 runs)
- `hlsfactory-agent-OswaldHe__InTAR`
- `hlsfactory-agent-SFU-HiAccel__AutoNTT`
- `hlsfactory-agent-SFU-HiAccel__FORC`
- `hlsfactory-agent-SFU-HiAccel__HiSpMV`
- `hlsfactory-agent-SFU-HiAccel__blaze`
