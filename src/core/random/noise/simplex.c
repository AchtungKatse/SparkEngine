#include "Spark/math/smath.h"

// Constant Numbers
#define F2 (0.5 * (S_SQRT_THREE - 1.0))
#define G2 ((3.0 - S_SQRT_THREE) / 6.0)
#define INT_ONE 65535

// Private functions
// #define fast_floor(x) (x > 0 ? (int)x : (int)x - 1)
s32 fast_floor(const f32 x) {
    return x > 0 ? (int)x : (int)x - 1;
}

s32 fast_floor_int(const s32 x) {
    return (x >> 16) << 16;
}

f32 dot_2d(const s32 vector[2], const vec2 pos) {
    return pos.x * vector[0] + pos.y * vector[1];
}

// Private data
const static s32 grad2[12][2];
// const static s32 grad3[12][3];
// const static s32 grad4[32][4];
const static int perm[512];

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
u32 simplex_2d_int(vec2i pos) {
    // Skew to square grid
    const s32 s = (pos.x + pos.y) * F2 * INT_ONE;
    const s32 i = fast_floor_int(pos.x + s) >> 16;
    const s32 j = fast_floor_int(pos.y + s) >> 16;

    // Unskew cell to x,y space
    const s32 t  = (i + j) * G2 * INT_ONE;
    const s32 x0 = (i - t) >> 16;
    const s32 y0 = (j - t) >> 16;

    const s32 x_dst = pos.x - x0;
    const s32 y_dst = pos.y - y0;

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
    const s32 x1 = x0 - i1 + G2;
    const s32 y1 = y0 - j1 + G2;
    const s32 x2 = x0 - INT_ONE + INT_ONE * 2 * G2;
    const s32 y2 = y0 - INT_ONE + INT_ONE * 2 * G2;

    // Calculate gradient indices of simplex corners
    const s32 ii = i & 255;
    const s32 jj = j & 255;
    const s32 gi0 = perm[ii +      perm[jj     ]] % 12;
    const s32 gi1 = perm[ii + i1 + perm[jj + j1]] % 12;
    const s32 gi2 = perm[ii + 1 +  perm[jj + 1 ]] % 12;

    // Noise distributions
    s32 noise = 70 * INT_ONE;

    // Calculate the contribution from the three corners
    s32 t0 = INT_ONE / 2 - x0 * x0 - y0 * y0;
    if (t0 >= 0) {
        t0 *= t0;
        noise += t0 * t0 * dot_2d(grad2[gi0], (vec2) { .x = x0, .y = y0 }); // (x,y) of grad3 used for 2D gradient
    }

    s32 t1 = INT_ONE / 2 - x1 * x1 - y1 * y1;
    if (t1 >= 0) {
        t1 *= t1;
        noise += t1 * t1 * dot_2d(grad2[gi1], (vec2) { .x = x1, .y = y1 }); // (x,y) of grad3 used for 2D gradient
    }

    s32 t2 = INT_ONE / 2 - x2 * x2 - y2 * y2;
    if (t2 >= 0) {
        t2 *= t2;
        noise += t2 * t2 * dot_2d(grad2[gi2], (vec2) { .x = x2, .y = y2 }); // (x,y) of grad3 used for 2D gradient
    }

    return noise;
}

const static s32 grad2[12][2] = {
    {1,1},{-1,1},{1,-1},{-1,-1},
    {1,0},{-1,0},{1,0},{-1,0},
    {0,1},{0,-1},{0,1},{0,-1}
};

// const static s32 grad3[12][3] = {
//     {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
//     {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
//     {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}
// };
//
// const static s32 grad4[32][4]= {{0,1,1,1}, {0,1,1,-1}, {0,1,-1,1}, {0,1,-1,-1},
//     {0,-1,1,1}, {0,-1,1,-1}, {0,-1,-1,1}, {0,-1,-1,-1},
//     {1,0,1,1}, {1,0,1,-1}, {1,0,-1,1}, {1,0,-1,-1},
//     {-1,0,1,1}, {-1,0,1,-1}, {-1,0,-1,1}, {-1,0,-1,-1},
//     {1,1,0,1}, {1,1,0,-1}, {1,-1,0,1}, {1,-1,0,-1},
//     {-1,1,0,1}, {-1,1,0,-1}, {-1,-1,0,1}, {-1,-1,0,-1},
//     {1,1,1,0}, {1,1,-1,0}, {1,-1,1,0}, {1,-1,-1,0},
//     {-1,1,1,0}, {-1,1,-1,0}, {-1,-1,1,0}, {-1,-1,-1,0}
// };
const static int perm[512] = {
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
