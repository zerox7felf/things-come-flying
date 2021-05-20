// engine.h

#ifndef _ENGINE_H
#define _ENGINE_H

#include "common.h"
#include "matrix_math.h"

typedef struct Engine {
	i32 is_running;
	float delta_time;
} Engine;

extern Engine engine;

i32 engine_start();

#endif
