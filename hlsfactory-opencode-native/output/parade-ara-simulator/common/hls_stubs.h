#ifndef HLS_STUBS_H
#define HLS_STUBS_H

// Stub implementations for HLS simulation without full PARADE framework
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <vector>

#ifndef simics_assert
#define simics_assert(x) assert(x)
#endif

struct InterruptArgs {
    int threadID;
    int lcaccID;
    int lcaccMode;
    int status;
    int taskIndex;
    uint32_t v[10];
};

inline void LCAcc_Command(int thread, int lcaccID, int cmd, void* buf, uint32_t size, int a, int b) {}
inline void LCAcc_Reserve(int thread, int a, int b) {}
inline void LCAcc_Request(int thread, int type, int count) {}
inline void LCAcc_Free(int thread, int lcaccID) {}
inline void LCAcc_DeclareLCAccUse(int thread, int type, int count) {}
inline void LCAcc_SendBiNCurve(int thread, uint32_t a, uint32_t b, uint32_t c) {}
inline void LWI_RegisterInterruptHandler(InterruptArgs* args) {}
inline void LWI_UnregisterInterruptHandler(int thread, int lcaccID) {}
inline InterruptArgs* LWI_CheckInterrupt(int thread) { return nullptr; }
inline void LWI_ClearInterrupt(int thread) {}
inline void KillSimulation() { exit(0); }
inline void BarrierTick(int a, int b) {}
inline void BarrierWait(int a, int b) {}
inline void ResetStats() {}
inline void Touch(int thread, void* ptr, size_t size) {}

#endif
