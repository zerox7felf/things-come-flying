// matrix_math.hpp
// based on HandMadeMath

#ifndef _MATRIX_MATH_HPP
#define _MATRIX_MATH_HPP

#include "common.hpp"

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

inline v3 operator+(v3 a, v3 b) {
	v3 result = a;

	result.x += b.x;
	result.y += b.y;
	result.z += b.z;

	return result;
}

inline v3 operator-(v3 a, v3 b) {
	v3 result = a;

	result.x -= b.x;
	result.y -= b.y;
	result.z -= b.z;

	return result;
}

inline v3 operator*(v3 a, v3 b) {
	v3 result = a;

	result.x *= b.x;
	result.y *= b.y;
	result.z *= b.z;

	return result;
}

inline v3 operator*(v3 a, float scalar) {
	v3 result = a;

	result.x *= scalar;
	result.y *= scalar;
	result.z *= scalar;

	return result;
}

inline mat4 mat4d(float diagonal);

inline mat4 inverse(mat4 a);

inline mat4 translate(v3 t);

inline mat4 translate_mat4(mat4 m, v3 t);

inline mat4 multiply_mat4(mat4 a, mat4 b);

inline v4 multiply_mat4_v4(mat4 a, v4 b);

inline v3 multiply_mat4_v3(mat4 m, v3 a, float* w);

inline v3 v3_from_v4(v4 a);

inline v4 v4_from_v3(v3 a, float w);

inline mat4 scale_mat4(v3 scale);

inline mat4 rotate(float angle, v3 axis);

inline float dot(v3 a, v3 b);

inline float length_square_v3(v3 a);

inline float length_v3(v3 a);

inline v3 diff_v3(v3 a, v3 b);

inline v3 normalize(v3 a);

inline v3 cross_product(v3 a, v3 b);

inline float to_radians(float angle);

inline mat4 perspective(float fov, float aspect, float z_near, float z_far);

inline mat4 orthographic(float left, float right, float bottom, float top, float z_near, float z_far);

inline mat4 look_at(v3 eye, v3 center, v3 up);

#if USE_SSE

inline __m128 linear_combine(__m128 a, mat4 b);

#endif

inline mat4 transpose(mat4 a);

inline mat4 mat4d(float diagonal) {
	mat4 result = {0};

	result.elements[0][0] = diagonal;
	result.elements[1][1] = diagonal;
	result.elements[2][2] = diagonal;
	result.elements[3][3] = diagonal;

	return result;
}

inline mat4 inverse(mat4 a) {
	mat4 result;

	result.elements[0][0] = a.elements[0][0]; result.elements[0][1] = a.elements[1][0]; result.elements[0][2] = a.elements[2][0]; result.elements[0][3] = 0.0f;
	result.elements[1][0] = a.elements[0][1]; result.elements[1][1] = a.elements[1][1]; result.elements[1][2] = a.elements[2][1]; result.elements[1][3] = 0.0f;
	result.elements[2][0] = a.elements[0][2]; result.elements[2][1] = a.elements[1][2]; result.elements[2][2] = a.elements[2][2]; result.elements[2][3] = 0.0f;

	result.elements[3][0] = -(a.elements[3][0] * result.elements[0][0] + a.elements[3][1] * result.elements[1][0] + a.elements[3][2] * result.elements[2][0]);
	result.elements[3][1] = -(a.elements[3][0] * result.elements[0][1] + a.elements[3][1] * result.elements[1][1] + a.elements[3][2] * result.elements[2][1]);
	result.elements[3][2] = -(a.elements[3][0] * result.elements[0][2] + a.elements[3][1] * result.elements[1][2] + a.elements[3][2] * result.elements[2][2]);
	result.elements[3][3] = 1.0f;

	return result;
}

inline mat4 translate(v3 t) {
	mat4 result = mat4d(1.0f);

	result.elements[3][0] += t.x;
	result.elements[3][1] += t.y;
	result.elements[3][2] += t.z;

	return result;
}

inline mat4 translate_mat4(mat4 m, v3 t) {
	mat4 result = m;

	result.elements[3][0] += t.x;
	result.elements[3][1] += t.y;
	result.elements[3][2] += t.z;

	return result;
}

inline mat4 multiply_mat4(mat4 a, mat4 b) {
	mat4 result = {0};

#if USE_SSE
	mat4 left = transpose(a);
	mat4 right = transpose(b);

	result.rows[0] = linear_combine(left.rows[0], right);
	result.rows[1] = linear_combine(left.rows[1], right);
	result.rows[2] = linear_combine(left.rows[2], right);
	result.rows[3] = linear_combine(left.rows[3], right);

	result = transpose(result);
#else
	for (i32 col = 0; col < 4; col++) {
		for (i32 row = 0; row < 4; row++) {
			float sum = 0;
			for (i32 current = 0; current < 4; current++) {
				sum += a.elements[current][row] * b.elements[col][current];
			}
			result.elements[col][row] = sum;
		}
	}
#endif
	return result;
}

inline v3 multiply_mat4_v3(mat4 m, v3 a, float* w) {
	v3 result;
	float x = a.x, y = a.x, z = a.z;

	result.x = x * m.elements[0][0] + y * m.elements[1][0] + z * m.elements[2][0] + *w * m.elements[3][0];
	result.y = x * m.elements[0][1] + y * m.elements[1][1] + z * m.elements[2][1] + *w * m.elements[3][1];
	result.z = x * m.elements[0][2] + y * m.elements[1][2] + z * m.elements[2][2] + *w * m.elements[3][2];
	*w =	   x * m.elements[0][3] + y * m.elements[1][3] + z * m.elements[2][3] + *w * m.elements[3][3];

	return result;
}

inline v4 multiply_mat4_v4(mat4 m, v4 a) {
	v4 result;
	float x = a.x, y = a.x, z = a.z, w = a.w;

	result.x = x * m.elements[0][0] + y * m.elements[1][0] + z * m.elements[2][0] + w * m.elements[3][0];
	result.y = x * m.elements[0][1] + y * m.elements[1][1] + z * m.elements[2][1] + w * m.elements[3][1];
	result.z = x * m.elements[0][2] + y * m.elements[1][2] + z * m.elements[2][2] + w * m.elements[3][2];
	result.w = x * m.elements[0][3] + y * m.elements[1][3] + z * m.elements[2][3] + w * m.elements[3][3];

	return result;
}

inline v3 v3_from_v4(v4 a) {
	return V3(a.x, a.y, a.z);
}

inline v4 v4_from_v3(v3 a, float w) {
	return V4(a.x, a.y, a.z, w);
}

inline mat4 scale_mat4(v3 scale) {
	mat4 result = mat4d(1.0f);

	result.elements[0][0] = scale.x;
	result.elements[1][1] = scale.y;
	result.elements[2][2] = scale.z;

	return result;
}

inline mat4 rotate(float angle, v3 axis) {
	mat4 result = mat4d(1.0f);
	axis = normalize(axis);

	float sin_theta = sinf(to_radians(angle));
	float cos_theta = cosf(to_radians(angle));
	float cos_value = 1.0f - cos_theta;

	result.elements[0][0] = (axis.x * axis.x * cos_value) + cos_theta;
	result.elements[0][1] = (axis.x * axis.y * cos_value) + (axis.z * sin_theta);
	result.elements[0][2] = (axis.x * axis.z * cos_value) - (axis.y * sin_theta);

	result.elements[1][0] = (axis.y * axis.x * cos_value) - (axis.z * sin_theta);
	result.elements[1][1] = (axis.y * axis.y * cos_value) + cos_theta;
	result.elements[1][2] = (axis.y * axis.z * cos_value) + (axis.x * sin_theta);

	result.elements[2][0] = (axis.z * axis.x * cos_value) + (axis.y * sin_theta);
	result.elements[2][1] = (axis.z * axis.y * cos_value) - (axis.x * sin_theta);
	result.elements[2][2] = (axis.z * axis.z * cos_value) + cos_theta;

	return result;
}

inline float dot(v3 a, v3 b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

inline float length_square_v3(v3 a) {
	return dot(a, a);
}

inline float length_v3(v3 a) {
	return sqrtf(length_square_v3(a));
}

inline v3 diff_v3(v3 a, v3 b) {
	v3 result = a;

	result.x -= b.x;
	result.y -= b.y;
	result.z -= b.z;

	return result;
}

inline v3 normalize(v3 a) {
	v3 result = {0};
	float length = length_v3(a);

	if (length != 0.0f) { 	// To be sure that we don't get a math error (divide by 0)
		result.x = a.x * (1.0f / length);
		result.y = a.y * (1.0f / length);
		result.z = a.z * (1.0f / length);
	}
	return result;
}

inline v3 cross_product(v3 a, v3 b) {
	v3 result = {0};

	result.x = (a.y * b.z) - (a.z * b.y);
	result.y = (a.z * b.x) - (a.x * b.z);
	result.z = (a.x * b.y) - (a.y * b.x);

	return result;
}

inline float to_radians(float angle) {
	return angle * (PI32 / 180.0f);
}

mat4 perspective(float fov, float aspect, float z_near, float z_far) {
	mat4 result = {0};

	float tan_theta_over2 = tanf(fov * (PI32 / 360.0f));

	result.elements[0][0] = 1.0f / tan_theta_over2;
	result.elements[1][1] = aspect / tan_theta_over2;
	result.elements[2][3] = -1.0f;
	result.elements[2][2] = (z_near + z_far) / (z_near - z_far);
	result.elements[3][2] = (1.0f * z_near * z_far) / (z_near - z_far);
	result.elements[3][3] = 0.0f;

	return result;
}

inline mat4 orthographic(float left, float right, float bottom, float top, float z_near, float z_far) {
	mat4 result = {0};

	result.elements[0][0] = 2.0f / (right - left);
	result.elements[1][1] = 2.0f / (top - bottom);
	result.elements[2][2] = 2.0f / (z_near - z_far);
	result.elements[3][3] = 1.0f;

	result.elements[3][0] = (left + right) / (left - right);
	result.elements[3][1] = (bottom + top) / (bottom - top);
	result.elements[3][2] = (z_far + z_near) / (z_near - z_far);

	return result;
}

inline mat4 look_at(v3 eye, v3 center, v3 up) {
	mat4 result = mat4d(1.0f);

	v3 front = normalize(diff_v3(center, eye));
	v3 side = normalize(cross_product(front, up));
	v3 u = cross_product(side, front);

	result.elements[0][0] = side.x;
	result.elements[0][1] = u.x;
	result.elements[0][2] = -front.x;

	result.elements[1][0] = side.y;
	result.elements[1][1] = u.y;
	result.elements[1][2] = -front.y;

	result.elements[2][0] = side.z;
	result.elements[2][1] = u.z;
	result.elements[2][2] = -front.z;

	result.elements[3][0] = -dot(side, eye);
	result.elements[3][1] = -dot(u, eye);
	result.elements[3][2] = dot(front, eye);
	result.elements[3][3] = 1.0f;

	return result;
}

#if USE_SSE

inline __m128 linear_combine(__m128 a, mat4 b) {
	__m128 result;

	result = _mm_mul_ps(_mm_shuffle_ps(a, a, 0x00), b.rows[0]);
	result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0x55), b.rows[1]));
	result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xaa), b.rows[2]));
	result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xff), b.rows[3]));

	return result;
}

inline mat4 transpose(mat4 a) {
	mat4 result = a;

	_MM_TRANSPOSE4_PS(result.rows[0], result.rows[1], result.rows[2], result.rows[3]);

	return result;
}

#else

inline transpose(mat4 a) {
	mat4 result = a;

	return result;
}

#endif

#endif
