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
/*
  Purpose:

    MAIN is the main program for FEYNMAN_KAC_2D.

  Discussion:

    This program is derived from section 2.5, exercise 2.2 of Petersen and Arbenz.

   
    with the boundary condition U(boundary(D)) = 1.

    The V(X,Y) is the potential function:

      V = 2 * ( (X/A^2)^2 + (Y/B^2)^2 ) + 1/A^2 + 1/B^2.

    The analytic solution of this problem is already known:

      U(X,Y) = exp ( (X/A)^2 + (Y/B)^2 - 1 ).

    Our method is via the Feynman-Kac Formula.

    The idea is to start from any (x,y) in D, and
    compute (x+Wx(t),y+Wy(t)) where 2D Brownian motion
    (Wx,Wy) is updated each step by sqrt(h)*(z1,z2),
    each z1,z2 are independent approximately Gaussian 
    random variables with zero mean and variance 1. 

    Each (x1(t),x2(t)) is advanced until (x1,x2) exits 
    the domain D.  

    Upon its first exit from D, the sample path (x1,x2) is stopped and a 
    new sample path at (x,y) is started until N such paths are completed.
 
    The Feynman-Kac formula gives the solution here as

      U(X,Y) = (1/N) sum(1 <= I <= N) Y(tau_i),

    where

      Y(tau) = exp( -int(s=0..tau) v(x1(s),x2(s)) ds),

    and tau = first exit time for path (x1,x2). 

    The integration procedure is a second order weak accurate method:

      X(t+h)  = [ x1(t) + sqrt ( h ) * z1 ]
                [ x2(t) + sqrt ( h ) * z2 ]

    Here Z1, Z2 are approximately normal univariate Gaussians. 

    An Euler predictor approximates Y at the end of the step

      Y_e     = (1 - h*v(X(t)) * Y(t), 

    A trapezoidal rule completes the step:

      Y(t+h)  = Y(t) - (h/2)*[v(X(t+h))*Y_e + v(X(t))*Y(t)].

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    31 May 2012

  Author:

    Original C 3D version by Wesley Petersen.
    C 2D version by John Burkardt.

  Reference:

    Peter Arbenz, Wesley Petersen,
    Introduction to Parallel Computing:
    A Practical Guide with Examples in C,
    Oxford, 2004,
    ISBN: 0-19-851577-4,
    LC: QA76.59.P47.
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <chrono>



// --- from kernel.h ---
void fk (
    const int ni,
    const int nj,
          int seed,
    const int N,
    const double a,
    const double b,
    const double h,
    const double rth,
    int * n_inside,
    double * err)
{
  int i = _bid_x * BLOCK_DIM_X + _tid_x + 1;
  int j = _bid_y * BLOCK_DIM_Y + _tid_y + 1;
  if (i <= ni && j <= nj) {
    double x = ( ( double ) ( nj - j     ) * ( - a )
               + ( double ) (      j - 1 ) *     a )
               / ( double ) ( nj     - 1 );

    double y = ( ( double ) ( ni - i     ) * ( - b )
               + ( double ) (      i - 1 ) *     b ) 
               / ( double ) ( ni     - 1 );

    double dx;
    double dy;
    double us;
    double ut;
    double vh;
    double vs;
    double x1;
    double x2;
    double w;
    double w_exact;
    double we;
    double wt;
    double chk = pow ( x / a, 2.0 ) + pow ( y / b, 2.0 );

    if ( 1.0 < chk )
    {
      w_exact = 1.0;
      wt = 1.0;
    }
    else {
      (*n_inside += 1);
      w_exact = exp ( pow ( x / a, 2.0 ) + pow ( y / b, 2.0 ) - 1.0 );
      wt = 0.0;
      for ( int k = 0; k < N; k++ )
      {
        x1 = x;
        x2 = y;
        w = 1.0;  
        chk = 0.0;
        while ( chk < 1.0 )
        {
          ut = r8_uniform_01 ( &seed );
          if ( ut < 1.0 / 2.0 )
          {
            us = r8_uniform_01 ( &seed ) - 0.5;
            if ( us < 0.0)
              dx = - rth;
            else
              dx = rth;
          } 
          else
          {
            dx = 0.0;
          }

          ut = r8_uniform_01 ( &seed );
          if ( ut < 1.0 / 2.0 )
          {
            us = r8_uniform_01 ( &seed ) - 0.5;
            if ( us < 0.0 )
              dy = - rth;
            else
              dy = rth;
          }
          else
          {
            dy = 0.0;
          }
          vs = potential ( a, b, x1, x2 );
          x1 = x1 + dx;
          x2 = x2 + dy;

          vh = potential ( a, b, x1, x2 );

          we = ( 1.0 - h * vs ) * w;
          w = w - 0.5 * h * ( vh * we + vs * w ); 

          chk = pow ( x1 / a, 2.0 ) + pow ( x2 / b, 2.0 );
        }
        wt += w;
      }
      wt /= ( double ) ( N ); 
      (*err += pow ( w_exact - wt, 2.0 ));
    }
  }
}



// --- from util.h ---
int i4_ceiling ( double x )

/*
  Purpose:

    I4_CEILING rounds an R8 up to the nearest I4.

  Discussion:

    The "ceiling" of X is the value of X rounded towards plus infinity.

  Example:

    X        I4_CEILING(X)

   -1.1      -1
   -1.0      -1
   -0.9       0
   -0.1       0
    0.0       0
    0.1       1
    0.9       1
    1.0       1
    1.1       2
    2.9       3
    3.0       3
    3.14159   4

  Licensing:

    This code is distributed under the GNU LGPL license.

  Modified:

    10 November 2011

  Author:

    John Burkardt

  Parameters:

    Input, double X, the number whose ceiling is desired.

    Output, int I4_CEILING, the ceiling of X.
*/
{
  int value;

  value = ( int ) x;

  if ( value < x )
  {
    value = value + 1;
  }

  return value;
}

double potential ( double a, double b, double x, double y )

/*
  Purpose:

    POTENTIAL evaluates the potential function V(X,Y,Z).

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    19 February 2008

  Author:

    John Burkardt

  Parameters:

    Input, double A, B, the parameters that define the ellipse.

    Input, double X, Y, the coordinates of the point.

    Output, double POTENTIAL, the value of the potential function at (X,Y).
*/
{
  double value;

  value = 2.0 * ( pow ( x / a / a, 2.0 )
                + pow ( y / b / b, 2.0 ) )
              + 1.0 / a / a
              + 1.0 / b / b;

  return value;
}

double r8_uniform_01 ( int *seed )

/*
  Purpose:

    R8_UNIFORM_01 returns a unit pseudorandom R8.

  Discussion:

    This routine implements the recursion

      seed = 16807 * seed mod ( 2^31 - 1 )
      r8_uniform_01 = seed / ( 2^31 - 1 )

    The integer arithmetic never requires more than 32 bits,
    including a sign bit.

    If the initial seed is 12345, then the first three computations are

      Input     Output      R8_UNIFORM_01
      SEED      SEED

         12345   207482415  0.096616
     207482415  1790989824  0.833995
    1790989824  2035175616  0.947702

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    11 August 2004

  Author:

    John Burkardt

  Reference:

    Paul Bratley, Bennett Fox, Linus Schrage,
    A Guide to Simulation,
    Springer Verlag, pages 201-202, 1983.

    Pierre L'Ecuyer,
    Random Number Generation,
    in Handbook of Simulation
    edited by Jerry Banks,
    Wiley Interscience, page 95, 1998.

    Bennett Fox,
    Algorithm 647:
    Implementation and Relative Efficiency of Quasirandom
    Sequence Generators,
    ACM Transactions on Mathematical Software,
    Volume 12, Number 4, pages 362-376, 1986.

    Peter Lewis, Allen Goodman, James Miller,
    A Pseudo-Random Number Generator for the System/360,
    IBM Systems Journal,
    Volume 8, pages 136-143, 1969.

  Parameters:

    Input/output, int *SEED, the "seed" value.  Normally, this
    value should not be 0.  On output, SEED has been updated.

    Output, double R8_UNIFORM_01, a new pseudorandom variate, strictly between
    0 and 1.
*/
{
  int k;
  double r;

  k = *seed / 127773;

  *seed = 16807 * ( *seed - k * 127773 ) - k * 2836;

  if ( *seed < 0 )
  {
    *seed = *seed + 2147483647;
  }
/*
  Although SEED can be represented exactly as a 32 bit integer,
  it generally cannot be represented exactly as a 32 bit real number!
*/
  r = ( double ) ( *seed ) * 4.656612875E-10;

  return r;
}

