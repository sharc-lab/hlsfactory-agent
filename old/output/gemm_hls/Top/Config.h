#pragma once
using Data_t = float;
constexpr int kMemoryWidthBytesN = 64;
constexpr int kMemoryWidthBytesK = 64;
constexpr int kMemoryWidthBytesM = 64;
constexpr unsigned long kSizeN = 64;
constexpr unsigned long kSizeK = 64;
constexpr unsigned long kSizeM = 64;
constexpr unsigned long kOuterTileSizeN = 32;
constexpr unsigned long kOuterTileSizeM = 32;
constexpr unsigned long kInnerTileSizeN = 8;
constexpr int kComputeTileSizeM = 8;
constexpr int kComputeTileSizeN = 1;
constexpr int kTransposeWidthBytes = 64;
