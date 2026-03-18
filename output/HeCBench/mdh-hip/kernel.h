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

// --- from WKFUtils.cu ---
/***************************************************************************
 *cr
 *cr            (C) Copyright 1995-2009 John E. Stone
 *cr
 ***************************************************************************/
/***************************************************************************
 * RCS INFORMATION:
 *
 *      $RCSfile: WKFUtils.C,v $
 *      $Author: johns $        $Locker:  $             $State: Exp $
 *      $Revision: 1.1 $       $Date: 2009/10/26 14:59:44 $
 *
 ***************************************************************************/
/*
 * Copyright (c) 1994-2009 John E. Stone
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#if defined(ARCH_AIX4)
#include <strings.h>
#endif

#if defined(__irix)
#include <bstring.h>
#endif

#if defined(__hpux)
#include <time.h>
#endif // HPUX
#endif // _MSC_VER

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
typedef struct {
  DWORD starttime;
  DWORD endtime;
} wkf_timer;






#else

// Unix with gettimeofday()
typedef struct {
  struct timeval starttime, endtime;
  struct timezone tz;
} wkf_timer;






#endif

// system independent routines to create and destroy timers



/// initialize status message timer

/// return true if it's time to print a status update message

/// destroy message timer

#ifdef __cplusplus
}
#endif



// --- from main.cu ---
/*
 * A set of simple Multiple Debye-Huckel (MDH) kernels 
 * inspired by APBS:
 *   http://www.poissonboltzmann.org/ 
 *
 * This code was all originally written by David Gohara on MacOS X, 
 * and has been subsequently been modified by John Stone, porting to Linux,
 * adding vectorization, and several other platform-specific 
 * performance optimizations.
 * 
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <hip/hip_runtime.h>





#define SEP printf("\n")







// reference CPU kernel





// --- from WKFUtils.h ---
/***************************************************************************
 *cr
 *cr            (C) Copyright 1995-2009 John E. Stone
 *cr
 ***************************************************************************/
/***************************************************************************
 * RCS INFORMATION:
 *
 *      $RCSfile: WKFUtils.h,v $
 *      $Author: johns $        $Locker:  $             $State: Exp $
 *      $Revision: 1.1 $       $Date: 2009/10/26 14:59:45 $
 *
 ***************************************************************************/
/*
 * Copyright (c) 1994-2009 John E. Stone
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef WKF_UTILS_INC
#define WKF_UTILS_INC 1

#ifdef __cplusplus
extern "C" {
#endif

typedef void * wkf_timerhandle;            ///< a timer handle
wkf_timerhandle wkf_timer_create(void);    ///< create a timer (clears timer)
void wkf_timer_destroy(wkf_timerhandle);   ///< create a timer (clears timer)
void wkf_timer_start(wkf_timerhandle);     ///< start a timer  (clears timer)
void wkf_timer_stop(wkf_timerhandle);      ///< stop a timer
double wkf_timer_time(wkf_timerhandle);    ///< report elapsed time in seconds
double wkf_timer_timenow(wkf_timerhandle); ///< report elapsed time in seconds
double wkf_timer_start_time(wkf_timerhandle); ///< report wall starting time
double wkf_timer_stop_time(wkf_timerhandle); ///< report wall stopping time

typedef struct {
  wkf_timerhandle timer;
  double updatetime;
} wkfmsgtimer;

/// initialize periodic status message timer
extern wkfmsgtimer * wkf_msg_timer_create(double updatetime);

/// return true if it's time to print a status update message
extern int wkf_msg_timer_timeout(wkfmsgtimer *time);

/// destroy message timer
void wkf_msg_timer_destroy(wkfmsgtimer * mt);

#ifdef __cplusplus
}
#endif

#endif
