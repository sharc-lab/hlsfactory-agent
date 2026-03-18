#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
using std::min;
using std::max;

// CUDA thread/block dimension constants (adjust for your workload)
#ifndef BLOCK_DIM_X
#define BLOCK_DIM_X 256
#endif
#ifndef GRID_DIM_X
#define GRID_DIM_X 1
#endif
#ifndef BLOCK_DIM_Y
#define BLOCK_DIM_Y 1
#endif
#ifndef GRID_DIM_Y
#define GRID_DIM_Y 1
#endif
#ifndef BLOCK_DIM_Z
#define BLOCK_DIM_Z 1
#endif
#ifndef GRID_DIM_Z
#define GRID_DIM_Z 1
#endif

// --- from conv3d_s4.cu ---
  cudnnHandle_t cudnn;
  checkCUDNN(cudnnCreate(&cudnn));

  cudnnTensorDescriptor_t input_descriptor;
  checkCUDNN(cudnnCreateTensorDescriptor(&input_descriptor));
  checkCUDNN(cudnnSetTensor4dDescriptor(input_descriptor,
                                        /*format=*/CUDNN_TENSOR_NCHW,
                                        /*dataType=*/CUDNN_DATA_FLOAT,
                                        /*batch_size=*/N,
                                        /*channels=*/C,
                                        /*image_height=*/Hin,
                                        /*image_width=*/Win));

  cudnnFilterDescriptor_t kernel_descriptor;
  checkCUDNN(cudnnCreateFilterDescriptor(&kernel_descriptor));
  checkCUDNN(cudnnSetFilter4dDescriptor(kernel_descriptor,
                                        /*dataType=*/CUDNN_DATA_FLOAT,
                                        /*format=*/CUDNN_TENSOR_NCHW,
                                        /*out_channels=*/M,
                                        /*in_channels=*/C,
                                        /*kernel_height=*/K,
                                        /*kernel_width=*/K));

  cudnnConvolutionDescriptor_t convolution_descriptor;
  checkCUDNN(cudnnCreateConvolutionDescriptor(&convolution_descriptor));
  checkCUDNN(cudnnSetConvolution2dDescriptor(convolution_descriptor,
                                             /*pad_height=*/0,
                                             /*pad_width=*/0,
                                             /*vertical_stride=*/1,
                                             /*horizontal_stride=*/1,
                                             /*dilation_height=*/1,
                                             /*dilation_width=*/1,
                                             /*mode=*/CUDNN_CROSS_CORRELATION,
                                             /*computeType=*/CUDNN_DATA_FLOAT));

  int batch_size{0}, channels{0}, height{0}, width{0};
  checkCUDNN(cudnnGetConvolution2dForwardOutputDim(convolution_descriptor,
                                                   input_descriptor,
                                                   kernel_descriptor,
                                                   &batch_size,
                                                   &channels,
                                                   &height,
                                                   &width));

  #ifdef DEBUG
  std::cerr << "Output Image(NxHxWxC): " << batch_size << " x "
            << height << " x " << width << " x " << channels << std::endl;
  #endif

  cudnnTensorDescriptor_t output_descriptor;
  checkCUDNN(cudnnCreateTensorDescriptor(&output_descriptor));
  checkCUDNN(cudnnSetTensor4dDescriptor(output_descriptor,
                                        /*format=*/CUDNN_TENSOR_NCHW,
                                        /*dataType=*/CUDNN_DATA_FLOAT,
                                        /*batch_size=*/N,
                                        /*channels=*/M,
                                        /*image_height=*/Hout,
                                        /*image_width=*/Wout));

  int requestedAlgoCount = CUDNN_CONVOLUTION_FWD_ALGO_COUNT;
  int returnedAlgoCount = -1;
  cudnnConvolutionFwdAlgoPerf_t results[CUDNN_CONVOLUTION_FWD_ALGO_COUNT];

  checkCUDNN(cudnnFindConvolutionForwardAlgorithm(cudnn,
                                                  input_descriptor,
                                                  kernel_descriptor,
                                                  convolution_descriptor,
                                                  output_descriptor,
                                                  requestedAlgoCount,
                                                  &returnedAlgoCount,
                                                  results));

  #ifdef DEBUG
  std::cout << "Testing cudnnFindConvolutionForwardAlgorithm ...\n";
  #endif

  cudnnConvolutionFwdAlgo_t convolution_algorithm = results[0].algo;

  size_t workspace_bytes{0};
  checkCUDNN(cudnnGetConvolutionForwardWorkspaceSize(cudnn,
                                                     input_descriptor,
                                                     kernel_descriptor,
                                                     convolution_descriptor,
                                                     output_descriptor,
                                                     convolution_algorithm,
                                                     &workspace_bytes));
  void* d_workspace{nullptr};

  const float alpha = 1.0f, beta = 0.0f;

  start = std::chrono::steady_clock::now();

  end = std::chrono::steady_clock::now();
  time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average kernel execution time of conv3d_s4 kernel: %f (us)\n",
         (time * 1e-3f) / repeat);

  cudnnDestroyTensorDescriptor(input_descriptor);
  cudnnDestroyTensorDescriptor(output_descriptor);
  cudnnDestroyFilterDescriptor(kernel_descriptor);
  cudnnDestroyConvolutionDescriptor(convolution_descriptor);
  cudnnDestroy(cudnn);


// --- from main.cu ---
/*
  Reference
  Chapter 16 in Programming massively parallel processors,
  A hands-on approach (D. Kirk and W. Hwu)
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <iostream>

#define TILE_WIDTH 16

#define II(n,c,h,w) ((n)*C*Hin*Win+(c)*Hin*Win+(h)*Win+w)
#define WI(n,c,h,w) ((n)*C*K*K+(c)*K*K+(h)*K+w)
#define OI(n,c,h,w) ((n)*M*Hout*Wout+(c)*Hout*Wout+(h)*Wout+w)

#ifdef CUDNN_CONV
#include <cudnn.h>
#define checkCUDNN(expression)                               \
  {                                                          \
    cudnnStatus_t status = (expression);                     \
  }
#endif

template <typename T>

template<typename T>

template<typename T>

template<typename T>

// Hin = Hout-1+K; max(h+p) is Hin - 1 as max(h) = Hout-1 and max(p) = K-1
template <typename T>

template <typename T>

