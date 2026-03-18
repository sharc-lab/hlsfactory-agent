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

// --- from gaussianElim.cu ---
#include <math.h>
#include <sys/time.h>

#define BLOCK_SIZE_0 256
#define BLOCK_SIZE_1_X 16
#define BLOCK_SIZE_1_Y 16


// create both matrix and right hand side, Ke Wang 2013/08/12 11:51:06

// reference implementation for verification




/*------------------------------------------------------

// Ke Wang add a function to generate input internally


/*------------------------------------------------------
/*------------------------------------------------------
/*------------------------------------------------------

/*------------------------------------------------------



// --- from utils.cu ---
/****************************************************************************\ 
 * Copyright (c) 2011, Advanced Micro Devices, Inc.                           *
 * All rights reserved.                                                       *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions         *
 * are met:                                                                   *
 *                                                                            *
 * Redistributions of source code must retain the above copyright notice,     *
 * this list of conditions and the following disclaimer.                      *
 *                                                                            *
 * Redistributions in binary form must reproduce the above copyright notice,  *
 * this list of conditions and the following disclaimer in the documentation  *
 * and/or other materials provided with the distribution.                     *
 *                                                                            *
 * Neither the name of the copyright holder nor the names of its contributors *
 * may be used to endorse or promote products derived from this software      *
 * without specific prior written permission.                                 *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED  *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 * If you use the software (in whole or in part), you shall adhere to all     *
 * applicable U.S., European, and other export laws, including but not        *
 * limited to the U.S. Export Administration Regulations (EAR), (15 C.F.R.  *
 * Sections 730 through 774), and E.U. Council Regulation (EC) No 1334/2000   *
 * of 22 June 2000.  Further, pursuant to Section 740.6 of the EAR, you       *
 * hereby certify that, except pursuant to a license granted by the United    *
 * States Department of Commerce Bureau of Industry and Security or as        *
 * otherwise permitted pursuant to a License Exception under the U.S. Export  *
 * Administration Regulations ("EAR"), you will not (1) export, re-export or  *
 * release to a national of a country in Country Groups D:1, E:1 or E:2 any   *
 * restricted technology, software, or source code you receive hereunder,     *
 * or (2) export to Country Groups D:1, E:1 or E:2 the direct product of such *
 * technology or software, if such foreign produced direct product is subject *
 * to national security controls as identified on the Commerce Control List   *
 *(currently found in Supplement 1 to Part 774 of EAR).  For the most current *
 * Country Group listings, or for additional information about the EAR or     *
 * your obligations under those regulations, please refer to the U.S. Bureau  *
 * of Industry and Securitys website at http://www.bis.doc.gov/.             *
 \****************************************************************************/

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>


static bool usingImages = true;

//! A wrapper for malloc that checks the return value

// This function checks to make sure a file exists before we open it

// This function checks to make sure a directory exists 

// Parse the command line arguments

// This function that takes a positive integer 'value' and returns
// the nearest multiple of 'multiple' (used for padding columns)

// Concatenate two strings and return a pointer to the new string

// Set the value of using images to true if they are being
// used, or false if they are not

// Return whether or not images are being used


// --- from gaussianElim.h ---
#ifndef _GAUSSIANELIM
#define _GAUSSIANELIM

#include <iostream>
#include <vector>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

void printUsage();
int parseCommandline(int argc, char *argv[], char* filename,
                     int *q, int *t, int *size);
                     
void InitPerRun(int size,float *m);
void ForwardSub(float *a, float *b, float *m, int size,int timing);
void BackSub(float *a, float *b, float *finalVec, int size);
void Fan1(float *m, float *a, int Size, int t);
void Fan2(float *m, float *a, float *b,int Size, int j1, int t);
//void Fan3(float *m, float *b, int Size, int t);
void InitMat(FILE *fp, int size, float *ary, int nrow, int ncol);
void InitAry(FILE *fp, float *ary, int ary_size);
void PrintMat(float *ary, int size, int nrow, int ncolumn);
void PrintAry(float *ary, int ary_size);
#endif


// --- from utils.h ---
/****************************************************************************\ 
 * Copyright (c) 2011, Advanced Micro Devices, Inc.                           *
 * All rights reserved.                                                       *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions         *
 * are met:                                                                   *
 *                                                                            *
 * Redistributions of source code must retain the above copyright notice,     *
 * this list of conditions and the following disclaimer.                      *
 *                                                                            *
 * Redistributions in binary form must reproduce the above copyright notice,  *
 * this list of conditions and the following disclaimer in the documentation  *
 * and/or other materials provided with the distribution.                     *
 *                                                                            *
 * Neither the name of the copyright holder nor the names of its contributors *
 * may be used to endorse or promote products derived from this software      *
 * without specific prior written permission.                                 *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED  *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 * If you use the software (in whole or in part), you shall adhere to all     *
 * applicable U.S., European, and other export laws, including but not        *
 * limited to the U.S. Export Administration Regulations (EAR), (15 C.F.R.  *
 * Sections 730 through 774), and E.U. Council Regulation (EC) No 1334/2000   *
 * of 22 June 2000.  Further, pursuant to Section 740.6 of the EAR, you       *
 * hereby certify that, except pursuant to a license granted by the United    *
 * States Department of Commerce Bureau of Industry and Security or as        *
 * otherwise permitted pursuant to a License Exception under the U.S. Export  *
 * Administration Regulations ("EAR"), you will not (1) export, re-export or  *
 * release to a national of a country in Country Groups D:1, E:1 or E:2 any   *
 * restricted technology, software, or source code you receive hereunder,     *
 * or (2) export to Country Groups D:1, E:1 or E:2 the direct product of such *
 * technology or software, if such foreign produced direct product is subject *
 * to national security controls as identified on the Commerce Control List   *
 *(currently found in Supplement 1 to Part 774 of EAR).  For the most current *
 * Country Group listings, or for additional information about the EAR or     *
 * your obligations under those regulations, please refer to the U.S. Bureau  *
 * of Industry and Securitys website at http://www.bis.doc.gov/.             *
 \****************************************************************************/

#ifndef _UTILS_
#define _UTILS_

// Wrapper for malloc
void* alloc(size_t size);

// Checks for existence of directory
void checkDir(char* dirpath);

// Check for existence of file
void checkFile(char* filename);

// Parse the input command line options to the program
void parseArguments(int argc, char** argv, char** input, char** events, 
    char** ipts, char* devicePref, bool* verifyResults);

// Print the program usage information
void printUsage();

// Rounds up size to the nearest multiple of multiple
unsigned int roundUp(unsigned int value, unsigned int multiple);

// Concatenate two strings, creating a new one
char* smartStrcat(char* str1, char* str2);

// Set the value of usingImages
void setUsingImages(bool val);

// Return whether or not images are being used
bool isUsingImages(); 

#endif
