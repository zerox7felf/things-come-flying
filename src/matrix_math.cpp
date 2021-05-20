// matrix_math.cpp
// based on HandMadeMath

#include <math.h>

#include "matrix_math.h"

inline mat4 mat4d(float diagonal) {
	mat4 result = {0};

	result.elements[0][0] = diagonal;
	result.elements[1][1] = diagonal;
	result.elements[2][2] = diagonal;
	result.elements[3][3] = diagonal;

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

#if USE_SSE && 0
	// TODO(lucas): Implement SSE matrix multiply
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

inline mat4 scale_mat4(v3 scale) {
	mat4 result = mat4d(1.0f);

	result.elements[0][0] = scale.x;
	result.elements[1][1] = scale.y;
	result.elements[2][2] = scale.z;

	return result;
}

inline mat4 rotate(float angle, v3 axis) {
	mat4 result = mat4d(1.0f);
	axis = normalize_v3(axis);

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

inline float dot_v3(v3 a, v3 b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

inline float length_square_v3(v3 a) {
	return dot_v3(a, a);
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

inline v3 normalize_v3(v3 a) {
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

  v3 front = normalize_v3(diff_v3(center, eye));
  v3 side = normalize_v3(cross_product(front, up));
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

  result.elements[3][0] = -dot_v3(side, eye);
  result.elements[3][1] = -dot_v3(u, eye);
  result.elements[3][2] = dot_v3(front, eye);
  result.elements[3][3] = 1.0f;

  return result;
}
