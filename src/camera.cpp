// camera.cpp

#include "common.hpp"
#include "matrix_math.hpp"
#include "renderer.hpp"
#include "engine.hpp"	// To get access to the mouse cursor
#include "window.hpp"
#include "camera.hpp"

static double mouse_sensitivity = 0.05;
static double move_speed = 24;
static float move_interpolation_speed = 5;

#define camera_zoom_default 20.0f	// Zoom-out factor
#define camera_zoom_interpolation_speed 10.0f

Camera camera;
static double last_x = 0.0f;
static double last_y = 0.0f;
static double delta_x = 0.0f;
static double delta_y = 0.0f;

void camera_initialize(v3 pos) {
	camera.pos = pos;
	camera.target_pos = pos;
	camera.up = V3(0.0f, 1.0f, 0.0f);	// y axis is up
	camera.right = V3(1.0f, 0.0f, 0.0f);
	camera.forward = V3(0.0f, 0.0f, 1.0f);
	camera.pitch = 0.0f;
	camera.yaw = 90.0f;
	camera.zoom = camera.zoom_target = camera_zoom_default;
	camera.interpolate = 1;
	camera.interactive_mode = 0;
}

void camera_update(Engine* engine) {
	delta_x = engine->mouse_x - last_x;
	delta_y = last_y - engine->mouse_y;

	last_x = engine->mouse_x;
	last_y = engine->mouse_y;

	delta_x *= mouse_sensitivity;
	delta_y *= mouse_sensitivity;

	if (camera.interactive_mode) {
		camera.yaw += delta_x;
		camera.pitch += delta_y;

        float current_move_speed = move_speed;
		if (key_down[GLFW_KEY_LEFT_ALT]) {
			current_move_speed *= 0.15;
        }
		if (key_down[GLFW_KEY_W]) {
			camera.target_pos = camera.target_pos + camera.forward * engine->delta_time * current_move_speed;
		}
		if (key_down[GLFW_KEY_S]) {
			camera.target_pos = camera.target_pos - camera.forward * engine->delta_time * current_move_speed;
		}
		if (key_down[GLFW_KEY_A]) {
			camera.target_pos = camera.target_pos - camera.right * engine->delta_time * current_move_speed;
		}
		if (key_down[GLFW_KEY_D]) {
			camera.target_pos = camera.target_pos + camera.right * engine->delta_time * current_move_speed;
		}
		if (key_down[GLFW_KEY_LEFT_SHIFT]) {
			camera.target_pos.y += engine->delta_time * current_move_speed;
        }
		if (key_down[GLFW_KEY_LEFT_CONTROL]) {
			camera.target_pos.y -= engine->delta_time * current_move_speed;
        }
	}
	else {
		camera.yaw += 2.5f * engine->delta_time;
	}

	if (camera.pitch >= 89.0f) {
		camera.pitch = 89.0f;
	}
	if (camera.pitch <= -89.0f) {
		camera.pitch = -89.0f;
	}

	v3 camera_dir = V3(
		cos(to_radians(camera.yaw)) * cos(to_radians(camera.pitch)),
		sin(to_radians(camera.pitch)),
		sin(to_radians(camera.yaw)) * cos(to_radians(camera.pitch))
	);

	if (camera.interpolate) {
		camera.pos = lerp(camera.pos, camera.target_pos, move_interpolation_speed * engine->delta_time);
	}
	else {
		camera.pos = camera.target_pos;
	}

	camera.zoom = lerp(camera.zoom, camera.zoom_target, camera_zoom_interpolation_speed * engine->delta_time);

	camera.forward = normalize(camera_dir);
	camera.right = normalize(cross_product(camera.forward, V3(0.0f, 1.0f, 0.0f)));
	camera.up = normalize(cross_product(camera.right, camera.forward)); // Upwards in local space

	view = look_at(camera.pos, camera.pos + camera.forward, camera.up);
}
