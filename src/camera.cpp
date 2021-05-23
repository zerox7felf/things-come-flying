// camera.cpp

#include "common.hpp"
#include "matrix_math.hpp"
#include "renderer.hpp"
#include "engine.hpp"	// To get access to the mouse cursor
#include "camera.hpp"

#define mouse_sensitivity 0.05f

Camera camera;
static double last_x = 0.0f;
static double last_y = 0.0f;
static double delta_x = 0.0f;
static double delta_y = 0.0f;

void camera_initialize(v3 pos) {
	camera.pos = pos;
	camera.up = V3(0.0f, 1.0f, 0.0f);	// y axis is up
	camera.right = V3(1.0f, 0.0f, 0.0f);
	camera.forward = V3(0.0f, 0.0f, 1.0f);
	camera.pitch = 0.0f;
	camera.yaw = 90.0f;
}

void camera_update() {
	delta_x = engine.mouse_x - last_x;
	delta_y = last_y - engine.mouse_y;

	last_x = engine.mouse_x;
	last_y = engine.mouse_y;

	delta_x *= mouse_sensitivity;
	delta_y *= mouse_sensitivity;

	camera.yaw += delta_x;
	camera.pitch += delta_y;

	if (camera.pitch >= 89.0f) {
		camera.pitch = 89.0f;
	}
	if (camera.pitch <= -89.0f) {
		camera.pitch = -89.0f;
	}

	v3 camera_dir = V3(
		cosf(to_radians(camera.yaw)) * cosf(to_radians(camera.pitch)),
		sinf(to_radians(camera.pitch)),
		sinf(to_radians(camera.yaw)) * cosf(to_radians(camera.pitch))
	);

	camera.forward = normalize(camera_dir);
	camera.right = normalize(cross_product(camera.forward, V3(0.0f, 1.0f, 0.0f)));
	camera.up = normalize(cross_product(camera.right, camera.forward));

	view = look_at(camera.pos, camera.pos + camera.forward, camera.up);
}
