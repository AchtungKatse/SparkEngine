#pragma once
#include "Spark/math/math_types.h"

float simplex_2d(vec2 pos);
s32 simplex_2d_int(vec2i pos);
void simplex_2d_int_simd(vec2i pos, vec2i size, s32* out_noise);

f32 simplex_3d(vec3 pos);
s32 simplex_3d_int(vec3i pos);
