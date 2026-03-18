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
// Copyright (C) 2013-2018 Altera Corporation, San Jose, California, USA. All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without ion, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
// whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// This agreement shall be governed in all respects by the laws of the State of California and
// by the laws of the United States of America.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
using namespace aocl_utils;

#define MANUAL_VECTOR      8 
#define NUM_THREADS_PER_WG 64
#define BLOOM_1            5 
#define BLOOM_2            0x7FFFF
#define BLOOM_SIZE         14
#define docEndingTag       0xFFFFFFFF

// Params
uint block_size = 64;
uint repeat = 100;
uint total_num_docs = 256*1024;
uint total_doc_size = 0;
uint total_doc_size_no_padding = 0;

// Host Buffers
scoped_aligned_ptr<uint> h_docWordFrequencies_dimm1;
scoped_aligned_ptr<uint> h_docWordFrequencies_dimm2;
scoped_aligned_ptr<ulong> h_profileWeights;
scoped_aligned_ptr<ulong> h_docInfo;
scoped_aligned_ptr<uint> h_isWordInProfileHash;
scoped_aligned_ptr<uint> h_startingDocID;
scoped_aligned_ptr<uint> h_numItemsPerThread;
scoped_aligned_ptr<ulong> h_profileScore;
scoped_aligned_ptr<uint> h_docSizes;

static uint m_z = 1;
static uint m_w = 1;


#define DOC_LEN_SIGMA 100
#define AVG_DOC_LEN   350


// High-resolution timer.









// --- from options.cu ---
// Copyright (C) 2013-2018 Altera Corporation, San Jose, California, USA. All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
// whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// This agreement shall be governed in all respects by the laws of the State of California and
// by the laws of the United States of America.

#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <vector>

namespace aocl_utils {

Options::Options() {
}

Options::Options(int num, char *argv[]) {
  return m_options.find(name) != m_options.end();
}

std::string &Options::get(const std::string &name) {
  return m_options[name];
}

const std::string &Options::get(const std::string &name) const {
  OptionMap::const_iterator it = m_options.find(name);
  return it->second;
}

void Options::addFromCommandLine(int num, char *argv[]) {
}

void Options::errorNameless() const {
  std::cerr << "No name provided for option.\n";
  std::cerr << "Option '" << name << "' does not exist.\n";
  std::cerr << "Value for option '" << name << "' is not of the right type (value = '"
            << get(name) << "').\n";
  exit(1);
}

} // ns aocl_utils



// --- from options.h ---
// Copyright (C) 2013-2018 Altera Corporation, San Jose, California, USA. All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
// whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// This agreement shall be governed in all respects by the laws of the State of California and
// by the laws of the United States of America.

// Declares a utility class used to parse command-line options.

#ifndef AOCL_UTILS_OPTIONS_H
#define AOCL_UTILS_OPTIONS_H

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace aocl_utils {

class Options {
public:
  typedef std::vector<std::string> StringVec;

  Options();
  Options(int num, char *argv[]);

  bool has(const std::string &name) const;
  std::string &get(const std::string &name); // will create an empty option if it does not exist
  const std::string &get(const std::string &name) const; // error if option does not exist

  void set(const std::string &name, const std::string &value) { get(name) = value; }

  // Command line options must be of the following form:
  //  [-]-name (indicates option exists)
  //  [-]-name=value
  //
  // This function assumes that the values are from main(int, char *).
  // This means that the argv[0] is skipped.
  void addFromCommandLine(int num, char *argv[]);

  // This templated function converts the option value to the given type.
  // An assert is raised if the conversion fails.
  template<typename T>
  T get(const std::string &name) const;

  template<typename T>
  void set(const std::string &name, const T &value);

  // Non-options are arguments processed in addFromCommandLine
  // that were not recognized as options.
  const StringVec &getNonOptions() const { return m_nonoptions; }
  size_t getNonOptionCount() const { return m_nonoptions.size(); }
  const std::string &getNonOption(size_t i) const { return m_nonoptions[i]; }

private:
  typedef std::map<std::string, std::string> OptionMap;

  // Displays an error message indicating that a nameless option
  // was provided.
  void errorNameless() const;

  // Displays an error message indicating that the given option
  // has the wrong type and then exits with an error code.
  void errorWrongType(const std::string &name) const;

  // Displays an error message indicating that the given option
  // does not exist and then exits with an error code.
  void errorNonExistent(const std::string &name) const;

  OptionMap m_options;
  StringVec m_nonoptions;

  Options(const Options &); // not implemented
  void operator =(const Options &); // not implemented
};

template<typename T>
T Options::get(const std::string &name) const {
  std::stringstream ss;
  ss << get(name);

  T v;
  ss >> v;
  if(ss.fail() || !ss.eof()) {
    // Failed to parse or did not consume the whole string value.
    errorWrongType(name);
  }
  return v;
}

// Specialization for bool. 
template<>
inline bool Options::get<bool>(const std::string &name) const {
  if(has(name)) {
    const std::string &v = get(name);
    if(v == "1") {
      return true;
    }
  }
  return false;
}

// Specialization for std::string. Simply returns the option string.
// Requires specialization because using stringstream to read the string
// will stop at the first whitespace character (which is wrong).
template<>
inline std::string Options::get<std::string>(const std::string &name) const {
  return get(name);
}

// This assumes the type T can be serialized to a string and back (when get
// is called).
template<typename T>
void Options::set(const std::string &name, const T &value) {
  std::stringstream ss;
  ss << value;
  set(name, ss.str());
}

} // ns aocl_utils

#endif



// --- from scoped_ptrs.h ---
// Copyright (C) 2013-2018 Altera Corporation, San Jose, California, USA. All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
// whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// This agreement shall be governed in all respects by the laws of the State of California and
// by the laws of the United States of America.

// Scoped pointer definitions.

#ifndef AOCL_UTILS_SCOPED_PTRS_H
#define AOCL_UTILS_SCOPED_PTRS_H

namespace aocl_utils {

// Interface is essentially the combination of std::auto_ptr and boost's smart pointers,
// along with some small extensions (auto conversion to T*).
void *alignedMalloc(size_t size) {
  void *result = NULL;
  int rc;
  rc = posix_memalign (&result, 1024, size);
  return result;
}

void alignedFree(void * ptr) {
  free (ptr);
}

// scoped_ptr: assumes pointer was allocated with operator new; destroys with operator delete
template<typename T>
class scoped_ptr {
public:
  typedef scoped_ptr<T> this_type;

  scoped_ptr() : m_ptr(NULL) {}
  scoped_ptr(T *ptr) : m_ptr(ptr) {}
  ~scoped_ptr() { reset(); }

  T *get() const { return m_ptr; }
  operator T *() const { return m_ptr; }
  T *operator ->() const { return m_ptr; }
  T &operator *() const { return *m_ptr; }

  this_type &operator =(T *ptr) { reset(ptr); return *this; }

  void reset(T *ptr = NULL) { delete m_ptr; m_ptr = ptr; }
  T *release() { T *ptr = m_ptr; m_ptr = NULL; return ptr; }

private:
  T *m_ptr;

  // noncopyable
  scoped_ptr(const this_type &);
  this_type &operator =(const this_type &);
};

// scoped_array: assumes pointer was allocated with operator new[]; destroys with operator delete[]
// Also supports allocation/reset with a number, which is the number of
// elements of type T.
template<typename T>
class scoped_array {
public:
  typedef scoped_array<T> this_type;

  scoped_array() : m_ptr(NULL) {}
  scoped_array(T *ptr) : m_ptr(NULL) { reset(ptr); }
  explicit scoped_array(size_t n) : m_ptr(NULL) { reset(n); }
  ~scoped_array() { reset(); }

  T *get() const { return m_ptr; }
  operator T *() const { return m_ptr; }
  T *operator ->() const { return m_ptr; }
  T &operator *() const { return *m_ptr; }
  T &operator [](int index) const { return m_ptr[index]; }

  this_type &operator =(T *ptr) { reset(ptr); return *this; }

  void reset(T *ptr = NULL) { delete[] m_ptr; m_ptr = ptr; }
  void reset(size_t n) { reset(new T[n]); }
  T *release() { T *ptr = m_ptr; m_ptr = NULL; return ptr; }

private:
  T *m_ptr;

  // noncopyable
  scoped_array(const this_type &);
  this_type &operator =(const this_type &);
};

// scoped_aligned_ptr: assumes pointer was allocated with alignedMalloc; destroys with alignedFree
// Also supports allocation/reset with a number, which is the number of
// elements of type T
template<typename T>
class scoped_aligned_ptr {
public:
  typedef scoped_aligned_ptr<T> this_type;

  scoped_aligned_ptr() : m_ptr(NULL) {}
  scoped_aligned_ptr(T *ptr) : m_ptr(NULL) { reset(ptr); }
  explicit scoped_aligned_ptr(size_t n) : m_ptr(NULL) { reset(n); }
  ~scoped_aligned_ptr() { reset(); }

  T *get() const { return m_ptr; }
  operator T *() const { return m_ptr; }
  T *operator ->() const { return m_ptr; }
  T &operator *() const { return *m_ptr; }
  T &operator [](int index) const { return m_ptr[index]; }

  this_type &operator =(T *ptr) { reset(ptr); return *this; }

  void reset(T *ptr = NULL) { if(m_ptr) alignedFree(m_ptr); m_ptr = ptr; }
  void reset(size_t n) { reset((T*) alignedMalloc(sizeof(T) * n)); }
  T *release() { T *ptr = m_ptr; m_ptr = NULL; return ptr; }

private:
  T *m_ptr;

  // noncopyable
  scoped_aligned_ptr(const this_type &);
  this_type &operator =(const this_type &);
};

#if USE_SVM_API == 1
// scoped_SVM_aligned_ptr: assumes pointer was allocated with clSVMAlloc; destroys with clSVMFree
// Also supports allocation/reset with a number, which is the number of
// elements of type T
template<typename T>
class scoped_SVM_aligned_ptr {
public:
	typedef scoped_SVM_aligned_ptr<T> this_type;

	scoped_SVM_aligned_ptr() : m_ptr(NULL) {}
	scoped_SVM_aligned_ptr(T *ptr) : m_ptr(NULL) { reset(ptr); }
	explicit scoped_SVM_aligned_ptr(cl_context ctx, size_t n) : m_ptr(NULL) { reset(ctx, n); }
	~scoped_SVM_aligned_ptr() { reset(); }

	T *get() const { return m_ptr; }
	operator T *() const { return m_ptr; }
	T *operator ->() const { return m_ptr; }
	T &operator *() const { return *m_ptr; }
	T &operator [](int index) const { return m_ptr[index]; }

	this_type &operator =(T *ptr) { reset(ptr); return *this; }

	void reset(T *ptr = NULL) { if (m_ptr) clSVMFree(m_ctx, m_ptr); m_ptr = ptr; }
	void reset(cl_context ctx, size_t n) { reset((T*)clSVMAlloc(ctx, 0, sizeof(T) * n, 0)); m_ctx = ctx; }
	T *release() { T *ptr = m_ptr; m_ptr = NULL; return ptr; }

private:
	T *m_ptr;
	cl_context m_ctx;

	// noncopyable
	scoped_SVM_aligned_ptr(const this_type &);
	this_type &operator =(const this_type &);
};
#endif /* USE_SVM_API == 1 */

} // ns aocl_utils

#endif

