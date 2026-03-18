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

// --- from Filter.cu ---
﻿#ifdef __CUDACC__
#endif
#ifdef __HIPCC__
#include <hip/hip_runtime.h>
#endif






// --- from main.cu ---
//******************************************************************
//cuSTSG is used to reconstruct high-quality NDVI time series data(MODIS/SPOT) based on STSG
//
//This procedure cuSTSG is the source code for the first version of cuSTSG.
//This is a parallel computing code using GPU.
//
//Coded by Yang Xue
// Reference:Xue Yang, Jin Chen, Qingfeng Guan, Huan Gao, and Wei Xia.
// Enhanced Spatial-Temporal Savitzky-Golay Method for Reconstructing High-quality NDVI Time Series: Reduced Sensitivity
// to Quality Flags and Improved Computational Efficiency.Transactions on Geoscience and Remote Sensing
//******************************************************************

#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <gdal/gdal_priv.h>

using namespace std;



// --- from Filter.h ---
#pragma once

#ifndef FILTER_H
#define FILTER_H

void Short_to_Float(const short *imgNDVI, const unsigned char *imgQA, int n_X, int n_Y, int n_B, int n_Years,
float * img_NDVI, float * img_QA);

void Generate_NDVI_reference(float cosyear, int win_NDVI, const float *img_NDVI, const float *img_QA, int n_X, int n_Y, int n_B, int n_Years, 
float * reference_data, float * d_res_3, int * d_res_vec_res1);

void Compute_d_res(const float *img_NDVI, const float*img_QA, const float *reference_data,
int StartY, int TotalY, int Buffer_Up, int Buffer_Dn, int n_X, int n_Y, int n_B, int n_Years, int win, float *d_res);

void STSG_filter(const float *img_NDVI, const float *img_QA, const float *reference_data, int StartY, int TotalY, int Buffer_Up, int Buffer_Dn, int n_X, int n_Y, int n_B, int n_Years, int win, float sampcorr, int snow_address,
float * vector_out, float * d_vector_in, float * d_res, float * d_res_3, int * d_index);

#endif // !FILTER_H
