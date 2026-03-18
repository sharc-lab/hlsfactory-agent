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

// --- from collectives.cu ---
#include <vector>
#include <stdexcept>
#include <cassert>
#include <cstring>
#include <iostream>

#include <hip/hip_runtime.h>
#include <mpi.h>


struct MPIGlobalState {
    // The CUDA device to run on, or -1 for CPU-only.
    int device = -1;

    // A CUDA stream (if device >= 0) initialized on the device
    hipStream_t stream;

    // Whether the global state (and MPI) has been initialized.
    bool initialized = false;
};

// MPI relies on global state for most of its internal operations, so we cannot
// design a library that avoids global state. Instead, we centralize it in this
// single global struct.
static MPIGlobalState global_state;

// Initialize the library, including MPI and if necessary the CUDA device.
// If device == -1, no GPU is used; otherwise, the device specifies which CUDA
// device should be used. All data passed to other functions must be on that device.
//
// An exception is thrown if MPI or CUDA cannot be initialized.

// Allocate a new memory buffer on CPU or GPU.

// Deallocate an allocated memory buffer.

// Copy data from one memory buffer to another on CPU or GPU.
// Both buffers must resize on the same device.

// GPU kernel for adding two vectors elementwise.

// Copy data from one memory buffer to another on CPU or GPU.
// Both buffers must resize on the same device.

// Collect the input buffer sizes from all ranks using standard MPI collectives.
// These collectives are not as efficient as the ring collectives, but they
// transmit a very small amount of data, so that is OK.
std::vector<size_t> AllgatherInputLengths(int size, size_t this_rank_length) {
    std::vector<size_t> lengths(size);
    MPI_Allgather(&this_rank_length, 1, MPI_UNSIGNED_LONG,
                  &lengths[0], 1, MPI_UNSIGNED_LONG, MPI_COMM_WORLD);
    return lengths;
}

/* Perform a ring allreduce on the data. The lengths of the data chunks passed
 * to this function must be the same across all MPI processes. The output
 * memory will be allocated and written into `output`.
 *
 * Assumes that all MPI processes are doing an allreduce of the same data,
 * with the same size.
 *
 * A ring allreduce is a bandwidth-optimal way to do an allreduce. To do the allreduce,
 * the nodes involved are arranged in a ring:
 *
 *                   .--0--.
 *                  /       \
 *                 3         1
 *                  \       /
 *                   *--2--*
 *
 *  Each node always sends to the next clockwise node in the ring, and receives
 *  from the previous one.
 *
 *  The allreduce is done in two parts: a scatter-reduce and an allgather. In
 *  the scatter reduce, a reduction is done, so that each node ends up with a
 *  chunk of the final output tensor which has contributions from all other
 *  nodes.  In the allgather, those chunks are distributed among all the nodes,
 *  so that all nodes have the entire output tensor.
 *
 *  Both of these operations are done by dividing the input tensor into N
 *  evenly sized chunks (where N is the number of nodes in the ring).
 *
 *  The scatter-reduce is done in N-1 steps. In the ith step, node j will send
 *  the (j - i)th chunk and receive the (j - i - 1)th chunk, adding it in to
 *  its existing data for that chunk. For example, in the first iteration with
 *  the ring depicted above, you will have the following transfers:
 *
 *      Segment 0:  Node 0 --> Node 1
 *      Segment 1:  Node 1 --> Node 2
 *      Segment 2:  Node 2 --> Node 3
 *      Segment 3:  Node 3 --> Node 0
 *
 *  In the second iteration, you'll have the following transfers:
 *
 *      Segment 0:  Node 1 --> Node 2
 *      Segment 1:  Node 2 --> Node 3
 *      Segment 2:  Node 3 --> Node 0
 *      Segment 3:  Node 0 --> Node 1
 *
 *  After this iteration, Node 2 has 3 of the four contributions to Segment 0.
 *  The last iteration has the following transfers:
 *
 *      Segment 0:  Node 2 --> Node 3
 *      Segment 1:  Node 3 --> Node 0
 *      Segment 2:  Node 0 --> Node 1
 *      Segment 3:  Node 1 --> Node 2
 *
 *  After this iteration, Node 3 has the fully accumulated Segment 0; Node 0
 *  has the fully accumulated Segment 1; and so on. The scatter-reduce is complete.
 *
 *  Next, the allgather distributes these fully accumululated chunks across all nodes.
 *  Communication proceeds in the same ring, once again in N-1 steps. At the ith step,
 *  node j will send chunk (j - i + 1) and receive chunk (j - i). For example, at the
 *  first iteration, the following transfers will occur:
 *
 *      Segment 0:  Node 3 --> Node 0
 *      Segment 1:  Node 0 --> Node 1
 *      Segment 2:  Node 1 --> Node 2
 *      Segment 3:  Node 2 --> Node 3
 *
 * After the first iteration, Node 0 will have a fully accumulated Segment 0
 * (from Node 3) and Segment 1. In the next iteration, Node 0 will send its
 * just-received Segment 0 onward to Node 1, and receive Segment 3 from Node 3.
 * After this has continued for N - 1 iterations, all nodes will have a the fully
 * accumulated tensor.
 *

// The ring allgather. The lengths of the data chunks passed to this function
// may differ across different devices. The output memory will be allocated and
// written into `output`.
//
// For more information on the ring allgather, read the documentation for the
// ring allreduce, which includes a ring allgather as the second stage.


// --- from main.cu ---

#include <mpi.h>
#include <hip/hip_runtime.h>

#include <stdexcept>
#include <iostream>
#include <vector>


// Test program for baidu-allreduce collectives, should be run using `mpirun`.


// --- from timer.cu ---
#ifdef _WIN32
    #include <windows.h>
#elif __APPLE__
    #include <mach/mach_time.h>
#endif


namespace timer
{

Timer::Timer()
{
    beginning = rdtsc();
    beginningS = ( beginning + 0.0 ) * 1.0e-9;
    running = true;
}

void Timer::stop()
{
    ending = rdtsc();

    endingS = ( ending + 0.0 ) * 1.0e-9;

    running = false;
}

Timer::Cycle Timer::cycles() const
{
    else
    {
    else
    {
        return endingS - beginningS;
    }
}

Timer::Second Timer::absolute() const
{
    else
    {
        return endingS;
    }
}

Timer::Cycle Timer::rdtsc()
{
#ifdef _WIN32
    Cycle cycles = 0;
    Cycle frequency = 0;
    QueryPerformanceFrequency((LARGE_INTEGER*) &frequency);
    QueryPerformanceCounter((LARGE_INTEGER*) &cycles);
    return cycles / frequency;

#elif __APPLE__
    uint64_t absolute_time = mach_absolute_time();
    mach_timebase_info_data_t info = {0,0};

    if (info.denom == 0) mach_timebase_info(&info);
    uint64_t elapsednano = absolute_time * (info.numer / info.denom);

    timespec spec;
    spec.tv_sec  = elapsednano * 1e-9;
    spec.tv_nsec = elapsednano - (spec.tv_sec * 1e9);
    return spec.tv_nsec + (Cycle)spec.tv_sec * 1e9;
#else
    timespec spec;

    clock_gettime( CLOCK_REALTIME, &spec );

    return spec.tv_nsec + (Cycle)spec.tv_sec * 1e9;
#endif

}

}


// --- from collectives.h ---
#ifndef BAIDU_ALLREDUCE_COLLECTIVES_H_
#define BAIDU_ALLREDUCE_COLLECTIVES_H_ value

#include <cstddef>

#include <mpi.h>

#define NO_DEVICE -1

/*
 * This file contains the implementation of the baidu-allreduce communication
 * collectives, and provides the following functions:
 *
 *    void InitCollectives(int device);
 *    void RingAllreduce(float* data, size_t length, float** output);
 *    void RingAllgather(float* data, size_t length, float** output);
 *
 */

// Initialize the library, including MPI and if necessary the CUDA device.
// If device == -1, no GPU is used; otherwise, the device specifies which CUDA
// device should be used. All data passed to other functions must be on that device.
void InitCollectives(int device);

// The ring allreduce. The lengths of the data chunks passed to this function
// must be the same across all MPI processes. The output memory will be
// allocated and written into `output`.
void RingAllreduce(float* data, size_t length, float** output);

// The ring allgather. The lengths of the data chunks passed to this function
// may differ across different devices. The output memory will be allocated and
// written into `output`.
void RingAllgather(float* data, size_t length, float** output);

#endif /* ifndef BAIDU_ALLREDUCE_COLLECTIVES_H_ */


// --- from timer.h ---
#pragma once

#ifndef TIMER_SMALL_SECOND
    #define TIMER_SMALL_SECOND 5e-324
#endif

#ifndef TIMER_LARGE_SECOND
    #define TIMER_LARGE_SECOND 5e324
#endif

#include <ctime>

namespace timer
{

class Timer
{
public:
    /*! \brief A type for seconds */
    typedef double Second;

    /*! \brief A type for representing clock ticks */
    typedef long long unsigned Cycle;

private:
    /*! An integer representing the value of the cycle counter when the
        last start() function was called
    */
    Cycle beginning;

    /*! An integer representing the value of the cycle counter when the
        last stop() function was called
    */
    Cycle ending;

    /*! A floating point number representing the value of the system
        clock when the last start() function was called
    */
    Second beginningS;

    /*! A floating point number representing the value of the system
        clock when the last stop() function was called
    */
    Second endingS;

    /*! Read a cycle counter using either assembly or an OS interface.
        \return a 64 bit value representing the current number of
        clock cycles since the last reset
    */
    static Cycle rdtsc();

    /*! \brief Is the Timer running? */
    bool running;

public:

    /*! \brief The constructor initializes the private variables
        and makes sure that the Timer is not running.
    */
    Timer();

    /*! A function that is used to set beginning to the value of
        the hardware Timer
    */
    void start();

    /*! A function that is used to set ending to the value of
        the hardware Timer
    */
    void stop();

    /*! A function that is used to determine the number of clock cycles
        between the last time start was called and the last time that
        end was called.
        \return the difference between ending and beginning
    */
    Cycle cycles() const;

    /*! A function that is used to determine the number of seconds
        between the last time start was called and the last time that
        end was called.
        \return the difference between ending and beginning
    */
    Second seconds() const;

    /*! \brief Get the absolute number of seconds elapsed since system
            start
        \return That time
    */
    Second absolute() const;

};

}
