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

std::vector<MyRect> detectObjects(
  MyImage* _img, MySize minSize, MySize maxSize, myCascade* cascade,
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

#endif

  /* iterate over the image pyramid */


  freeImage(img1);
  freeSumImage(sum1);
  freeSumImage(sqsum1);
#ifdef GPU

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


// --- from image.cu ---
/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   image.c
 *
 *  Author          :   Francesco Comaschi (f.comaschi@tue.nl)
 *
 *  Date            :   November 12, 2012
 *
 *  Function        :   Functions to manage .pgm images and integral images
 *
 *  History         :
 *      12-11-12    :   Initial version.
 *
 *
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
#include <string.h>
#include <ctype.h>


//int chartoi(const char *string)
//{
//  int i;
//  i=0;
//  while(*string)
//  {
//    // i<<3 is equivalent of multiplying by 2*2*2 or 8
//    // so i<<3 + i<<1 means multiply by 10
//    i=(i<<3) + (i<<1) + (*string - '0');
//    string++;
//
//    // Dont increment i!
//
//  }
//  return(i);
//}












// --- from main.cu ---
/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   faceDetection.cpp
 *
 *  Author          :   Francesco Comaschi (f.comaschi@tue.nl)
 *
 *  Date            :   November 12, 2012
 *
 *  Function        :   Main function for face detection
 *
 *  History         :
 *      12-11-12    :   Initial version.
 *
 *
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <chrono>

using namespace std;



// --- from rectangles.cu ---








/* draw white bounding boxes around detected faces */


// --- from stdio-wrapper.cu ---
/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   stdio-wrapper.c
 *
 *  Author          :   Francesco Comaschi (f.comaschi@tue.nl)
 *
 *  Date            :   November 12, 2012
 *
 *  Function        :   Wrapper for Microblaze implementation
 *
 *  History         :
 *      12-11-12    :   Initial version.
 *
 *
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


#ifdef microblaze
#include <filestatus.h>
#include <sysace_stdio.h>
#include <fat.h>





//char * sysace_fgets(char *buf, int bsize, SYSACE_FILE *fp)



#endif



// --- from haar.h ---
/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   haar.h
 *
 *  Author          :   Francesco Comaschi (f.comaschi@tue.nl)
 *
 *  Date            :   November 12, 2012
 *
 *  Function        :   Haar features evaluation for face detection
 *
 *  History         :
 *      12-11-12    :   Initial version.
 *
 *
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

#ifndef __HAAR_H__
#define __HAAR_H__

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define MAXLABELS 50

typedef  int sumtype;
typedef int sqsumtype;

typedef struct MyPoint
{
  int x;
  int y;
}
MyPoint;

typedef struct
{
  int width;
  int height;
}
MySize;

typedef struct
{
  int x;
  int y;
  int width;
  int height;
}
MyRect;

typedef struct myCascade
{
  // number of stages 
  int  n_stages;
  int total_nodes;
  float scale; 

  // size of the window used in the training set (20 x 20)
  MySize orig_window_size;

  int inv_window_area;

  MyIntImage sum;
  MyIntImage sqsum;

  // pointers to the corner of the actual detection window
  sqsumtype *pq0, *pq1, *pq2, *pq3;
  sumtype *p0, *p1, *p2, *p3;

} myCascade;

/* sets images for haar classifier cascade */
void setImageForCascadeClassifier( myCascade* cascade, MyIntImage* sum, MyIntImage* sqsum, 
#ifdef GPU
    int* d_rectangles_array,
#endif
    int total_nodes);

/* runs the cascade on the specified window */
int runCascadeClassifier( myCascade* cascade, MyPoint pt, int start_stage);

int readTextClassifier(const char* info_file, const char* class_file);
void releaseTextClassifier();

void groupRectangles(std::vector<MyRect>& _vec, int groupThreshold, float eps);

/* draw white bounding boxes around detected faces */
void drawRectangle(MyImage* image, MyRect r);

std::vector<MyRect> detectObjects( MyImage* image, MySize minSize, MySize maxSize,
    myCascade* cascade,
    float scale_factor,
    int min_neighbors, int total_nodes);

#endif


// --- from image.h ---
/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   image.h
 *
 *  Author          :   Francesco Comaschi (f.comaschi@tue.nl)
 *
 *  Date            :   November 12, 2012
 *
 *  Function        :   Functions to manage .pgm images and integral images
 *
 *  History         :
 *      12-11-12    :   Initial version.
 *
 *
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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
  int width;
  int height;
  int maxgrey;
  unsigned char* data;
  int flag;
}
MyImage;

typedef struct 
{
  int width;
  int height;
  int* data;
  int flag;
}
MyIntImage;

int readPgm(char *fileName, MyImage* image);
int writePgm(char *fileName, MyImage* image);
int cpyPgm(MyImage *src, MyImage *dst);
void createImage(int width, int height, MyImage *image);
void createSumImage(int width, int height, MyIntImage *image);
int freeImage(MyImage* image);
int freeSumImage(MyIntImage* image);
void setImage(int width, int height, MyImage *image);
void setSumImage(int width, int height, MyIntImage *image);

#ifdef __cplusplus
}
#endif

#endif


// --- from stdio-wrapper.h ---
/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   stdio-wrapper.h
 *
 *  Author          :   Francesco Comaschi (f.comaschi@tue.nl)
 *
 *  Date            :   November 12, 2012
 *
 *  Function        :   Wrapper for Microblaze implementation
 *
 *  History         :
 *      12-11-12    :   Initial version.
 *
 *
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

#ifndef STDIO_WRAPPER_INCLUDED
#define STDIO_WRAPPER_INCLUDED

#define VERBOSE

#ifdef microblaze
# include <sysace_stdio.h>

# define  FILE	 SYSACE_FILE
# define  fopen  sysace_fopen
# define  fclose sysace_fclose
# define  fread  sysace_fread
# define  fwrite sysace_fwrite
# define  ftell  sysace_ftell
# define  fseek  sysace_fseek
# define  fgetc  sysace_fgetc
# define  putc   sysace_putc
# define  fputc   sysace_putc
# define  fgets  sysace_fgets
# define  fputs	 sysace_fputs
# define  feof	 sysace_feof
# define  printf  xil_printf
# define  fprintf  xil_printf

int sysace_fgetc(SYSACE_FILE *stream);
int sysace_putc(int c, SYSACE_FILE *stream);
char * sysace_fgets(char *buf, int bsize, SYSACE_FILE *fp);
int sysace_fputs(const char *s, SYSACE_FILE *iop);
int sysace_feof(SYSACE_FILE *stream);
long sysace_ftell(SYSACE_FILE *stream );
int sysace_fseek(SYSACE_FILE *stream, long offset, int whence );

#else
#include <stdio.h>
# define SYSACE_FILE   FILE
# define sysace_fopen  fopen
# define sysace_fclose fclose
# define sysace_fread  fread
# define sysace_fwrite fwrite
# define sysace_ftell  ftell
# define sysace_fseek  fseek
# define sysace_fgetc  fgetc
# define sysace_putc   putc
# define sysace_fgets  fgets
# define sysace_fputs  fputs
# define sysace_feof   feof
# define xil_printf    printf
#endif

#endif /* STDIO_WRAPPER_INCLUDED */

