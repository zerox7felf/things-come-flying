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

typedef union Value_map_value {
    float constant;
    Texture map;
} Value_map_value;

enum Value_map_type {
    VALUE_MAP_MAP = 0,
    VALUE_MAP_CONST = 1,
};

typedef struct Value_map {
    Value_map_value value;
    u8 type;
} Value_map;

typedef struct Material {
    Value_map ambient;
    Value_map diffuse;
    Value_map specular;
    float shininess;
    Texture color_map;
    Texture texture1;
    float texture_mix;
} Material;

extern mat4 projection;
extern mat4 ortho_projection;
extern mat4 view;
extern mat4 model;

enum Fbo_type {
	FBO_STANDARD_FRAMEBUFFER = -1,
	FBO_COLOR,
	FBO_BRIGHTNESS_EXTRACT,
	FBO_COMBINE,
	FBO_V_BLUR,
	FBO_H_BLUR,

	MAX_FBO,
};

typedef struct Fbo_attributes {
	u32 shader_id;
	union {
		struct {
			u32 texture1;
			float mix;
		} combine;
		struct {
			u8 vertical;
		} blur;
		struct {
			float factor;
		} extract;
	};
} Fbo_attributes;

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
	u8 use_post_processing;
	u8 initialized;
} Render_state;

i32 renderer_initialize();

void renderer_framebuffer_callback(i32 width, i32 height);

void renderer_clear_fbo();

void renderer_bind_fbo(i32 fbo_id);

void renderer_unbind_fbo();

void render_fbo(i32 fbo_id, i32 target_fbo, Fbo_attributes attr);

void renderer_post_process();

void renderer_toggle_post_processing();

void renderer_clear_fbos();

void render_mesh(v3 position, v3 rotation, v3 size, u32 mesh_id, Material material);

void render_skybox(u32 skybox_id, float brightness);

void renderer_destroy();

#endif
