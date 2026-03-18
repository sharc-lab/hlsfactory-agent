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
  miopenHandle_t h;
  checkMIOpen(miopenCreate(&h));

  miopenTensorDescriptor_t input_descriptor;
  checkMIOpen(miopenCreateTensorDescriptor(&input_descriptor));
  checkMIOpen(miopenSet4dTensorDescriptor(input_descriptor,
                                          /*dataType=*/miopenFloat,
                                          /*batch_size=*/N,
                                          /*channels=*/C,
                                          /*image_height=*/Hin,
                                          /*image_width=*/Win));

  miopenTensorDescriptor_t kernel_descriptor;
  checkMIOpen(miopenCreateTensorDescriptor(&kernel_descriptor));
  checkMIOpen(miopenSet4dTensorDescriptor(kernel_descriptor,
                                          /*dataType=*/miopenFloat,
                                          /*out_channels=*/M,
                                          /*in_channels=*/C,
                                          /*kernel_height=*/K,
                                          /*kernel_width=*/K));

  miopenConvolutionDescriptor_t convolution_descriptor;
  checkMIOpen(miopenCreateConvolutionDescriptor(&convolution_descriptor));
  checkMIOpen(miopenInitConvolutionDescriptor(convolution_descriptor,
                                              /*mode=*/miopenConvolution,
                                              /*pad_height=*/0,
                                              /*pad_width=*/0,
                                              /*vertical_stride=*/1,
                                              /*horizontal_stride=*/1,
                                              /*dilation_height=*/1,
                                              /*dilation_width=*/1));

  int batch_size{0}, channels{0}, height{0}, width{0};
  checkMIOpen(miopenGetConvolutionForwardOutputDim(convolution_descriptor,
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

  miopenTensorDescriptor_t output_descriptor;
  checkMIOpen(miopenCreateTensorDescriptor(&output_descriptor));
  checkMIOpen(miopenSet4dTensorDescriptor(output_descriptor,
                                          /*dataType=*/miopenFloat,
                                          /*batch_size=*/N,
                                          /*channels=*/M,
                                          /*image_height=*/Hout,
                                          /*image_width=*/Wout));

  size_t workspace_bytes{0};
  checkMIOpen(miopenConvolutionForwardGetWorkSpaceSize(h,
                                                       input_descriptor,
                                                       kernel_descriptor,
                                                       convolution_descriptor,
                                                       output_descriptor,
                                                       &workspace_bytes));
  void* d_workspace{nullptr};

  int requestedAlgoCount = 2;
  int returnedAlgoCount = -1;
  miopenConvAlgoPerf_t results[2];

  checkMIOpen(miopenFindConvolutionForwardAlgorithm(h,
                                                    input_descriptor,
                                                    dX, //d_input,
                                                    kernel_descriptor,
                                                    dW, //d_kernel,
                                                    convolution_descriptor,
                                                    output_descriptor,
                                                    dY, //d_output
                                                    requestedAlgoCount,
                                                    &returnedAlgoCount,
                                                    results,
                                                    d_workspace,
                                                    workspace_bytes,
                                                    true));

  #ifdef DEBUG
  std::cout << "Testing miopenFindConvolutionForwardAlgorithm ...\n";
  #endif

  miopenConvFwdAlgorithm_t convolution_algorithm = results[0].fwd_algo;

  const float alpha = 1.0f, beta = 0.0f;

  start = std::chrono::steady_clock::now();

  hipDeviceSynchronize();
  end = std::chrono::steady_clock::now();
  time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average kernel execution time of conv3d_s4 kernel: %f (us)\n",
         (time * 1e-3f) / repeat);

  miopenDestroyTensorDescriptor(input_descriptor);
  miopenDestroyTensorDescriptor(output_descriptor);
  miopenDestroyTensorDescriptor(kernel_descriptor);
  miopenDestroyConvolutionDescriptor(convolution_descriptor);
  miopenDestroy(h);


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
#include <hip/hip_runtime.h>

#define TILE_WIDTH 16

#define II(n,c,h,w) ((n)*C*Hin*Win+(c)*Hin*Win+(h)*Win+w)
#define WI(n,c,h,w) ((n)*C*K*K+(c)*K*K+(h)*K+w)
#define OI(n,c,h,w) ((n)*M*Hout*Wout+(c)*Hout*Wout+(h)*Wout+w)

#ifdef MIOPEN_CONV
#include <miopen/miopen.h>
#define checkMIOpen(expression)                              \
  {                                                          \
    miopenStatus_t status = (expression);                    \
  }
#endif

template <typename T>

template<typename T>

template<typename T>

template<typename T>

// Hin = Hout-1+K; max(h+p) is Hin - 1 as max(h) = Hout-1 and max(p) = K-1
template <typename T>

template <typename T>

