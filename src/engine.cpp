// engine.cpp

#include "engine.h"
#include "window.h"

typedef struct Engine {
	i32 is_running;
	float delta_time;
} Engine;

static void engine_initialize(Engine* engine);
static i32 engine_run(Engine* engine);

void engine_initialize(Engine* engine) {
	engine->is_running = 1;
	engine->delta_time = 0;
}

i32 engine_run(Engine* engine) {
	i32 tick = 0;
	while (engine->is_running && window_poll_events() >= 0) {

		window_swap_buffers();
		window_clear_buffers(0, 0, 0);
	}
	return NoError;
}

i32 engine_start() {
	i32 result = NoError;
	Engine engine;
	engine_initialize(&engine);

	if ((result = window_open("Solar System", 800, 600, 0)) == NoError) {
		engine_run(&engine);
		window_close();
	}
	return result;
}
