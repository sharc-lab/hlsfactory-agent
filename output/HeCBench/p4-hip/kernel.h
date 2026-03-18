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

// --- from main.cu ---
/*
 * SPDX-FileCopyrightText: Copyright (c) 2021 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <hip/hip_runtime.h>
#include "params.h"
#include "reference.h"

void postprocess (
  const float * cls_input,
        float * box_input,
  const float * dir_cls_input,
  const float * anchors,
  const float * anchor_bottom_heights,
        float * bndbox_output,
        float * score_output,
        int * object_counter,
  const float min_x_range,
  const float max_x_range,
  const float min_y_range,
  const float max_y_range,
  const int feature_x_size,
  const int feature_y_size,
  const int num_anchors,
  const int num_classes,
  const int num_box_values,
  const float score_thresh,
  const float dir_offset)
{
  int loc_index = _bid_x;
  int ith_anchor = _tid_x;
  if (ith_anchor >= num_anchors) return;

  int col = loc_index % feature_x_size;
  int row = loc_index / feature_x_size;
  float x_offset = min_x_range + col * (max_x_range - min_x_range) / (feature_x_size - 1);
  float y_offset = min_y_range + row * (max_y_range - min_y_range) / (feature_y_size - 1);
  int cls_offset = loc_index * num_anchors * num_classes + ith_anchor * num_classes;
  float dev_cls[2] = {-1.f, 0.f};

  const float *scores = cls_input + cls_offset;
  float max_score = 1.f / (1.f + expf(-scores[0]));
  int cls_id = 0;
  dev_cls[0] = static_cast<float>(cls_id);
  dev_cls[1] = max_score;

}

