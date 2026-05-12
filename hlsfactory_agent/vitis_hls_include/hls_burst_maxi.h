// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.

// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
#ifndef X_HLS_BURST_MAXI_SIM_H
#define X_HLS_BURST_MAXI_SIM_H

/*
 * This file contains a C++ simulation model of hls::burst_maxi
 */
#ifndef __cplusplus

#error C++ is required to include this header file

#else

#include <list>
#include <map>
#include <iostream>
#include <assert.h>
#include "ap_int.h"

namespace hls {

struct MAXIAccessRecord {
  unsigned read_disp;
  unsigned write_disp;
  // (byte offset, byte length, sizeof(ACCESS_TYPE))
  std::list<std::tuple<size_t, unsigned, unsigned>> ReadQ;
  std::list<std::tuple<size_t, unsigned, unsigned>> WriteQ;
  std::list<std::tuple<size_t, unsigned, unsigned>> WriteRespQ;
  ~MAXIAccessRecord() {
    if (!WriteRespQ.empty()) {
#ifdef ALLOW_INSUFFICIENT_MAXI_RESPONSES
      std::cerr << "Warning: Insufficient burst_maxi::write_response calls. It may lead to potential dead lock if the number of write_response calls doesn't match the number of write_request calls." << std::endl;
#else
      std::cerr << "Error: Insufficient burst_maxi::write_response calls. It may lead to potential dead lock if the number of write_response calls doesn't match the number of write_request calls." << std::endl;
      abort();
#endif
    }

    if (!ReadQ.empty()) {
#ifdef ALLOW_INSUFFICIENT_MAXI_READS
      std::cerr << "Warning: Insufficient burst_maxi::read calls. It may lead to potential dead lock if the number of read calls doesn't match the number of data fetched by read_request." << std::endl;
#else
      std::cerr << "Error: Insufficient burst_maxi::read calls. It may lead to potential dead lock if the number of read calls doesn't match the number of data fetched by read_request." << std::endl;
      abort();
#endif
    }

    if (!WriteQ.empty()) {
#ifdef ALLOW_INSUFFICIENT_MAXI_WRITES
      std::cerr << "Warning: Insufficient burst_maxi::write calls. It may lead to potential dead lock if the number of write calls doesn't match the number of data reserved by write_request." << std::endl;
#else
      std::cerr << "Error: Insufficient burst_maxi::write calls. It may lead to potential dead lock if the number of write calls doesn't match the number of data reserved by write_request." << std::endl;
      abort();
#endif
    }
  }
};

template<typename T>
class burst_maxi {
public:
  burst_maxi(T *p) : Ptr(p) {
    unsigned bitwidth = sizeof(T) * 8;
    assert(bitwidth != 0 && !(bitwidth & (bitwidth - 1)) &&
           "Error: bit width of hls::burst_maxi is not poower-of-2.");
    // Reset the MAXI access record to this pointer
    MAXIAccessRecord &R = getMAXIPointer2AccessRecordMap()[p];
    R.read_disp = 0;
    R.write_disp = 0;
    R.ReadQ.clear();
    R.WriteQ.clear();
    R.WriteRespQ.clear();
  }

  void read_request(size_t offset, unsigned len) {
    if (len == 0)
      return;
    MAXIAccessRecord &R = getMAXIPointer2AccessRecordMap()[Ptr];
    size_t byte_offset = offset * sizeof(T);
    unsigned byte_len = len * sizeof(T);
    unsigned element_size = sizeof(T);
    R.ReadQ.push_back(std::make_tuple(byte_offset, byte_len, element_size));
    std::list<std::tuple<size_t, unsigned, unsigned>> CurrentWriteQ = R.WriteQ;
    CurrentWriteQ.insert(CurrentWriteQ.end(), 
                         R.WriteRespQ.begin(), R.WriteRespQ.end());
    for (auto Pair : CurrentWriteQ) {
      if (overlap(byte_offset, byte_len, std::get<0>(Pair), std::get<1>(Pair))) {
        std::cerr << "Error: MAXI read request(byte offset = " << byte_offset << ", byte len = " << byte_len << ") overlaps with previous write request(byte offset = " << std::get<0>(Pair) << ", byte len = " << std::get<1>(Pair) << ")." << std::endl;
        abort();
      }
    }
  }

  template<typename ACCESS_TYPE = T>
  void read_request_unaligned(size_t byte_offset, unsigned num_of_access_elements) {
    static_assert(sizeof(ACCESS_TYPE ) <= sizeof(T));
    if (num_of_access_elements == 0)
      return;
    MAXIAccessRecord &R = getMAXIPointer2AccessRecordMap()[Ptr];
    unsigned byte_len = num_of_access_elements * sizeof(ACCESS_TYPE);
    unsigned element_size = sizeof(ACCESS_TYPE);
    R.ReadQ.push_back(std::make_tuple(byte_offset, byte_len, element_size));
    std::list<std::tuple<size_t, unsigned, unsigned>> CurrentWriteQ = R.WriteQ;
    CurrentWriteQ.insert(CurrentWriteQ.end(), 
                         R.WriteRespQ.begin(), R.WriteRespQ.end());
    for (auto Pair : CurrentWriteQ) {
      if (overlap(byte_offset, byte_len, std::get<0>(Pair), std::get<1>(Pair))) {
        std::cerr << "Error: MAXI read request(byte offset = " << byte_offset << ", byte len = " << byte_len << ") overlaps with previous write request(byte offset = " << std::get<0>(Pair) << ", byte len = " << std::get<1>(Pair) << ")." << std::endl;
        abort();
      }
    }
  }

  template<typename ACCESS_TYPE = T>
  ACCESS_TYPE read() {
    static_assert(sizeof(ACCESS_TYPE) <= sizeof(T));
    MAXIAccessRecord &R = getMAXIPointer2AccessRecordMap()[Ptr];
    assert(!R.ReadQ.empty() && "Error: MAXI read without request."); 
    auto tuple = R.ReadQ.front();

    size_t byte_offset = std::get<0>(tuple);
    unsigned byte_len = std::get<1>(tuple);
    unsigned element_size = std::get<2>(tuple);

    if (sizeof(ACCESS_TYPE) != element_size) {
      std::cerr << "Error: The data size of read (" 
      << sizeof(ACCESS_TYPE) << " bytes) is different from the data size of read_request (" 
      << element_size << " bytes)." << std::endl;
      abort();
    }
    char *src_p = reinterpret_cast<char *>(Ptr);
    size_t cur_byte_offer = byte_offset + R.read_disp;
    ACCESS_TYPE V;
    memcpy(&V, src_p + cur_byte_offer, sizeof(ACCESS_TYPE));
    R.read_disp += sizeof(ACCESS_TYPE);
    if (R.read_disp > byte_len) {
      std::cerr << "Error: The read element was outside the requested range." << std::endl;
      abort();
    }
    if (R.read_disp == byte_len) {
      R.read_disp = 0;
      R.ReadQ.pop_front();
    }
    return V;     
  }

  void write_request(size_t offset, unsigned len) {
    MAXIAccessRecord &R = getMAXIPointer2AccessRecordMap()[Ptr];
    size_t byte_offset = offset * sizeof(T);
    unsigned byte_len = len * sizeof(T);
    unsigned element_size = sizeof(T);
    if (len == 0) {
      R.WriteRespQ.push_back(std::make_tuple(byte_offset, 0, element_size));
      return;
    }
    for (auto tuple : R.ReadQ) {
      if (overlap(byte_offset, byte_len, std::get<0>(tuple), std::get<1>(tuple))) {
        std::cerr << "Error: MAXI write request(byte offset = " << byte_offset << ", type len = " << byte_len << ") overlaps with previous read request(byte offset = " << std::get<0>(tuple) << ", byte len = " << std::get<1>(tuple) << ")." << std::endl;
        abort();
      }
    }
    R.WriteQ.push_back(std::make_tuple(byte_offset, byte_len, element_size));
  }

  template<typename ACCESS_TYPE = T>
  void write_request_unaligned(size_t byte_offset, unsigned num_of_access_elements) {
    static_assert(sizeof(ACCESS_TYPE ) <= sizeof(T));
    MAXIAccessRecord &R = getMAXIPointer2AccessRecordMap()[Ptr];
 
    unsigned byte_len = num_of_access_elements * sizeof(ACCESS_TYPE);
    unsigned element_size = sizeof(ACCESS_TYPE);
    if (num_of_access_elements == 0) {
      R.WriteRespQ.push_back(std::make_tuple(byte_offset, 0, element_size));
      return;
    }
    for (auto tuple : R.ReadQ) {
      if (overlap(byte_offset, byte_len, std::get<0>(tuple), std::get<1>(tuple))) {
        std::cerr << "Error: MAXI write request(byte offset = " << byte_offset << ", type len = " << byte_len << ") overlaps with previous read request(byte offset = " << std::get<0>(tuple) << ", byte len = " << std::get<1>(tuple) << ")." << std::endl;
        abort();
      }
    }
    R.WriteQ.push_back(std::make_tuple(byte_offset, byte_len, element_size));
  }

  template<typename ACCESS_TYPE = T, typename INPUT_TYPE>
  void write(const INPUT_TYPE &val, ap_int<sizeof(ACCESS_TYPE)> byte_enable_mask = -1) {
    static_assert(sizeof(ACCESS_TYPE ) <= sizeof(T));
    MAXIAccessRecord &R = getMAXIPointer2AccessRecordMap()[Ptr];
    assert(!R.WriteQ.empty() && "Error: MAXI write without request."); 
    auto tuple = R.WriteQ.front();

    size_t byte_offset = std::get<0>(tuple);
    unsigned byte_len = std::get<1>(tuple);
    unsigned element_size = std::get<2>(tuple);

    if (sizeof(ACCESS_TYPE) != element_size) {
      std::cerr << "Error: The data size of write (" 
      << sizeof(ACCESS_TYPE) << " bytes) is different from the data size of write_request (" 
      << element_size << " bytes)." << std::endl;
      abort();
    }
    char *src_p = reinterpret_cast<char *>(Ptr);
    char *byte_addr = src_p + byte_offset + R.write_disp;
  
    R.write_disp += sizeof(ACCESS_TYPE);
    if (R.write_disp > byte_len) {
      std::cerr << "Error: The write element was outside the requested range." << std::endl;
      abort();
    }

    ACCESS_TYPE Src = (ACCESS_TYPE)val;
    for (unsigned i = 0; i < sizeof(ACCESS_TYPE); i++) {
      if (byte_enable_mask[i]) 
        byte_addr[i] = reinterpret_cast<char *>(&Src)[i];
    }

    if (R.write_disp == byte_len) {
      R.write_disp = 0;
      R.WriteRespQ.push_back(tuple);
      R.WriteQ.pop_front();
    }   
  }
 
  void write_response() {
    MAXIAccessRecord &R = getMAXIPointer2AccessRecordMap()[Ptr];
    assert(!R.WriteRespQ.empty() && "Error: bad MAXI write response. Possible: 1) no corresponding write request; 2) some data still not written.");
    R.WriteRespQ.pop_front();
  }

private:
  T *Ptr;
  bool overlap(size_t a, unsigned a_len, size_t b, unsigned b_len) {
    return a <= b ? a + a_len > b : b + b_len > a;
  }
  static std::map<void *, MAXIAccessRecord> &getMAXIPointer2AccessRecordMap() {
    static std::map<void *, MAXIAccessRecord> Map;
    return Map;
  }
};


} // namespace hls

#endif // __cplusplus
#endif  // X_HLS_STREAM_SIM_H


