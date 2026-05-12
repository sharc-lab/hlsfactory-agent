// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.

// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689

#ifndef X_HLS_STREAM_COMMON_SIM_H
#define X_HLS_STREAM_COMMON_SIM_H

/*
 * This file contains a C++ model of hls::stream.
 * It defines C simulation model.
 */
#ifndef __cplusplus

#error C++ is required to include this header file

#else

//////////////////////////////////////////////
// C level simulation models for hls::stream
//////////////////////////////////////////////
#include <queue>
#include <iostream>
#include <typeinfo>
#include <string>
#include <sstream>
#include <unordered_map>
#include <cstring>
#include <array>
#include <limits>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <condition_variable>

#ifndef _MSC_VER
#include <cxxabi.h>
#include <stdlib.h>
#endif

namespace hls {
#if !defined(__HLS_COSIM__) && defined(__VITIS_HLS__)
// We are in bcsim mode, where reads must be non-blocking
#define ALLOW_EMPTY_HLS_STREAM_READS
#ifdef X_HLS_TASK_H
#error "bcsim is not supported with hls::tasks"
#endif
#endif

template<bool DIRECTIO>
class stream_globals {
public:
  static void print_max_size() {
#ifndef DISABLE_MAX_HLS_STREAM_DEPTH_PRINT
    std::cout << "INFO [HLS SIM]: The maximum depth reached by any hls::stream() instance in the design is " << get_max_size() << std::endl;
#endif
  }

  static void incr_blocked_counter() {
    get_blocked_counter()++;
  }

  static void decr_blocked_counter() {
    get_blocked_counter()--;
  }

  static void incr_task_counter() {
    get_task_counter()++;
  }

  static void decr_task_counter() {
    get_task_counter()--;
  }

  static void start_threads() {
    // These initializations must be in ONE static function that is called elsewhere
#if defined(__HLS_COSIM__) 
    static std::thread t(deadlock_thread);
#endif
    static std::atomic_flag init_done = ATOMIC_FLAG_INIT;
    if (!init_done.test_and_set()) {
      // Perform global initialization actions once
      // Register function executed at exit
      if (!DIRECTIO)
        std::atexit(print_max_size);
#if defined(__HLS_COSIM__) 
      // Detach the thread to avoid error at end with unwaited thread
      t.detach();
#endif
    }
  }

  static std::atomic<int> &get_max_size() {
    static std::atomic<int> max_size(0);

    return max_size;
  }

  static void wait_for_all_blocked() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    while (hls::stream_globals<DIRECTIO>::get_blocked_counter() < hls::stream_globals<DIRECTIO>::get_task_counter()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }


private:
  static bool check_deadlock() {
    return get_blocked_counter() > get_task_counter();
  }

#ifndef HLS_STREAM_THREAD_UNSAFE
  static std::mutex &get_mutex() {
      static std::mutex mutex;

      return mutex;
  }
#endif

  static void deadlock_thread() {
    while (1) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      if (check_deadlock()) {
        if (!DIRECTIO) {
          if (get_task_counter()) {
            std::cout << "ERROR [HLS SIM]: deadlock detected when simulating hls::tasks." 
                    << std::endl;
            std::cout << "Execute C simulation in debug mode in the GUI and examine the"
                    << " source code location of all the blocked hls::stream::read()"
                    << " calls to debug." << std::endl;
          } else {
            std::cout << "ERROR [HLS SIM]: an hls::stream is read while empty,"
                    << " which may result in RTL simulation hanging." << std::endl;
            std::cout << "If this is not expected, execute C simulation in debug mode in"
                    << " the GUI and examine the source code location of the blocked"
                    << " hls::stream::read() call to debug." << std::endl;
            std::cout << "If this is expected, add -DALLOW_EMPTY_HLS_STREAM_READS"
                    << " to -cflags to turn this error into a warning and allow empty"
                    << " hls::stream reads to return the default value for the data type."
                    << std::endl;
          }
        } else {
          std::cout << "ERROR [HLS SIM]: an hls::directio is read while empty,"
                  << " which may result in RTL simulation hanging." << std::endl;
          std::cout << "If this is not expected, execute C simulation in debug mode in"
                  << " the GUI and examine the source code location of the blocked"
                  << " hls::directio::read() call to debug." << std::endl;
        }
        abort();
      }
    }
  }

  static std::atomic<int> &get_task_counter() {
      static std::atomic<int> task_counter(0);

      return task_counter;
  }

  static std::atomic<int> &get_blocked_counter() {
      static std::atomic<int> blocked_counter(0);

      return blocked_counter;
  }
};

} // namespace hls

#endif // __cplusplus
#endif  // X_HLS_STREAM_COMMON_SIM_H


