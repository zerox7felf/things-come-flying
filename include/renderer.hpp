// renderer.hpp

#ifndef _RENDERER_HPP
#define _RENDERER_HPP

#include "resource.hpp"
#include "matrix_math.hpp"

typedef struct Model {
  u32 draw_count;
  u32 vao;
  u32 vbo;
  u32 ebo;
} Model;

typedef struct Fbo {
	u32 texture;
	u32 depth;
	u32 fbo;
	i32 width;
	i32 height;
} Fbo;

typedef struct Texture {
	u32 id = 0;
	v2 offset = V2(0, 0);
} Texture;

typedef struct Material {
  float ambient;
  float diffuse;
  float specular;
  float shininess;
  Texture texture0;
  Texture texture1;
  float texture_mix;
} Material;

extern mat4 projection;
extern mat4 ortho_projection;
extern mat4 view;
extern mat4 model;

enum Fbo_type {
	FBO_COLOR,
	FBO_COMBINE,

	MAX_FBO,
};

typedef struct Render_state {
	u32 textures[MAX_TEXTURE];
	u32 texture_count;

	Fbo fbos[MAX_FBO];
	u32 fbo_count;

	u32 cube_maps[MAX_CUBE_MAP];
	u32 cube_map_count;

	Model models[MAX_MESH];
	u32 model_count;

	Resources resources;
	i32 depth_func;
	u8 initialized;
} Render_state;

i32 renderer_initialize();

void renderer_framebuffer_callback(i32 width, i32 height);

void renderer_clear_fbo(u32 fbo_id);

void renderer_bind_fbo(u32 fbo_id);

void renderer_unbind_fbo();

void render_fbo(u32 fbo_id);

void render_mesh(v3 position, v3 rotation, v3 size, u32 mesh_id, Material material);

void render_skybox(u32 skybox_id, float brightness);

void renderer_destroy();

#endif
