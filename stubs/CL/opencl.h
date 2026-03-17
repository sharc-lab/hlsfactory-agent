/* Minimal OpenCL stub for clang syntax-only compilation */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef int cl_int;
typedef unsigned int cl_uint;
typedef long cl_long;
typedef unsigned long cl_ulong;
typedef float cl_float;
typedef double cl_double;
typedef unsigned char cl_uchar;
typedef char cl_char;
typedef unsigned short cl_ushort;
typedef short cl_short;
typedef size_t cl_size_t;

typedef void* cl_platform_id;
typedef void* cl_device_id;
typedef void* cl_context;
typedef void* cl_command_queue;
typedef void* cl_program;
typedef void* cl_kernel;
typedef void* cl_mem;
typedef void* cl_event;
typedef void* cl_sampler;

typedef cl_uint cl_bool;
typedef cl_ulong cl_bitfield;
typedef cl_bitfield cl_device_type;
typedef cl_bitfield cl_mem_flags;
typedef cl_bitfield cl_map_flags;
typedef cl_uint cl_platform_info;
typedef cl_uint cl_device_info;
typedef cl_uint cl_context_info;
typedef cl_uint cl_command_queue_info;
typedef cl_uint cl_mem_object_type;
typedef cl_uint cl_mem_info;
typedef cl_uint cl_kernel_info;
typedef cl_uint cl_program_info;
typedef cl_uint cl_program_build_info;
typedef cl_uint cl_event_info;
typedef cl_uint cl_profiling_info;
typedef cl_int cl_kernel_work_group_info;
typedef cl_uint cl_command_type;

#define CL_SUCCESS 0
#define CL_TRUE 1
#define CL_FALSE 0
#define CL_MEM_READ_WRITE (1 << 0)
#define CL_MEM_WRITE_ONLY (1 << 1)
#define CL_MEM_READ_ONLY  (1 << 2)
#define CL_MEM_USE_HOST_PTR (1 << 3)
#define CL_MEM_ALLOC_HOST_PTR (1 << 4)
#define CL_MEM_COPY_HOST_PTR (1 << 5)
#define CL_DEVICE_TYPE_ALL 0xFFFFFFFF
#define CL_DEVICE_TYPE_CPU (1 << 1)
#define CL_DEVICE_TYPE_GPU (1 << 2)
#define CL_DEVICE_TYPE_ACCELERATOR (1 << 3)
#define CL_DEVICE_TYPE_DEFAULT (1 << 0)
#define CL_QUEUE_PROFILING_ENABLE (1 << 2)
#define CL_PROGRAM_BUILD_LOG 0x1183
#define CL_BUILD_SUCCESS 0

static inline cl_int clGetPlatformIDs(cl_uint n, cl_platform_id* p, cl_uint* np) { return CL_SUCCESS; }
static inline cl_int clGetDeviceIDs(cl_platform_id p, cl_device_type t, cl_uint n, cl_device_id* d, cl_uint* nd) { return CL_SUCCESS; }
static inline cl_context clCreateContext(const void* p, cl_uint n, const cl_device_id* d, void* pfn, void* ud, cl_int* e) { return (cl_context)0; }
static inline cl_command_queue clCreateCommandQueue(cl_context c, cl_device_id d, cl_bitfield p, cl_int* e) { return (cl_command_queue)0; }
static inline cl_program clCreateProgramWithSource(cl_context c, cl_uint n, const char** s, const size_t* l, cl_int* e) { return (cl_program)0; }
static inline cl_int clBuildProgram(cl_program p, cl_uint n, const cl_device_id* d, const char* o, void* pfn, void* ud) { return CL_SUCCESS; }
static inline cl_kernel clCreateKernel(cl_program p, const char* n, cl_int* e) { return (cl_kernel)0; }
static inline cl_mem clCreateBuffer(cl_context c, cl_mem_flags f, size_t s, void* hp, cl_int* e) { return (cl_mem)0; }
static inline cl_int clSetKernelArg(cl_kernel k, cl_uint i, size_t s, const void* v) { return CL_SUCCESS; }
static inline cl_int clEnqueueNDRangeKernel(cl_command_queue q, cl_kernel k, cl_uint wd, const size_t* go, const size_t* gs, const size_t* ls, cl_uint n, const cl_event* wl, cl_event* e) { return CL_SUCCESS; }
static inline cl_int clEnqueueReadBuffer(cl_command_queue q, cl_mem b, cl_bool bl, size_t o, size_t s, void* p, cl_uint n, const cl_event* wl, cl_event* e) { return CL_SUCCESS; }
static inline cl_int clEnqueueWriteBuffer(cl_command_queue q, cl_mem b, cl_bool bl, size_t o, size_t s, const void* p, cl_uint n, const cl_event* wl, cl_event* e) { return CL_SUCCESS; }
static inline cl_int clFinish(cl_command_queue q) { return CL_SUCCESS; }
static inline cl_int clFlush(cl_command_queue q) { return CL_SUCCESS; }
static inline cl_int clReleaseMemObject(cl_mem m) { return CL_SUCCESS; }
static inline cl_int clReleaseKernel(cl_kernel k) { return CL_SUCCESS; }
static inline cl_int clReleaseProgram(cl_program p) { return CL_SUCCESS; }
static inline cl_int clReleaseCommandQueue(cl_command_queue q) { return CL_SUCCESS; }
static inline cl_int clReleaseContext(cl_context c) { return CL_SUCCESS; }
static inline cl_int clGetProgramBuildInfo(cl_program p, cl_device_id d, cl_program_build_info pi, size_t pvs, void* pv, size_t* pvsr) { return CL_SUCCESS; }
static inline cl_int clGetDeviceInfo(cl_device_id d, cl_device_info pi, size_t pvs, void* pv, size_t* pvsr) { return CL_SUCCESS; }
static inline cl_int clGetPlatformInfo(cl_platform_id p, cl_platform_info pi, size_t pvs, void* pv, size_t* pvsr) { return CL_SUCCESS; }
static inline cl_int clGetKernelWorkGroupInfo(cl_kernel k, cl_device_id d, cl_kernel_work_group_info pi, size_t pvs, void* pv, size_t* pvsr) { return CL_SUCCESS; }
static inline cl_int clWaitForEvents(cl_uint n, const cl_event* el) { return CL_SUCCESS; }
static inline cl_int clReleaseEvent(cl_event e) { return CL_SUCCESS; }
static inline void* clEnqueueMapBuffer(cl_command_queue q, cl_mem b, cl_bool bl, cl_map_flags f, size_t o, size_t s, cl_uint n, const cl_event* wl, cl_event* e, cl_int* ec) { return (void*)0; }
static inline cl_int clEnqueueUnmapMemObject(cl_command_queue q, cl_mem m, void* mp, cl_uint n, const cl_event* wl, cl_event* e) { return CL_SUCCESS; }
static inline cl_program clCreateProgramWithBinary(cl_context c, cl_uint n, const cl_device_id* d, const size_t* l, const unsigned char** b, cl_int* bs, cl_int* e) { return (cl_program)0; }

#ifdef __cplusplus
}
#endif
