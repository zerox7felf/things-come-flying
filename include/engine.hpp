// engine.hpp

#ifndef _ENGINE_HPP
#define _ENGINE_HPP

#include "common.hpp"
#include "memory.hpp"
#include "resource.hpp"
#include "matrix_math.hpp"

typedef struct Engine {
	u8 is_running;
	u8 animation_playing;
	float delta_time;
	float total_time;
	float time_scale;
	double mouse_x;
	double mouse_y;
	double scroll_x;
	double scroll_y;
} Engine;

extern Engine engine;

i32 engine_start();

#endif
