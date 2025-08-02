#pragma once
#include <stdint.h>

#define INVALID_ID_U16     0xFFFFU 
#define INVALID_ID     0xFFFFFFFFU 
#define INVALID_ID_U64 0xFFFFFFFFFFFFFFFFU

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef float   f32;
typedef double  f64;

typedef _Bool  b8;
typedef int32_t b32;

#define NULL 0

#define true 1
#define false 0

// ===========================
// Exporting
// ===========================
#define SPKEXPORT 1

#ifdef SPKEXPORT
#ifdef _MSC_VER
#defien SAPI __declspec(dllexport)
#else
#define SAPI __attribute__((visibility("default")))
#endif
#else
#ifdef _MSC_VER
#defien SAPI __declspec(dllimport)
#else
#define SAPI
#endif
#endif

// ===========================
// Platform specific
// ===========================
#define SPARK_PLATFORM_LINUX
#ifdef _MSC_VER

#define SPARK_PLATFORM_WINDOWS
#define SNO_DISCARD(msg)
#define SINLINE __forceinline
#define SALIGNED(alignment) alignas(alignment)
#define thread_local thread_local

#else

#define SPARK_PLATFORM_LINUX
#define SNO_DISCARD __attribute__((warn_unused_result))
#define SINLINE static inline
#define SALIGNED(alignment) __attribute__((aligned( alignment )))
#define STATIC_ASSERT _Static_assert
#define thread_local _Thread_local

#endif

// ==============================================
// Making sure datatypes expected sizes
// ==============================================
STATIC_ASSERT(sizeof(s8) == 1, "Size of s8 != 1");
STATIC_ASSERT(sizeof(s16) == 2, "Size of s16 != 2");
STATIC_ASSERT(sizeof(s32) == 4, "Size of s32 != 4");
STATIC_ASSERT(sizeof(s64) == 8, "Size of s64 != 8");

STATIC_ASSERT(sizeof(u8) == 1, "Size of u8 != 1");
STATIC_ASSERT(sizeof(u16) == 2, "Size of u16 != 2");
STATIC_ASSERT(sizeof(u32) == 4, "Size of u32 != 4");
STATIC_ASSERT(sizeof(u64) == 8, "Size of u64 != 8");

STATIC_ASSERT(sizeof(f32) == 4, "Size of f32 != 4");
STATIC_ASSERT(sizeof(f64) == 8, "Size of f64 != 8");

STATIC_ASSERT(sizeof(b8) == 1, "Size of b8 != 1");
STATIC_ASSERT(sizeof(b32) == 4, "Size of b32 != 4");

#define SCLAMP(value, min, max) ((value <= min) ? min : ((value >= max) ? max : value));
