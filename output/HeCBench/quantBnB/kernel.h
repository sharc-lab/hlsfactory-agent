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
#ifndef BLOCK_DIM_Z
#define BLOCK_DIM_Z 1
#endif

// --- from main.cu ---
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

template <int STOCHASTIC>
uint8_t
dQuantize(float* smem_code, const float rand, float x)
{
    int pivot = 127;
    int upper_pivot = 255;
    int lower_pivot = 0;

    float lower = -1.0f;
    float upper = 1.0f;

    float val = smem_code[pivot];

    if(upper_pivot == 255)
        upper = smem_code[upper_pivot];
    if(lower_pivot == 0)
        lower = smem_code[lower_pivot];

    else
    {
      else
      {
        float dist_to_lower = fabsf(lower-x);
        float dist_full = val-lower;
        if(rand >= dist_to_lower/dist_full) return lower_pivot;
        else return pivot;
      }
    }
}

#define NUM 4

template<int TH, int BLOCK_SIZE>



// --- from block_load.h ---
/******************************************************************************
 * Copyright (c) 2011, Duane Merrill.  All rights reserved.
 * Copyright (c) 2011-2016, NVIDIA CORPORATION.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/**
 * \file
 * Operations for reading linear tiles of data into the GPU thread block.
 */

#pragma once

#include <iterator>
#include <type_traits>

template <
    typename        InputT,
    int             ITEMS_PER_THREAD,
    typename        InputIteratorT>
inline void LoadDirectBlocked(
    int             linear_tid,
    InputIteratorT  block_itr,
    InputT          (&items)[ITEMS_PER_THREAD])
{
    #pragma unroll
    for (int ITEM = 0; ITEM < ITEMS_PER_THREAD; ITEM++)
    {
        items[ITEM] = block_itr[(linear_tid * ITEMS_PER_THREAD) + ITEM];
    }
}

template <
    typename        InputT,
    int             ITEMS_PER_THREAD,
    typename        InputIteratorT>
inline void LoadDirectBlocked(
    int             linear_tid,
    InputIteratorT  block_itr,
    InputT          (&items)[ITEMS_PER_THREAD],
    int             valid_items)                ///< [in] Number of valid items to load
{

    #pragma unroll
    for (int ITEM = 0; ITEM < ITEMS_PER_THREAD; ITEM++)
    {
        if ((linear_tid * ITEMS_PER_THREAD) + ITEM < valid_items)
        {
            items[ITEM] = block_itr[(linear_tid * ITEMS_PER_THREAD) + ITEM];
        }
    }
}

template <
    typename        InputT,
    typename        DefaultT,
    int             ITEMS_PER_THREAD,
    typename        InputIteratorT>
inline void LoadDirectBlocked(
    int             linear_tid,
    InputIteratorT  block_itr,
    InputT          (&items)[ITEMS_PER_THREAD],
    int             valid_items,
    DefaultT        oob_default)
{
    #pragma unroll
    for (int ITEM = 0; ITEM < ITEMS_PER_THREAD; ITEM++)
        items[ITEM] = oob_default;

    LoadDirectBlocked(linear_tid, block_itr, items, valid_items);
}

//-----------------------------------------------------------------------------
// Generic BlockLoad abstraction
//-----------------------------------------------------------------------------

enum BlockLoadAlgorithm
{
    BLOCK_LOAD_DIRECT
};

template <
    typename            InputT,
    int                 BLOCK_DIM_X,
    int                 ITEMS_PER_THREAD,
    BlockLoadAlgorithm  ALGORITHM           = BLOCK_LOAD_DIRECT,
    int                 BLOCK_DIM_Y         = 1,
    int                 BLOCK_DIM_Z         = 1>
class BlockLoad
{
private:

    /******************************************************************************
     * Constants and typed definitions
     ******************************************************************************/

    enum
    {
        /// The thread block size in threads
        BLOCK_THREADS = BLOCK_DIM_X * BLOCK_DIM_Y * BLOCK_DIM_Z,
    };

    /******************************************************************************
     * Algorithmic variants
     ******************************************************************************/

    /// Load helper
    template <BlockLoadAlgorithm _POLICY, int DUMMY>
    struct LoadInternal;

    /**
     * BLOCK_LOAD_DIRECT specialization of load helper
     */
    template <int DUMMY>
    struct LoadInternal<BLOCK_LOAD_DIRECT, DUMMY>
    {
        /// Shared memory storage layout type
        typedef NullType TempStorage;

        /// Linear thread-id
        int linear_tid;

        /// Constructor
        inline LoadInternal(
            TempStorage &/*temp_storage*/,
            int linear_tid)
        :
            linear_tid(linear_tid)
        {}

        /// Load a linear segment of items from memory
        template <typename InputIteratorT>
        inline void Load(
            InputIteratorT  block_itr,
            InputT          (&items)[ITEMS_PER_THREAD])
        {
            LoadDirectBlocked(linear_tid, block_itr, items);
        }

        /// Load a linear segment of items from memory, guarded by range
        template <typename InputIteratorT>
        inline void Load(
            InputIteratorT  block_itr,
            InputT          (&items)[ITEMS_PER_THREAD],
            int             valid_items)
        {
            LoadDirectBlocked(linear_tid, block_itr, items, valid_items);
        }

        /// Load a linear segment of items from memory, guarded by range, with a fall-back assignment of out-of-bound elements
        template <typename InputIteratorT, typename DefaultT>
        inline void Load(
            InputIteratorT  block_itr,
            InputT          (&items)[ITEMS_PER_THREAD],
            int             valid_items,
            DefaultT        oob_default)
        {
            LoadDirectBlocked(linear_tid, block_itr, items, valid_items, oob_default);
        }

    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /// Internal load implementation to use
    typedef LoadInternal<ALGORITHM, 0> InternalLoad;

    /// Shared memory storage layout type
    typedef typename InternalLoad::TempStorage _TempStorage;

    /******************************************************************************
     * Utility methods
     ******************************************************************************/

    /// Internal storage allocator
    inline _TempStorage& PrivateStorage()
    {
        _TempStorage private_storage;
        return private_storage;
    }

    /******************************************************************************
     * Thread fields
     ******************************************************************************/

    /// Thread reference to shared storage
    _TempStorage &temp_storage;

    /// Linear thread-id
    int linear_tid;

public:

    struct TempStorage : Uninitialized<_TempStorage> {};

    /******************************************************************//**
     * \name Collective constructors
     *********************************************************************/

    /**
     * \brief Collective constructor using a private static allocation of shared memory as temporary storage.
     */
    inline BlockLoad()
    :
        temp_storage(PrivateStorage()),
        linear_tid(RowMajorTid(BLOCK_DIM_X, BLOCK_DIM_Y, BLOCK_DIM_Z))
    {}

    /**
     * \brief Collective constructor using the specified memory allocation as temporary storage.
     */
    inline BlockLoad(
        TempStorage &temp_storage)             ///< [in] Reference to memory allocation having layout type TempStorage
    :
        temp_storage(temp_storage.Alias()),
        linear_tid(RowMajorTid(BLOCK_DIM_X, BLOCK_DIM_Y, BLOCK_DIM_Z))
    {}

    /******************************************************************//**
     * \name Data movement
     *********************************************************************/

    template <typename InputIteratorT>
    inline void Load(
        InputIteratorT  block_itr,                  ///< [in] The thread block's base input iterator for loading from
        InputT          (&items)[ITEMS_PER_THREAD]) ///< [out] Data to load
    {
        InternalLoad(temp_storage, linear_tid).Load(block_itr, items);
    }

    template <typename InputIteratorT>
    inline void Load(
        InputIteratorT  block_itr,                  ///< [in] The thread block's base input iterator for loading from
        InputT          (&items)[ITEMS_PER_THREAD], ///< [out] Data to load
        int             valid_items)                ///< [in] Number of valid items to load
    {
        InternalLoad(temp_storage, linear_tid).Load(block_itr, items, valid_items);
    }

    template <typename InputIteratorT, typename DefaultT>
    inline void Load(
        InputIteratorT  block_itr,                  ///< [in] The thread block's base input iterator for loading from
        InputT          (&items)[ITEMS_PER_THREAD], ///< [out] Data to load
        int             valid_items,                ///< [in] Number of valid items to load
        DefaultT        oob_default)                ///< [in] Default value to assign out-of-bound items
    {
        InternalLoad(temp_storage, linear_tid).Load(block_itr, items, valid_items, oob_default);
    }

};



// --- from block_store.h ---
/******************************************************************************
 * Copyright (c) 2011, Duane Merrill.  All rights reserved.
 * Copyright (c) 2011-2018, NVIDIA CORPORATION.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/**
 * \file
 * Operations for writing linear segments of data from the CUDA thread block
 */

#pragma once

#include <iterator>
#include <type_traits>

template <
    typename            T,
    int                 ITEMS_PER_THREAD,
    typename            OutputIteratorT>
inline void StoreDirectBlocked(
    int                 linear_tid,
    OutputIteratorT     block_itr,
    T                   (&items)[ITEMS_PER_THREAD])
{
    OutputIteratorT thread_itr = block_itr + (linear_tid * ITEMS_PER_THREAD);

    // Store directly in thread-blocked order
    #pragma unroll
    for (int ITEM = 0; ITEM < ITEMS_PER_THREAD; ITEM++)
    {
        thread_itr[ITEM] = items[ITEM];
    }
}

template <
    typename            T,
    int                 ITEMS_PER_THREAD,
    typename            OutputIteratorT>
inline void StoreDirectBlocked(
    int                 linear_tid,
    OutputIteratorT     block_itr,
    T                   (&items)[ITEMS_PER_THREAD],
    int                 valid_items)
{
    OutputIteratorT thread_itr = block_itr + (linear_tid * ITEMS_PER_THREAD);

    // Store directly in thread-blocked order
    #pragma unroll
    for (int ITEM = 0; ITEM < ITEMS_PER_THREAD; ITEM++)
    {
        if (ITEM + (linear_tid * ITEMS_PER_THREAD) < valid_items)
        {
            thread_itr[ITEM] = items[ITEM];
        }
    }
}

//-----------------------------------------------------------------------------
// Generic BlockStore abstraction
//-----------------------------------------------------------------------------

enum BlockStoreAlgorithm
{
    BLOCK_STORE_DIRECT,
};

template <
    typename                T,
    int                     BLOCK_DIM_X,
    int                     ITEMS_PER_THREAD,
    BlockStoreAlgorithm     ALGORITHM           = BLOCK_STORE_DIRECT,
    int                     BLOCK_DIM_Y         = 1,
    int                     BLOCK_DIM_Z         = 1>
class BlockStore
{
private:
    /******************************************************************************
     * Constants and typed definitions
     ******************************************************************************/

    /// Constants
    enum
    {
        /// The thread block size in threads
        BLOCK_THREADS = BLOCK_DIM_X * BLOCK_DIM_Y * BLOCK_DIM_Z,
    };

    /******************************************************************************
     * Algorithmic variants
     ******************************************************************************/

    /// Store helper
    template <BlockStoreAlgorithm _POLICY, int DUMMY>
    struct StoreInternal;

    /**
     * BLOCK_STORE_DIRECT specialization of store helper
     */
    template <int DUMMY>
    struct StoreInternal<BLOCK_STORE_DIRECT, DUMMY>
    {
        /// Shared memory storage layout type
        typedef NullType TempStorage;

        /// Linear thread-id
        int linear_tid;

        /// Constructor
        inline StoreInternal(
            TempStorage &/*temp_storage*/,
            int linear_tid)
        :
            linear_tid(linear_tid)
        {}

        /// Store items into a linear segment of memory
        template <typename OutputIteratorT>
        inline void Store(
            OutputIteratorT     block_itr,                  ///< [in] The thread block's base output iterator for storing to
            T                   (&items)[ITEMS_PER_THREAD]) ///< [in] Data to store
        {
            StoreDirectBlocked(linear_tid, block_itr, items);
        }

        /// Store items into a linear segment of memory, guarded by range
        template <typename OutputIteratorT>
        inline void Store(
            OutputIteratorT     block_itr,                  ///< [in] The thread block's base output iterator for storing to
            T                   (&items)[ITEMS_PER_THREAD], ///< [in] Data to store
            int                 valid_items)                ///< [in] Number of valid items to write
        {
            StoreDirectBlocked(linear_tid, block_itr, items, valid_items);
        }
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /// Internal load implementation to use
    typedef StoreInternal<ALGORITHM, 0> InternalStore;

    /// Shared memory storage layout type
    typedef typename InternalStore::TempStorage _TempStorage;

    /******************************************************************************
     * Utility methods
     ******************************************************************************/

    /// Internal storage allocator
    inline _TempStorage& PrivateStorage()
    {
        _TempStorage private_storage;
        return private_storage;
    }

    /******************************************************************************
     * Thread fields
     ******************************************************************************/

    /// Thread reference to shared storage
    _TempStorage &temp_storage;

    /// Linear thread-id
    int linear_tid;

public:

    /// \smemstorage{BlockStore}
    struct TempStorage : Uninitialized<_TempStorage> {};

    /******************************************************************//**
     * \name Collective constructors
     *********************************************************************/

    /**
     * \brief Collective constructor using a private static allocation of shared memory as temporary storage.
     */
    inline BlockStore()
    :
        temp_storage(PrivateStorage()),
        linear_tid(RowMajorTid(BLOCK_DIM_X, BLOCK_DIM_Y, BLOCK_DIM_Z))
    {}

    /**
     * \brief Collective constructor using the specified memory allocation as temporary storage.
     */
    inline BlockStore(
        TempStorage &temp_storage)             ///< [in] Reference to memory allocation having layout type TempStorage
    :
        temp_storage(temp_storage.Alias()),
        linear_tid(RowMajorTid(BLOCK_DIM_X, BLOCK_DIM_Y, BLOCK_DIM_Z))
    {}

    template <typename OutputIteratorT>
    inline void Store(
        OutputIteratorT     block_itr,                  ///< [out] The thread block's base output iterator for storing to
        T                   (&items)[ITEMS_PER_THREAD]) ///< [in] Data to store
    {
        InternalStore(temp_storage, linear_tid).Store(block_itr, items);
    }

    template <typename OutputIteratorT>
    inline void Store(
        OutputIteratorT     block_itr,                  ///< [out] The thread block's base output iterator for storing to
        T                   (&items)[ITEMS_PER_THREAD], ///< [in] Data to store
        int                 valid_items)                ///< [in] Number of valid items to write
    {
        InternalStore(temp_storage, linear_tid).Store(block_itr, items, valid_items);
    }
};


// --- from code.h ---
float code[] = {-0.992968738079071, -0.9789062738418579, -0.96484375, -0.9507812261581421, -0.936718761920929, -0.922656238079071, -0.9085937738418579, -0.89453125, -0.8804687261581421, -0.866406261920929, -0.852343738079071, -0.8382812738418579, -0.82421875, -0.8101562261581421, -0.796093761920929, -0.782031238079071, -0.7679687738418579, -0.75390625, -0.7398437261581421, -0.725781261920929, -0.7117187976837158, -0.6976562738418579, -0.68359375, -0.6695312261581421, -0.655468761920929, -0.6414062976837158, -0.6273437738418579, -0.61328125, -0.5992187261581421, -0.585156261920929, -0.5710937976837158, -0.5570312738418579, -0.54296875, -0.5289062261581421, -0.5148437023162842, -0.500781238079071, -0.48671871423721313, -0.47265625, -0.4585937261581421, -0.44453126192092896, -0.43046873807907104, -0.4164062440395355, -0.40234375, -0.3882812261581421, -0.37421876192092896, -0.36015623807907104, -0.3460937440395355, -0.33203125, -0.3179687261581421, -0.30390626192092896, -0.28984373807907104, -0.2757812738418579, -0.26171875, -0.24765624105930328, -0.23359374701976776, -0.21953125298023224, -0.20546874403953552, -0.19140625, -0.17734375596046448, -0.16328124701976776, -0.14921875298023224, -0.13515624403953552, -0.12109375, -0.10703125596046448, -0.09859374910593033, -0.09578125923871994, -0.09296875447034836, -0.09015624970197678, -0.08734375238418579, -0.08453124761581421, -0.08171875774860382, -0.07890625298023224, -0.07609374821186066, -0.07328125089406967, -0.07046874612569809, -0.0676562562584877, -0.06484375149011612, -0.062031250447034836, -0.05921875312924385, -0.05640624836087227, -0.053593751043081284, -0.05078125, -0.047968748956918716, -0.04515625163912773, -0.04234374687075615, -0.039531249552965164, -0.03671875223517418, -0.033906251192092896, -0.031093750149011612, -0.028281250968575478, -0.025468749925494194, -0.02265625074505806, -0.019843751564621925, -0.017031250521540642, -0.014218750409781933, -0.011406250298023224, -0.009718749672174454, -0.009156249463558197, -0.008593750186264515, -0.008031249977648258, -0.0074687497690320015, -0.006906250026077032, -0.006343749817460775, -0.005781250074505806, -0.0052187503315508366, -0.0046562496572732925, -0.004093749914318323, -0.0035312497057020664, -0.002968749962747097, -0.002406249986961484, -0.001843750011175871, -0.001281249918974936, -0.0009437500848434865, -0.0008312499849125743, -0.0007187500596046448, -0.0006062500760890543, -0.000493750034365803, -0.0003812500217463821, -0.0002687500382307917, -0.00015625001105945557, -8.874999912222847e-05, -6.625000241911039e-05, -4.374999844003469e-05, -2.1249998098937795e-05, -7.749999895168003e-06, -3.250000190746505e-06, -5.500000384017767e-07, 0, 5.500000384017767e-07, 3.250000190746505e-06, 7.749999895168003e-06, 2.1249998098937795e-05, 4.374999844003469e-05, 6.625000241911039e-05, 8.874999912222847e-05, 0.00015625001105945557, 0.0002687500382307917, 0.0003812500217463821, 0.000493750034365803, 0.0006062500760890543, 0.0007187500596046448, 0.0008312499849125743, 0.0009437500848434865, 0.001281249918974936, 0.001843750011175871, 0.002406249986961484, 0.002968749962747097, 0.0035312497057020664, 0.004093749914318323, 0.0046562496572732925, 0.0052187503315508366, 0.005781250074505806, 0.006343749817460775, 0.006906250026077032, 0.0074687497690320015, 0.008031249977648258, 0.008593750186264515, 0.009156249463558197, 0.009718749672174454, 0.011406250298023224, 0.014218750409781933, 0.017031250521540642, 0.019843751564621925, 0.02265625074505806, 0.025468749925494194, 0.028281250968575478, 0.031093750149011612, 0.033906251192092896, 0.03671875223517418, 0.039531249552965164, 0.04234374687075615, 0.04515625163912773, 0.047968748956918716, 0.05078125, 0.053593751043081284, 0.05640624836087227, 0.05921875312924385, 0.062031250447034836, 0.06484375149011612, 0.0676562562584877, 0.07046874612569809, 0.07328125089406967, 0.07609374821186066, 0.07890625298023224, 0.08171875774860382, 0.08453124761581421, 0.08734375238418579, 0.09015624970197678, 0.09296875447034836, 0.09578125923871994, 0.09859374910593033, 0.10703125596046448, 0.12109375, 0.13515624403953552, 0.14921875298023224, 0.16328124701976776, 0.17734375596046448, 0.19140625, 0.20546874403953552, 0.21953125298023224, 0.23359374701976776, 0.24765624105930328, 0.26171875, 0.2757812738418579, 0.28984373807907104, 0.30390626192092896, 0.3179687261581421, 0.33203125, 0.3460937440395355, 0.36015623807907104, 0.37421876192092896, 0.3882812261581421, 0.40234375, 0.4164062440395355, 0.43046873807907104, 0.44453126192092896, 0.4585937261581421, 0.47265625, 0.48671871423721313, 0.500781238079071, 0.5148437023162842, 0.5289062261581421, 0.54296875, 0.5570312738418579, 0.5710937976837158, 0.585156261920929, 0.5992187261581421, 0.61328125, 0.6273437738418579, 0.6414062976837158, 0.655468761920929, 0.6695312261581421, 0.68359375, 0.6976562738418579, 0.7117187976837158, 0.725781261920929, 0.7398437261581421, 0.75390625, 0.7679687738418579, 0.782031238079071, 0.796093761920929, 0.8101562261581421, 0.82421875, 0.8382812738418579, 0.852343738079071, 0.866406261920929, 0.8804687261581421, 0.89453125, 0.9085937738418579, 0.922656238079071, 0.936718761920929, 0.9507812261581421, 0.96484375, 0.9789062738418579, 0.992968738079071, 1.0};


// --- from utils.h ---
#pragma once

#define LOG_WARP_THREADS(unused) (5)
#define WARP_THREADS(unused) (1 << LOG_WARP_THREADS(0))
#define PTX_WARP_THREADS        WARP_THREADS(0)
#define PTX_LOG_WARP_THREADS    LOG_WARP_THREADS(0)

struct NullType
{
    //using value_type = NullType;

    template <typename T>
    inline NullType& operator =(const T&) { return *this; }

    inline bool operator ==(const NullType&) { return true; }

    inline bool operator !=(const NullType&) { return false; }
};

/// Structure alignment
template <typename T>
struct AlignBytes
{
    struct Pad
    {
        T       val;
        char    byte;
    };

    enum
    {
        /// The "true CUDA" alignment of T in bytes
        ALIGN_BYTES = sizeof(Pad) - sizeof(T)
    };

    /// The "truly aligned" type
    typedef T Type;
};

#define __ALIGN_BYTES(t, b)         \
    template <> struct AlignBytes<t>    \
    { enum { ALIGN_BYTES = b }; typedef __align__(b) t Type; };

__ALIGN_BYTES(short4, 8)
__ALIGN_BYTES(ushort4, 8)
__ALIGN_BYTES(int2, 8)
__ALIGN_BYTES(uint2, 8)
__ALIGN_BYTES(long long, 8)
__ALIGN_BYTES(unsigned long long, 8)
__ALIGN_BYTES(float2, 8)
__ALIGN_BYTES(double, 8)
__ALIGN_BYTES(long2, 16)
__ALIGN_BYTES(ulong2, 16)
__ALIGN_BYTES(int4, 16)
__ALIGN_BYTES(uint4, 16)
__ALIGN_BYTES(float4, 16)
__ALIGN_BYTES(long4, 16)
__ALIGN_BYTES(ulong4, 16)
__ALIGN_BYTES(longlong2, 16)
__ALIGN_BYTES(ulonglong2, 16)
__ALIGN_BYTES(double2, 16)
__ALIGN_BYTES(longlong4, 16)
__ALIGN_BYTES(ulonglong4, 16)
__ALIGN_BYTES(double4, 16)

template <typename T> struct AlignBytes<volatile T> : AlignBytes<T> {};
template <typename T> struct AlignBytes<const T> : AlignBytes<T> {};
template <typename T> struct AlignBytes<const volatile T> : AlignBytes<T> {};

template <bool Test, class T1, class T2>
using conditional_t = typename std::conditional<Test, T1, T2>::type;

/// Unit-words of data movement
template <typename T>
struct UnitWord
{
    enum {
        ALIGN_BYTES = AlignBytes<T>::ALIGN_BYTES
    };

    template <typename Unit>
    struct IsMultiple
    {
        enum {
            UNIT_ALIGN_BYTES    = AlignBytes<Unit>::ALIGN_BYTES,
            IS_MULTIPLE         = (sizeof(T) % sizeof(Unit) == 0) && (int(ALIGN_BYTES) % int(UNIT_ALIGN_BYTES) == 0)
        };
    };

    /// Biggest shuffle word that T is a whole multiple of and is not larger than
    /// the alignment of T
    using ShuffleWord = conditional_t<
      IsMultiple<int>::IS_MULTIPLE,
      unsigned int,
      conditional_t<IsMultiple<short>::IS_MULTIPLE,
                                 unsigned short,
                                 unsigned char>>;

    /// Biggest volatile word that T is a whole multiple of and is not larger than
    /// the alignment of T
    using VolatileWord =
      conditional_t<IsMultiple<long long>::IS_MULTIPLE,
                                 unsigned long long,
                                 ShuffleWord>;

    /// Biggest memory-access word that T is a whole multiple of and is not larger
    /// than the alignment of T
    using DeviceWord =
      conditional_t<IsMultiple<longlong2>::IS_MULTIPLE,
                                 ulonglong2,
                                 VolatileWord>;
};

template <typename T>
struct Uninitialized
{
    /// Biggest memory-access word that T is a whole multiple of and is not larger than the alignment of T
    typedef typename UnitWord<T>::DeviceWord DeviceWord;

    static constexpr std::size_t DATA_SIZE = sizeof(T);
    static constexpr std::size_t WORD_SIZE = sizeof(DeviceWord);
    static constexpr std::size_t WORDS = DATA_SIZE / WORD_SIZE;

    /// Backing storage
    DeviceWord storage[WORDS];

    /// Alias
    inline T& Alias()
    {
        return reinterpret_cast<T&>(*this);
    }
};

/**
 * \brief Returns the row-major linear thread identifier for a multidimensional thread block
 */
inline int RowMajorTid(int block_dim_x, int block_dim_y, int block_dim_z)
{
    return ((block_dim_z == 1) ? 0 : (_tid_z * block_dim_x * block_dim_y)) +
            ((block_dim_y == 1) ? 0 : (_tid_y * block_dim_x)) +
            _tid_x;
}
