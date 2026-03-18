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

// --- from haar.cu ---
/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  If not, see <http://www.gnu.org/licenses/>
 *
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */

#include <stdio.h>
#include <assert.h>
#include <hip/hip_runtime.h>
#include "haar.h"
#include "image.h"
#include "stdio-wrapper.h"

/* TODO: use matrices */
/* classifier parameters */
/************************************
 * Notes:
 * To paralleism the filter,
 * these monolithic arrays may
 * need to be splitted or duplicated
 ***********************************/
static int *stages_array;
static int *rectangles_array;
static int *weights_array;
static int *alpha1_array;
static int *alpha2_array;
static int *tree_thresh_array;
static int *stages_thresh_array;
static int **scaled_rectangles_array;

int clock_counter = 0;
float n_features = 0;

int iter_counter = 0;

/* compute integral images */
void integralImages( MyImage *src, MyIntImage *sum, MyIntImage *sqsum );

/* scale down the image */
void ScaleImage_Invoker( myCascade* _cascade, float _factor, int sum_row, int sum_col, std::vector<MyRect>& _vec);

/* compute scaled image */

/*******************************************************
 * Function: detectObjects
 * Description: It calls all the major steps
 ******************************************************/

std::vector<MyRect> detectObjects( MyImage* _img, MySize minSize, MySize maxSize, myCascade* cascade,
    float scaleFactor, int minNeighbors, int total_nodes)
{
  /* group overlaping windows */
  const float GROUP_EPS = 0.4f;
  /* pointer to input image */
  MyImage *img = _img;
  /***********************************
   * create structs for images
   * see haar.h for details 
   * img1: normal image (unsigned char)
   * sum1: integral image (int)
   * sqsum1: square integral image (int)
   **********************************/
  MyImage image1Obj;
  MyIntImage sum1Obj;
  MyIntImage sqsum1Obj;
  /* pointers for the created structs */
  MyImage *img1 = &image1Obj;
  MyIntImage *sum1 = &sum1Obj;
  MyIntImage *sqsum1 = &sqsum1Obj;

  /********************************************************
   * allCandidates is the preliminaray face candidate,
   * which will be refined later.
   *
   * std::vector is a sequential container 
   * http://en.wikipedia.org/wiki/Sequence_container_(C++) 
   *
   * Each element of the std::vector is a "MyRect" struct 
   * MyRect struct keeps the info of a rectangle (see haar.h)
   * The rectangle contains one face candidate 
   *****************************************************/
  std::vector<MyRect> allCandidates;

  /* scaling factor */
  float factor;

  /* maxSize */

  /* window size of the training set */
  MySize winSize0 = cascade->orig_window_size;

  /* malloc for img1: unsigned char */
  createImage(img->width, img->height, img1);
  /* malloc for sum1: unsigned char */
  createSumImage(img->width, img->height, sum1);
  /* malloc for sqsum1: unsigned char */
  createSumImage(img->width, img->height, sqsum1);

  /* initial scaling factor */
  factor = 1;

#ifdef GPU
  int *d_rectangles_array;
  hipMalloc((void**)&d_rectangles_array, sizeof(int)*total_nodes*12);
  hipMemcpy(d_rectangles_array, rectangles_array, sizeof(int)*total_nodes*12, hipMemcpyHostToDevice);
#endif

  /* iterate over the image pyramid */


  freeImage(img1);
  freeSumImage(sum1);
  freeSumImage(sqsum1);
#ifdef GPU
  hipFree(d_rectangles_array);
#endif
  return allCandidates;
}

/***********************************************
 * Note:
 * The int_sqrt is softwar integer squre root.

#ifdef GPU
#endif


/****************************************************
 * evalWeakClassifier:
 * the actual computation of a haar filter.
 * More info:
 * http://en.wikipedia.org/wiki/Haar-like_features
 ***************************************************/



/*****************************************************

/***********************************************************
 * This function downsample an image using nearest neighbor
 * It is used to build the image pyramid
 **********************************************************/


/* End of file. */
