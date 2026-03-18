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
/* -------------------------------------------------------------------------- */
/* Rainflow cycle counting algorithm according to:                            */
/* ASTM E1049-85,                                                             */
/* Standard Practices for Cycle Counting in Fatigue Analysis,                 */
/* ASTM International. (https://doi.org/10.1520/E1049-85R17)                  */
/*                                                                            */
/* By: Carlos Souto - csouto@fe.up.pt                                         */
/* -------------------------------------------------------------------------- */
/* Extracted from ASTM E1049-85 – Rainflow Counting:                          */
/* Rules for this method are as follows: let X denote range under             */
/* consideration; Y, previous range adjacent to X; and S, starting point in   */
/* the history.                                                               */
/* (1) Read next peak or valley. If out of data, go to Step 6.                */
/* (2) If there are less than three points, go to Step 1. Form ranges X and Y */
/*     using the three most recent peaks and valleys that have not been       */
/*     discarded.                                                             */
/* (3) Compare the absolute values of ranges X and Y.                         */
/*     (a) If X < Y, go to Step 1.                                            */
/*     (b) If X >= Y, go to Step 4.                                           */
/* (4) If range Y contains the starting point S, go to Step 5; otherwise,     */
/*     count range Y as one cycle; discard the peak and valley of Y; and go   */
/*     to Step 2.                                                             */
/* (5) Count range Y as one-half cycle; discard the first point (peak or      */
/*     valley) in range Y; move the starting point to the second point in     */
/*     range Y; and go to Step 2.                                             */
/* (6) Count each range that has not been previously counted as one-half      */
/*     cycle.                                                                 */
/* -------------------------------------------------------------------------- */
/* License: BSD-3-Clause - https://opensource.org/licenses/BSD-3-Clause       */
/*                                                                            */
/* Copyright (c) 2021, Carlos Daniel Santos Souto.                            */
/* All rights reserved.                                                       */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or without         */
/* modification, are permitted provided that the following conditions are     */
/* met:                                                                       */
/*                                                                            */
/* 1. Redistributions of source code must retain the above copyright notice,  */
/*    this list of conditions and the following disclaimer.                   */
/* 2. Redistributions in binary form must reproduce the above copyright       */
/*    notice, this list of conditions and the following disclaimer in the     */
/*    documentation and/or other materials provided with the distribution.    */
/* 3. Neither the name of the copyright holder nor the names of its           */
/*    contributors may be used to endorse or promote products derived from    */
/*    this software without specific prior written permission.                */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    */
/* IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  */
/* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     */
/* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          */
/* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        */
/* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         */
/* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     */
/* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/* -------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>

/// <summary>
/// Utility function to get the local extrema (maxima and minima) of the provided 
/// stress-time history as a sequence of peaks and valleys (bad in-between data 
/// points – that are neither peaks nor valleys – are removed).
/// </summary>
/// <param name="history">The provided stress-time history.</param>
/// <returns>The obtained sequence of peaks and valleys (local extrema).</returns>

/// <summary>
/// Rainflow counting algorithm according to ASTM E1049-85.
/// </summary>
/// <param name="history">The provided stress-time history.</param>
/// <returns>The cycle counts returned as a matrix: column 0 are the counts; column 1 are the ranges; column 2 are the mean values.</returns>



