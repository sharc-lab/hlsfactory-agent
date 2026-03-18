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

// --- from kernels.cu ---
/**********************************************************************
  Copyright ©2013 Advanced Micro Devices, Inc. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  • Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  • Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************/

/**
 * @brief Compare two strings with specified length
 * @param text       start position on text string
 * @param pattern    start position on pattern string
 * @param length     Length to compare
 * @return 0-failure, 1-success
 */

/**
 * @brief Naive kernel version of string search.
 *        Find all pattern positions in the given text
 * @param text               Input Text
 * @param textLength         Length of the text
 * @param pattern            Pattern string
 * @param patternLength      Pattern length
 * @param resultBuffer       Result of all matched positions
 * @param resultCountPerWG   Result counts per Work-Group
 * @param maxSearchLength    Maximum search positions for each work-group
 * @param localPattern       local buffer for the search pattern
 */

/**
 * @brief Load-Balance kernel version of string search.
 *        Find all pattern positions in the given text
 * @param text               Input Text
 * @param textLength         Length of the text
 * @param pattern            Pattern string
 * @param patternLength      Pattern length
 * @param resultBuffer       Result of all matched positions
 * @param resultCountPerWG   Result counts per Work-Group
 * @param maxSearchLength    Maximum search positions for each work-group
 * @param localPattern       local buffer for the search pattern
 * @param stack1             local stack for store initial 2-byte match 
 * @param stack2             local stack for store initial 10-byte match positions
 */


// --- from main.cu ---
/**********************************************************************
  Copyright ©2013 Advanced Micro Devices, Inc. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  • Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  • Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************/

#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <climits>
#include <chrono>




// --- from StringSearch.h ---
/**********************************************************************
Copyright ©2013 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

• Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
• Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

#ifndef STRINGSEARCH_H_
#define STRINGSEARCH_H_

#define LOCAL_SIZE      256
#define COMPARE(x,y)    ((toupper(x) == toupper(y)))
#define SEARCH_BYTES_PER_WORKITEM   512
#define TOLOWER(x) (('A' <= (x) && (x) <= 'Z') ? ((x) - 'A' + 'a') : (x))

// Enable filtering when the search pattern has more than 16 characters
#define ENABLE_2ND_LEVEL_FILTER

typedef unsigned char uchar;

#endif
