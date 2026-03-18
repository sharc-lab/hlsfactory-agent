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

// --- from main.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <random>





// --- from kernels.h ---
void zoom_in_kernel(
    const float *input_tensor,float *output_tensor,
    int input_h, int input_w, int output_h, int output_w,
    size_t pitch, int out_h_start, int out_h_end,
    int out_w_start, int out_w_end)
{
  float staging_tile[4096];

  // H -> block Y, row
  // W -> block X, col
  int out_start_h = _bid_y * BLOCK_DIM_Y;
  int out_end_h   = out_start_h + BLOCK_DIM_Y;
  int out_start_w = _bid_x * BLOCK_DIM_X;
  int out_end_w   = out_start_w + BLOCK_DIM_X;

  size_t img_start_offset = _bid_z * pitch;

  float ratio_h = (float)input_h / output_h;
  float ratio_w = (float)input_w / output_w;

  // ideally should go in unified register
  int smem_load_h_start = floorf(out_start_h * ratio_h);
  int smem_load_h_end = ceilf(out_end_h * ratio_h);
  int smem_h_load_stretch = smem_load_h_end - smem_load_h_start;

  int smem_load_w_start = floorf(out_start_w * ratio_w);
  int smem_load_w_end = ceilf(out_end_w * ratio_w);
  int smem_w_load_stretch = smem_load_w_end - smem_load_w_start;

  for (int i = _tid_y; i < smem_h_load_stretch; i+=BLOCK_DIM_Y) {
    for (int j = _tid_x; j < smem_w_load_stretch; j+=BLOCK_DIM_X) {

      if (((i+smem_load_h_start) < input_h) &&
          ((j+smem_load_w_start) < input_w)) {
        staging_tile[i * smem_w_load_stretch + j] = \
                      input_tensor[img_start_offset +
                      (smem_load_h_start + i) * input_w +
                      smem_load_w_start + j];
      } else {
        staging_tile[i * smem_w_load_stretch + j] = 0.0f;
      }
    }
  }

  int out_pixel_h = out_start_h + _tid_y;
  int out_pixel_w = out_start_w + _tid_x;

  if (out_pixel_h < output_h && out_pixel_w < output_w
      && out_pixel_h >= out_h_start && out_pixel_h < out_h_end
      && out_pixel_w >= out_w_start && out_pixel_w < out_w_end) {

    // compute pixels oh, ow span
    int start_h = floorf(out_pixel_h * ratio_h);
    int end_h = ceilf((out_pixel_h+1) * ratio_h);

    int start_w = floorf(out_pixel_w * ratio_w);
    int end_w = ceilf((out_pixel_w+1) * ratio_w);

    int del_h = end_h - start_h;
    int del_w = end_w - start_w;

    float sum_ = 0.0f;

    for (int i = 0; i < del_h; i++) {
      for (int j = 0; j < del_w; j++) {
        int smem_row = (start_h + i) - smem_load_h_start;
        int smem_col = (start_w + j) - smem_load_w_start;
        sum_ += staging_tile[smem_row * smem_w_load_stretch + smem_col];
      }
    }

    output_tensor[img_start_offset +
      ((out_pixel_h - out_h_start) * input_w) +
      (out_pixel_w - out_w_start)] = sum_ / (del_h * del_w);
  }
}

void zoom_out_kernel(
    const float *input_tensor, float *output_tensor,
    int input_h, int input_w, int output_h, int output_w,
    size_t pitch, int out_h_start, int out_h_end, int out_w_start,
    int out_w_end)
{
  float staging_tile[4096];

  // H -> block Y, row
  // W -> block X, col
  int out_start_h = _bid_y * BLOCK_DIM_Y;
  int out_end_h   = out_start_h + BLOCK_DIM_Y;
  int out_start_w = _bid_x * BLOCK_DIM_X;
  int out_end_w   = out_start_w + BLOCK_DIM_X;

  size_t img_start_offset = _bid_z * pitch;

  float ratio_h = (float)input_h / output_h;
  float ratio_w = (float)input_w / output_w;

  // ideally should go in unified register
  int smem_load_h_start = floorf(out_start_h * ratio_h);
  int smem_load_h_end = ceilf(out_end_h * ratio_h);
  int smem_h_load_stretch = smem_load_h_end - smem_load_h_start;

  int smem_load_w_start = floorf(out_start_w * ratio_w);
  int smem_load_w_end = ceilf(out_end_w * ratio_w);
  int smem_w_load_stretch = smem_load_w_end - smem_load_w_start;

  for (int i = _tid_y; i < smem_h_load_stretch; i+=BLOCK_DIM_Y) {
    for (int j = _tid_x; j < smem_w_load_stretch; j+=BLOCK_DIM_X) {

      if (((i+smem_load_h_start) < input_h) &&
          ((j+smem_load_w_start) < input_w)) {
        staging_tile[i * smem_w_load_stretch + j] = \
                      input_tensor[img_start_offset +
                      (smem_load_h_start + i)*input_w +
                      smem_load_w_start + j];
      } else {
        staging_tile[i * smem_w_load_stretch + j] = 0.0f;
      }
    }
  }

  int out_pixel_h = out_start_h + _tid_y;
  int out_pixel_w = out_start_w + _tid_x;

  if (out_pixel_h < output_h && out_pixel_w < output_w) {

    // compute pixels oh, ow span
    int start_h = floorf(out_pixel_h * ratio_h);
    int end_h = ceilf((out_pixel_h+1) * ratio_h);

    int start_w = floorf(out_pixel_w * ratio_w);
    int end_w = ceilf((out_pixel_w+1) * ratio_w);

    int del_h = end_h - start_h;
    int del_w = end_w - start_w;

    float sum_ = 0.0f;

    for (int i = 0; i < del_h; i++) {
      for (int j = 0; j < del_w; j++) {
        int smem_row = (start_h + i) - smem_load_h_start;
        int smem_col = (start_w + j) - smem_load_w_start;
        sum_ += staging_tile[smem_row * smem_w_load_stretch + smem_col];
      }
    }

    output_tensor[img_start_offset +
      ((out_pixel_h + out_h_start) * input_w) +
      (out_pixel_w + out_w_start)] = sum_ / (del_h * del_w);
  }
}

void zoom_out_edge_pad(
    float *output_tensor,
    int height, int width,
    size_t pitch, int no_padding_h_start,
    int no_padding_w_start,
    int no_padding_h_end, int no_padding_w_end) {
  // H -> block Y, row
  // W -> block X, col

  int out_pixel_h = _bid_y * BLOCK_DIM_Y + _tid_y;
  int out_pixel_w = _bid_x * BLOCK_DIM_X + _tid_x;

  size_t img_start_offset = _bid_z * pitch;

  // no_padding_h_end, no_padding_w_end --> w_cropped+wstart, same for height
  int out_location = img_start_offset + (out_pixel_h * width) + out_pixel_w;

  if (out_pixel_h < height && out_pixel_w < width) {
    if (out_pixel_h < no_padding_h_start && out_pixel_w >= no_padding_w_start
        && out_pixel_w < no_padding_w_end) {
      // top pad
      output_tensor[out_location] = output_tensor[img_start_offset +
        (no_padding_h_start * width) + out_pixel_w];
    } else if (out_pixel_h >= no_padding_h_end
        && out_pixel_w >= no_padding_w_start
        && out_pixel_w < no_padding_w_end) {
      // bottom pad
      output_tensor[out_location] = output_tensor[img_start_offset +
        ((no_padding_h_end-1) * width) + out_pixel_w];
    } else if (out_pixel_w < no_padding_w_start
        && out_pixel_h >= no_padding_h_start
        && out_pixel_h < no_padding_h_end) {
      // left pad
      output_tensor[out_location] = output_tensor[img_start_offset +
        (out_pixel_h * width) + no_padding_w_start];
    } else if (out_pixel_w >= no_padding_w_end
        && out_pixel_h >= no_padding_h_start
        && out_pixel_h < no_padding_h_end) {
      // right pad
      output_tensor[out_location] = output_tensor[img_start_offset +
        (out_pixel_h * width) + (no_padding_w_end-1)];
    } else if (out_pixel_h < no_padding_h_start
        && out_pixel_w < no_padding_w_start) {
      // top-left corner
      output_tensor[out_location] = output_tensor[img_start_offset +
        (no_padding_h_start * width) +
        no_padding_w_start];
    } else if (out_pixel_h < no_padding_h_start
        && out_pixel_w >= no_padding_w_end) {
      // top-right corner
      output_tensor[out_location] = output_tensor[img_start_offset +
        (no_padding_h_start * width) +
        (no_padding_w_end-1)];
    } else if (out_pixel_h >= no_padding_h_end
        && out_pixel_w < no_padding_w_start) {
      // bottom-left corner
      output_tensor[out_location] = output_tensor[img_start_offset +
        ((no_padding_h_end-1) * width) +
        no_padding_w_start];
    } else if (out_pixel_h >= no_padding_h_end
        && out_pixel_w >= no_padding_w_end) {
      // bottom-right corner
      output_tensor[out_location] = output_tensor[img_start_offset +
        ((no_padding_h_end-1) * width) +
        (no_padding_w_end-1)];
    }
  }
}

