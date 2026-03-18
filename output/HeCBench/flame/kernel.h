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

// --- from iteration.cu ---
/*
 * (c) 2009-2010 Christoph Schied <Christoph.Schied@uni-ulm.de>
 *
 * This file is part of flame.
 *
 * flame is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * flame is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with flame.  If not, see <http://www.gnu.org/licenses/>.
 */



#undef M_PI
#define M_PI 3.1415926535f




// --- from kernels.cu ---
/*
 * (c) 2009-2010 Christoph Schied <Christoph.Schied@uni-ulm.de>
 *
 * This file is part of flame.
 *
 * flame is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * flame is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with flame.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Each thread has a fixed assigned function and picks a random data-set by
 * indexing into the data array through a permutation array. The threads write
 * out their calculated data and read another randomly selected point by
 * indexing into another permutation array.
 *
 * This approach needs synchronization across all threads and threadblocks. To
 * achieve this, the algorithm has been split into three kernels:
 *  - initialize: initializes the iterations by selecting a random point of
 *    the set of starting points and applies one iteration
 *  - iterate: reads one point, iterates, writes out
 *  - generate points: reads iterated points and generates a bigger amount of
 *    points out of those points.
 *
 */

/* calculates the function a thread has to evaluate by using a binary search
 */

/* selects a random point out of the set of startingpoints and calculates one
 * iteration
 */

/* reads one point, applies one iteration step and writes the point out */

/* after applying n iterations, the points converged close enough. new points
 * are created out of these converged points by applying another iteration
 * step and these new points get written into the destination VBO
 */



// --- from main.cu ---
/*
 * (c) 2009-2010 Christoph Schied <Christoph.Schied@uni-ulm.de>
 *
 * This file is part of flame.
 *
 * flame is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * flame is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with flame.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/time.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include "flame.hpp"

#define VARPARM(a,b) const_mem_params.variation_parameters[a][b]
#define FUNC_COLOR(a) const_mem_params.function_colors[a]
#define FUNC_WEIGHT(a) function_weights[a]
#define AFFINE_PRE(a, b) const_mem_params.pre_transform_params[a][b]
#define AFFINE_POST(a, b) const_mem_params.post_transform_params[a][b]



// --- from utils.cu ---
/*
 * (c) 2009-2010 Christoph Schied <Christoph.Schied@uni-ulm.de>
 *
 * This file is part of flame.
 *
 * flame is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * flame is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with flame.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>
//#include <vector_types.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include "flame.hpp"




