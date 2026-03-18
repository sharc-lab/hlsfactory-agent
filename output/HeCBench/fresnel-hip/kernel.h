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

// --- from cosine.cu ---
////////////////////////////////////////////////////////////////////////////////
// File: fresnel_auxiliary_cosine_integral.c                                  //
// Routine(s):                                                                //
//    Fresnel_Auxiliary_Cosine_Integral                                       //
//    xFresnel_Auxiliary_Cosine_Integral                                      //
////////////////////////////////////////////////////////////////////////////////

#include <math.h>           // required for fabs()
#include <float.h>          // required for DBL_EPSILON

//                         Externally Defined Routines                        //
extern "C" double xChebyshev_Tn_Series(double x, const double a[], int degree);

//                         Internally Defined Routines                        //
double      Fresnel_Auxiliary_Cosine_Integral( double x );
double xFresnel_Auxiliary_Cosine_Integral( double x );

static double Chebyshev_Expansion_0_1(double x);
static double Chebyshev_Expansion_1_3(double x);
static double Chebyshev_Expansion_3_5(double x);
static double Chebyshev_Expansion_5_7(double x);
static double Asymptotic_Series( double x );

//                         Internally Defined Constants                       //
static double const sqrt_2pi = 2.506628274631000502415765284811045253006;

////////////////////////////////////////////////////////////////////////////////
// double xFresnel_Auxiliary_Cosine_Integral( double x )                 //
//                                                                            //
//  Description:                                                              //
//     The Fresnel auxiliary cosine integral, f(x), is the integral from 0 to //
//     infinity of the integrand                                              //
//                     sqrt(2/pi) exp(-2xt) cos(t^2) dt                       //
//     where x >= 0.                                                          //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel auxiliary cosine integral  //
//                     f() where x >= 0.                                      //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel auxiliary cosine integral f evaluated at      //
//     x >= 0.                                                                //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                           //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = xFresnel_Auxiliary_Cosine_Integral( x );                           //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// static double Chebyshev_Expansion_0_1( double x )                //
//                                                                            //
//  Description:                                                              //
//     Evaluate the Fresnel auxiliary cosine integral, f(x), on the interval  //
//     0 < x <= 1 using the Chebyshev interpolation formula.                  //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel auxiliary cosine integral  //
//                     where 0 < x <= 1.                                      //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel auxiliary cosine integral f evaluated at      //
//     x where 0 < x <= 1.                                                    //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                      //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = Chebyshev_Expansion_0_1(x);                                        //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// static double Chebyshev_Expansion_1_3( double x )                //
//                                                                            //
//  Description:                                                              //
//     Evaluate the Fresnel auxiliary cosine integral, f(x), on the interval  //
//     1 < x <= 3 using the Chebyshev interpolation formula.                  //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel auxiliary cosine integral  //
//                     where 1 < x <= 3.                                      //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel auxiliary cosine integral f evaluated at      //
//     x where 1 < x <= 3.                                                    //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                      //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = Chebyshev_Expansion_1_3(x);                                        //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// static double Chebyshev_Expansion_3_5( double x )                //
//                                                                            //
//  Description:                                                              //
//     Evaluate the Fresnel auxiliary cosine integral, g(x), on the interval  //
//     3 < x <= 5 using the Chebyshev interpolation formula.                  //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel auxiliary cosine integral  //
//                     where 3 < x <= 5.                                      //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel auxiliary cosine integral f evaluated at      //
//     x where 3 < x <= 5.                                                    //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                      //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = Chebyshev_Expansion_3_5(x);                                        //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// static double Chebyshev_Expansion_5_7( double x )                //
//                                                                            //
//  Description:                                                              //
//     Evaluate the Fresnel auxiliary cosine integral, g(x), on the interval  //
//     5 < x <= 7 using the Chebyshev interpolation formula.                  //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel auxiliary cosine integral  //
//                     where 5 < x <= 7.                                      //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel auxiliary cosine integral f evaluated at      //
//     x where 5 < x <= 7.                                                    //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                      //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = Chebyshev_Expansion_5_7(x);                                        //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// static double Asymptotic_Series( double x )                      //
//                                                                            //
//  Description:                                                              //
//     For a large argument x, the auxiliary Fresnel cosine integral, f(x),   //
//     can be expressed as the asymptotic series                              //
//      f(x) ~ 1/(x*sqrt(2pi))[1 - 3/4x^4 + 105/16x^8 + ... +                 //
//                                                (4j-1)!!/(-4x^4)^j + ... ]  //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel auxiliary cosine integral  //
//                     where x > 7.                                           //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel auxiliary cosine integral f evaluated at      //
//     x where x > 7.                                                         //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                      //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = Asymptotic_Series( x );                                            //
////////////////////////////////////////////////////////////////////////////////
#define NUM_ASYMPTOTIC_TERMS 35


// --- from fresnel.cu ---
////////////////////////////////////////////////////////////////////////////////
// File: fresnel_sine_integral.c                                              //
// Routine(s):                                                                //
//    Fresnel_Sine_Integral                                                   //
//    xFresnel_Sine_Integral                                                  //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Note:                                                                     //
//     There are several different definitions of what is called the          //
//     Fresnel sine integral.  The definition of the Fresnel sine integral,   //
//     S(x) programmed below is the integral from 0 to x of the integrand     //
//                          sqrt(2/pi) sin(t^2) dt.                           //
////////////////////////////////////////////////////////////////////////////////

#include <math.h>           // required for fabs(), cos(), cos()
#include <float.h>          // required for LDBL_EPSILON

//                         Externally Defined Routines                        //
extern double xFresnel_Auxiliary_Cosine_Integral(double x);
extern double xFresnel_Auxiliary_Sine_Integral(double x);

//                         Internally Defined Routines                        //
double      Fresnel_Sine_Integral( double x );
double xFresnel_Sine_Integral( double x );

static double Power_Series_S( double x );

////////////////////////////////////////////////////////////////////////////////
// double Fresnel_Sine_Integral( double x )                                   //
//                                                                            //
//  Description:                                                              //
//     The Fresnel sine integral, S(x), is the integral with integrand        //
//                          sqrt(2/pi) sin(t^2) dt                            //
//     where the integral extends from 0 to x.                                //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel sine integral S().              //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel sine integral S evaluated at x.               //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                           //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = Fresnel_Sine_Integral( x );                                        //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// double xFresnel_Sine_Integral( double x )                        //
//                                                                            //
//  Description:                                                              //
//     The Fresnel sine integral, S(x), is the integral with integrand        //
//                          sqrt(2/pi) sin(t^2) dt                            //
//     where the integral extends from 0 to x.                                //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel sine integral S().         //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel sine integral S evaluated at x.               //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                      //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = xFresnel_Sine_Integral( x );                                       //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// static double Power_Series_S( double x )                         //
//                                                                            //
//  Description:                                                              //
//     The power series representation for the Fresnel sine integral, S(x),   //
//      is                                                                    //
//               x^3 sqrt(2/pi) Sum (-x^4)^j / [(4j+3) (2j+1)!]               //
//     where the sum extends over j = 0, ,,,.                                 //
//                                                                            //
//  Arguments:                                                                //
//     double  x                                                         //
//                The argument of the Fresnel sine integral S().              //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel sine integral S evaluated at x.               //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                      //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = Power_Series_S( x );                                               //
////////////////////////////////////////////////////////////////////////////////



// --- from main.cu ---
#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include <hip/hip_runtime.h>





// --- from sine.cu ---
////////////////////////////////////////////////////////////////////////////////
// File: fresnel_auxiliary_sine_integral.c                                    //
// Routine(s):                                                                //
//    Fresnel_Auxiliary_Sine_Integral                                         //
//    xFresnel_Auxiliary_Sine_Integral                                        //
////////////////////////////////////////////////////////////////////////////////

#include <math.h>           // required for fabs()                      
#include <float.h>          // required for DBL_EPSILON

//                         Externally Defined Routines                        //
extern "C" double xChebyshev_Tn_Series(double x, const double a[], int degree);

//                         Internally Defined Routines                        //
double      Fresnel_Auxiliary_Sine_Integral( double x );
double xFresnel_Auxiliary_Sine_Integral( double x );

static double Chebyshev_Expansion_0_1(double x);
static double Chebyshev_Expansion_1_3(double x);
static double Chebyshev_Expansion_3_5(double x);
static double Chebyshev_Expansion_5_7(double x);
static double Asymptotic_Series( double x );

//                         Internally Defined Constants                       //
static double const sqrt_2pi = 2.506628274631000502415765284811045253006;

////////////////////////////////////////////////////////////////////////////////
// double xFresnel_Auxiliary_Sine_Integral( double x )                   //
//                                                                            //
//  Description:                                                              //
//     The Fresnel auxiliary sine integral, g(x), is the integral from 0 to   //
//     infinity of the integrand                                              //
//                     sqrt(2/pi) exp(-2xt) sin(t^2) dt                       //
//     where x >= 0.                                                          //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel auxiliary sine integral    //
//                     g() where x >= 0.                                      //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel auxiliary sine integral g evaluated at        //
//     x >= 0.                                                                //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                           //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = xFresnel_Auxiliary_Sine_Integral( x );                             //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// static double Chebyshev_Expansion_0_1( double x )                //
//                                                                            //
//  Description:                                                              //
//     Evaluate the Fresnel auxiliary sine integral, g(x), on the interval    //
//     0 < x <= 1 using the Chebyshev interpolation formula.                  //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel auxiliary sine integral    //
//                     where 0 < x <= 1.                                      //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel auxiliary sine integral g evaluated at        //
//     x where 0 < x <= 1.                                                    //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                      //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = Chebyshev_Expansion_0_1(x);                                        //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// static double Chebyshev_Expansion_1_3( double x )                //
//                                                                            //
//  Description:                                                              //
//     Evaluate the Fresnel auxiliary sine integral, g(x), on the interval    //
//     1 < x <= 3 using the Chebyshev interpolation formula.                  //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel auxiliary sine integral    //
//                     where 1 < x <= 3.                                      //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel auxiliary sine integral g evaluated at        //
//     x where 1 < x <= 3.                                                    //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                      //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = Chebyshev_Expansion_1_3(x);                                        //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// static double Chebyshev_Expansion_3_5( double x )                //
//                                                                            //
//  Description:                                                              //
//     Evaluate the Fresnel auxiliary sine integral, g(x), on the interval    //
//     3 < x <= 5 using the Chebyshev interpolation formula.                  //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel auxiliary sine integral    //
//                     where 3 < x <= 5.                                      //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel auxiliary sine integral g evaluated at        //
//     x where 3 < x <= 5.                                                    //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                      //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = Chebyshev_Expansion_3_5(x);                                        //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// static double Chebyshev_Expansion_5_7( double x )                //
//                                                                            //
//  Description:                                                              //
//     Evaluate the Fresnel auxiliary sine integral, g(x), on the interval    //
//     5 < x <= 7 using the Chebyshev interpolation formula.                  //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel auxiliary sine integral    //
//                     where 5 < x <= 7.                                      //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel auxiliary sine integral g evaluated at        //
//     x where 5 < x <= 7.                                                    //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                      //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = Chebyshev_Expansion_5_7(x);                                        //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// static double Asymptotic_Series( double x )                      //
//                                                                            //
//  Description:                                                              //
//     For a large argument x, the auxiliary Fresnel sine integral, g(x),     //
//     can be expressed as the asymptotic series                              //
//      g(x) ~ 1/(x^3 * sqrt(8pi))[1 - 15/4x^4 + 945/16x^8 + ... +            //
//                                                (4j+1)!!/(-4x^4)^j + ... ]  //
//                                                                            //
//  Arguments:                                                                //
//     double  x  The argument of the Fresnel auxiliary sine integral    //
//                     where x > 7.                                           //
//                                                                            //
//  Return Value:                                                             //
//     The value of the Fresnel auxiliary sine integral g evaluated at        //
//     x where x > 7.                                                         //
//                                                                            //
//  Example:                                                                  //
//     double y, x;                                                      //
//                                                                            //
//     ( code to initialize x )                                               //
//                                                                            //
//     y = Asymptotic_Series( x );                                            //
////////////////////////////////////////////////////////////////////////////////
#define NUM_ASYMPTOTIC_TERMS 35


// --- from xchebyshev.cu ---
////////////////////////////////////////////////////////////////////////////////
// File: xchebyshev_Tn_series.c                                               //
// Routine(s):                                                                //
//    xChebyshev_Tn_Series                                                    //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// double xChebyshev_Tn_Series(double x, double a[],int degree)//
//                                                                            //
//  Description:                                                              //
//     This routine uses Clenshaw's recursion algorithm to evaluate a given   //
//     polynomial p(x) expressed as a linear combination of Chebyshev         //
//     polynomials of the first kind, Tn, at a point x,                       //
//       p(x) = a[0] + a[1]*T[1](x) + a[2]*T[2](x) + ... + a[deg]*T[deg](x).  //
//                                                                            //
//     Clenshaw's recursion formula applied to Chebyshev polynomials of the   //
//     first kind is:                                                         //
//     Set y[degree + 2] = 0, y[degree + 1] = 0, then for k = degree, ..., 1  //
//     set y[k] = 2 * x * y[k+1] - y[k+2] + a[k].  Finally                    //
//     set y[0] = x * y[1] - y[2] + a[0].  Then p(x) = y[0].                  //
//                                                                            //
//  Arguments:                                                                //
//     double x                                                          //
//        The point at which to evaluate the polynomial.                      //
//     double a[]                                                        //
//        The coefficients of the expansion in terms of Chebyshev polynomials,//
//        i.e. a[k] is the coefficient of T[k](x).  Note that in the calling  //
//        routine a must be defined double a[N] where N >= degree + 1.        //
//     int    degree                                                          //
//        The degree of the polynomial p(x).                                  //
//                                                                            //
//  Return Value:                                                             //
//     The value of the polynomial at x.                                      //
//     If degree is negative, then 0.0 is returned.                           //
//                                                                            //
//  Example:                                                                  //
//     double x, a[N], p;                                                //
//     int    deg = N - 1;                                                    //
//                                                                            //
//     ( code to initialize x, and a[i] i = 0, ... , a[deg] )                 //
//                                                                            //
//     p = xChebyshev_Tn_Series(x, a, deg);                                   //
////////////////////////////////////////////////////////////////////////////////

extern "C" double xChebyshev_Tn_Series(double x, double a[], int degree)
{
  double yp2 = 0.0;
  double yp1 = 0.0;
  double y = 0.0;
  double two_x = x + x;
  int k;

  // Check that degree >= 0.  If not, then return 0. //

  if ( degree < 0 ) return 0.0;

  // Apply Clenshaw's recursion save the last iteration. //

  for (k = degree; k >= 1; k--, yp2 = yp1, yp1 = y) 
    y = two_x * yp1 - yp2 + a[k];

  // Now apply the last iteration and return the result. //

  return x * yp1 - yp2 + a[0];
}
