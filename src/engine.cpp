// engine.cpp

#include "engine.h"
#include "window.h"
#include "camera.h"
#include "renderer.h"

Engine engine;

static void engine_initialize(Engine* engine);
static i32 engine_run(Engine* engine);

void engine_initialize(Engine* engine) {
	engine->is_running = 1;
	engine->delta_time = 0;
	engine->mouse_x = 0;
	engine->mouse_y = 0;
	camera_initialize(V3(0, 0, 0));
}

i32 engine_run(Engine* engine) {
	while (engine->is_running && window_poll_events() >= 0) {
		if (key_down[GLFW_KEY_ESCAPE]) {
			engine->is_running = 0;
		}
		if (key_down[GLFW_KEY_F11]) {
			window_toggle_fullscreen();
		}

		window_get_cursor(&engine->mouse_x, &engine->mouse_y);
		camera_update();

		render_cube(V3(0, 0, -20), V3(0, 0, 0), V3(2, 1, 1));
		render_cube(V3(0, 0, 20), V3(0, 0, 0), V3(1, 2, 1));

		window_swap_buffers();
		window_clear_buffers(0, 0, 0);
	}
	return NoError;
}

i32 engine_start() {
	i32 result = NoError;
	engine_initialize(&engine);

	if ((result = window_open("Solar System", 800, 600, 0 /* fullscreen */, 0 /* vsync */)) == NoError) {
		renderer_initialize();
		engine_run(&engine);
		window_close();
	}
	return result;
}
