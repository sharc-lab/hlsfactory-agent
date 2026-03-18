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

// --- from FDTD3dGPU.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include <iostream>
#include <algorithm>
#include <chrono>




// --- from FDTD3dReference.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *





// --- from cmd_arg_reader.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *

// --- from main.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include <iostream>
#include <iomanip>

// Forward declarations
bool runTest(int argc, const char **argv);




// --- from shrUtils.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

// *********************************************************************
// Generic Utilities for NVIDIA GPU Computing SDK 
// *********************************************************************

// includes
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>

using namespace std;

// size of PGM file header 
const unsigned int PGMHeaderSize = 0x40;
#define MIN_EPSILON_ERROR 1e-3f

// Deallocate memory allocated within shrUtils
// *********************************************************************

// Helper function to init data arrays 
// *********************************************************************

// Helper function to print data arrays 
// *********************************************************************

// Optional LogFileName Override function
// *********************************************************************
char* cLogFilePathAndName = NULL;

// Function to log standardized information to console, file or both
// *********************************************************************

// Function to log standardized information to console, file or both
// *********************************************************************

// Function to log standardized information to console, file or both
// *********************************************************************

//////////////////////////////////////////////////////////////////////////////
//! Find the path for a file assuming that
//! files are found in the searchPath.
//!
//! @return the path if succeeded, otherwise 0
//! @param filename         name of the file
//! @param executable_path  optional absolute path of the executable
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//! Read file \filename and return the data
//! @return shrTRUE if reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
//////////////////////////////////////////////////////////////////////////////
template<class T>
  shrBOOL
shrReadFile( const char* filename, T** data, unsigned int* len, bool verbose) 
{
  // check input arguments
  ARGCHECK(NULL != filename);
  ARGCHECK(NULL != len);

  // intermediate storage for the data read
  std::vector<T>  data_read;

  // open file for reading

  // read all data elements 
  T token;

  // the last element is read twice
  data_read.pop_back();

  // check if reading result is consistent

  fh.close();

  // check if the given handle is already initialized
  else 
  {
    // allocate storage for the data read
    *data = (T*) malloc( sizeof(T) * data_read.size());
    // store signal size
    *len = static_cast<unsigned int>( data_read.size());
  }

  // copy data
  memcpy( *data, &data_read.front(), sizeof(T) * data_read.size());

  return shrTRUE;
}

//////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename 
//! @return shrTRUE if writing the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  data to write
//! @param len  number of data elements in data, -1 on error
//! @param epsilon  epsilon for comparison
//////////////////////////////////////////////////////////////////////////////
template<class T>
  shrBOOL
shrWriteFile( const char* filename, const T* data, unsigned int len,
    const T epsilon, bool verbose) 
{
  ARGCHECK(NULL != filename);
  ARGCHECK(NULL != data);

  // open file for writing

  // first write epsilon
  fh << "# " << epsilon << "\n";

  // write data

  // Check if writing succeeded

  // file ends with nl
  fh << std::endl;

  return shrTRUE;
}

////////////////////////////////////////////////////////////////////////////////
//! Read file \filename containg single precision floating point data 
//! @return shrTRUEif reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Read file \filename containg double precision floating point data 
//! @return shrTRUEif reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Read file \filename containg integer data 
//! @return shrTRUEif reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Read file \filename containg unsigned integer data 
//! @return shrTRUEif reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Read file \filename containg char / byte data 
//! @return shrTRUEif reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Read file \filename containg unsigned char / byte data 
//! @return shrTRUEif reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename for single precision floating point data
//! @return shrTRUEif writing the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  data to write
//! @param len  number of data elements in data, -1 on error
//! @param epsilon  epsilon for comparison
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename for double precision floating point data
//! @return shrTRUEif writing the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  data to write
//! @param len  number of data elements in data, -1 on error
//! @param epsilon  epsilon for comparison
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename for integer data
//! @return shrTRUEif writing the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  data to write
//! @param len  number of data elements in data, -1 on error
//! @param epsilon  epsilon for comparison
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename for unsigned integer data
//! @return shrTRUEif writing the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  data to write
//! @param len  number of data elements in data, -1 on error
//! @param epsilon  epsilon for comparison
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename for byte / char data
//! @return shrTRUEif writing the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  data to write
//! @param len  number of data elements in data, -1 on error
//! @param epsilon  epsilon for comparison
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename for byte / char data
//! @return shrTRUEif writing the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  data to write
//! @param len  number of data elements in data, -1 on error
//! @param epsilon  epsilon for comparison
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename for unsigned byte / char data
//! @return shrTRUEif writing the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  data to write
//! @param len  number of data elements in data, -1 on error
//! @param epsilon  epsilon for comparison
////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//! Load PGM or PPM file
//! @note if data == NULL then the necessary memory is allocated in the 
//!       function and w and h are initialized to the size of the image
//! @return shrTRUE if the file loading succeeded, otherwise shrFALSE
//! @param file        name of the file to load
//! @param data        handle to the memory for the image file data
//! @param w        width of the image
//! @param h        height of the image
//! @param channels number of channels in image
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//! Write / Save PPM or PGM file
//! @note Internal usage only
//! @param file  name of the image file
//! @param data  handle to the data read
//! @param w     width of the image
//! @param h     height of the image
//////////////////////////////////////////////////////////////////////////////  

////////////////////////////////////////////////////////////////////////////////
//! Load PPM image file (with unsigned char as data element type), padding 4th component
//! @return shrTrue if reading the file succeeded, otherwise shrFALSE
//! @param file  name of the image file
//! @param data  handle to the data read
//! @param w     width of the image
//! @param h     height of the image
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Save PPM image file (with unsigned char as data element type, padded to 4 byte)
//! @return shrTrue if reading the file succeeded, otherwise shrFALSE
//! @param file  name of the image file
//! @param data  handle to the data read
//! @param w     width of the image
//! @param h     height of the image
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Save PGM image file (with unsigned char as data element type)
//! @param file  name of the image file
//! @param data  handle to the data read
//! @param w     width of the image
//! @param h     height of the image
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Load PGM image file (with unsigned char as data element type)
//! @return shrTRUE if reading the file succeeded, otherwise shrFALSE
//! @param file  name of the image file
//! @param data  handle to the data read
//! @param w     width of the image
//! @param h     height of the image
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Check if command line argument \a flag-name is given
//! @return shrTRUE if command line argument \a flag_name has been given, otherwise shrFALSE
//! @param argc  argc as passed to main()
//! @param argv  argv as passed to main()
//! @param flag_name  name of command line flag
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Get the value of a command line argument of type int
//! @return shrTRUE if command line argument \a arg_name has been given and
//!         is of the requested type, otherwise shrFALSE
//! @param argc  argc as passed to main()
//! @param argv  argv as passed to main()
//! @param arg_name  name of the command line argument
//! @param val  value of the command line argument
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Get the value of a command line argument of type unsigned int
//! @return shrTRUE if command line argument \a arg_name has been given and
//!         is of the requested type, otherwise shrFALSE
//! @param argc  argc as passed to main()
//! @param argv  argv as passed to main()
//! @param arg_name  name of the command line argument
//! @param val  value of the command line argument
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Get the value of a command line argument of type float
//! @return shrTRUE if command line argument \a arg_name has been given and
//!         is of the requested type, otherwise shrFALSE
//! @param argc  argc as passed to main()
//! @param argv  argv as passed to main()
//! @param arg_name  name of the command line argument
//! @param val  value of the command line argument
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Get the value of a command line argument of type string
//! @return shrTRUE if command line argument \a arg_name has been given and
//!         is of the requested type, otherwise shrFALSE
//! @param argc  argc as passed to main()
//! @param argv  argv as passed to main()
//! @param arg_name  name of the command line argument
//! @param val  value of the command line argument
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////// 
//! Compare two arrays of arbitrary type       
//! @return shrTRUE if \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param epsilon    epsilon to use for the comparison
//////////////////////////////////////////////////////////////////////////////
template<class T, class S>
  shrBOOL  
compareData( const T* reference, const T* data, const unsigned int len, 
    const S epsilon, const float threshold) 
{
  ARGCHECK( epsilon >= 0);

  bool result = true;
  unsigned int error_count = 0;


}

////////////////////////////////////////////////////////////////////////////// 
//! Compare two arrays of arbitrary type       
//! @return shrTRUE if \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param epsilon    epsilon to use for the comparison
//////////////////////////////////////////////////////////////////////////////
template<class T, class S>
  shrBOOL  
compareDataAsFloat( const T* reference, const T* data, const unsigned int len, 
    const S epsilon) 
{
  ARGCHECK(epsilon >= 0);

  // If we set epsilon to be 0, let's set a minimum threshold
  float max_error = MAX( (float)epsilon, MIN_EPSILON_ERROR );
  int error_count = 0;
  bool result = true;

  ARGCHECK(epsilon >= 0);

  // If we set epsilon to be 0, let's set a minimum threshold
  float max_error = MAX( (float)epsilon, MIN_EPSILON_ERROR);
  int error_count = 0;
  bool result = true;


}

////////////////////////////////////////////////////////////////////////////////
//! Compare two float arrays
//! @return shrTRUE if \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Compare two integer arrays
//! @return shrTRUE if \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Compare two unsigned integer arrays, with epsilon and threshold
//! @return shrTRUE if \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Compare two integer arrays
//! @return shrTRUE if \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Compare two integer arrays (inc Threshold for # of pixel we can have errors)
//! @return  shrTRUE if \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Compare two integer arrays
//! @return  shrTRUE if \a reference and \a data are identical, 
//!          otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Compare two float arrays with an epsilon tolerance for equality
//! @return shrTRUE if \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param epsilon    epsilon to use for the comparison
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Compare two float arrays with an epsilon tolerance for equality and a 
//!     threshold for # pixel errors
//! @return  shrTRUE if \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param epsilon    epsilon to use for the comparison
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Compare two float arrays using L2-norm with an epsilon tolerance for equality
//! @return shrTRUE if \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param epsilon    epsilon to use for the comparison
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Compare two PPM image files with an epsilon tolerance for equality
//! @return shrTRUE if \a reference and \a data are identical, otherwise shrFALSE
//! @param src_file   filename for the image to be compared
//! @param data       filename for the reference data / gold image
//! @param epsilon    epsilon to use for the comparison
//! @param threshold  threshold of pixels that can still mismatch to pass (i.e. 0.15f = 15% must pass)
//! @param verboseErrors output details of image mismatch to std::cerr
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Compare two PGM image files with an epsilon tolerance for equality
//! @return shrTRUE if \a reference and \a data are identical, otherwise shrFALSE
//! @param src_file   filename for the image to be compared
//! @param data       filename for the reference data / gold image
//! @param epsilon    epsilon to use for the comparison
//! @param threshold  threshold of pixels that can still mismatch to pass (i.e. 0.15f = 15% must pass)
////////////////////////////////////////////////////////////////////////////////

// Load raw data from disk

// Round Up Division function


// --- from FDTD3dGPU.h ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#ifndef _FDTD3DGPU_H_
#define _FDTD3DGPU_H_

#include <cstddef>
#if defined(_WIN32) && defined(_MSC_VER)
typedef unsigned __int64 memsize_t;
#else
#include <stdint.h>
typedef uint64_t memsize_t;
#endif

// default value used for global memory  
#define MEMORY_SIZE 134217728

// default values used for shared memory  
#define k_blockDimMaxY 16
#define k_blockDimMaxX 32

// The values are set to give reasonable performance, they can be changed
// but note that setting an excessively large work group size can result in
// build failure due to insufficient local memory, even though it would be
// clamped before execution. This is because the maximum work group size
// cannot be determined before the build.
#define k_localWorkX    32
#define k_localWorkY    8
#define k_localWorkMin  128
#define k_localWorkMax  1024

// The values are set to give reasonable runtimes, they can
// be changed but note that running very large dimensions can
// take a very long time and you should avoid running on your
// primary display in this case.
#define k_dim_min           96
#define k_dim_max           376
#define k_dim_qa            248

// Note that the maximum radius is defined here as 4 since the
// minimum work group height is 4, if you have a larger work
// group then you can increase the radius accordingly.
#define k_radius_min        2
#define k_radius_max        4
#define k_radius_default    4

// The values are set to give reasonable runtimes, they can
// be changed but note that running a very large number of
// timesteps can take a very long time and you should avoid
// running on your primary display in this case.
#define k_timesteps_min     1
#define k_timesteps_max     10
#define k_timesteps_default 5

bool fdtdGPU(float *output, const float *input, const float *coeff, const int dimx, const int dimy, const int dimz, const int radius, const int timesteps, const int argc, const char **argv);

#endif


// --- from FDTD3dReference.h ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#ifndef _FDTD3DREFERENCE_H_
#define _FDTD3DREFERENCE_H_

void generateRandomData(float *data, const int dimx, const int dimy, const int dimz, const float lowerBound, const float upperBound);
void generatePatternData(float *data, const int dimx, const int dimy, const int dimz, const float lowerBound, const float upperBound);
bool fdtdReference(float *output, const float *input, const float *coeff, const int dimx, const int dimy, const int dimz, const int radius, const int timesteps);
bool compareData(const float *output, const float *reference, const int dimx, const int dimy, const int dimz, const int radius, const float tolerance=0.0001f);

#endif


// --- from cmd_arg_reader.h ---
/*
* Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/

/* CUda UTility Library */

#ifndef _CMDARGREADER_H_
#define _CMDARGREADER_H_

// includes, system
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <typeinfo>

//! Preprocessed command line arguments
//! @note Lazy evaluation: The arguments are converted from strings to
//!       the correct data type upon request. Converted values are stored 
//!       in an additonal map so that no additional conversion is
//!       necessary. Arrays of command line arguments are stored in 
//!       std::vectors 
//! @note Usage: 
//!          const std::string* file = 
//!                       CmdArgReader::getArg< std::string>( "model")
//!          const std::vector< std::string>* files = 
//!          CmdArgReader::getArg< std::vector< std::string> >( "model")
//! @note All command line arguments begin with '--' followed by the token; 
//!   token and value are seperated by '='; example --samples=50
//! @note Arrays have the form --model=[one.obj,two.obj,three.obj] 
//!       (without whitespaces)

//! Command line argument parser
class CmdArgReader 
{
    template<class> friend class TestCmdArgReader;

protected:

    //! @param self handle to the only instance of this class
    static  CmdArgReader*  self;

public:

    //! Public construction interface
    //! @return a handle to the class instance
    //! @param argc number of command line arguments (as given to main())
    //! @param argv command line argument string (as given to main())
    static void  init( const int argc, const char** argv);

public:

    //! Get the value of the command line argument with given name
    //! @return A const handle to the requested argument.
    //! If the argument does not exist or if it 
    //!  is not from type T NULL is returned
    //! @param name the name of the requested argument
    //! @note T the type of the argument requested
    template<class T>
        static inline const T* getArg( const std::string& name);

    //! Check if a command line argument with the given name exists
    //! @return  true if a command line argument with name \a name exists,
    //!          otherwise false
    //! @param name  name of the command line argument in question
    static inline bool existArg( const std::string& name);

    //! Get the original / raw argc program argument
    static inline int& getRArgc();

    //! Get the original / raw argv program argument
    static inline char**& getRArgv();

public:

    //! Destructor
    ~CmdArgReader();

protected:

    //! Constructor, default
    CmdArgReader();

private:

    // private helper functions

    //! Get the value of the command line argument with given name
    //! @note Private helper function for 'getArg' to work on the members
    //! @return A const handle to the requested argument. If the argument
    //!         does not exist or if it  is not from type T a NULL pointer
    //!         is returned.
    //! @param name the name of the requested argument
    //! @note T the type of the argument requested
    template<class T>
        inline const T* getArgHelper( const std::string& name);

    //! Check if a command line argument with name \a name exists
    //! @return true if a command line argument of name \a name exists, 
    //!         otherwise false
    //! @param name the name of the requested argument
    inline bool existArgHelper( const std::string& name) const;

    //! Read args as token value pair into map for better processing
    //!  (Even the values remain strings until the parameter values is 
    //!   requested by the program.)
    //! @param argc the argument count (as given to 'main')
    //! @param argv the char* array containing the command line arguments
    void  createArgsMaps( const int argc, const char** argv);

    //! Helper for "casting" the strings from the map with the unprocessed 
    //! values to the correct 
    //!  data type.
    //! @return true if conversion succeeded, otherwise false
    //! @param element the value as string
    //! @param val the value as type T
    template<class T>
        static inline bool convertToT( const std::string& element, T& val);

public:

    // typedefs internal

    //! container for a processed command line argument
    //! typeid is used to easily be able to decide if a re-requested token-value
    //! pair match the type of the first conversion
    typedef std::pair< const std::type_info*, void*>  ValType;
    //! map of already converted values
    typedef std::map< std::string, ValType >          ArgsMap;
    //! iterator for the map of already converted values
    typedef ArgsMap::iterator                         ArgsMapIter;
    typedef ArgsMap::const_iterator                   ConstArgsMapIter;

    //! map of unprocessed (means unconverted) token-value pairs
    typedef std::map< std::string, std::string>            UnpMap;
    //! iterator for the map of unprocessed (means unconverted) token-value pairs
    typedef std::map< std::string, std::string>::iterator  UnpMapIter;

private:

#ifdef _WIN32
#  pragma warning( disable: 4251)
#endif

    //! rargc original value of argc
    static  int  rargc;

    //! rargv contains command line arguments in raw format
    static char**  rargv;

    //! args Map containing the already converted token-value pairs
    ArgsMap     args;

    //! args Map containing the unprocessed / unconverted token-value pairs
    UnpMap     unprocessed;

    //! iter Iterator for the map with the already converted token-value 
    //!  pairs (to avoid frequent reallocation)
    ArgsMapIter iter;

    //! iter Iterator for the map with the unconverted token-value 
    //!  pairs (to avoid frequent reallocation)
    UnpMapIter iter_unprocessed;

#ifdef _WIN32
#  pragma warning( default: 4251)
#endif

private:

    //! Constructor, copy (not implemented)
    CmdArgReader( const CmdArgReader&);

    //! Assignment operator (not implemented)
    CmdArgReader& operator=( const CmdArgReader&);
};

// variables, exported (extern)

// functions, inlined (inline)

////////////////////////////////////////////////////////////////////////////////
//! Conversion function for command line argument arrays
//! @note This function is used each type for which no template specialization
//!  exist (which will cause errors if the type does not fulfill the std::vector
//!  interface).
////////////////////////////////////////////////////////////////////////////////
template<class T>
/*static*/ inline bool
CmdArgReader::convertToT( const std::string& element, T& val)
{
    // preallocate storage
    val.resize( std::count( element.begin(), element.end(), ',') + 1);

    unsigned int i = 0;
    std::string::size_type pos_start = 1;  // leave array prefix '['
    std::string::size_type pos_end = 0;

    // do for all elements of the comma seperated list
    while( std::string::npos != ( pos_end = element.find(',', pos_end+1)) ) 
    {
        // convert each element by the appropriate function
        if ( ! convertToT< typename T::value_type >( 
            std::string( element, pos_start, pos_end - pos_start), val[i])) 
        {
            return false;
        }

        pos_start = pos_end + 1;
        ++i;
    }

    std::string tmp1(  element, pos_start, element.length() - pos_start - 1);

    // process last element (leave array postfix ']')
    if ( ! convertToT< typename T::value_type >( std::string( element,
        pos_start,
        element.length() - pos_start - 1),
        val[i])) 
    {
        return false;
    }

    // possible to process all elements?
    return true;
}

////////////////////////////////////////////////////////////////////////////////
//! Conversion function for command line arguments of type int
////////////////////////////////////////////////////////////////////////////////
template<>
inline bool
CmdArgReader::convertToT<int>( const std::string& element, int& val) 
{
    std::istringstream ios( element);
    ios >> val;

    bool ret_val = false;
    if ( ios.eof()) 
    {
        ret_val = true;
    }

    return ret_val;
}

////////////////////////////////////////////////////////////////////////////////
//! Conversion function for command line arguments of type float
////////////////////////////////////////////////////////////////////////////////
template<>
inline bool
CmdArgReader::convertToT<float>( const std::string& element, float& val) 
{
    std::istringstream ios( element);
    ios >> val;

    bool ret_val = false;
    if ( ios.eof()) 
    {
        ret_val = true;
    }

    return ret_val;
}

////////////////////////////////////////////////////////////////////////////////
//! Conversion function for command line arguments of type double
////////////////////////////////////////////////////////////////////////////////
template<>
inline bool
CmdArgReader::convertToT<double>( const std::string& element, double& val) 
{
    std::istringstream ios( element);
    ios >> val;

    bool ret_val = false;
    if ( ios.eof()) 
    {
        ret_val = true;
    }

    return ret_val;
}

////////////////////////////////////////////////////////////////////////////////
//! Conversion function for command line arguments of type string
////////////////////////////////////////////////////////////////////////////////
template<>
inline bool
CmdArgReader::convertToT<std::string>( const std::string& element, 
                                      std::string& val)
{
    val = element;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
//! Conversion function for command line arguments of type bool
////////////////////////////////////////////////////////////////////////////////
template<>
inline bool
CmdArgReader::convertToT<bool>( const std::string& element, bool& val) 
{
    // check if value is given as string-type { true | false }
    if ( "true" == element) 
    {
        val = true;
        return true;
    }
    else if ( "false" == element) 
    {
        val = false;
        return true;
    }
    // check if argument is given as integer { 0 | 1 }
    else 
    {
        int tmp;
        if ( convertToT<int>( element, tmp)) 
        {
            if ( 1 == tmp) 
            {
                val = true;
                return true;
            }
            else if ( 0 == tmp) 
            {
                val = false;
                return true;
            }
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//! Get the value of the command line argument with given name
//! @return A const handle to the requested argument. If the argument does
//!  not exist or if it is not from type T NULL is returned
//! @param T the type of the argument requested
//! @param name the name of the requested argument
////////////////////////////////////////////////////////////////////////////////
template<class T>
/*static*/ const T*
CmdArgReader::getArg( const std::string& name) 
{
    if( ! self) 
    {
        RUNTIME_EXCEPTION("CmdArgReader::getArg(): CmdArgReader not initialized.");
        return NULL;
    }

    return self->getArgHelper<T>( name);
}

////////////////////////////////////////////////////////////////////////////////
//! Check if a command line argument with the given name exists
//! @return  true if a command line argument with name \a name exists,
//!          otherwise false
//! @param name  name of the command line argument in question
////////////////////////////////////////////////////////////////////////////////
/*static*/ inline bool 
CmdArgReader::existArg( const std::string& name) 
{
    if( ! self) 
    {
        RUNTIME_EXCEPTION("CmdArgReader::getArg(): CmdArgReader not initialized.");
        return false;
    }

    return self->existArgHelper( name);
}

////////////////////////////////////////////////////////////////////////////////
//! @brief Get the value of the command line argument with given name
//! @return A const handle to the requested argument. If the argument does
//!  not exist or if it is not from type T NULL is returned
//! @param T the type of the argument requested
//! @param name the name of the requested argument
////////////////////////////////////////////////////////////////////////////////
template<class T>
const T*
CmdArgReader::getArgHelper( const std::string& name) 
{
    // check if argument already processed and stored in correct type
    if ( args.end() != (iter = args.find( name))) 
    {
        if ( (*(iter->second.first)) == typeid( T) ) 
        {
            return (T*) iter->second.second;
        }
    }
    else 
    {
        T* tmp = new T;

        // check the array with unprocessed values
        if ( unprocessed.end() != (iter_unprocessed = unprocessed.find( name))) 
        {
            // try to "cast" the string to the type requested
            if ( convertToT< T >( iter_unprocessed->second, *tmp)) 
            {
                // add the token element pair to map of already converted values
                args[name] = std::make_pair( &(typeid( T)), (void*) tmp);

                return tmp;
            }
        }

        // not used while not inserted into the map -> cleanup
        delete tmp;
    }

    // failed, argument not available
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//! Check if a command line argument with name \a name exists
//! @return true if a command line argument of name \a name exists, 
//!         otherwise false
//! @param name the name of the requested argument
////////////////////////////////////////////////////////////////////////////////
inline bool
CmdArgReader::existArgHelper( const std::string& name) const 
{
    bool ret_val = false;

    // check if argument already processed and stored in correct type
    if( args.end() != args.find( name)) 
    {
        ret_val = true;
    }
    else 
    {

        // check the array with unprocessed values
        if ( unprocessed.end() != unprocessed.find( name)) 
        {
            ret_val = true; 
        }
    }

    return ret_val;
}

////////////////////////////////////////////////////////////////////////////////
//! Get the original / raw argc program argument
////////////////////////////////////////////////////////////////////////////////
/*static*/ inline int&
CmdArgReader::getRArgc() 
{
    if( ! self) 
    {
        RUNTIME_EXCEPTION("CmdArgReader::getRArgc(): CmdArgReader not initialized.");
    }

    return rargc;
}

////////////////////////////////////////////////////////////////////////////////
//! Get the original / raw argv program argument
////////////////////////////////////////////////////////////////////////////////
/*static*/ inline char**&
CmdArgReader::getRArgv() 
{
    if( ! self) 
    {
        RUNTIME_EXCEPTION("CmdArgReader::getRArgc(): CmdArgReader not initialized.");
    }

    return rargv;
}

// functions, exported (extern)

#endif // #ifndef _CMDARGREADER_H_



// --- from exception.h ---
/*
* Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/

/* CUda UTility Library */
#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

// includes, system
#include <exception>
#include <stdexcept>
#include <iostream>
#include <stdlib.h>

//! Exception wrapper.
//! @param Std_Exception Exception out of namespace std for easy typing.
template<class Std_Exception>
class Exception : public Std_Exception 
{
public:

    //! @brief Static construction interface
    //! @return Alwayss throws ( Located_Exception<Exception>)
    //! @param file file in which the Exception occurs
    //! @param line line in which the Exception occurs
    //! @param detailed details on the code fragment causing the Exception
    static void throw_it( const char* file, 
                          const int line,
                          const char* detailed = "-" );  

    //! Static construction interface
    //! @return Alwayss throws ( Located_Exception<Exception>)
    //! @param file file in which the Exception occurs
    //! @param line line in which the Exception occurs
    //! @param detailed details on the code fragment causing the Exception
    static void throw_it( const char* file, 
                          const int line,      
                          const std::string& detailed);  

    //! Destructor
    virtual ~Exception() throw(); 

private:

    //! Constructor, default (private)
    Exception(); 

    //! Constructor, standard
    //! @param str string returned by what()
    Exception( const std::string& str); 

};

////////////////////////////////////////////////////////////////////////////////
//! Exception handler function for arbitrary exceptions
//! @param ex exception to handle
////////////////////////////////////////////////////////////////////////////////
template<class Exception_Typ>
inline void
handleException( const Exception_Typ& ex) 
{
    std::cerr << ex.what() << std::endl;

    exit( EXIT_FAILURE);
}

//! Convenience macros

//! Exception caused by dynamic program behavior, e.g. file does not exist
#define RUNTIME_EXCEPTION( msg) \
    Exception<std::runtime_error>::throw_it( __FILE__, __LINE__, msg)

//! Logic exception in program, e.g. an assert failed
#define LOGIC_EXCEPTION( msg) \
    Exception<std::logic_error>::throw_it( __FILE__, __LINE__, msg)

//! Out of range exception
#define RANGE_EXCEPTION( msg) \
    Exception<std::range_error>::throw_it( __FILE__, __LINE__, msg)

////////////////////////////////////////////////////////////////////////////////
//! Implementation

// includes, system
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
//! Static construction interface.
//! @param  Exception causing code fragment (file and line) and detailed infos.
////////////////////////////////////////////////////////////////////////////////
/*static*/ template<class Std_Exception>
void
Exception<Std_Exception>::
throw_it( const char* file, const int line, const char* detailed) 
{
    std::stringstream s;

    // Quiet heavy-weight but exceptions are not for 
    // performance / release versions
    s << "Exception in file '" << file << "' in line " << line << "\n"
      << "Detailed description: " << detailed << "\n";

    throw Exception( s.str());
}

////////////////////////////////////////////////////////////////////////////////
//! Static construction interface.
//! @param  Exception causing code fragment (file and line) and detailed infos.
////////////////////////////////////////////////////////////////////////////////
/*static*/ template<class Std_Exception>
void
Exception<Std_Exception>::
throw_it( const char* file, const int line, const std::string& msg) 
{
    throw_it( file, line, msg.c_str());
}

////////////////////////////////////////////////////////////////////////////////
//! Constructor, default (private).
////////////////////////////////////////////////////////////////////////////////
template<class Std_Exception>
Exception<Std_Exception>::Exception() :
 Exception("Unknown Exception.\n")
{ }

////////////////////////////////////////////////////////////////////////////////
//! Constructor, standard (private).
//! String returned by what().
////////////////////////////////////////////////////////////////////////////////
template<class Std_Exception>
Exception<Std_Exception>::Exception( const std::string& s) :
 Std_Exception( s)
{ }   

////////////////////////////////////////////////////////////////////////////////
//! Destructor
////////////////////////////////////////////////////////////////////////////////
template<class Std_Exception>
Exception<Std_Exception>::~Exception() throw() { }

// functions, exported

#endif // #ifndef _EXCEPTION_H_



// --- from shrUtils.h ---
/*
* Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/

#ifndef SHR_UTILS_H
#define SHR_UTILS_H

// *********************************************************************
// Generic utilities for NVIDIA GPU Computing SDK 
// *********************************************************************

// reminders for output window and build log
#ifdef _WIN32
    #pragma message ("Note: including windows.h")
    #pragma message ("Note: including math.h")
    #pragma message ("Note: including assert.h")
#endif

// OS dependent includes
#ifdef _WIN32
    // Headers needed for Windows
    #include <windows.h>
#else
    // Headers needed for Linux
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <sys/time.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdarg.h>
#endif

// Other headers needed for both Windows and Linux
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Un-comment the following #define to enable profiling code in SDK apps
//#define GPU_PROFILING

// Defines and enum for use with logging functions
// *********************************************************************
#define DEFAULTLOGFILE "SdkConsoleLog.txt"
#define MASTERLOGFILE "SdkMasterLog.csv"
enum LOGMODES 
{
    LOGCONSOLE = 1, // bit to signal "log to console" 
    LOGFILE    = 2, // bit to signal "log to file" 
    LOGBOTH    = 3, // convenience union of first 2 bits to signal "log to both"
    APPENDMODE = 4, // bit to set "file append" mode instead of "replace mode" on open
    MASTER     = 8, // bit to signal master .csv log output
    ERRORMSG   = 16, // bit to signal "pre-pend Error" 
    CLOSELOG   = 32  // bit to close log file, if open, after any requested file write
};
#define HDASHLINE "-----------------------------------------------------------\n"

// Standardized boolean
enum shrBOOL
{
    shrFALSE = 0,
    shrTRUE = 1
};

// Standardized MAX, MIN and CLAMP
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)
#define CLAMP(a, b, c) MIN(MAX(a, b), c)    // double sided clip of input a
#define TOPCLAMP(a, b) (a < b ? a:b)	    // single top side clip of input a

// Error and Exit Handling Macros... 
// *********************************************************************
// Full error handling macro with Cleanup() callback (if supplied)... 
// (Companion Inline Function lower on page)
#define shrCheckErrorEX(a, b, c) __shrCheckErrorEX(a, b, c, __FILE__ , __LINE__) 

// Short version without Cleanup() callback pointer
// Both Input (a) and Reference (b) are specified as args
#define shrCheckError(a, b) shrCheckErrorEX(a, b, 0) 

// Standardized Exit Macro for leaving main()... extended version
// (Companion Inline Function lower on page)
#define shrExitEX(a, b, c) __shrExitEX(a, b, c)

// Standardized Exit Macro for leaving main()... short version
// (Companion Inline Function lower on page)
#define shrEXIT(a, b)        __shrExitEX(a, b, EXIT_SUCCESS)

// Simple argument checker macro
#define ARGCHECK(a) if((a) != shrTRUE)return shrFALSE 

// Define for user-customized error handling
#define STDERROR "file %s, line %i\n\n" , __FILE__ , __LINE__

// Function to deallocate memory allocated within shrUtils
// *********************************************************************
extern "C" void shrFree(void* ptr);

// *********************************************************************
// Helper function to log standardized information to Console, to File or to both
//! Examples: shrLogEx(LOGBOTH, 0, "Function A\n"); 
//!         : shrLogEx(LOGBOTH | ERRORMSG, ciErrNum, STDERROR);
//! 
//! Automatically opens file and stores handle if needed and not done yet
//! Closes file and nulls handle on request
//! 
//! @param 0 iLogMode: LOGCONSOLE, LOGFILE, LOGBOTH, APPENDMODE, MASTER, ERRORMSG, CLOSELOG.  
//!          LOGFILE and LOGBOTH may be | 'd  with APPENDMODE to select file append mode instead of overwrite mode 
//!          LOGFILE and LOGBOTH may be | 'd  with CLOSELOG to "write and close" 
//!          First 3 options may be | 'd  with MASTER to enable independent write to master data log file
//!          First 3 options may be | 'd  with ERRORMSG to start line with standard error message
//! @param 2 dValue:    
//!          Positive val = double value for time in secs to be formatted to 6 decimals. 
//!          Negative val is an error code and this give error preformatting.
//! @param 3 cFormatString: String with formatting specifiers like printf or fprintf.  
//!          ALL printf flags, width, precision and type specifiers are supported with this exception: 
//!              Wide char type specifiers intended for wprintf (%S and %C) are NOT supported
//!              Single byte char type specifiers (%s and %c) ARE supported 
//! @param 4... variable args: like printf or fprintf.  Must match format specifer type above.  
//! @return 0 if OK, negative value on error or if error occurs or was passed in. 
// *********************************************************************
extern "C" int shrLogEx(int iLogMode, int iErrNum, const char* cFormatString, ...);

// Short version of shrLogEx defaulting to shrLogEx(LOGBOTH, 0, 
// *********************************************************************
extern "C" int shrLog(const char* cFormatString, ...);

// *********************************************************************
// Delta timer function for up to 3 independent timers using host high performance counters 
// Maintains state for 3 independent counters
//! Example: double dElapsedTime = shrDeltaTime(0);
//! 
//! @param 0 iCounterID: Which timer to check/reset. (0, 1, 2)
//! @return delta time of specified counter since last call in seconds.  Otherwise -9999.0 if error
// *********************************************************************
extern "C" double shrDeltaT(int iCounterID);

// Optional LogFileNameOverride function
// *********************************************************************
extern "C" void shrSetLogFileName (const char* cOverRideName);

// Helper function to init data arrays 
// *********************************************************************
extern "C" void shrFillArray(float* pfData, int iSize);

// Helper function to print data arrays 
// *********************************************************************
extern "C" void shrPrintArray(float* pfData, int iSize);

////////////////////////////////////////////////////////////////////////////
//! Find the path for a filename
//! @return the path if succeeded, otherwise 0
//! @param filename        name of the file
//! @param executablePath  optional absolute path of the executable
////////////////////////////////////////////////////////////////////////////
extern "C" char* shrFindFilePath(const char* filename, const char* executablePath);

////////////////////////////////////////////////////////////////////////////
//! Read file \filename containing single precision floating point data
//! @return shrTRUE if reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
//! @note If a NULL pointer is passed to this function and it is initialized 
//!       within shrUtils, then free() has to be used to deallocate the memory
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrReadFilef( const char* filename, float** data, unsigned int* len, 
              bool verbose = false);

////////////////////////////////////////////////////////////////////////////
//! Read file \filename containing double precision floating point data
//! @return shrTRUE if reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
//! @note If a NULL pointer is passed to this function and it is
//! @note If a NULL pointer is passed to this function and it is initialized 
//!       within shrUtils, then free() has to be used to deallocate the memory
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrReadFiled( const char* filename, double** data, unsigned int* len, 
              bool verbose = false);

////////////////////////////////////////////////////////////////////////////
//! Read file \filename containing integer data
//! @return shrTRUE if reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
//! @note If a NULL pointer is passed to this function and it is
//! @note If a NULL pointer is passed to this function and it is initialized 
//!       within shrUtils, then free() has to be used to deallocate the memory
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrReadFilei( const char* filename, int** data, unsigned int* len, bool verbose = false);

////////////////////////////////////////////////////////////////////////////
//! Read file \filename containing unsigned integer data
//! @return shrTRUE if reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
//! @note If a NULL pointer is passed to this function and it is 
//! @note If a NULL pointer is passed to this function and it is initialized 
//!       within shrUtils, then free() has to be used to deallocate the memory
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrReadFileui( const char* filename, unsigned int** data, 
               unsigned int* len, bool verbose = false);

////////////////////////////////////////////////////////////////////////////
//! Read file \filename containing char / byte data
//! @return shrTRUE if reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
//! @note If a NULL pointer is passed to this function and it is 
//! @note If a NULL pointer is passed to this function and it is initialized 
//!       within shrUtils, then free() has to be used to deallocate the memory
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrReadFileb( const char* filename, char** data, unsigned int* len, 
              bool verbose = false);

////////////////////////////////////////////////////////////////////////////
//! Read file \filename containing unsigned char / byte data
//! @return shrTRUE if reading the file succeeded, otherwise shrFALSE
//! @param filename name of the source file
//! @param data  uninitialized pointer, returned initialized and pointing to
//!        the data read
//! @param len  number of data elements in data, -1 on error
//! @note If a NULL pointer is passed to this function and it is
//! @note If a NULL pointer is passed to this function and it is initialized 
//!       within shrUtils, then free() has to be used to deallocate the memory
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrReadFileub( const char* filename, unsigned char** data, 
               unsigned int* len, bool verbose = false);

////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename containing single precision floating point 
//! data
//! @return shrTRUE if writing the file succeeded, otherwise shrFALSE
//! @param filename name of the file to write
//! @param data  pointer to data to write
//! @param len  number of data elements in data, -1 on error
//! @param epsilon  epsilon for comparison
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrWriteFilef( const char* filename, const float* data, unsigned int len,
               const float epsilon, bool verbose = false);

////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename containing double precision floating point 
//! data
//! @return shrTRUE if writing the file succeeded, otherwise shrFALSE
//! @param filename name of the file to write
//! @param data  pointer to data to write
//! @param len  number of data elements in data, -1 on error
//! @param epsilon  epsilon for comparison
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrWriteFiled( const char* filename, const float* data, unsigned int len,
               const double epsilon, bool verbose = false);

////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename containing integer data
//! @return shrTRUE if writing the file succeeded, otherwise shrFALSE
//! @param filename name of the file to write
//! @param data  pointer to data to write
//! @param len  number of data elements in data, -1 on error
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrWriteFilei( const char* filename, const int* data, unsigned int len,
               bool verbose = false);

////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename containing unsigned integer data
//! @return shrTRUE if writing the file succeeded, otherwise shrFALSE
//! @param filename name of the file to write
//! @param data  pointer to data to write
//! @param len  number of data elements in data, -1 on error
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrWriteFileui( const char* filename, const unsigned int* data, 
                unsigned int len, bool verbose = false);

////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename containing char / byte data
//! @return shrTRUE if writing the file succeeded, otherwise shrFALSE
//! @param filename name of the file to write
//! @param data  pointer to data to write
//! @param len  number of data elements in data, -1 on error
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrWriteFileb( const char* filename, const char* data, unsigned int len, 
               bool verbose = false);

////////////////////////////////////////////////////////////////////////////
//! Write a data file \filename containing unsigned char / byte data
//! @return shrTRUE if writing the file succeeded, otherwise shrFALSE
//! @param filename name of the file to write
//! @param data  pointer to data to write
//! @param len  number of data elements in data, -1 on error
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrWriteFileub( const char* filename, const unsigned char* data,
                unsigned int len, bool verbose = false);

////////////////////////////////////////////////////////////////////////////
//! Load PPM image file (with unsigned char as data element type), padding 
//! 4th component
//! @return shrTRUE if reading the file succeeded, otherwise shrFALSE
//! @param file  name of the image file
//! @param OutData  handle to the data read
//! @param w     width of the image
//! @param h     height of the image
//! 
//! Note: If *OutData is NULL this function allocates buffer that must be freed by caller
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrLoadPPM4ub(const char* file, unsigned char** OutData, 
                             unsigned int *w, unsigned int *h);

////////////////////////////////////////////////////////////////////////////
//! Save PPM image file (with unsigned char as data element type, padded to 
//! 4 bytes)
//! @return shrTRUE if saving the file succeeded, otherwise shrFALSE
//! @param file  name of the image file
//! @param data  handle to the data read
//! @param w     width of the image
//! @param h     height of the image
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrSavePPM4ub( const char* file, unsigned char *data, 
               unsigned int w, unsigned int h);

////////////////////////////////////////////////////////////////////////////////
//! Save PGM image file (with unsigned char as data element type)
//! @return shrTRUE if saving the file succeeded, otherwise shrFALSE
//! @param file  name of the image file
//! @param data  handle to the data read
//! @param w     width of the image
//! @param h     height of the image
////////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrSavePGMub( const char* file, unsigned char *data, 
              unsigned int w, unsigned int h); 

////////////////////////////////////////////////////////////////////////////
//! Load PGM image file (with unsigned char as data element type)
//! @return shrTRUE if saving the file succeeded, otherwise shrFALSE
//! @param file  name of the image file
//! @param data  handle to the data read
//! @param w     width of the image
//! @param h     height of the image
//! @note If a NULL pointer is passed to this function and it is initialized 
//!       within shrUtils, then free() has to be used to deallocate the memory
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrLoadPGMub( const char* file, unsigned char** data,
                  unsigned int *w,unsigned int *h);

////////////////////////////////////////////////////////////////////////////
// Command line arguments: General notes
// * All command line arguments begin with '--' followed by the token; 
//   token and value are seperated by '='; example --samples=50
// * Arrays have the form --model=[one.obj,two.obj,three.obj] 
//   (without whitespaces)
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//! Check if command line argument \a flag-name is given
//! @return shrTRUE if command line argument \a flag_name has been given, 
//!         otherwise shrFALSE
//! @param argc  argc as passed to main()
//! @param argv  argv as passed to main()
//! @param flag_name  name of command line flag
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrCheckCmdLineFlag( const int argc, const char** argv, 
                     const char* flag_name);

////////////////////////////////////////////////////////////////////////////
//! Get the value of a command line argument of type int
//! @return shrTRUE if command line argument \a arg_name has been given and
//!         is of the requested type, otherwise shrFALSE
//! @param argc  argc as passed to main()
//! @param argv  argv as passed to main()
//! @param arg_name  name of the command line argument
//! @param val  value of the command line argument
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrGetCmdLineArgumenti( const int argc, const char** argv, 
                        const char* arg_name, int* val);

////////////////////////////////////////////////////////////////////////////
//! Get the value of a command line argument of type unsigned int
//! @return shrTRUE if command line argument \a arg_name has been given and
//!         is of the requested type, otherwise shrFALSE
//! @param argc  argc as passed to main()
//! @param argv  argv as passed to main()
//! @param arg_name  name of the command line argument
//! @param val  value of the command line argument
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrGetCmdLineArgumentu( const int argc, const char** argv, 
                        const char* arg_name, unsigned int* val);

////////////////////////////////////////////////////////////////////////////
//! Get the value of a command line argument of type float
//! @return shrTRUE if command line argument \a arg_name has been given and
//!         is of the requested type, otherwise shrFALSE
//! @param argc  argc as passed to main()
//! @param argv  argv as passed to main()
//! @param arg_name  name of the command line argument
//! @param val  value of the command line argument
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrGetCmdLineArgumentf( const int argc, const char** argv, 
                        const char* arg_name, float* val);

////////////////////////////////////////////////////////////////////////////
//! Get the value of a command line argument of type string
//! @return shrTRUE if command line argument \a arg_name has been given and
//!         is of the requested type, otherwise shrFALSE
//! @param argc  argc as passed to main()
//! @param argv  argv as passed to main()
//! @param arg_name  name of the command line argument
//! @param val  value of the command line argument
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrGetCmdLineArgumentstr( const int argc, const char** argv, 
                          const char* arg_name, char** val);

////////////////////////////////////////////////////////////////////////////
//! Get the value of a command line argument list those element are strings
//! @return shrTRUE if command line argument \a arg_name has been given and
//!         is of the requested type, otherwise shrFALSE
//! @param argc  argc as passed to main()
//! @param argv  argv as passed to main()
//! @param arg_name  name of the command line argument
//! @param val  command line argument list
//! @param len  length of the list / number of elements
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrGetCmdLineArgumentListstr( const int argc, const char** argv, 
                              const char* arg_name, char** val, 
                              unsigned int* len);

////////////////////////////////////////////////////////////////////////////
//! Compare two float arrays
//! @return shrTRUEif \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrComparef( const float* reference, const float* data,
             const unsigned int len);

////////////////////////////////////////////////////////////////////////////
//! Compare two integer arrays
//! @return shrTRUEif \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrComparei( const int* reference, const int* data, 
             const unsigned int len ); 

////////////////////////////////////////////////////////////////////////////////
//! Compare two unsigned integer arrays, with epsilon and threshold
//! @return shrTRUEif \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param threshold  tolerance % # of comparison errors (0.15f = 15%)
////////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrCompareuit( const unsigned int* reference, const unsigned int* data,
            const unsigned int len, const float epsilon, const float threshold );

////////////////////////////////////////////////////////////////////////////
//! Compare two unsigned char arrays
//! @return shrTRUEif \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrCompareub( const unsigned char* reference, const unsigned char* data,
              const unsigned int len ); 

////////////////////////////////////////////////////////////////////////////////
//! Compare two integers with a tolernance for # of byte errors
//! @return shrTRUEif \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param epsilon    epsilon to use for the comparison
//! @param threshold  tolerance % # of comparison errors (0.15f = 15%)
////////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrCompareubt( const unsigned char* reference, const unsigned char* data,
             const unsigned int len, const float epsilon, const float threshold );

////////////////////////////////////////////////////////////////////////////////
//! Compare two integer arrays witha n epsilon tolerance for equality
//! @return shrTRUEif \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param epsilon    epsilon to use for the comparison
////////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrCompareube( const unsigned char* reference, const unsigned char* data,
             const unsigned int len, const float epsilon );

////////////////////////////////////////////////////////////////////////////
//! Compare two float arrays with an epsilon tolerance for equality
//! @return shrTRUEif \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param epsilon    epsilon to use for the comparison
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrComparefe( const float* reference, const float* data,
              const unsigned int len, const float epsilon );

////////////////////////////////////////////////////////////////////////////////
//! Compare two float arrays with an epsilon tolerance for equality and a 
//!     threshold for # pixel errors
//! @return shrTRUEif \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param epsilon    epsilon to use for the comparison
////////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrComparefet( const float* reference, const float* data,
             const unsigned int len, const float epsilon, const float threshold );

////////////////////////////////////////////////////////////////////////////
//! Compare two float arrays using L2-norm with an epsilon tolerance for 
//! equality
//! @return shrTRUEif \a reference and \a data are identical, otherwise shrFALSE
//! @param reference  handle to the reference data / gold image
//! @param data       handle to the computed data
//! @param len        number of elements in reference and data
//! @param epsilon    epsilon to use for the comparison
////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrCompareL2fe( const float* reference, const float* data,
                const unsigned int len, const float epsilon );

////////////////////////////////////////////////////////////////////////////////
//! Compare two PPM image files with an epsilon tolerance for equality
//! @return shrTRUEif \a reference and \a data are identical, otherwise shrFALSE
//! @param src_file   filename for the image to be compared
//! @param data       filename for the reference data / gold image
//! @param epsilon    epsilon to use for the comparison
//! @param threshold  threshold of pixels that can still mismatch to pass (i.e. 0.15f = 15% must pass)
//! $param verboseErrors output details of image mismatch to std::err
////////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrComparePPM( const char *src_file, const char *ref_file, const float epsilon, const float threshold);

////////////////////////////////////////////////////////////////////////////////
//! Compare two PGM image files with an epsilon tolerance for equality
//! @return shrTRUEif \a reference and \a data are identical, otherwise shrFALSE
//! @param src_file   filename for the image to be compared
//! @param data       filename for the reference data / gold image
//! @param epsilon    epsilon to use for the comparison
//! @param threshold  threshold of pixels that can still mismatch to pass (i.e. 0.15f = 15% must pass)
//! $param verboseErrors output details of image mismatch to std::err
////////////////////////////////////////////////////////////////////////////////
extern "C" shrBOOL shrComparePGM( const char *src_file, const char *ref_file, const float epsilon, const float threshold);

extern "C" unsigned char* shrLoadRawFile(const char* filename, size_t size);

extern "C" size_t shrRoundUp(int group_size, int global_size);

// companion inline function for error checking and exit on error WITH Cleanup Callback (if supplied)
// *********************************************************************
inline void __shrCheckErrorEX(int iSample, int iReference, void (*pCleanup)(int), const char* cFile, const int iLine)
{
    if (iReference != iSample)
    {
        shrLogEx(LOGBOTH | ERRORMSG, iSample, "line %i , in file %s !!!\n\n" , iLine, cFile); 
        if (pCleanup != NULL)
        {
            pCleanup(EXIT_FAILURE);
        }
        else 
        {
            shrLogEx(LOGBOTH | CLOSELOG, 0, "Exiting...\n");
            exit(EXIT_FAILURE);
        }
    }
}

// Standardized Exit
// *********************************************************************
inline void __shrExitEX(int argc, const char** argv, int iExitCode)
{
#ifdef WIN32
    if (!shrCheckCmdLineFlag(argc, argv, "noprompt") && !shrCheckCmdLineFlag(argc, argv, "qatest")) 
#else 
    if (shrCheckCmdLineFlag(argc, argv, "prompt") && !shrCheckCmdLineFlag(argc, argv, "qatest")) 
#endif
    {
        shrLogEx(LOGBOTH | CLOSELOG, 0, "\nPress <Enter> to Quit...\n");                  
        getchar();                                                           
    }       
    else 
    {
        shrLogEx(LOGBOTH | CLOSELOG, 0, "%s Exiting...\n", argv[0]); 
    }
    fflush(stderr);                                                         
    exit(iExitCode);
}

#endif
