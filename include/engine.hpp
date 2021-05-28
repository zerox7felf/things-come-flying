// engine.hpp

#ifndef _ENGINE_HPP
#define _ENGINE_HPP

#include "common.hpp"
#include "memory.hpp"
#include "resource.hpp"
#include "matrix_math.hpp"
#include "entity.hpp"

#define MAX_ENTITY 128

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
	struct Entity entities[MAX_ENTITY];
	u32 entity_count;
	u32 target_entity_index;
} Engine;

extern Engine engine;

Entity* engine_push_empty_entity(Engine* engine);

Entity* engine_get_target(Engine* engine);

i32 engine_start();

#endif
