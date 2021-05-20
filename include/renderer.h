// renderer.h

#ifndef _RENDERER_H
#define _RENDERER_H

#include "matrix_math.h"

typedef struct Model {
  u32 draw_count;
  u32 vao;
  u32 vbo;
  u32 ebo;
} Model;

extern mat4 projection;
extern mat4 view;
extern mat4 model;

i32 renderer_initialize();

void render_cube(v3 position, v3 rotation, v3 size);

void renderer_destroy();

#endif
