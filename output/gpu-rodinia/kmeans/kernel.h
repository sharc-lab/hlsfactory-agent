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

// --- from kmeans_cuda.cu ---
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define THREADS_PER_DIM 16
#define BLOCKS_PER_DIM 16
#define THREADS_PER_BLOCK THREADS_PER_DIM*THREADS_PER_DIM

//#define BLOCK_DELTA_REDUCE
//#define BLOCK_CENTER_REDUCE

#define CPU_DELTA_REDUCE
#define CPU_CENTER_REDUCE

extern "C"
int setup(int argc, char** argv);									/* function prototype */

// GLOBAL!!!!!
unsigned int num_threads_perdim = THREADS_PER_DIM;					/* sqrt(256) -- see references for this choice */
unsigned int num_blocks_perdim = BLOCKS_PER_DIM;					/* temporary */
unsigned int num_threads = num_threads_perdim*num_threads_perdim;	/* number of threads */
unsigned int num_blocks = num_blocks_perdim*num_blocks_perdim;		/* number of blocks */

/* _d denotes it resides on the device */
int    *membership_new;												/* newly assignment membership */
float  *feature_d;													/* inverted data array */
float  *feature_flipped_d;											/* original (not inverted) data array */
int    *membership_d;												/* membership on the device */
float  *block_new_centers;											/* sum of points in a cluster (per block) */
float  *clusters_d;													/* cluster centers on the device */
float  *block_clusters_d;											/* per block calculation of cluster centers */
int    *block_deltas_d;												/* per block calculation of deltas */

/* -------------- allocateMemory() ------------------- */
/* allocate device memory, calculate number of blocks and threads, and invert the data array */
extern "C"
/* -------------- allocateMemory() end ------------------- */

/* -------------- deallocateMemory() ------------------- */
/* free host and device memory */
extern "C"
/* -------------- deallocateMemory() end ------------------- */

////////////////////////////////////////////////////////////////////////////////
// Program main																  //


//																			  //
////////////////////////////////////////////////////////////////////////////////

/* ------------------- kmeansCuda() ------------------------ */    
extern "C"
int	// delta -- had problems when return value was of float type
kmeansCuda(float  **feature,				/* in: [npoints][nfeatures] */
           int      nfeatures,				/* number of attributes for each point */
           int      npoints,				/* number of data points */
           int      nclusters,				/* number of clusters */
           int     *membership,				/* which cluster the point belongs to */
		   float  **clusters,				/* coordinates of cluster centers */
		   int     *new_centers_len,		/* number of elements in each cluster */
           float  **new_centers				/* sum of elements in each cluster */
		   )
{
	int delta = 0;			/* if point has moved */
	int i,j;				/* counters */

	cudaSetDevice(1);

	/* copy membership (host to device) */

	/* copy clusters (host to device) */

	/* set up texture */
    cudaChannelFormatDesc chDesc0 = cudaCreateChannelDesc<float>();
    t_features.filterMode = cudaFilterModePoint;   
    t_features.normalized = false;
    t_features.channelDesc = chDesc0;

	if(cudaBindTexture(NULL, &t_features, feature_d, &chDesc0, npoints*nfeatures*sizeof(float)) != CUDA_SUCCESS)
        printf("Couldn't bind features array to texture!\n");

	cudaChannelFormatDesc chDesc1 = cudaCreateChannelDesc<float>();
    t_features_flipped.filterMode = cudaFilterModePoint;   
    t_features_flipped.normalized = false;
    t_features_flipped.channelDesc = chDesc1;

	if(cudaBindTexture(NULL, &t_features_flipped, feature_flipped_d, &chDesc1, npoints*nfeatures*sizeof(float)) != CUDA_SUCCESS)
        printf("Couldn't bind features_flipped array to texture!\n");

	cudaChannelFormatDesc chDesc2 = cudaCreateChannelDesc<float>();
    t_clusters.filterMode = cudaFilterModePoint;   
    t_clusters.normalized = false;
    t_clusters.channelDesc = chDesc2;

	if(cudaBindTexture(NULL, &t_clusters, clusters_d, &chDesc2, nclusters*nfeatures*sizeof(float)) != CUDA_SUCCESS)
        printf("Couldn't bind clusters array to texture!\n");

	/* copy clusters to constant memory */
	cudaMemcpyToSymbol("c_clusters",clusters[0],nclusters*nfeatures*sizeof(float),0,cudaMemcpyHostToDevice);

    /* setup execution parameters.
	   changed to 2d (source code on NVIDIA CUDA Programming Guide) */
    dim3  grid( num_blocks_perdim, num_blocks_perdim );
    dim3  threads( num_threads_perdim*num_threads_perdim );
    
	/* execute the kernel */

                                      nfeatures,
                                      npoints,
                                      nclusters,
                                      membership_d,
                                      clusters_d,
									  block_clusters_d,
									  block_deltas_d);

	cudaThreadSynchronize();

	/* copy back membership (device to host) */

#ifdef BLOCK_CENTER_REDUCE
    /*** Copy back arrays of per block sums ***/
    float * block_clusters_h = (float *) malloc(
        num_blocks_perdim * num_blocks_perdim * 
        nclusters * nfeatures * sizeof(float));
        

        num_blocks_perdim * num_blocks_perdim * 
        nclusters * nfeatures * sizeof(float), 
        cudaMemcpyDeviceToHost);
#endif
#ifdef BLOCK_DELTA_REDUCE
    int * block_deltas_h = (int *) malloc(
        num_blocks_perdim * num_blocks_perdim * sizeof(int));
        

        num_blocks_perdim * num_blocks_perdim * sizeof(int), 
        cudaMemcpyDeviceToHost);
#endif
    
	/* for each point, sum data points in each cluster
	   and see if membership has changed:
	     if so, increase delta and change old membership, and update new_centers;
	     otherwise, update new_centers */
	delta = 0;
	for (i = 0; i < npoints; i++)
	{		
		int cluster_id = membership_new[i];
		new_centers_len[cluster_id]++;
#ifdef CPU_CENTER_REDUCE
#endif
	}
	

#ifdef BLOCK_DELTA_REDUCE	
    /*** calculate global sums from per block sums for delta and the new centers ***/    
	
	//debug
	//printf("\t \t reducing %d block sums to global sum \n",num_blocks_perdim * num_blocks_perdim);
        
#endif
#ifdef BLOCK_CENTER_REDUCE	
	
	for(int j = 0; j < nclusters;j++) {
	}

	

#ifdef CPU_CENTER_REDUCE
	//debug
	/*for(int j = 0; j < nclusters;j++) {
	}*/
#endif

#ifdef BLOCK_CENTER_REDUCE
	for(int j = 0; j < nclusters;j++) {
		for(int k = 0; k < nfeatures;k++)
			new_centers[j][k]= block_new_centers[j*nfeatures + k];		
	}
#endif

#endif

	return delta;
	
}
/* ------------------- kmeansCuda() end ------------------------ */    



// --- from kmeans_cuda_kernel.cu ---
#ifndef _KMEANS_CUDA_KERNEL_H_
#define _KMEANS_CUDA_KERNEL_H_

#include <stdio.h>


// FIXME: Make this a runtime selectable variable!
#define ASSUMED_NR_CLUSTERS 32

#define SDATA( index)      CUT_BANK_CHECKER(sdata, index)

// t_features has the layout dim0[points 0-m-1]dim1[ points 0-m-1]...
texture<float, 1, cudaReadModeElementType> t_features;
// t_features_flipped has the layout point0[dim 0-n-1]point1[dim 0-n-1]
texture<float, 1, cudaReadModeElementType> t_features_flipped;
texture<float, 1, cudaReadModeElementType> t_clusters;

float c_clusters[ASSUMED_NR_CLUSTERS*34];		/* constant memory for cluster centers */

/* ----------------- invert_mapping() --------------------- */
/* inverts data array from row-major to column-major.

   [p0,dim0][p0,dim1][p0,dim2] ... 
   [p1,dim0][p1,dim1][p1,dim2] ... 
   [p2,dim0][p2,dim1][p2,dim2] ... 
										to
   [dim0,p0][dim0,p1][dim0,p2] ...
   [dim1,p0][dim1,p1][dim1,p2] ...
   [dim2,p0][dim2,p1][dim2,p2] ...
*/
/* ----------------- invert_mapping() end --------------------- */

/* to turn on the GPU delta and center reduction */
//#define GPU_DELTA_REDUCTION
//#define GPU_NEW_CENTER_REDUCTION

/* ----------------- kmeansPoint() --------------------- */
/* find the index of nearest cluster centers and change membership*/
#endif // #ifndef _KMEANS_CUDA_KERNEL_H_


// --- from getopt.h ---


/* getopt.h */
/* Declarations for getopt.
   Copyright (C) 1989-1994, 1996-1999, 2001 Free Software 
   Foundation, Inc. This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute 
   it and/or modify it under the terms of the GNU Lesser 
   General Public License as published by the Free Software 
   Foundation; either version 2.1 of the License, or 
   (at your option) any later version.

   The GNU C Library is distributed in the hope that it will 
   be useful, but WITHOUT ANY WARRANTY; without even the 
   implied warranty of MERCHANTABILITY or FITNESS FOR A 
   PARTICULAR PURPOSE.  See the GNU Lesser General Public 
   License for more details.

   You should have received a copy of the GNU Lesser General 
   Public License along with the GNU C Library; if not, write 
   to the Free Software Foundation, Inc., 59 Temple Place,
   Suite 330, Boston, MA 02111-1307 USA.  */

  

  

#ifndef _GETOPT_H

#ifndef __need_getopt
# define _GETOPT_H 1
#endif

/* If __GNU_LIBRARY__ is not already defined, either we are being used
   standalone, or this is the first header included in the source file.
   If we are being used with glibc, we need to include <features.h>, but
   that does not exist if we are standalone.  So: if __GNU_LIBRARY__ is
   not defined, include <ctype.h>, which will pull in <features.h> for us
   if it's from glibc.  (Why ctype.h?  It's guaranteed to exist and it
   doesn't flood the namespace with stuff the way some other headers do.)  */
#if !defined __GNU_LIBRARY__
# include <ctype.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* For communication from `getopt' to the caller.
   When `getopt' finds an option that takes an argument,
   the argument value is returned here.
   Also, when `ordering' is RETURN_IN_ORDER,
   each non-option ARGV-element is returned here.  */

extern char *optarg;

/* Index in ARGV of the next element to be scanned.
   This is used for communication to and from the caller
   and for communication between successive calls to `getopt'.

   On entry to `getopt', zero means this is the first call; initialize.

   When `getopt' returns -1, this is the index of the first of the
   non-option elements that the caller should itself scan.

   Otherwise, `optind' communicates from one call to the next
   how much of ARGV has been scanned so far.  */

extern int optind;

/* Callers store zero here to inhibit the error message `getopt' prints
   for unrecognized options.  */

extern int opterr;

/* Set to an option character which was unrecognized.  */

extern int optopt;

#ifndef __need_getopt
/* Describe the long-named options requested by the application.
   The LONG_OPTIONS argument to getopt_long or getopt_long_only is a vector
   of `struct option' terminated by an element containing a name which is
   zero.

   The field `has_arg' is:
   no_argument          (or 0) if the option does not take an argument,
   required_argument    (or 1) if the option requires an argument,
   optional_argument    (or 2) if the option takes an optional argument.

   If the field `flag' is not NULL, it points to a variable that is set
   to the value given in the field `val' when the option is found, but
   left unchanged if the option is not found.

   To have a long-named option do something other than set an `int' to
   a compiled-in constant, such as set a value from `optarg', set the
   option's `flag' field to zero and its `val' field to a nonzero
   value (the equivalent single-letter option character, if there is
   one).  For long options that have a zero `flag' field, `getopt'
   returns the contents of the `val' field.  */

struct option
{
# if (defined __STDC__ && __STDC__) || defined __cplusplus
  const char *name;
# else
  char *name;
# endif
  /* has_arg can't be an enum because some compilers complain about
     type mismatches in all the code that assumes it is an int.  */
  int has_arg;
  int *flag;
  int val;
};

/* Names for the values of the `has_arg' field of `struct option'.  */

# define no_argument            0
# define required_argument      1
# define optional_argument      2
#endif  /* need getopt */

/* Get definitions and prototypes for functions to process the
   arguments in ARGV (ARGC of them, minus the program name) for
   options given in OPTS.

   Return the option character from OPTS just read.  Return -1 when
   there are no more options.  For unrecognized options, or options
   missing arguments, `optopt' is set to the option letter, and '?' is
   returned.

   The OPTS string is a list of characters which are recognized option
   letters, optionally followed by colons, specifying that that letter
   takes an argument, to be placed in `optarg'.

   If a letter in OPTS is followed by two colons, its argument is
   optional.  This behavior is specific to the GNU `getopt'.

   The argument `--' causes premature termination of argument
   scanning, explicitly telling `getopt' that there are no more
   options.

   If OPTS begins with `--', then non-option arguments are treated as
   arguments to the option '\0'.  This behavior is specific to the GNU
   `getopt'.  */

#if (defined __STDC__ && __STDC__) || defined __cplusplus
# ifdef __GNU_LIBRARY__
/* Many other libraries have conflicting prototypes for getopt, with
   differences in the consts, in stdlib.h.  To avoid compilation
   errors, only prototype getopt for the GNU C library.  */
extern int getopt (int ___argc, char *const *___argv, const char *__shortopts);
# else /* not __GNU_LIBRARY__ */
extern int getopt ();
# endif /* __GNU_LIBRARY__ */

# ifndef __need_getopt
extern int getopt_long (int ___argc, char *const *___argv,
                        const char *__shortopts,
                        const struct option *__longopts, int *__longind);
extern int getopt_long_only (int ___argc, char *const *___argv,
                             const char *__shortopts,
                             const struct option *__longopts, int *__longind);

/* Internal only.  Users should not call this directly.  */
extern int _getopt_internal (int ___argc, char *const *___argv,
                             const char *__shortopts,
                             const struct option *__longopts, int *__longind,
                             int __long_only);
# endif
#else /* not __STDC__ */
extern int getopt ();
# ifndef __need_getopt
extern int getopt_long ();
extern int getopt_long_only ();

extern int _getopt_internal ();
# endif
#endif /* __STDC__ */

#ifdef  __cplusplus
}
#endif

/* Make sure we later can get all the definitions and declarations.  */
#undef __need_getopt

#endif /* getopt.h */



// --- from kmeans.h ---
/*****************************************************************************/
/*IMPORTANT:  READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.         */
/*By downloading, copying, installing or using the software you agree        */
/*to this license.  If you do not agree to this license, do not download,    */
/*install, copy or use the software.                                         */
/*                                                                           */
/*                                                                           */
/*Copyright (c) 2005 Northwestern University                                 */
/*All rights reserved.                                                       */

/*Redistribution of the software in source and binary forms,                 */
/*with or without modification, is permitted provided that the               */
/*following conditions are met:                                              */
/*                                                                           */
/*1       Redistributions of source code must retain the above copyright     */
/*        notice, this list of conditions and the following disclaimer.      */
/*                                                                           */
/*2       Redistributions in binary form must reproduce the above copyright   */
/*        notice, this list of conditions and the following disclaimer in the */
/*        documentation and/or other materials provided with the distribution.*/ 
/*                                                                            */
/*3       Neither the name of Northwestern University nor the names of its    */
/*        contributors may be used to endorse or promote products derived     */
/*        from this software without specific prior written permission.       */
/*                                                                            */
/*THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS    */
/*IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED      */
/*TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT AND         */
/*FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL          */
/*NORTHWESTERN UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,       */
/*INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES          */
/*(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR          */
/*SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)          */
/*HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,         */
/*STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN    */
/*ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             */
/*POSSIBILITY OF SUCH DAMAGE.                                                 */
/******************************************************************************/

#ifndef _H_FUZZY_KMEANS
#define _H_FUZZY_KMEANS

#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38
#endif

/* rmse.c */
float   euclid_dist_2        (float*, float*, int);
int     find_nearest_point   (float* , int, float**, int);
float	rms_err(float**, int, int, float**, int);

/* cluster.c */
int     cluster(int, int, float**, int, int, float, int*, float***, float*, int, int);

/* kmeans_clustering.c */
float **kmeans_clustering(float**, int, int, int, float, int*);

#endif


// --- from unistd.h ---
/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* bos53H src/bos/usr/include/unistd.h 1.38.4.46                          */
/*                                                                        */
/* Licensed Materials - Property of IBM                                   */
/*                                                                        */
/* (C) COPYRIGHT International Business Machines Corp. 1985,1995          */
/* All Rights Reserved                                                    */
/*                                                                        */
/* US Government Users Restricted Rights - Use, duplication or            */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.      */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
/* @(#)82     1.38.4.46  src/bos/usr/include/unistd.h, incstd, bos53H, h2006_17B8 4/25/06 11:53:09 */
/*
 * COMPONENT_NAME: (INCSTD) Standard Include Files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 2006
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T
 * All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 * The copyright notice above does not evidence any
 * actual or intended publication of such source code.
 */

#ifndef _H_UNISTD
#define _H_UNISTD

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#include <strict_stdtypes.h>

#ifndef _H_TYPES
#include <sys/types.h>
#endif

#include <end_strict_stdtypes.h>

#ifndef _H_ACCESS
#include <sys/access.h>	/* for the "access" function */
#endif

/*
 * POSIX requires that certain values be included in unistd.h.  It also
 * requires that when _POSIX_SOURCE is defined only those standard
 * specific values are present.  This header includes all the POSIX
 * required entries.
 */

#ifdef _POSIX_SOURCE
#ifdef _LARGE_FILES
#define lseek lseek64
#endif

/* Symbolic constants for the "lseek" function: */
#ifndef SEEK_SET
#define SEEK_SET 0	/* Set file pointer to "offset" */
#define SEEK_CUR 1	/* Set file pointer to current plus "offset" */
#define SEEK_END 2	/* Set file pointer to EOF plus "offset" */
#endif /* SEEK_SET */

#ifdef _NO_PROTO

#ifndef _KERNEL
extern int access();
extern unsigned int alarm();
extern int chdir();
extern int chown();
extern int close();
extern char *ctermid();
extern int dup();
extern int dup2();
extern int execl();
extern int execv();
extern int execle();
extern int execve();
extern int execlp();
extern int execvp();
extern void _exit();
extern pid_t fork();
extern long fpathconf();
extern char *getcwd();
extern gid_t getegid();
extern uid_t geteuid();
extern gid_t getgid();
extern int getgroups();
extern char *getlogin();
extern pid_t getpgrp();
extern pid_t getpid();
extern pid_t getppid();
extern uid_t getuid();
extern int isatty();
extern int link();
extern off_t lseek();
extern long pathconf();
extern int pause();
extern int pipe();
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE >= 500 )
extern int pthread_atfork();
#endif
extern int read();
extern int rmdir();
extern int setgid();
extern int setpgid();
extern int setsid();
extern int setuid();
extern unsigned int sleep();
extern long sysconf();
extern pid_t tcgetpgrp();
extern int tcsetpgrp();
extern char *ttyname();
extern int unlink();
extern int write();
#endif		/* !_KERNEL	*/

#else		/* POSIX required prototypes */

#ifndef _KERNEL
extern int access(const char *, int);
extern unsigned int alarm(unsigned int);
extern int chdir(const char *);
extern int chown(const char *, uid_t, gid_t);
extern int close(int);
extern char *ctermid(char *);
extern int dup(int);
extern int dup2(int, int);
extern int execl(const char *, const char *, ...);
extern int execv(const char *, char *const []);
extern int execle(const char *, const char *, ...);
extern int execve(const char *, char *const [], char *const []);
extern int execlp(const char *, const char *, ...);
extern int execvp(const char *, char *const []);
extern void _exit(int);
extern pid_t fork(void);
extern long fpathconf(int, int);
extern char *getcwd(char *, size_t);
extern gid_t getegid(void);
extern uid_t geteuid(void);
extern gid_t getgid(void);
extern int getgroups(int, gid_t []);
extern char *getlogin(void);
#ifndef _BSD
extern pid_t getpgrp(void);
#endif /* _BSD */
extern pid_t getpid(void);
extern pid_t getppid(void);
extern uid_t getuid(void);
extern int isatty(int);
extern int link(const char *, const char *);
extern off_t lseek(int, off_t, int);
#ifdef _LARGE_FILE_API
extern off64_t	lseek64(int, off64_t, int);
#endif
extern long pathconf(const char *, int);
extern int pause(void);
extern int pipe(int []);
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE >= 500 )
extern int pthread_atfork(void (*)(void), void (*)(void), void (*)(void));
#endif
extern ssize_t read(int, void *, size_t);
extern int rmdir(const char *);
extern int setgid(gid_t);
extern int setpgid(pid_t, pid_t);
extern pid_t setsid(void);
extern int setuid(uid_t);
extern unsigned int sleep(unsigned int);
extern long sysconf(int);
extern pid_t tcgetpgrp(int);
extern int tcsetpgrp(int, pid_t);
extern char *ttyname(int);
extern int unlink(const char *);
extern ssize_t write(int, const void *, size_t);
#endif		/* !_KERNEL	*/
#endif		/* !_NO_PROTO	*/

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#define _POSIX_JOB_CONTROL	1
#define _POSIX_SAVED_IDS	1

#define _POSIX_VERSION		200112L
#define _POSIX2_VERSION		200112L
#define _POSIX2_C_VERSION	200112L

#ifdef _XOPEN_SOURCE

#define _XOPEN_VERSION		600
#define _XOPEN_XCU_VERSION	4
#define _XOPEN_XPG3		1
#define _XOPEN_XPG4		1
#define _XOPEN_UNIX		1

#define _XOPEN_REALTIME		(-1)
#define _XOPEN_REALTIME_THREADS	(-1)

#if (_XOPEN_SOURCE >= 600)
#define _XOPEN_STREAMS		1
#endif

#define _XBS5_ILP32_OFF32	1
#define _XBS5_ILP32_OFFBIG	1
#define _XBS5_LP64_OFF64	1
#define _XBS5_LPBIG_OFFBIG	1

#define _POSIX2_C_BIND		200112L
#define _POSIX2_C_DEV		200112L
#define _POSIX2_CHAR_TERM	1
#define _POSIX2_LOCALEDEF	200112L
#define _POSIX2_UPE		200112L
#define _POSIX2_FORT_DEV	(-1)
#define _POSIX2_FORT_RUN	(-1)
#define _POSIX2_SW_DEV		(-1)

#if (_POSIX_C_SOURCE >= 200112L)
#define _POSIX_REGEXP         1
#define _POSIX_SHELL          1
#define _POSIX2_PBS           (-1)
#define _POSIX2_PBS_ACCOUNTING        (-1)
#define _POSIX2_PBS_CHECKPOINT        (-1)
#define _POSIX2_PBS_LOCATE    (-1)
#define _POSIX2_PBS_MESSAGE   (-1)
#define _POSIX2_PBS_TRACK     (-1)
#define _V6_ILP32_OFF32               1
#define _V6_ILP32_OFFBIG      1
#define _V6_LP64_OFF64                1
#define _V6_LPBIG_OFFBIG      1

#define _POSIX_ADVISORY_INFO   200112L
#define _POSIX_BARRIERS        200112L
#define _POSIX_CLOCK_SELECTION 200112L
#define _POSIX_CPUTIME         200112L
#define _POSIX_MONOTONIC_CLOCK 200112L

#ifdef _POSIX_RAW_SOCKETS
#undef _POSIX_RAW_SOCKETS
#endif

#define _POSIX_SPAWN           200112L
#define _POSIX_SPIN_LOCKS      200112L
#define _POSIX_SPORADIC_SERVER (-1)
#define _POSIX_THREAD_CPUTIME  200112L
#define _POSIX_THREAD_SPORADIC_SERVER (-1)
#define _POSIX_TIMEOUTS	200112L
#define _POSIX_TRACE           (-1)
#define _POSIX_TRACE_EVENT_FILTER     (-1)
#define _POSIX_TRACE_INHERIT   (-1)
#define _POSIX_TRACE_LOG       (-1)
#define _POSIX_TYPED_MEMORY_OBJECTS   (-1)

#endif /* _POSIX_C_SOURCE >= 200112L */

#define _XOPEN_CRYPT		1
#define _XOPEN_SHM		1
#define _XOPEN_ENH_I18N		1
#define _XOPEN_LEGACY		(-1)
#ifndef __64BIT__
#define _UNIX_ABI		(-1)
#define _UNIX_ABI_IA64		(-1)
#define _UNIX_ABI_BIG_ENDIAN	(-1)
#define _UNIX_ABI_LITTLE_ENDIAN	(-1)
#endif /* __64BIT__ */

extern  char    *optarg;
extern  int     optind, opterr, optopt;

#ifdef _NO_PROTO
	extern	size_t	confstr();
	extern  char    *crypt();
	extern  void    encrypt();
	extern  int     fsync();
	extern	int	getopt();
	extern	int	nice();
	extern  void    swab();
#if (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE<200112L) || defined(_ALL_SOURCE)
	extern  char    *getpass();
	extern  int     chroot();
#endif
#else
	extern	size_t	confstr(int, char*, size_t);
	extern  char    *crypt(const char *, const char *);
	extern  void    encrypt(char *, int);
	extern  int     fsync(int);
	extern	int	getopt(int, char* const*, const char*);
	extern	int	nice(int);
	extern  void    swab(const void *, void *, ssize_t);
	extern int	fdatasync(int);
#if (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE<200112L) || defined(_ALL_SOURCE)
	extern  char    *getpass(const char *);
	extern  int     chroot(const char *);
#endif
#endif

#endif /* _XOPEN _SOURCE */

/* Threads options for 1003.1c and XPG UNIX98 */
#define _POSIX_THREADS				200112L
#define _POSIX_THREAD_ATTR_STACKADDR            200112L
#define _POSIX_THREAD_ATTR_STACKSIZE		200112L
#define _POSIX_THREAD_PROCESS_SHARED		200112L
#define _POSIX_THREAD_SAFE_FUNCTIONS		200112L
#ifdef _ALL_SOURCE
#define _POSIX_REENTRANT_FUNCTIONS		_POSIX_THREAD_SAFE_FUNCTIONS
#endif

/* Realtime threads options for 1003.1c and XPG UNIX98 */
#define	 _POSIX_THREAD_PRIORITY_SCHEDULING	(-1)
#define	 _POSIX_THREAD_PRIO_INHERIT		(-1)
#define	 _POSIX_THREAD_PRIO_PROTECT		(-1)

#undef  _POSIX_THREAD_FORKALL

/* Realtime options for 1003.1c and XPG UNIX98 */
#define _POSIX_ASYNCHRONOUS_IO			200112L
#define _POSIX_FSYNC				200112L
#define _POSIX_MAPPED_FILES			200112L
#define _POSIX_MEMLOCK			        200112L
#define _POSIX_MEMLOCK_RANGE		        200112L
#define _POSIX_MEMORY_PROTECTION		200112L
#define _POSIX_MESSAGE_PASSING			200112L
#define _POSIX_PRIORITIZED_IO			200112L
#define _POSIX_PRIORITY_SCHEDULING		200112L
#define _POSIX_REALTIME_SIGNALS			200112L
#define _POSIX_SEMAPHORES			200112L
#define _POSIX_SHARED_MEMORY_OBJECTS            200112L
#define _POSIX_SYNCHRONIZED_IO			200112L
#define _POSIX_TIMERS				200112L

#define _POSIX_ASYNC_IO				(-1)
#undef	_POSIX_SYNC_IO
#define _POSIX_PRIO_IO				(-1)

#define _POSIX_CHOWN_RESTRICTED	 0
#define _POSIX_VDISABLE		 0xFF
#define _POSIX_NO_TRUNC		 0

  /* UNIX03 and POSIX01 */
  /* Always enabled */
#define _POSIX_IPV6				200112L
#define _POSIX_RAW_SOCKETS			200112L

#ifndef NULL
#define NULL	0
#endif

#if (_POSIX_C_SOURCE >= 200112L)
#define _POSIX_READER_WRITER_LOCKS            200112L
#endif

/* arguments for the confstr() function */

#define _CS_PATH	1

	/* compile,link,lib,lint flags for 32bit, no_LARGE_FILES system */
#define _CS_XBS5_ILP32_OFF32_CFLAGS	2
#define _CS_XBS5_ILP32_OFF32_LDFLAGS	3
#define _CS_XBS5_ILP32_OFF32_LIBS	4
#define _CS_XBS5_ILP32_OFF32_LINTFLAGS	5

	/* compile,link,lib,lint flags for 32bit, _LARGE_FILES system */
#define _CS_XBS5_ILP32_OFFBIG_CFLAGS	6
#define _CS_XBS5_ILP32_OFFBIG_LDFLAGS	7
#define _CS_XBS5_ILP32_OFFBIG_LIBS	8
#define _CS_XBS5_ILP32_OFFBIG_LINTFLAGS	9

	/* compile,link,lib,lint flags for LP64 64bit system */
#define _CS_XBS5_LP64_OFF64_CFLAGS	10
#define _CS_XBS5_LP64_OFF64_LDFLAGS	11
#define _CS_XBS5_LP64_OFF64_LIBS	12
#define _CS_XBS5_LP64_OFF64_LINTFLAGS	13

	/* compile,link,lib,lint flags for ILP64 64bit system */
	/* AIX does not currently support this */
#define _CS_XBS5_LPBIG_OFFBIG_CFLAGS	14
#define _CS_XBS5_LPBIG_OFFBIG_LDFLAGS	15
#define _CS_XBS5_LPBIG_OFFBIG_LIBS	16
#define _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS	17

#define _CS_AIX_BOOTDEV				24
#define _CS_AIX_MODEL_CODE			25
#define _CS_AIX_ARCHITECTURE			26
#define _CS_AIX_MODEL_CLASS			40

#if (_POSIX_C_SOURCE >= 200112L)
#define _CS_POSIX_V6_ILP32_OFF32_CFLAGS		27
#define _CS_POSIX_V6_ILP32_OFF32_LDFLAGS	28
#define _CS_POSIX_V6_ILP32_OFF32_LIBS		29
#define _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS	30
#define _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS	31
#define _CS_POSIX_V6_ILP32_OFFBIG_LIBS		32
#define _CS_POSIX_V6_LP64_OFF64_CFLAGS		33
#define _CS_POSIX_V6_LP64_OFF64_LDFLAGS		34
#define _CS_POSIX_V6_LP64_OFF64_LIBS		35
#define _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS	36
#define _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS	37
#define _CS_POSIX_V6_LPBIG_OFFBIG_LIBS		38
#define _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS      39
#endif

	/* Values for the above */
#define _CSPATH		"/usr/bin:/usr/vac/bin"

	/* ILP32_OFF32 */
#define _CSPOSIX_V6_ILP32_OFF32_CFLAGS	"-q32"
#define _CSXBS5_ILP32_OFF32_CFLAGS	_CSPOSIX_V6_ILP32_OFF32_CFLAGS

#ifdef __ia64
#define _CSXBS5_ILP32_OFF32_LDFLAGS	""
#else /* POWER */
#define _CSPOSIX_V6_ILP32_OFF32_LDFLAGS "-b32"
#define _CSXBS5_ILP32_OFF32_LDFLAGS	_CSPOSIX_V6_ILP32_OFF32_LDFLAGS
#endif

#define _CSPOSIX_V6_ILP32_OFF32_LIBS	"-lc -lpthread -lm"
#define _CSXBS5_ILP32_OFF32_LIBS	_CSPOSIX_V6_ILP32_OFF32_LIBS

#define _CSXBS5_ILP32_OFF32_LINTFLAGS	""

	/* ILP32_OFFOFFBIG */
#define _CSPOSIX_V6_ILP32_OFFBIG_CFLAGS "-q32 -D_LARGE_FILES -qlonglong"
#define _CSXBS5_ILP32_OFFBIG_CFLAGS	_CSPOSIX_V6_ILP32_OFFBIG_CFLAGS

#ifdef __ia64
#define _CSXBS5_ILP32_OFFBIG_LDFLAGS	""
#else /* POWER */
#define _CSPOSIX_V6_ILP32_OFFBIG_LDFLAGS "-b32"
#define _CSXBS5_ILP32_OFFBIG_LDFLAGS	_CSPOSIX_V6_ILP32_OFFBIG_LDFLAGS
#endif

#define _CSPOSIX_V6_ILP32_OFFBIG_LIBS	"-lc -lpthread -lm"
#define _CSXBS5_ILP32_OFFBIG_LIBS	_CSPOSIX_V6_ILP32_OFFBIG_LIBS

#define _CSXBS5_ILP32_OFFBIG_LINTFLAGS	"-D_LARGE_FILES -qlonglong"

	/* LP64_OFF64 */
#define _CSPOSIX_V6_LP64_OFF64_CFLAGS	"-q64"
#define _CSXBS5_LP64_OFF64_CFLAGS	_CSPOSIX_V6_LP64_OFF64_CFLAGS

#ifdef __ia64
#define _CSXBS5_LP64_OFF64_LDFLAGS	""
#else /* POWER */
#define _CSPOSIX_V6_LP64_OFF64_LDFLAGS	"-b64"
#define _CSXBS5_LP64_OFF64_LDFLAGS	_CSPOSIX_V6_LP64_OFF64_LDFLAGS
#endif

#define _CSPOSIX_V6_LP64_OFF64_LIBS	"-lc -lpthread -lm"
#define _CSXBS5_LP64_OFF64_LIBS		_CSPOSIX_V6_LP64_OFF64_LIBS

#define _CSXBS5_LP64_OFF64_LINTFLAGS	"-D__64BIT__"

	/* LPBIG_OFFBIG */
#define _CSPOSIX_V6_LPBIG_OFFBIG_CFLAGS "-q64"
#define _CSXBS5_LPBIG_OFFBIG_CFLAGS	_CSPOSIX_V6_LPBIG_OFFBIG_CFLAGS

#ifdef __ia64
#define _CSXBS5_LPBIG_OFFBIG_LDFLAGS	""
#else /* POWER */
#define _CSPOSIX_V6_LPBIG_OFFBIG_LDFLAGS "-b64"
#define _CSXBS5_LPBIG_OFFBIG_LDFLAGS	_CSPOSIX_V6_LPBIG_OFFBIG_LDFLAGS
#endif

#define _CSPOSIX_V6_LPBIG_OFFBIG_LIBS	"-lc -lpthread -lm"
#define _CSXBS5_LPBIG_OFFBIG_LIBS	_CSPOSIX_V6_LPBIG_OFFBIG_LIBS

#define _CSXBS5_LPBIG_OFFBIG_LINTFLAGS	"-D__64BIT__"

#if (_POSIX_C_SOURCE >= 200112L)
#define _CSPOSIX_V6_WIDTH_RESTRICTED_ENVS \
		"POSIX_V6_ILP32_OFF32\n"  \
		"POSIX_V6_ILP32_OFFBIG\n" \
		"POSIX_V6_LP64_OFF64\n"  \
		"POSIX_V6_LPBIG_OFFBIG"
#endif

/* arguments for the pathconf() function */

#define _PC_CHOWN_RESTRICTED	10
#define _PC_LINK_MAX		11
#define _PC_MAX_CANON		12
#define _PC_MAX_INPUT		13
#define _PC_NAME_MAX		14
#define _PC_NO_TRUNC		15
#define _PC_PATH_MAX		16
#define _PC_PIPE_BUF		17
#define _PC_VDISABLE		18
#define _PC_ASYNC_IO		19
#define _PC_SYNC_IO		20
#define _PC_PRIO_IO		21
#define _PC_FILESIZEBITS	22  /* # bits needed to hold offset */
#define _PC_AIX_DISK_PARTITION	23
#define _PC_AIX_DISK_SIZE	24
#if (_POSIX_C_SOURCE >= 200112L)
#define _PC_SYMLINK_MAX         25
#define _PC_ALLOC_SIZE_MIN      26
#define _PC_REC_INCR_XFER_SIZE  27
#define _PC_REC_MAX_XFER_SIZE   28
#define _PC_REC_MIN_XFER_SIZE   29
#define _PC_REC_XFER_ALIGN      30
#define _PC_2_SYMLINKS          31
#endif

/* arguments for the sysconf() function, the defined numbers are used as
 * array index in sysconf().
 *
 * POSIX.1(1990), Table 4-2
 */
#define _SC_ARG_MAX			0
#define _SC_CHILD_MAX			1
#define _SC_CLK_TCK			2
#define _SC_NGROUPS_MAX			3
#define _SC_OPEN_MAX			4
#define _SC_STREAM_MAX			5
#define _SC_TZNAME_MAX			6
#define _SC_JOB_CONTROL			7
#define _SC_SAVED_IDS			8
#define _SC_VERSION			9

/* POSIX.1(1990), Table 2-3, required by command getconf */

#define _SC_POSIX_ARG_MAX		10
#define _SC_POSIX_CHILD_MAX		11
#define _SC_POSIX_LINK_MAX		12
#define _SC_POSIX_MAX_CANON		13
#define _SC_POSIX_MAX_INPUT		14
#define _SC_POSIX_NAME_MAX		15
#define _SC_POSIX_NGROUPS_MAX		16
#define _SC_POSIX_OPEN_MAX		17
#define _SC_POSIX_PATH_MAX		18
#define _SC_POSIX_PIPE_BUF		19
#define _SC_POSIX_SSIZE_MAX		20
#define _SC_POSIX_STREAM_MAX		21
#define _SC_POSIX_TZNAME_MAX		22

/* POSIX.2 (Draft 10), Table 41)	*/

#define _SC_BC_BASE_MAX			23
#define _SC_BC_DIM_MAX			24
#define _SC_BC_SCALE_MAX		25
#define _SC_BC_STRING_MAX		26
#define _SC_EQUIV_CLASS_MAX		27
#define _SC_EXPR_NEST_MAX		28
#define _SC_LINE_MAX			29
#define _SC_RE_DUP_MAX			30
#define _SC_2_VERSION			31
#define _SC_2_C_DEV			32
#define _SC_2_FORT_DEV			33
#define _SC_2_FORT_RUN			34
#define _SC_2_LOCALEDEF			35
#define _SC_2_SW_DEV			36

/* POSIX.2 (Draft 10), Table 13)	*/

#define _SC_POSIX2_BC_BASE_MAX		37
#define _SC_POSIX2_BC_DIM_MAX		38
#define _SC_POSIX2_BC_SCALE_MAX		39
#define _SC_POSIX2_BC_STRING_MAX	40
#define _SC_POSIX2_EQUIV_CLASS_MAX	41
#define _SC_POSIX2_EXPR_NEST_MAX	42
#define _SC_POSIX2_LINE_MAX		43
#define _SC_POSIX2_RE_DUP_MAX		44
#define _SC_PASS_MAX			45
#define _SC_XOPEN_VERSION		46
#define _SC_ATEXIT_MAX			47
#if _XOPEN_SOURCE_EXTENDED==1
#define _SC_PAGE_SIZE			48
#endif /* _XOPEN_SOURCE_EXTENDED */
#define _SC_AES_OS_VERSION		49
#define _SC_COLL_WEIGHTS_MAX		50
#define _SC_2_C_BIND			51
#define _SC_2_C_VERSION			52
#define _SC_2_UPE			53
#define _SC_2_CHAR_TERM			54
#define _SC_XOPEN_SHM			55
#define _SC_XOPEN_CRYPT			56
#define _SC_XOPEN_ENH_I18N		57
#if _XOPEN_SOURCE_EXTENDED==1
#define _SC_PAGESIZE			_SC_PAGE_SIZE
#define _SC_IOV_MAX			58
#endif /* _XOPEN_SOURCE_EXTENDED */
#define _SC_THREAD_SAFE_FUNCTIONS	59
#define _SC_THREADS			60
#define _SC_THREAD_ATTR_STACKADDR	61
#define _SC_THREAD_ATTR_STACKSIZE	62
#define _SC_THREAD_FORKALL		63
#define _SC_THREAD_PRIORITY_SCHEDULING	64
#define _SC_THREAD_PRIO_INHERIT		65
#define _SC_THREAD_PRIO_PROTECT		66
#define _SC_THREAD_PROCESS_SHARED	67
#define _SC_THREAD_KEYS_MAX		68
#define _SC_THREAD_DATAKEYS_MAX		_SC_THREAD_KEYS_MAX
#define _SC_THREAD_STACK_MIN		69
#define _SC_THREAD_THREADS_MAX		70
#ifdef _ALL_SOURCE
#define _SC_NPROCESSORS_CONF		71
#define _SC_NPROCESSORS_ONLN		72
#endif /* _ALL_SOURCE */
#define _SC_XOPEN_UNIX			73

#if (_XOPEN_SOURCE >= 500)

/* POSIX 1003.1c and XPG UNIX98 */
/* look to defines above for meanings */
#define _SC_AIO_LISTIO_MAX			75
#define _SC_AIO_MAX				76
#define _SC_AIO_PRIO_DELTA_MAX			77
#define _SC_ASYNCHRONOUS_IO			78
#define _SC_DELAYTIMER_MAX			79
#define _SC_FSYNC				80
#define _SC_GETGR_R_SIZE_MAX			81
#define _SC_GETPW_R_SIZE_MAX			82
#define _SC_LOGIN_NAME_MAX			83
#define _SC_MAPPED_FILES			84
#define _SC_MEMLOCK				85
#define _SC_MEMLOCK_RANGE			86
#define _SC_MEMORY_PROTECTION			87
#define _SC_MESSAGE_PASSING			88
#define _SC_MQ_OPEN_MAX				89
#define _SC_MQ_PRIO_MAX				90
#define _SC_PRIORITIZED_IO			91
#define _SC_PRIORITY_SCHEDULING			92
#define _SC_REALTIME_SIGNALS			93
#define _SC_RTSIG_MAX				94
#define _SC_SEMAPHORES				95
#define _SC_SEM_NSEMS_MAX			96
#define _SC_SEM_VALUE_MAX			97
#define _SC_SHARED_MEMORY_OBJECTS		98
#define _SC_SIGQUEUE_MAX			99
#define _SC_SYNCHRONIZED_IO			100
#define _SC_THREAD_DESTRUCTOR_ITERATIONS	101
#define _SC_TIMERS				102
#define _SC_TIMER_MAX				103
#define _SC_TTY_NAME_MAX			104
#define _SC_XBS5_ILP32_OFF32			105
#define _SC_XBS5_ILP32_OFFBIG			106
#define _SC_XBS5_LP64_OFF64			107
#define _SC_XBS5_LPBIG_OFFBIG			108
#define _SC_XOPEN_XCU_VERSION			109
#define _SC_XOPEN_REALTIME			110
#define _SC_XOPEN_REALTIME_THREADS		111
#define _SC_XOPEN_LEGACY			112
#endif /* _XOPEN_SOURCE >= 500 */

#ifdef _ALL_SOURCE
#define _SC_REENTRANT_FUNCTIONS		_SC_THREAD_SAFE_FUNCTIONS
#define _SC_PHYS_PAGES				113
#define _SC_AVPHYS_PAGES			114
#define _SC_LPAR_ENABLED			115
#define _SC_LARGE_PAGESIZE			116
#endif /* _ALL_SOURCE */

#define _SC_AIX_KERNEL_BITMODE			117
#define _SC_AIX_REALMEM				118
#define _SC_AIX_HARDWARE_BITMODE		119
#define _SC_AIX_MP_CAPABLE			120

#define _SC_V6_ILP32_OFF32			121
#define _SC_V6_ILP32_OFFBIG			122
#define _SC_V6_LP64_OFF64			123
#define _SC_V6_LPBIG_OFFBIG			124

#define _SC_XOPEN_STREAMS			125

#if (_POSIX_C_SOURCE >= 200112L)
#define _SC_HOST_NAME_MAX			126
#define _SC_REGEXP				127
#define _SC_SHELL				128
#define _SC_SYMLOOP_MAX				129
#define _SC_ADVISORY_INFO			130
#define _SC_FILE_LOCKING			131
#define _SC_2_PBS				132
#define _SC_2_PBS_ACCOUNTING			133
#define _SC_2_PBS_CHECKPOINT			134
#define _SC_2_PBS_LOCATE			135
#define _SC_2_PBS_MESSAGE			136
#define _SC_2_PBS_TRACK				137
#define _SC_BARRIERS				138
#define _SC_CLOCK_SELECTION			139
#define _SC_CPUTIME				140
#define _SC_MONOTONIC_CLOCK			141
#define _SC_READER_WRITER_LOCKS			142
#define _SC_SPAWN				143
#define _SC_SPIN_LOCKS				144
#define _SC_SPORADIC_SERVER			145
#define _SC_THREAD_CPUTIME			146
#define _SC_THREAD_SPORADIC_SERVER              147
#define _SC_TIMEOUTS				148
#define _SC_TRACE				149
#define _SC_TRACE_EVENT_FILTER			150
#define _SC_TRACE_INHERIT			151
#define _SC_TRACE_LOG				152
#define _SC_TYPED_MEMORY_OBJECTS		153
#define _SC_IPV6				154
#define _SC_RAW_SOCKETS				155
#define _SC_SS_REPL_MAX				156
#define _SC_TRACE_EVENT_NAME_MAX		157
#define _SC_TRACE_NAME_MAX			158
#define _SC_TRACE_SYS_MAX			159
#define _SC_TRACE_USER_EVENT_MAX		160
#endif /* _POSIX_C_SOURCE >= 200112L */

#ifdef _ALL_SOURCE
#define _SC_AIX_UKEYS				161
#endif /* _ALL_SOURCE */

#endif /* _POSIX_SOURCE */

#if _XOPEN_SOURCE_EXTENDED==1
#ifdef _LARGE_FILES
#define	ftruncate	ftruncate64
#define	truncate	truncate64
#endif

#ifndef _H_LOCKF
#include <sys/lockf.h>		/* lockf definitions for portability	*/
#endif

#ifdef _NO_PROTO
#if (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE<200112L) || defined(_ALL_SOURCE)
	extern int		brk();
	extern int              getpagesize();
#ifndef _MSGQSUPPORT
	extern int		__fd_getdtablesize();
	static int		getdtablesize()
				{
					return __fd_getdtablesize();
				}
#else
	extern int              getdtablesize();
#endif /* _MSGQSUPPORT */

	extern void             *sbrk();
#endif /* _POSIX_C_SOURCE<200112L */
	extern int		fchdir();
	extern int		fchown();
	extern int		ftruncate();
	extern long		gethostid();
	extern int		gethostname();
	extern pid_t		getpgid();
	extern pid_t		getsid();
	extern char		*getwd();
	extern int		lchown();
	extern int		readlink();
	extern pid_t		setpgrp();
	extern int		setregid();
	extern int		setreuid();
	extern int		symlink();
	extern void		sync();
	extern int		truncate();
	extern useconds_t	ualarm();
	extern int		usleep();
	extern pid_t		vfork();
#else /* _NO_PROTO */
#if (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE<200112L) || defined(_ALL_SOURCE)
	extern int		brk(void *);
	extern int              getpagesize(void);
#ifndef _MSGQSUPPORT
	extern int		__fd_getdtablesize(void);
	static int		getdtablesize()
				{
					return __fd_getdtablesize();
				}
#else
	extern int              getdtablesize(void);
#endif /* _MSGQSUPPORT */
#ifdef _LINUX_SOURCE_COMPAT
        extern void             *sbrk(ptrdiff_t);
#elif (_XOPEN_SOURCE >= 500) || defined(__64BIT__)
        extern void             *sbrk(intptr_t);
#else
        extern void             *sbrk(int);
#endif
#endif /* _POSIX_C_SOURCE<200112L */
	extern int		fchdir(int);
	extern int		fchown(int, uid_t, gid_t);
	extern int		ftruncate(int, off_t);
#ifdef _LARGE_FILE_API
	extern int		ftruncate64(int, off64_t);
#endif
	extern int		gethostname(char *, size_t);
	extern long		gethostid(void);
	extern pid_t		getpgid(pid_t);
	extern pid_t		getsid(pid_t);
	extern char		*getwd(char *);
	extern int		lchown(const char *, uid_t, gid_t);

#if (defined(_SUSV3_READLINK) || \
     (!defined(_ALL_SOURCE) && (_POSIX_C_SOURCE >= 200112L))) 
    /* If SUSV3 readlink specifically requested or if strict SUSv3 
     * environment requested */ 
#ifdef __64BIT__
static ssize_t readlink(const char * __path,
                              char * __buf, size_t __bufsize)
{
	extern ssize_t __readlink64(const char *, char *, size_t);
	return __readlink64(__path, __buf, __bufsize);
}
#else
	extern ssize_t readlink(const char *, char *, size_t);
#endif /* __64BIT__ */
#else
	extern int readlink(const char *, char *, size_t);
#endif /* _SUSV3_READLINK || !_ALL_SOURCE && _POSIX_C_SOURCE >= 200112L */

#ifndef _BSD
	extern pid_t		setpgrp(void);
#endif /* _BSD */
	extern int		setregid(gid_t, gid_t);
	extern int		setreuid(uid_t, uid_t);
	extern int		symlink(const char *, const char *);
	extern void		sync(void);
	extern int		truncate(const char *, off_t);
#ifdef _LARGE_FILE_API
	extern int		truncate64(const char *, off64_t);
#endif
	extern useconds_t	ualarm(useconds_t, useconds_t);
	extern int		usleep(useconds_t);
	extern pid_t		vfork(void);
#if _XOPEN_SOURCE>=500
	extern int		getlogin_r(char *, size_t);
	extern int		ttyname_r(int, char *, size_t);

#ifdef _LARGE_FILES
#define pread		pread64
#define pwrite		pwrite64
#endif /* _LARGE_FILES */

	extern ssize_t		pread(int, void *, size_t, off_t);
	extern ssize_t		pwrite(int, const void *, size_t, off_t);
#ifdef _LARGE_FILE_API
	extern ssize_t		pread64(int, void *, size_t, off64_t);
	extern ssize_t		pwrite64(int, const void *, size_t, off64_t);
#endif /* _LARGE_FILE_API */
#endif /* _XOPEN_SOURCE>=500 */

#endif /* _NO_PROTO */

#endif /* _XOPEN_SOURCE_EXTENDED */

#ifdef _ALL_SOURCE

extern char **environ;

#ifndef _KERNEL
#ifdef _NO_PROTO
	extern pid_t		f_fork();
#else /* _NO_PROTO */
	extern pid_t		f_fork(void);
#endif /* _NO_PROTO */
#endif	/* _KERNEL */

#ifdef _NO_PROTO
	extern char *		cuserid();
	extern int		ioctl();
#ifdef __64BIT__
	extern int		ioctlx();
	extern int		ioctl32();
	extern int		ioctl32x();
#endif /* __64BIT__ */
	extern int		readx();
	extern int		setgroups();
	extern int		writex();
	extern int		setegid();
	extern int		seteuid();
	extern int		setrgid();
	extern int		setruid();
	extern offset_t		llseek();
	extern char *		getusershell();
	extern void		setusershell();
	extern void		endusershell();
	extern char *		get_current_dir_name();
	extern int		sysfs();
#else
	extern char *		cuserid(char *);
	extern int		setegid(gid_t);
	extern int		seteuid(uid_t);
	extern int		setrgid(gid_t);
	extern int		setruid(uid_t);
#ifndef _BSD
	extern int		ioctl(int, int, ...);
#endif /* _BSD */
#ifdef __64BIT__
	extern int		ioctlx(int, int, void *, long);
	extern int		ioctl32(int, int, ...);
	extern int		ioctl32x(int, int, unsigned int, unsigned int);
#endif /* __64BIT__ */
	extern int		setgroups(int, gid_t []);
#ifndef _KERNEL
	extern int	readx(int, char*, unsigned, long);
	extern int	writex(int, char*, unsigned, long);

#ifdef _LARGE_FILES
#define fclear fclear64
#define	fsync_range	fsync_range64
#endif
	extern off_t	fclear(int, off_t);
	extern int	fsync_range(int, int, off_t, off_t);
#ifdef _LARGE_FILE_API
	extern off64_t	fclear64(int, off64_t);
	extern int	fsync_range64(int, int, off64_t, off64_t);
#endif
	extern offset_t llseek(int, offset_t, int);
	extern char *	getusershell(void);
	extern void	setusershell(void);
	extern void	endusershell(void);
	extern char *	get_current_dir_name(void);
	extern int	sysfs(int, ...);
	extern int	finfo(const char *, int, void *, int32long64_t);
	extern int	ffinfo(int, int, void *, int32long64_t);

#endif /* ndef _KERNEL */

#endif /* _NO_PROTO */

#define _AES_OS_VERSION 1               /* OSF, AES version */

#endif /* _ALL_SOURCE */

#ifdef __cplusplus
}
#endif

#endif /* _H_UNISTD */
