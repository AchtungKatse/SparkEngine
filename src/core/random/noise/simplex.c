#include "Spark/math/smath.h"
#include "Spark/random/noise/simplex.h"
#include <immintrin.h>
#include <nmmintrin.h>
#pragma GCC target("avx2")

// Constant Numbers
#define F2 (0.5 * (S_SQRT_THREE - 1.0))
#define F3 (1.0 / 3.0)
#define G2 ((3.0 - S_SQRT_THREE) / 6.0)
#define G3 (1.0 / 6.0)
#define INT_ONE_BIT_COUNT 12
#define INT_ONE ((1 << INT_ONE_BIT_COUNT))
const u32 a = ((INT_ONE * INT_ONE) >> INT_ONE_BIT_COUNT);

#define X_PRIME 501125321
#define Y_PRIME 1136930381
#define Z_PRIME 1720413743
#define W_PRIME 1066037191


// Private functions
// #define fast_floor(x) (x > 0 ? (int)x : (int)x - 1)
s32 fast_floor(const f32 x) {
    return x > 0 ? (int)x : (int)x - 1;
}

#define fast_floor_int(x) (x & ~(INT_ONE - 1))
// s32 fast_floor_int(const s32 x) {
//     return (x >> 16) << 16;
// }

SINLINE s32 int_mul(s32 a, s32 b) {
    s32 result = (a * b) >> INT_ONE_BIT_COUNT;
    // Copy sign bit
    const static u32 sign_bit = (1 << 31);
    result |= (a ^ b) & sign_bit;
    return result;
}
SINLINE s32 calculate_t(s32 x, s32 y);
SINLINE s32 get_gradient_dot_fancy(s32 hash, s32 fX, s32 fY);
SINLINE s32 hash_primes(u32 seed, u32 prime_count, s32* primes);
SINLINE __m256i simd_masked_add(__m256i a, __m256i b, __m256i mask);
SINLINE __m256i simd_get_gradient_dot_fancy(__m256i hash, __m256i fX, __m256i fY );
SINLINE __m256i simd_hash_primes(u64 seed, u32 prime_count, __m256i* primes);
SINLINE __m256i simd_calculate_t(__m256i x, __m256i y);

f32 dot_2d(const f32 vector[2], const vec2 pos) {
    return pos.x * vector[0] + pos.y * vector[1];
}

SINLINE s32 dot_2d_int(const s32 vector[2], const vec2i pos) {
    return ((pos.x * vector[0]) >> INT_ONE_BIT_COUNT) + ((pos.y * vector[1]) >> INT_ONE_BIT_COUNT);
}

f32 dot_3d(const s8 vector[3], const vec3 pos) {
    return pos.x * vector[0] + pos.y * vector[1] + pos.z * vector[2];
}

s32 dot_3d_int(const s8 vector[3], const vec3i pos) {
    return ((pos.x * vector[0]) >> INT_ONE_BIT_COUNT) + ((pos.y * vector[1]) >> INT_ONE_BIT_COUNT) + ((pos.z * vector[2]) >> INT_ONE_BIT_COUNT);
}

// Private data
const static s32 grad2_y[12];
const static s32 grad2_x[12];
const static s16 grad2_int[12][2];
const static f32 grad2[12][2];
const static s8 grad3[12][3];
// const static s32 grad4[32][4];
const static s32 perm[512];

float simplex_2d(vec2 pos) {
    // Skew to square grid
    const float s = (pos.x + pos.y) * F2;
    const s32 i = fast_floor(pos.x + s);
    const s32 j = fast_floor(pos.y + s);

    // Unskew cell to x,y space
    const f32 t = (i + j) * G2;
    const float x0 = i - t;
    const float y0 = j - t;

    const float x_dst = pos.x - x0;
    const float y_dst = pos.y - y0;

    // Find the simplex coordinate
    s32 i1 = 0;
    s32 j1 = 1;
    if (x_dst > y_dst) {
        i1 = 1; 
        j1 = 0;
    }

    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6
    const float x1 = x0 - i1 + G2;
    const float y1 = y0 - j1 + G2;
    const float x2 = x0 - 1.0 + 2.0 * G2;
    const float y2 = y0 - 1.0 + 2.0 * G2;

    // Calculate gradient indices of simplex corners
    const s32 ii = i & 255;
    const s32 jj = j & 255;
    const s32 gi0 = perm[ii +      perm[jj     ]] % 12;
    const s32 gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
    const s32 gi2 = perm[ii + 1 +  perm[jj + 1 ]] % 12;

    // Noise distributions
    float noise = 70.0f;

    // Calculate the contribution from the three corners
    float t0 = 0.5 - x0 * x0 - y0 * y0;
    if (t0 >= 0) {
        t0 *= t0;
        noise += t0 * t0 * dot_2d(grad2[gi0], (vec2) { .x = x0, .y = y0 }); // (x,y) of grad3 used for 2D gradient
    }

    float t1 = 0.5 - x1 * x1 - y1 * y1;
    if (t1 >= 0) {
        t1 *= t1;
        noise += t1 * t1 * dot_2d(grad2[gi1], (vec2) { .x = x1, .y = y1 }); // (x,y) of grad3 used for 2D gradient
    }
    float t2 = 0.5 - x2 * x2 - y2 * y2;
    if (t2 >= 0) {
        t2 *= t2;
        noise += t2 * t2 * dot_2d(grad2[gi2], (vec2) { .x = x2, .y = y2 }); // (x,y) of grad3 used for 2D gradient
    }

    return noise;
}

/**
 * @brief Simplex noise using integers instead of floats. 65535 maps to 1.0
 *
 * @param pos Position in integer space
 * @return Simplex noise value
 */
s32 simplex_2d_int(vec2i pos) {
    const s32 f = ((pos.x + pos.y) * (s32)(F2 * INT_ONE)) >> INT_ONE_BIT_COUNT;
    s32 x0 = fast_floor_int(pos.x + f);
    s32 y0 = fast_floor_int(pos.y + f);

    const s32 i = (x0 >> INT_ONE_BIT_COUNT) * X_PRIME;
    const s32 j = (y0 >> INT_ONE_BIT_COUNT) * Y_PRIME;

    const s32 g = ((s32)(INT_ONE * G2) * (x0 + y0)) >> INT_ONE_BIT_COUNT;
    x0 = pos.x - (x0 - g);
    y0 = pos.y - (y0 - g);

    const b8 i1 = x0 > y0;
    s32 x1 = i1 ? x0 - INT_ONE : x0;
    s32 y1 = i1 ? y0 : y0 - INT_ONE;
    x1 += (s32)(INT_ONE * G2);
    y1 += (s32)(INT_ONE * G2);

    s32 x2 = x0 + (s32)(INT_ONE * (G2 * 2 - 1));
    s32 y2 = y0 + (s32)(INT_ONE * (G2 * 2 - 1));

    const s32 t0 = calculate_t(x0, y0);
    const s32 t1 = calculate_t(x1, y1);
    const s32 t2 = calculate_t(x2, y2);

    const s32 seed = 4;
    const s32 n0 = get_gradient_dot_fancy(hash_primes(seed, 2, (s32[]) {i, j}), x0, y0);
    const s32 n1 = get_gradient_dot_fancy(hash_primes(seed, 2, (s32[]) {
                i1 ? i + X_PRIME : i,
                i1 ? j : j + Y_PRIME,
                }), x1, y1);
    const s32 n2 = get_gradient_dot_fancy(hash_primes(seed, 2, (s32[]) {
                i + X_PRIME,
                j + Y_PRIME
                }), x2, y2);

    s32 noise =  ((n0 * t0) >> 16) + ((n1 * t1) >> 16) + ((n2 * t2) >> 16);
    noise *= ((s32)38.283687591552734375 * INT_ONE);
    noise >>= INT_ONE_BIT_COUNT;
    return noise;
}

/**
 * @brief Simplex noise using integers instead of floats. 65535 maps to 1.0
 *
 * @param pos Position in integer space
 * @return Simplex noise value
 */
void simplex_2d_int_simd(vec2i pos, vec2i size, s32* out_noise) {
    const u32 simd_size = sizeof(__m256i) / 4;
    for (u32 _x = 0, index = 0; _x < size.x / simd_size; _x++) {
        for (u32 _y = 0; _y < size.y; _y++, index ++) {
            const static s32 _add[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };
            const __m256i add = _mm256_stream_load_si256((const void*)_add);
            const __m256i x = _mm256_add_epi32(add, _mm256_set1_epi32(pos.x));

            const __m256i y = _mm256_set1_epi32(pos.y);
            const __m256i pos_sum = _mm256_add_epi32(x, y);

            // Skew to square grid
            // const s32 s = (pos.x + pos.y) * F2 * INT_ONE;
            // const s32 i = fast_floor_int(pos.x + s) >> INT_ONE_BIT_COUNT;
            // const s32 j = fast_floor_int(pos.y + s) >> INT_ONE_BIT_COUNT;
            __m256i f = _mm256_set1_epi32(F2 * INT_ONE);
            f = _mm256_mul_epi32(f, pos_sum);

            __m256i x0 = _mm256_add_epi32(x, f);
            x0 = _mm256_slli_epi32(x0, INT_ONE_BIT_COUNT);
            x0 = _mm256_srli_epi32(x0, INT_ONE_BIT_COUNT);
            __m256i y0 = _mm256_add_epi32(y, f);
            y0 = _mm256_slli_epi32(y0, INT_ONE_BIT_COUNT);
            y0 = _mm256_srli_epi32(y0, INT_ONE_BIT_COUNT);

            __m256i x_prime = _mm256_set1_epi32(X_PRIME);
            __m256i y_prime = _mm256_set1_epi32(Y_PRIME);
            __m256i i = _mm256_mul_epi32(x0, x_prime);
            __m256i j = _mm256_mul_epi32(y0, y_prime);

            __m256i SIMD_TWO_G2_PLUS_ONE = _mm256_set1_epi32(G2 * 2 - 1);
            __m256i SIMD_G2 = _mm256_set1_epi32(G2 * INT_ONE);
            __m256i SIMD_ONE = _mm256_set1_epi32(1);

            __m256i g = _mm256_add_epi32(x0, y0);
            g = _mm256_mul_epi32(SIMD_G2, g);

            x0 = _mm256_sub_epi32(x, _mm256_sub_epi32(x0, g));
            y0 = _mm256_sub_epi32(y, _mm256_sub_epi32(y0, g));

            __m256i i1 = _mm256_cmpgt_epi32(x0, y0);
            // mask32v i1 = x0 > y0;

            __m256i x1 = _mm256_blendv_epi8(x0, _mm256_sub_epi32(x0, SIMD_ONE), i1);
            __m256i y1 = _mm256_blendv_epi8(y0, _mm256_sub_epi32(y0, SIMD_ONE), i1);

            __m256i x2 = _mm256_add_epi32(x0, SIMD_TWO_G2_PLUS_ONE);
            __m256i y2 = _mm256_add_epi32(y0, SIMD_TWO_G2_PLUS_ONE);

            __m256i t0 = simd_calculate_t(x0, y0);
            __m256i t1 = simd_calculate_t(x1, y1);
            __m256i t2 = simd_calculate_t(x2, y2);

            const u32 seed = 0;
            __m256i n0 = simd_get_gradient_dot_fancy(simd_hash_primes(seed, 2, (__m256i[]) { i, j }), x0, y0);
            __m256i n1 = simd_get_gradient_dot_fancy(simd_hash_primes(seed, 2, (__m256i[]) { 
                        simd_masked_add(i, _mm256_set1_epi32(X_PRIME), i1),
                        simd_masked_add(j,  _mm256_set1_epi32(Y_PRIME), i1) }), x1, y1);
            __m256i n2 = simd_get_gradient_dot_fancy(simd_hash_primes(seed, 2, (__m256i[]) { 
                        _mm256_add_epi32(i, _mm256_set1_epi32(X_PRIME)),
                        _mm256_add_epi32(j, _mm256_set1_epi32(Y_PRIME)),
                        }), x2, y2);

            __m256i noise = _mm256_add_epi32(noise, _mm256_mul_epi32(n0, t0));
            noise = _mm256_add_epi32(noise, _mm256_mul_epi32(n1, t1));
            noise = _mm256_add_epi32(noise, _mm256_mul_epi32(n2, t2));
            noise = _mm256_mul_epi32(noise, _mm256_set1_epi32(50 * INT_ONE));
            noise = _mm256_set1_epi32(50 * INT_ONE);

            u32 offset = sizeof(s32) * index;
            _mm256_store_si256((__m256i*)out_noise + index, noise);
        }
    }
}

// 3D simplex noise
f32 simplex_3d(vec3 pos) {
    // Skew the input space to determine which simplex cell we're in
    const f32 s = (pos.x + pos.y + pos.z) * F3; // Very nice and simple skew factor for 3D

    const int i = fast_floor(pos.x + s);
    const int j = fast_floor(pos.y + s);
    const int k = fast_floor(pos.z + s);
    const f32 t = (i + j + k) * G3;


    const f32 X0 = i - t; // Unskew the cell origin back to (x,y,z) space
    const f32 Y0 = j - t;
    const f32 Z0 = k - t;

    const f32 x0 = pos.x - X0; // The x,y,z distances from the cell origin
    const f32 y0 = pos.y - Y0;
    const f32 z0 = pos.z - Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1 = 0; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2 = 0; // Offsets for third corner of simplex in (i,j,k) coords
                        // TODO: Lookup table
    enum simplex_order {
        ORDER_XYZ = 0,
        ORDER_XZY = 1,
        ORDER_ZXY = 2,
        ORDER_ZYX = 3,
        ORDER_YZX = 4,
        ORDER_YXZ = 5,
    };

    enum simplex_order order = ORDER_XYZ;

    if(x0>=y0) {
        if(y0>=z0) { 
            i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; // X Y Z order
        } 
        else if(x0>=z0) { 
            i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; // X Z Y order
        }
        else { 
            i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; // Z X Y order
        }
    }
    else { // x0<y0
        if(y0<z0) { 
            i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; // Z Y X order
        } 
        else if(x0<z0) { 
            i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; // Y Z X order
        } 
        else { 
            i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; // Y X Z order
        } 
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    const f32 x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    const f32 y1 = y0 - j1 + G3;
    const f32 z1 = z0 - k1 + G3;
    const f32 x2 = x0 - i2  + 2.0 * G3; // Offsets for third corner in (x,y,z) coords
    const f32 y2 = y0 - j2  + 2.0 * G3;
    const f32 z2 = z0 - k2  + 2.0 * G3;
    const f32 x3 = x0 - 1.0 + 3.0 * G3; // Offsets for last corner in (x,y,z) coords
    const f32 y3 = y0 - 1.0 + 3.0 * G3;
    const f32 z3 = z0 - 1.0 + 3.0 * G3;

    // Work out the hashed gradient indices of the four simplex corners
    const int ii = i & 255;
    const int jj = j & 255;
    const int kk = k & 255;
    const int gi0 = perm[ii +      perm[jj +      perm[kk     ]]] % 12;
    const int gi1 = perm[ii + i1 + perm[jj + j1 + perm[kk + k1]]] % 12;
    const int gi2 = perm[ii + i2 + perm[jj + j2 + perm[kk + k2]]] % 12;
    const int gi3 = perm[ii + 1  + perm[jj + 1  + perm[kk + 1 ]]] % 12;

    // Calculate the contribution from the four corners
    f32 n0, n1, n2, n3 = 0; // Noise contributions from the four corners
    f32 t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    f32 t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    f32 t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    f32 t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;

    if(t0 >= 0) {
        t0 *= t0;
        n0 = t0 * t0 * dot_3d(grad3[gi0], (vec3) {.x = x0, y0, z0} );
    }
    if(t1 >= 0) {
        t1 *= t1;
        n1 = t1 * t1 * dot_3d(grad3[gi1], (vec3) { x1, y1, z1 } );
    }
    if(t2 >= 0) {
        t2 *= t2;
        n2 = t2 * t2 * dot_3d(grad3[gi2], (vec3) { x2, y2, z2 } );
    }
    if(t3 >= 0) {
        t3 *= t3;
        n3 = t3 * t3 * dot_3d(grad3[gi3], (vec3) { x3, y3, z3 } );
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0*(n0 + n1 + n2 + n3);
}

// 3D simplex noise
s32 simplex_3d_int(vec3i pos) {
    // Skew the input space to determine which simplex cell we're in
    const s32 s = (pos.x + pos.y + pos.z) * (F3 * INT_ONE); // Very nice and simple skew factor for 3D

    const s32 i = fast_floor_int(pos.x + s) >> INT_ONE_BIT_COUNT;
    const s32 j = fast_floor_int(pos.y + s) >> INT_ONE_BIT_COUNT;
    const s32 k = fast_floor_int(pos.z + s) >> INT_ONE_BIT_COUNT;
    const s32 t = (i + j + k) * G3;


    const s32 X0 = i - t; // Unskew the cell origin back to (x,y,z) space
    const s32 Y0 = j - t;
    const s32 Z0 = k - t;

    const s32 x0 = pos.x - X0; // The x,y,z distances from the cell origin
    const s32 y0 = pos.y - Y0;
    const s32 z0 = pos.z - Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1 = 0; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2 = 0; // Offsets for third corner of simplex in (i,j,k) coords
                        // TODO: Lookup table
    enum simplex_order {
        ORDER_XYZ = 0,
        ORDER_XZY = 1,
        ORDER_ZXY = 2,
        ORDER_ZYX = 3,
        ORDER_YZX = 4,
        ORDER_YXZ = 5,
    };

    enum simplex_order order = ORDER_XYZ;

    if(x0>=y0) {
        if(y0>=z0) { 
            i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; // X Y Z order
        } 
        else if(x0>=z0) { 
            i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; // X Z Y order
        }
        else { 
            i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; // Z X Y order
        }
    }
    else { // x0<y0
        if(y0<z0) { 
            i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; // Z Y X order
        } 
        else if(x0<z0) { 
            i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; // Y Z X order
        } 
        else { 
            i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; // Y X Z order
        } 
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    const s32 x1 = x0 - i1 + G3 * INT_ONE; // Offsets for second corner in (x,y,z) coords
    const s32 y1 = y0 - j1 + G3 * INT_ONE;
    const s32 z1 = z0 - k1 + G3 * INT_ONE;
    const s32 x2 = x0 - i2  + 2.0 * G3 * INT_ONE; // Offsets for third corner in (x,y,z) coords
    const s32 y2 = y0 - j2  + 2.0 * G3 * INT_ONE;
    const s32 z2 = z0 - k2  + 2.0 * G3 * INT_ONE;
    const s32 x3 = x0 - 1.0 + 3.0 * G3 * INT_ONE; // Offsets for last corner in (x,y,z) coords
    const s32 y3 = y0 - 1.0 + 3.0 * G3 * INT_ONE;
    const s32 z3 = z0 - 1.0 + 3.0 * G3 * INT_ONE;

    // Work out the hashed gradient indices of the four simplex corners
    const s32 ii = i & 255;
    const s32 jj = j & 255;
    const s32 kk = k & 255;
    const s32 gi0 = perm[ii +      perm[jj +      perm[kk     ]]] % 12;
    const s32 gi1 = perm[ii + i1 + perm[jj + j1 + perm[kk + k1]]] % 12;
    const s32 gi2 = perm[ii + i2 + perm[jj + j2 + perm[kk + k2]]] % 12;
    const s32 gi3 = perm[ii + 1  + perm[jj + 1  + perm[kk + 1 ]]] % 12;

    // Calculate the contribution from the four corners
    s32 n0, n1, n2, n3 = 0; // Noise contributions from the four corners
    s32 t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    s32 t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    s32 t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    s32 t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;

    if(t0 >= 0) {
        t0 *= t0;
        n0 = t0 * t0 * dot_3d_int(grad3[gi0], (vec3i) {.x = x0, y0, z0} );
    }
    if(t1 >= 0) {
        t1 *= t1;
        n1 = t1 * t1 * dot_3d_int(grad3[gi1], (vec3i) { x1, y1, z1 } );
    }
    if(t2 >= 0) {
        t2 *= t2;
        n2 = t2 * t2 * dot_3d_int(grad3[gi2], (vec3i) { x2, y2, z2 } );
    }
    if(t3 >= 0) {
        t3 *= t3;
        n3 = t3 * t3 * dot_3d_int(grad3[gi3], (vec3i) { x3, y3, z3 } );
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return (32 * INT_ONE) *(n0 + n1 + n2 + n3);
}

// ====================
// Helper functions
// ====================
SINLINE s32 calculate_t(s32 x, s32 y) {
        //     float32v t0 = FS_FNMulAdd_f32( x0, x0, FS_FNMulAdd_f32( y0, y0, float32v( 0.5f ) ) );
        //
        // t0 = FS_Max_f32( t0, float32v( 0 ) );
        //
        // t0 *= t0; t0 *= t0;
    s32 _y = (-(y * y)) >> INT_ONE_BIT_COUNT;
    s32 _x = (-(x * x)) >> INT_ONE_BIT_COUNT;
    s32 t = _x + _y + INT_ONE / 2;
    if (t <= 0) {
        return 0;
    }

    t = (t * t) >> INT_ONE_BIT_COUNT;
    t = (t * t) >> INT_ONE_BIT_COUNT;
    return t;
}

SINLINE __m256i simd_calculate_t(__m256i x, __m256i y) {
    __m256i x_sqr = _mm256_mul_epi32(x, x);
    __m256i y_sqr = _mm256_mul_epi32(y, y);
    __m256i SIMD_HALF = _mm256_set1_epi32(INT_ONE / 2);
    __m256i t = _mm256_sub_epi32(SIMD_HALF, _mm256_add_epi32(x_sqr, y_sqr));


    const __m256i SIMD_ZERO = _mm256_set1_epi32(0);
    __m256i mask = _mm256_cmpgt_epi32(t, SIMD_ZERO);
    t = _mm256_blendv_epi8(SIMD_ZERO, t, mask);

    t = _mm256_mul_epi32(t, t);
    t = _mm256_mul_epi32(t, t);

    return t;
}

SINLINE s32 hash_primes(u32 seed, u32 prime_count, s32* primes) {
            //     int32v hash = seed;
            // hash ^= (primedPos ^ ...);
            //
            // hash *= int32v( 0x27d4eb2d );
            // return (hash >> 15) ^ hash;
            //
    s32 hash = seed;
    for (u32 i = 0; i < prime_count; i++) {
        hash ^= primes[i];
    }

    hash *= 0x27d4eb2d;
    return (hash >> 15) ^ hash;
}

SINLINE __m256i simd_hash_primes(u64 seed, u32 prime_count, __m256i* primes) {
    __m256i hash = _mm256_set1_epi64x(seed);
    for (u32 i = 0; i < prime_count; i++) {
        hash = _mm256_xor_si256(hash, primes[i]);
    }

    const __m256i mul = _mm256_set1_epi32(0x27d4eb2d);
    hash = _mm256_mul_epi32(hash, mul);
    hash = _mm256_srli_epi32(hash, 15);
    hash = _mm256_xor_si256(hash, hash);
    return hash;
}

SINLINE s32 get_gradient_dot_fancy(s32 hash, s32 fX, s32 fY ) {
    const s32 mul = INT_ONE / 1.3333333333333333333333f;
    s32 index = (hash & 0x3FFFFF) * mul;
    index >>= INT_ONE_BIT_COUNT;
    // s32 index = _mm256_mul_epi32(_mm256_and_si256(hash, _mm256_set1_epi32(0x3FFFFF)), _mm256_set1_epi32( INT_ONE / 1.3333333333333333f));

    // Bit-4 = Choose X Y ordering
    // s32 xy = (index >> 29) << 31;
    s32 xy = (index & ( 1 << 2 )) != 0;

    // __m256i xy = _mm256_slli_si256(index, 29);
    // xy = _mm256_srli_si256(xy, 31);

    s32 a = fX;
    s32 b = fY;
    if (xy) {
        a = fY;
        b = fX;
    }
    // __m256i a = _mm256_blendv_epi8(fY, fX, xy);
    // __m256i b = _mm256_blendv_epi8(fX, fY, xy);

    // Bit-1 = b flip sign
    // b = b ^ (index << 31);
    b *= -1;
    // b = _mm256_xor_si256(b, _mm256_slli_si256(index, 31));

    // Bit-2 = Mul a by 2 or Root3
    s32 amul2 = ( index & ( 1 << 1 ) ) != 0;
    // __m256i aMul2 = _mm256_srli_si256(_mm256_slli_si256(index, 30), 31);

    // a = _mm256_mul_epi32(a, _mm256_blendv_epi8( aMul2, _mm256_set1_epi32(2), _mm256_set1_epi32(S_SQRT_THREE * INT_ONE)));
    // b zero value if a mul 2
    // b = _mm256_and_si256(b, aMul2);
    if (amul2) {
        a *= 2;
    } else {
        a *= (s32)(INT_ONE * S_SQRT_THREE);
        a >>= INT_ONE_BIT_COUNT;
    }
    b = b * !amul2;

    // Bit-8 = Flip sign of a + b
    return (a + b);
}


SINLINE __m256i simd_get_gradient_dot_fancy(__m256i hash, __m256i fX, __m256i fY ) {
    __m256i index = _mm256_mul_epi32(_mm256_and_si256(hash, _mm256_set1_epi32(0x3FFFFF)), _mm256_set1_epi32( INT_ONE / 1.3333333333333333f));

    // Bit-4 = Choose X Y ordering
    __m256i xy = _mm256_slli_si256(index, 29);
    xy = _mm256_srli_si256(xy, 31);

    __m256i a = _mm256_blendv_epi8(fY, fX, xy);
    __m256i b = _mm256_blendv_epi8(fX, fY, xy);

    // Bit-1 = b flip sign
    b = _mm256_xor_si256(b, _mm256_slli_si256(index, 31));

    // Bit-2 = Mul a by 2 or Root3
    __m256i aMul2 = _mm256_srli_si256(_mm256_slli_si256(index, 30), 31);

    a = _mm256_mul_epi32(a, _mm256_blendv_epi8( aMul2, _mm256_set1_epi32(2), _mm256_set1_epi32(S_SQRT_THREE * INT_ONE)));
    // b zero value if a mul 2
    b = _mm256_and_si256(b, aMul2);

    // Bit-8 = Flip sign of a + b
    return _mm256_xor_si256(_mm256_add_epi32(a, b), _mm256_slli_epi32(_mm256_srli_epi32(index, 3), 31));
}

SINLINE __m256i simd_masked_add(__m256i a, __m256i b, __m256i mask) {
    return _mm256_add_epi32(a, _mm256_and_si256(b, mask));
}


// ====================
// Data
// ====================

const static s32 grad2_x[12] = {
    1,-1,1,-1,
    1,-1,1,-1,
    0,0,0,0
};
const static s32 grad2_y[12] = {
    1,1,-1,-1,
    0,0,0,0,
    1,-1,1,-1
};
const static s16 grad2_int[12][2] = {
    {INT_ONE,INT_ONE},{-INT_ONE,INT_ONE},{INT_ONE,-INT_ONE},{-INT_ONE,-INT_ONE},
    {INT_ONE,0},{-INT_ONE,0},{INT_ONE,0},{-INT_ONE,0},
    {0,INT_ONE},{0,-INT_ONE},{0,INT_ONE},{0,-INT_ONE}
};
const static f32 grad2[12][2] = {
    {1,1},{-1,1},{1,-1},{-1,-1},
    {1,0},{-1,0},{1,0},{-1,0},
    {0,1},{0,-1},{0,1},{0,-1}
};

const static s8 grad3[12][3] = {
    {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
    {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
    {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}
};

// const static s32 grad4[32][4]= {{0,1,1,1}, {0,1,1,-1}, {0,1,-1,1}, {0,1,-1,-1},
//     {0,-1,1,1}, {0,-1,1,-1}, {0,-1,-1,1}, {0,-1,-1,-1},
//     {1,0,1,1}, {1,0,1,-1}, {1,0,-1,1}, {1,0,-1,-1},
//     {-1,0,1,1}, {-1,0,1,-1}, {-1,0,-1,1}, {-1,0,-1,-1},
//     {1,1,0,1}, {1,1,0,-1}, {1,-1,0,1}, {1,-1,0,-1},
//     {-1,1,0,1}, {-1,1,0,-1}, {-1,-1,0,1}, {-1,-1,0,-1},
//     {1,1,1,0}, {1,1,-1,0}, {1,-1,1,0}, {1,-1,-1,0},
//     {-1,1,1,0}, {-1,1,-1,0}, {-1,-1,1,0}, {-1,-1,-1,0}
// };
const static s32 perm[512] = {
    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
};
