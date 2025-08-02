#pragma once
#include "Spark/defines.h"

typedef union vec2 {
    f32 values[2];
    struct { f32 x, y; };
    struct { f32 u, v; };
} vec2;

typedef struct vec2i {
    u32 x;
    u32 y;
} vec2i;

typedef union vec3 {
    f32 values[3];
    struct { f32 x, y, z; };
} vec3;

typedef struct vec3i {
    u32 x;
    u32 y;
    u32 z;
} vec3i;

typedef union vec4 {
#ifdef SUSE_SIMD
    __m128 data;
#endif
    struct { f32 x, y, z, w; };
} vec4;

typedef struct vec4i {
    u32 x, y, z, w;
} vec4i;

typedef vec4 quat;

typedef struct vertex_3d {
    vec3 position;
    vec2 texcoord;
} vertex_3d;

typedef struct vertex_2d {
    vec2 position;
    vec2 texcoord;
} vertex_2d;

