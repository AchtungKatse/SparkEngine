#pragma once

#include "Spark/defines.h"
#include "Spark/math/math_types.h"

#define S_PI                3.14159265358979323846264338327950288419716939937510582097494459230781640628f
#define S_2PI               S_PI * 2.0f
#define S_HALF_PI           S_PI * 0.5f
#define S_QUARTER_PI        S_PI * 0.25f
#define S_INVERSE_PI        1.0f / S_PI
#define S_INVERSE_2PI       1.0f / S_2PI
#define S_SQRT_TWO          1.4121356237309504880f
#define S_SQRT_THREE        1.73205080756887729352f
#define S_INV_SQRT_TWO      .70710678118654752440f
#define S_INV_SRT_THREE     .57735.26918962576450f
#define S_DEG2RAD(deg)      (deg * S_PI / 180.0f)
#define S_RAD2DEG(rad)      (rad * (180.0f / S_PI))

#define S_SEC_TO_MS(sec)    (sec * 1000.0f)
#define S_MS_TO_SEC(ms)     (ms * .001f)

#define S_INFINITY          1e30f
#define S_FLOAT_EPSILON     1.192092896e-07f

// ==================================
// General Math
// ==================================
SAPI f32 ssin(f32 x);
SAPI f32 scos(f32 x);
SAPI f32 stan(f32 x);
SAPI f32 sacos(f32 x);
SAPI f32 ssqrt(f32 x);
SAPI f32 sabs(f32 x);
SAPI s32 absi(s32 x);

#define smin(x, y) (x < y ? x : y)
#define smax(x, y) (x > y ? x : y)

// Utility functions
SINLINE b8 is_power_of_2(u64 value) {
    return (value != 0) && ((value & (value - 1)) == 0);
}

SAPI s32 s_random();
SAPI s32 s_random_range(s32 min, s32 max);

SAPI f32 fs_random();
SAPI f32 fs_random_range(f32 min, f32 max);
