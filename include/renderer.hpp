// renderer.hpp

#ifndef _RENDERER_HPP
#define _RENDERER_HPP

#include "matrix_math.hpp"

typedef struct Model {
  u32 draw_count;
  u32 vao;
  u32 vbo;
  u32 ebo;
} Model;

typedef struct Material {
  float emission;
  float shininess;
  u32 texture_id;
} Material;

extern mat4 projection;
extern mat4 view;
extern mat4 model;

i32 renderer_initialize();

void render_cube(v3 position, v3 rotation, v3 size);

void render_mesh(v3 position, v3 rotation, v3 size, Material material);

void renderer_destroy();

#endif
