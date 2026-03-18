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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <random>
#include <hip/hip_runtime.h>

typedef float IMAGE_T;
typedef int INT_T;

/* @func _clip
    Clip coordinate between low and high values.
    Parameters
    ----------
    x : int
        Coordinate to be clipped.
    low : int
        The lower bound.
    high : int
        The higher bound.
    Returns
    -------
    x : int
        `x` clipped between `high` and `low`.
    """
    assert 0 <= low <= high

    if x > high:
        return high
    elif x < low:
        return low
    else:
        return x
*/

/*@func _integ
    Integrate over the 2D integral image in the given window.
    Parameters
    ----------
    img : array
        The integral image over which to integrate.
    r : int
        The row number of the top left corner.
    c : int
        The column number of the top left corner.
    rl : int
        The number of rows over which to integrate.
    cl : int
        The number of columns over which to integrate.
    Returns
    -------
    ans : double
        The integral over the given window.
*/

/*@func hessian_matrix_det
    Compute the approximate Hessian Determinant over a 2D image.
    This method uses box filters over integral images to compute the
    approximate Hessian Determinant as described in [1]_.
    Parameters
    ----------
    img : array
        The integral image over which to compute Hessian Determinant.
    sigma : double
        Standard deviation used for the Gaussian kernel, used for the Hessian
        matrix
    Returns
    -------
    out : array
        The array of the Determinant of Hessians.
    References
    ----------
    .. [1] Herbert Bay, Andreas Ess, Tinne Tuytelaars, Luc Van Gool,
           "SURF: Speeded Up Robust Features"
           ftp://ftp.vision.ee.ethz.ch/publications/articles/eth_biwi_00517.pdf
    Notes
    -----
    The running time of this method only depends on size of the image. It is
    independent of `sigma` as one would expect. The downside is that the
    result for `sigma` less than `3` is not accurate, i.e., not similar to
    the result obtained if someone computed the Hessian and took its
    determinant.

    This function is derived from Scikit-Image _hessian_matrix_det (v0.19.3):
*/

