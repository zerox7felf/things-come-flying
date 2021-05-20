// matrix_math.h

#ifndef _MATRIX_MATH_H
#define _MATRIX_MATH_H

#include "common.h"

#ifndef NO_SSE
  #if __SSE__
    #define USE_SSE 1 // Use streaming SIMD extensions
  #endif
#endif

#if USE_SSE
  #include <xmmintrin.h>
#endif

typedef union mat4 {
  float elements[4][4];
#if USE_SSE
  __m128 rows[4];
#endif
} mat4;

extern mat4 mat4d(float diagonal);

extern mat4 translate(v3 t);

extern mat4 translate_mat4(mat4 m, v3 t);

extern mat4 multiply_mat4(mat4 a, mat4 b);

extern mat4 scale_mat4(v3 scale);

extern mat4 rotate(float angle, v3 axis);

extern float dot_v3(v3 a, v3 b);

extern float length_square_v3(v3 a);

extern float length_v3(v3 a);

extern v3 diff_v3(v3 a, v3 b);

extern v3 normalize_v3(v3 a);

extern v3 cross_product(v3 a, v3 b);

extern float to_radians(float angle);

extern mat4 perspective(float fov, float aspect, float z_near, float z_far);

extern mat4 orthographic(float left, float right, float bottom, float top, float z_near, float z_far);

extern mat4 look_at(v3 eye, v3 center, v3 up);

#endif
