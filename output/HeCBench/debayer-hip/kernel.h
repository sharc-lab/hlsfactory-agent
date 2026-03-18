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

// --- from main.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <hip/hip_runtime.h>
#include "util.h"



// --- from image.h ---
#ifndef IMAGE_H
#define IMAGE_H

typedef unsigned char uchar;

enum pattern_t {
  RGGB = 0,
  GRBG = 1,
  GBRG = 2,
  BGGR = 3
};

enum {
  ADDRESS_CLAMP = 0, //repeat border
  ADDRESS_ZERO = 1, //returns 0
  ADDRESS_REFLECT_BORDER_EXCLUSIVE = 2, //reflects at boundary and will not duplicate boundary elements
  ADDRESS_REFLECT_BORDER_INCLUSIVE = 3, //reflects at boundary and will duplicate boundary elements,
  ADDRESS_NOOP = 4 //programmer guarantees no reflection necessary
};

//coordinate is c, r for compatibility with climage and CUDA
INLINE uint2 tex2D(const int rows, const int cols, const int _c, const int _r,
            const uint sample_method) {
  int c = _c;
  int r = _r;
  if (sample_method == ADDRESS_REFLECT_BORDER_EXCLUSIVE) {
    c = c < 0 ? -c : c;
    c = c >= cols ? cols - (c - cols) - 2 : c;
    r = r < 0 ? -r : r;
    r = r >= rows ? rows - (r - rows) - 2 : r;
  } else if (sample_method == ADDRESS_CLAMP) {
    c = c < 0 ? 0 : c;
    c = c > cols - 1 ? cols - 1 : c;
    r = r < 0 ? 0 : r;
    r = r > rows - 1 ? rows - 1 : r;
  } else if (sample_method == ADDRESS_REFLECT_BORDER_INCLUSIVE) {
    c = c < 0 ? -c - 1 : c;
    c = c >= cols ? cols - (c - cols) - 1 : c;
    r = r < 0 ? -r - 1 : r;
    r = r >= rows ? rows - (r - rows) - 1 : r;
  } else if (sample_method == ADDRESS_ZERO) {
  } else if (sample_method == ADDRESS_NOOP) {
  } else {
    assert(false);
  }
  assert_val(r >= 0 && r < rows, r);
  assert_val(c >= 0 && c < cols, c);
  uint2 result; 
  result.x = r;
  result.y = c;
  return result;
}

INLINE uchar* image_line_at_(uchar *im_p, const uint im_rows, const uint im_cols, const uint image_pitch_p, const uint r) {
  assert_val(r >= 0 && r < im_rows, r);
  (void) im_cols;
  return im_p + r * image_pitch_p;
}
#define image_line_at(PixelT, im_p, im_rows, im_cols, image_pitch, r) ((PixelT *) image_line_at_((uchar *) (im_p), (im_rows), (im_cols), (image_pitch), (r)))

INLINE uchar* image_pixel_at_(uchar *im_p, const uint im_rows, const uint im_cols, const uint image_pitch_p, 
                       const uint r, const uint c, const uint sizeof_pixel) {
  assert_val(r >= 0 && r < im_rows, r);
  assert_val(c >= 0 && c < im_cols, c);
  return im_p + r * image_pitch_p + c * sizeof_pixel;
}
#define image_pixel_at(PixelT, im_p, im_rows, im_cols, image_pitch, r, c) (*((PixelT *) image_pixel_at_((uchar *)(im_p), (im_rows), (im_cols), (image_pitch), (r), (c), sizeof(PixelT))))

INLINE uchar* image_tex2D_(uchar *im_p, const uint im_rows, const uint im_cols, const uint image_pitch, 
                    const int r, const int c, const uint sizeof_pixel, const uint sample_method) {
  const uint2 p2 = tex2D((int) im_rows, (int) im_cols, c, r, sample_method);
  return image_pixel_at_(im_p, im_rows, im_cols, image_pitch, p2.x, p2.y, sizeof_pixel);
}

#define image_tex2D(PixelT, im_p, im_rows, im_cols, image_pitch, r, c, sample_method) \
  (((sample_method) == ADDRESS_ZERO) & (((r) < 0) | ((r) >= (im_rows)) | ((c) < 0) | ((c) >= (im_cols))) ? 0 : \
   *(PixelT *) image_tex2D_((uchar *)(im_p), (im_rows), (im_cols), (image_pitch), (r), (c), sizeof(PixelT), (sample_method)))

#ifndef OUTPUT_CHANNELS
#define OUTPUT_CHANNELS 3
#endif

#ifndef ALPHA_VALUE
#define ALPHA_VALUE UCHAR_MAX
#endif

#ifndef PIXELT
#define PIXELT uchar
#endif

#ifndef RGBPIXELBASET
#define RGBPIXELBASET PIXELT
#endif

#ifndef RGBPIXELT
#define RGBPIXELT PASTE(RGBPIXELBASET, OUTPUT_CHANNELS)
#endif
#ifndef LDSPIXELT
#define LDSPIXELT int
#endif

typedef PIXELT PixelT;
typedef RGBPIXELBASET RGBPixelBaseT;
typedef RGBPIXELT RGBPixelT;
typedef LDSPIXELT LDSPixelT;// for LDS's, having this large enough to prevent bank conflicts make's a large difference

#define kernel_size 5
#define tile_rows 5
#define tile_cols 32
#define apron_rows (tile_rows + kernel_size - 1)
#define apron_cols (tile_cols + kernel_size - 1)

#define half_ksize  (kernel_size/2)
#define shalf_ksize ((int) half_ksize)
#define half_ksize_rem (kernel_size - half_ksize)
#define n_apron_fill_tasks (apron_rows * apron_cols)
#define n_tile_pixels  (tile_rows * tile_cols)

#define pixel_at(type, basename, r, c) image_pixel_at(type, PASTE_2(basename, _p), height, width, PASTE_2(basename, _pitch), (r), (c))
#define tex2D_at(type, basename, r, c) image_tex2D(type, PASTE_2(basename, _p), height, width, PASTE_2(basename, _pitch), (r), (c), ADDRESS_REFLECT_BORDER_EXCLUSIVE)
#define apron_pixel(_t_r, _t_c) apron[(_t_r) * apron_cols + (_t_c)]

#define output_pixel_cast(x) PASTE3(convert_,RGBPIXELBASET,_sat)((x))

#endif


// --- from kernel.h ---
void malvar_he_cutler_demosaic (
  const uint height,
  const uint width,
  const uchar * input_image_p,
  const uint input_image_pitch, 
        uchar * output_image_p,
  const uint output_image_pitch, 
  const int bayer_pattern )
{
  LDSPixelT apron[apron_rows * apron_cols];

  const uint tile_col_blocksize = BLOCK_DIM_X;
  const uint tile_row_blocksize = BLOCK_DIM_Y;
  const uint tile_col_block = _bid_x;
  const uint tile_row_block = _bid_y;
  const uint tile_col = _tid_x;
  const uint tile_row = _tid_y;
  const uint g_c = BLOCK_DIM_X * _bid_x + _tid_x; 
  const uint g_r = BLOCK_DIM_Y * _bid_y + _tid_y;
  const bool valid_pixel_task = (g_r < height) & (g_c < width);

  const uint tile_flat_id = tile_row * tile_cols + tile_col;
  for(uint apron_fill_task_id = tile_flat_id; apron_fill_task_id < n_apron_fill_tasks; apron_fill_task_id += n_tile_pixels){
    const uint apron_read_row = apron_fill_task_id / apron_cols;
    const uint apron_read_col = apron_fill_task_id % apron_cols;
    const int ag_c = ((int)(apron_read_col + tile_col_block * tile_col_blocksize)) - shalf_ksize;
    const int ag_r = ((int)(apron_read_row + tile_row_block * tile_row_blocksize)) - shalf_ksize;

    apron[apron_read_row * apron_cols + apron_read_col] = tex2D_at(PixelT, input_image, ag_r, ag_c);
  }

  //valid tasks read from [half_ksize, (tile_rows|tile_cols) + kernel_size - 1)
  const uint a_c = tile_col + half_ksize;
  const uint a_r = tile_row + half_ksize;
  assert_val(a_c >= half_ksize && a_c < apron_cols - half_ksize, a_c);
  assert_val(a_r >= half_ksize && a_r < apron_rows - half_ksize, a_r);

  //note the following formulas are col, row convention and uses i,j - this is done to preserve readability with the originating paper
  const uint i = a_c;
  const uint j = a_r;
#define F(_i, _j) apron_pixel((_j), (_i))

  const int Fij = F(i,j);
  //symmetric 4,2,-1 response - cross
  const int R1 = (4*F(i, j) + 2*(F(i-1,j) + F(i,j-1) + F(i+1,j) + F(i,j+1)) - 
                    F(i-2,j) - F(i+2,j) - F(i,j-2) - F(i,j+2)) / 8;

  //left-right symmetric response - with .5,1,4,5 - theta
  const int R2 = (
      8*(F(i-1,j) + F(i+1,j)) +10*F(i,j) + F(i,j-2) + F(i,j+2)
      - 2*((F(i-1,j-1) + F(i+1,j-1) + F(i-1,j+1) + F(i+1,j+1)) + F(i-2,j) + F(i+2,j))) / 16;

  //top-bottom symmetric response - with .5,1,4,5 - phi
  const int R3 = (
      8*(F(i,j-1) + F(i,j+1)) +10*F(i,j) + F(i-2,j) + F(i+2,j)
      - 2*((F(i-1,j-1) + F(i+1,j-1) + F(i-1,j+1) + F(i+1,j+1)) + F(i,j-2) + F(i,j+2))) / 16;
  //symmetric 3/2s response - checker
  const int R4 = (
      12*F(i,j) - 3*(F(i-2,j) + F(i+2,j) + F(i,j-2) + F(i,j+2))
      + 4*(F(i-1,j-1) + F(i+1,j-1) + F(i-1,j+1) + F(i+1,j+1))) / 16;

  const int G_at_red_or_blue = R1;
  const int R_at_G_in_red = R2;
  const int B_at_G_in_blue = R2;
  const int R_at_G_in_blue = R3;
  const int B_at_G_in_red = R3;
  const int R_at_B = R4;
  const int B_at_R = R4;

#undef F
#undef j
#undef i
  //RGGB -> RedXY = (0, 0), GreenXY1 = (1, 0), GreenXY2 = (0, 1), BlueXY = (1, 1)
  //GRBG -> RedXY = (1, 0), GreenXY1 = (0, 0), GreenXY2 = (1, 1), BlueXY = (0, 1)
  //GBRG -> RedXY = (0, 1), GreenXY1 = (0, 0), GreenXY2 = (1, 1), BlueXY = (1, 0)
  //BGGR -> RedXY = (1, 1), GreenXY1 = (1, 0), GreenXY2 = (0, 1), BlueXY = (0, 0)
  const int r_mod_2 = g_r & 1;
  const int c_mod_2 = g_c & 1;
#define is_rggb (bayer_pattern == RGGB)
#define is_grbg (bayer_pattern == GRBG)
#define is_gbrg (bayer_pattern == GBRG)
#define is_bggr (bayer_pattern == BGGR)

  const int red_col = is_grbg | is_bggr;
  const int red_row = is_gbrg | is_bggr;
  const int blue_col = 1 - red_col;
  const int blue_row = 1 - red_row;

  const int in_red_row = r_mod_2 == red_row;
  const int in_blue_row = r_mod_2 == blue_row;
  const int is_red_pixel = (r_mod_2 == red_row) & (c_mod_2 == red_col);
  const int is_blue_pixel = (r_mod_2 == blue_row) & (c_mod_2 == blue_col);
  const int is_green_pixel = !(is_red_pixel | is_blue_pixel);
  assert(is_green_pixel + is_blue_pixel + is_red_pixel == 1);
  assert(in_red_row + in_blue_row == 1);

  //at R locations: R is original
  //at B locations it is the 3/2s symmetric response
  //at G in red rows it is the left-right symmmetric with 4s
  //at G in blue rows it is the top-bottom symmetric with 4s
  const RGBPixelBaseT R = output_pixel_cast(
      Fij * is_red_pixel +
      R_at_B * is_blue_pixel +
      R_at_G_in_red * (is_green_pixel & in_red_row) +
      R_at_G_in_blue * (is_green_pixel & in_blue_row)
      );
  //at B locations: B is original
  //at R locations it is the 3/2s symmetric response
  //at G in red rows it is the top-bottom symmmetric with 4s
  //at G in blue rows it is the left-right symmetric with 4s
  const RGBPixelBaseT B = output_pixel_cast(
      Fij * is_blue_pixel +
      B_at_R * is_red_pixel +
      B_at_G_in_red * (is_green_pixel & in_red_row) +
      B_at_G_in_blue * (is_green_pixel & in_blue_row)
      );
  //at G locations: G is original
  //at R locations: symmetric 4,2,-1
  //at B locations: symmetric 4,2,-1
  const RGBPixelBaseT G = output_pixel_cast(Fij * is_green_pixel + G_at_red_or_blue * (!is_green_pixel));

  if(valid_pixel_task){
    RGBPixelT output;
#if OUTPUT_CHANNELS == 3 || OUTPUT_CHANNELS == 4
    output.x = R;
    output.y = G;
    output.z = B;
#if OUTPUT_CHANNELS == 4
    output.w = ALPHA_VALUE;
#endif
#else
#error "Unsupported number of output channels"
#endif
    pixel_at(RGBPixelT, output_image, g_r, g_c) = output;
  }
}

