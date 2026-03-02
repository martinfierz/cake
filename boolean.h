int LSB(int32 x);
#define bitcount(x) __popcnt(x)


/*
//c fastops.h
#pragma once
// fastops.h - small wrappers to use efficient intrinsics for popcount / bit scans
// Works with MSVC (Visual Studio). Include before using bitcount/LSB/MSB.

#include <stdint.h>
#include <intrin.h>

#ifdef _MSC_VER
static inline int bitcount_u32(uint32_t x) { return (int)__popcnt((unsigned int)x); }
static inline int bitcount_u64(uint64_t x) { return (int)__popcnt64((unsigned long long)x); }

static inline int bitcount(unsigned int x) { return bitcount_u32((uint32_t)x); }

static inline int LSB_index(uint32_t x) {
    unsigned long idx;
    if (x == 0) return -1;
    _BitScanForward(&idx, x);
    return (int)idx;
}
static inline int MSB_index(uint32_t x) {
    unsigned long idx;
    if (x == 0) return -1;
    _BitScanReverse(&idx, x);
    return (int)idx;
}
#else
// Fallbacks for other compilers
static inline int bitcount_u32(uint32_t x) { return __builtin_popcount(x); }
static inline int bitcount_u64(uint64_t x) { return __builtin_popcountll(x); }
static inline int bitcount(unsigned int x) { return bitcount_u32((uint32_t)x); }
static inline int LSB_index(uint32_t x) { return x ? __builtin_ctz(x) : -1; }
static inline int MSB_index(uint32_t x) { return x ? 31 - __builtin_clz(x) : -1; }
#endif
**/