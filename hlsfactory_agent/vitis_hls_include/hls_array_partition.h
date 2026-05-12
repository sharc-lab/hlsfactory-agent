// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.

#ifndef __HLS_ARRAY_PARTITION_H__
#define __HLS_ARRAY_PARTITION_H__

#include <cstddef>
#include "hls_vector.h"

#ifdef __SYNTHESIS__
#define __NO_CTOR__ __attribute__((no_ctor))
#else
#define __NO_CTOR__
#endif

namespace hls {
template<typename _T, size_t _N, unsigned _K>
struct array_partition {
    _T data[_N] __NO_CTOR__;

protected:
    struct xbar {
        hls::vector<unsigned, _K> lane_to_bank;
        hls::vector<unsigned, _K> bank_to_lane;

        AP_INLINE AP_NODEBUG
        xbar(hls::vector<size_t, _K> idx, hls::vector<bool, _K> en = hls::vector<bool, _K>(true)) {
            for (unsigned lane = 0; lane < _K; ++lane) {
SYN_PRAGMA(HLS UNROLL)
                if (!en[lane])
                    continue;
                unsigned bank = idx[lane] % _K;
                lane_to_bank[lane] = bank;
                bank_to_lane[bank] = lane;
            }
#ifndef __SYNTHESIS__
            for (unsigned lane = 0; lane < _K; ++lane) {
                if (!en[lane])
                    continue;
                assert(bank_to_lane[lane_to_bank[lane]] == lane &&
                       "hls::array_partition: detected a bank collision");
            }
#endif // __SYNTHESIS__
        }
    };

public:
    AP_INLINE AP_NODEBUG
    array_partition() {
// Need to add the pragma here to ensure that the array is partitioned
// FIXME: Doesn't work at the top. (we rely on clang-tidy to solve this)
SYN_PRAGMA(HLS ARRAY_PARTITION variable=data cyclic factor=_K)
    }

#define UNROLLED_COPY(Decl, Accessor)                                          \
    AP_INLINE AP_NODEBUG Decl {                                                \
SYN_PRAGMA(HLS ARRAY_PARTITION variable=data cyclic factor=_K)                 \
        for (size_t i = 0; i < _N; ++i) {                                      \
SYN_PRAGMA(HLS UNROLL factor=_K)                                               \
            this->data[i] = Accessor[i];                                       \
        }                                                                      \
    }

    UNROLLED_COPY(array_partition(const array_partition& other), other.data)
    UNROLLED_COPY(array_partition(array_partition&& other), other.data)
    UNROLLED_COPY(array_partition& operator=(const array_partition& other), other.data)
    UNROLLED_COPY(array_partition& operator=(array_partition&& other), other.data)
    UNROLLED_COPY(array_partition& operator=(const _T (&other)[_N]), other)

#undef UNROLLED_COPY

    AP_INLINE AP_NODEBUG
    array_partition(std::initializer_list<_T> l) {
SYN_PRAGMA(HLS ARRAY_PARTITION variable=data cyclic factor=_K)
        for (size_t i = 0; i < _N && i < l.size(); ++i) {
SYN_PRAGMA(HLS UNROLL factor=_K)
            this->data[i] = l.begin()[i];
        }
    }

    /// Loads K values in parallel
    // - idx: the index to load from (the index doesn't need to align the bank, but they need to access different banks)
    // - returns: the value loaded from each index (the value will be loaded from each corresponding index)
    AP_INLINE AP_NODEBUG
    hls::vector<_T, _K> load(hls::vector<size_t, _K> idx, hls::vector<bool, _K> en = hls::vector<bool, _K>(true)) {
        xbar xbar(idx, en);
        
        hls::vector<size_t, _K> sorted_idx;
        hls::vector<bool, _K> sorted_en(false);
        hls::vector<_T, _K> sorted_vals;
        hls::vector<_T, _K> vals;
        for (unsigned lane = 0; lane < _K; ++lane) {
SYN_PRAGMA(HLS UNROLL)
            if (!en[lane])
                continue;
            sorted_idx[xbar.lane_to_bank[lane]] = idx[lane];
            sorted_en[xbar.lane_to_bank[lane]] = en[lane];
        }
        for (unsigned bank = 0; bank < _K; ++bank) {
SYN_PRAGMA(HLS UNROLL)
            // We could actually always load as the aligned_idx will never conflict
            // but we may access out-of-bound, so just in case we skip them
            if (!sorted_en[bank])
                continue;
            size_t aligned_idx = sorted_idx[bank] / _K * _K + bank;
            sorted_vals[bank] = data[aligned_idx];
        }
        for (unsigned lane = 0; lane < _K; ++lane) {
SYN_PRAGMA(HLS UNROLL)
            if (!en[lane])
                continue;
            vals[lane] = sorted_vals[xbar.lane_to_bank[lane]];
        }
        return vals;
    }

    /// Shorthand to load K consecutive values
    // - start_idx: the first index to load from (the index doesn't need to align to the fist bank)
    // Shorthand for `load({idx, idx+1, idx+2, ..., idx+K-1})`
    AP_INLINE AP_NODEBUG
    hls::vector<_T, _K> load(size_t start_idx, size_t n = _K) {
        auto idx = hls::vector<size_t, _K>::iota(start_idx);
        auto en = hls::vector<bool, _K>([n](int i) { return i < n; });
        return load(idx, en);
    }

    /// Store K values in parallel
    // - idx: the index to store at (the index doesn't need to align to the bank, but they need to access different bank)
    // - vals: the values to store at each index (the value will be stored at each corresponding index)
    // - we: write-enable mask for each index (will default to always true if unspecified)
    AP_INLINE AP_NODEBUG
    void store(hls::vector<size_t, _K> idx, hls::vector<_T, _K> vals, hls::vector<bool, _K> we = hls::vector<bool, _K>(true)) {
        xbar xbar(idx, we);

        hls::vector<size_t, _K> sorted_idx;
        hls::vector<_T, _K> sorted_vals __NO_CTOR__;
        hls::vector<bool, _K> sorted_we(false);
        for (unsigned lane = 0; lane < _K; ++lane) {
SYN_PRAGMA(HLS UNROLL)
            if (!we[lane])
                continue;
            sorted_idx[xbar.lane_to_bank[lane]] = idx[lane];
            sorted_vals[xbar.lane_to_bank[lane]] = vals[lane];
            sorted_we[xbar.lane_to_bank[lane]] = we[lane];
        }
        for (unsigned bank = 0; bank < _K; ++bank) {
SYN_PRAGMA(HLS UNROLL)
            if (!sorted_we[bank])
                continue;
            size_t aligned_idx = sorted_idx[bank] / _K * _K + bank;
            if (sorted_we[bank]) {
                data[aligned_idx] = sorted_vals[bank];
            }
        }
    }

    /// Shorthand to store K consecutive values
    // - start_idx: the first index to store at (the index doesn't need to align to the first bank)
    // - data: the values to store at each index
    // - we: write-enable mask for each index
    // Shorthand for `store({idx, idx+1, idx+2, ..., idx+K-1}, data, we)`
    AP_INLINE AP_NODEBUG
    void store(size_t start_idx, hls::vector<_T, _K> data, hls::vector<bool, _K> we = hls::vector<bool, _K>(true)) {
        auto idx = hls::vector<size_t, _K>::iota(start_idx);
        store(idx, data, we);
    }

    /// Shorthand to store up-to K consecutive values
    // - start_idx: the first index to store at (the index doesn't need to align to the first bank)
    // - data: the values to store at each index
    // - n: only write-enable the first n values
    // Shorthand for `store(start_idx, data, {true, ..., true, false, ..., false})` with n first write-enable to true, K-n last write-enable to false
    AP_INLINE AP_NODEBUG
    void store(size_t start_idx, hls::vector<_T, _K> data, size_t n = _K) {
        auto idx = hls::vector<size_t, _K>::iota(start_idx);
        auto we = hls::vector<bool, _K>([n](int i) { return i < n; });
        store(idx, data, we);
    }
};
} // namespace hls

#endif // __HLS_ARRAY_PARTITION_H__
