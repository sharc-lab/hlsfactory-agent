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




// --- from morphology.cu ---

enum class MorphOpType {
  ERODE,
  DILATE,
};

template <MorphOpType opType>

template <>
inline unsigned char elementOp<MorphOpType::ERODE>(unsigned char lhs, unsigned char rhs)
{
}

template <>
inline unsigned char borderValue<MorphOpType::ERODE>()
{
  return BLACK;
}

template <>
inline unsigned char borderValue<MorphOpType::DILATE>()
{
  return WHITE;
}

// NOTE: step-efficient parallel scan
template <MorphOpType opType>

// NOTE: step-efficient parallel scan
template <MorphOpType opType>

// NOTE: step-efficient parallel scan
template <MorphOpType opType>

template <MorphOpType opType>

template <MorphOpType opType>

template <MorphOpType opType>

extern "C"

extern "C"


// --- from morphology.h ---
#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <hip/hip_runtime.h>

#define BLACK 0
#define WHITE 255

/*!
 * \file morphology.h
 *
 * We use the van Herk/Gil-Werman (vHGW) algorithm, [van Herk,
 * Patt. Recog. Let. 13, pp. 517-521, 1992; Gil and Werman,
 * IEEE Trans PAMI 15(5), pp. 504-507, 1993.]
 *
 * Please refer to the leptonica documents for more details:
 * http://www.leptonica.com/binary-morphology.html
 *
 */

inline int roundUp(const int x, const int y)
{
    return (x + y - 1) / y; 
}

/*!
 * \brief erode()
 *
 * \param[in/out]   img_d: device memory pointer to source image
 * \param[in]       width: image width
 * \param[in]       height: image height
 * \param[in]       hsize: horizontal size of Sel; must be odd; origin implicitly in center
 * \param[in]       vsize: ditto
 */
extern "C"
double erode(unsigned char* img_d,
             unsigned char* tmp_d,
             const int width,
             const int height,
             const int hsize,
             const int vsize);

/*!
 * \brief dilate()
 *
 * \param[in/out]   img_d: device memory pointer to source image
 * \param[in]       width: image width
 * \param[in]       height: image height
 * \param[in]       hsize: horizontal size of Sel; must be odd; origin implicitly in center
 * \param[in]       vsize: ditto
 */
extern "C"
double dilate(unsigned char* img_d,
              unsigned char* tmp_d,
              const int width,
              const int height,
              const int hsize,
              const int vsize);

#endif /* MORPHOLOGY_H */
