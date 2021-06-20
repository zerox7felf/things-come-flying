// renderer.cpp

#include <GL/glew.h>
#include <string>

#if defined(__APPLE__)
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
#endif

#include "common.hpp"
#include "mesh.hpp"
#include "image.hpp"
#include "window.hpp"
#include "camera.hpp"
#include "renderer.hpp"

mat4 projection;
mat4 ortho_projection;
mat4 view;
mat4 model;

Render_state render_state = {};

/*u32 diffuse_shader = 0,
	skybox_shader = 0,
	texture_shader = 0,
	combine_shader = 0,
	blur_shader = 0,
    flare_shader = 0,
	brightness_extract_shader = 0;*/

Model cube_model;
Fbo* current_fbo = NULL;

#define SHADER_ERROR_BUFFER_SIZE 512

u32 quad_vbo = 0;
u32 quad_vao = 0;

static float quad_vertices[] = {
	// vertex,	uv
	0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,

	0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 1.0f, 0.0f,
};

#define quad_vertex_count ARR_SIZE(quad_data)

#define CUBE_SIZE 1.0f

float cube_vertices[] = {
	-CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE,
	-CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE,
	CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE,
	CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE,
	CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE,
	-CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE,

	-CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE,
	-CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE,
	-CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE,
	-CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE,
	-CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE,
	-CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE,

	CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE,
	CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE,
	CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE,
	CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE,
	CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE,
	CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE,

	-CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE,
	-CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE,
	CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE,
	CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE,
	CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE,
	-CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE,

	-CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE,
	CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE,
	CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE,
	CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE,
	-CUBE_SIZE,  CUBE_SIZE,  CUBE_SIZE,
	-CUBE_SIZE,  CUBE_SIZE, -CUBE_SIZE,

	-CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE,
	-CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE,
	CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE,
	CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE,
	-CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE,
	CUBE_SIZE, -CUBE_SIZE,  CUBE_SIZE
};

static void opengl_initialize(Render_state* renderer);
static i32 render_state_initialize(Render_state* renderer);
static i32 shader_compile_from_source(const char* vert_source, const char* frag_source, u32* program_out);
static i32 shader_compile_from_file(const char* path, u32* program_out);
static void upload_quad_data();
static i32 upload_texture(Render_state* renderer, Image* image, u32* texture_id);
static i32 upload_skybox_texture(Render_state* renderer, u32 skybox_id, u32* texture_id);
static i32 upload_model(Model* model, float* vertices, u32 vertex_count);
static i32 upload_model(Model* model, Mesh* mesh);
static void unload_model(Model* model);
static void unload_texture(u32* texture_id);
static void store_attribute(Model* model, i32 attribute_index, u32 count, u32 size, void* data);
static void fbos_initialize(Render_state* renderer, i32 width, i32 height);
static void fbos_unload(Render_state* renderer);
static void fbo_initialize(Fbo* fbo, i32 width, i32 height, i32 filter_method);
static void fbo_unload(Fbo* fbo);

i32 shader_compile_from_source(const char* vert_source, const char* frag_source, u32* program_out) {
	i32 result = NoError;
	i32 compile_report = 0;
	u32 program = 0;
	char err_log[SHADER_ERROR_BUFFER_SIZE] = {};
	u32 vert_shader = 0, frag_shader = 0;

	// Create and compile vertex shader
	vert_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_shader, 1, &vert_source, NULL);
	glCompileShader(vert_shader);

	// Fetch compilation status of vertex shader
	glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &compile_report);
	if (!compile_report) {
		glGetShaderInfoLog(vert_shader, SHADER_ERROR_BUFFER_SIZE, NULL, err_log);
		fprintf(stderr, "error in vertex shader: %s\n", err_log);
		result = Error;
		goto done;
	}

	// Create and compile fragment shader
	frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &frag_source, NULL);
	glCompileShader(frag_shader);

	// Fetch compilation status of fragment shader
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &compile_report);
	if (!compile_report) {
		glGetShaderInfoLog(frag_shader, SHADER_ERROR_BUFFER_SIZE, NULL, err_log);
		fprintf(stderr, "error in fragment shader: %s\n", err_log);
		result = Error;
		goto done;
	}

	program = glCreateProgram();
	glAttachShader(program, vert_shader);
	glAttachShader(program, frag_shader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_VALIDATE_STATUS, &compile_report);
#if 0
	if (!compile_report) {
		glGetProgramInfoLog(program, SHADER_ERROR_BUFFER_SIZE, NULL, err_log);
		fprintf(stderr, "shader compile error: %s\n", err_log);
		goto done;
	}
#endif

	*program_out = program;

done:
	if (vert_shader > 0)
		glDeleteShader(vert_shader);
	if (frag_shader > 0)
		glDeleteShader(frag_shader);
	return result;
}

i32 shader_compile_from_file(const char* path, u32* program_out) {
	i32 result = NoError;

	Buffer vert_source = {};
	Buffer frag_source = {};
	char vert_path[MAX_PATH_SIZE] = {0};
	char frag_path[MAX_PATH_SIZE] = {0};
	snprintf(vert_path, MAX_PATH_SIZE, "%s.vert", path);
	snprintf(frag_path, MAX_PATH_SIZE, "%s.frag", path);
	if ((result = read_and_null_terminate_file(vert_path, &vert_source)) != NoError) {
		goto done;
	}
	if ((result = read_and_null_terminate_file(frag_path, &frag_source)) != NoError) {
		goto done;
	}
	result = shader_compile_from_source(vert_source.data, frag_source.data, program_out);
done:
	buffer_free(&vert_source);
	buffer_free(&frag_source);
	return result;
}

void upload_quad_data() {
	glGenVertexArrays(1, &quad_vao);
	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	glBindVertexArray(quad_vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

i32 upload_texture(Render_state* renderer, Image* image, u32* texture_id) {
	i32 result = NoError;
	i32 texture_format = image->bytes_per_pixel == 4 ? GL_RGBA : GL_RGB;

	glGenTextures(1, texture_id);
	glBindTexture(GL_TEXTURE_2D, *texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, texture_format, image->width, image->height, 0, texture_format, GL_UNSIGNED_BYTE, image->buffer);
	glBindTexture(GL_TEXTURE_2D, 0);
	return result;
}

i32 upload_mipmap_texture(Render_state* renderer, Image* image, u32* texture_id) {
	i32 result = NoError;
	i32 texture_format = image->bytes_per_pixel == 4 ? GL_RGBA : GL_RGB;

	glGenTextures(1, texture_id);
	glBindTexture(GL_TEXTURE_2D, *texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D, texture_format, image->width, image->height, texture_format, GL_UNSIGNED_BYTE, image->buffer);
	glBindTexture(GL_TEXTURE_2D, 0);
	return result;
}

i32 upload_skybox_texture(Render_state* renderer, u32 skybox_id, u32* texture_id) {
	glGenTextures(1, texture_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, *texture_id);

	for (i32 i = 0; i < 6; i++) {
		Image* image = &renderer->resources.skybox_images[i + skybox_id];
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->buffer);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return NoError;
}

i32 upload_model(Model* model, float* vertices, u32 vertex_count) {
	model->draw_count = vertex_count / 3;

	glGenVertexArrays(1, &model->vao);
	glBindVertexArray(model->vao);

	glGenBuffers(1, &model->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, 3 * sizeof(float), GL_FALSE, NULL);
	glBindVertexArray(0);
	return NoError;
}

i32 upload_model(Model* model, Mesh* mesh) {
	model->draw_count = mesh->vertex_index_count;	// We are using indexed rendering, which means that the draw count is equal to the amount of indices on the mesh

	glGenVertexArrays(1, &model->vao);
	glBindVertexArray(model->vao);

	glGenBuffers(1, &model->ebo);

	store_attribute(model, 0, 3, mesh->vertex_count * sizeof(v3), &mesh->vertices[0]);
	store_attribute(model, 1, 2, mesh->uv_count * sizeof(v2), &mesh->uv[0]);
	store_attribute(model, 2, 3, mesh->normal_count * sizeof(v3), &mesh->normals[0]);
	store_attribute(model, 3, 3, mesh->tangent_count * sizeof(v3), &mesh->tangents[0]);
	store_attribute(model, 4, 3, mesh->bitangent_count * sizeof(v3), &mesh->bitangents[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->vertex_index_count * sizeof(u32), &mesh->vertex_indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
	return NoError;
}

void unload_model(Model* model) {
	glDeleteVertexArrays(1, &model->vao);
	glDeleteVertexArrays(1, &model->vbo);
	glDeleteBuffers(1, &model->ebo);
}

void unload_texture(u32* texture_id) {
	glDeleteTextures(1, texture_id);
}

void store_attribute(Model* model, i32 attribute_index, u32 count, u32 size, void* data) {
	glEnableVertexAttribArray(attribute_index);
	glGenBuffers(1, &model->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	glVertexAttribPointer(attribute_index, count, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void fbos_initialize(Render_state* renderer, i32 width, i32 height) {
	for (i32 i = 0; i < MAX_FBO; ++i) {
		Fbo* fbo = &renderer->fbos[i];
#if 1
		if (i == FBO_H_BLUR || i == FBO_V_BLUR || i == FBO_BRIGHTNESS_EXTRACT) {
			fbo_initialize(fbo, width, height, GL_LINEAR);
		}
		else {
			fbo_initialize(fbo, width, height, GL_NEAREST);
		}
#else
		fbo_initialize(fbo, width, height, GL_NEAREST);
#endif
		renderer->fbo_count++;
	}
}

void fbos_unload(Render_state* renderer) {
	for (i32 i = 0; i < MAX_FBO; ++i) {
		Fbo* fbo = &renderer->fbos[i];
		fbo_unload(fbo);
	}
	renderer->fbo_count = 0;
}

void fbo_initialize(Fbo* fbo, i32 width, i32 height, i32 filter_method) {
	fbo->width = width;
	fbo->height = height;

	glGenFramebuffers(1, &fbo->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);

	glGenTextures(1, &fbo->texture);
	glBindTexture(GL_TEXTURE_2D, fbo->texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_method);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_method);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// TODO: Other wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &fbo->depth);
	glBindTexture(GL_TEXTURE_2D, fbo->depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// NOTE(lucas): Was GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo->texture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbo->depth, 0);

	GLenum draw_buffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_DEPTH_ATTACHMENT,
	};
	glDrawBuffers(2, draw_buffers);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo->fbo);
	GLenum err = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (err != GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO error: %i\n", err);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void fbo_unload(Fbo* fbo) {
	glDeleteTextures(1, &fbo->texture);
	glDeleteTextures(1, &fbo->depth);
	glDeleteFramebuffers(1, &fbo->fbo);
	fbo->width = 0;
	fbo->height = 0;
}

void opengl_initialize(Render_state* renderer) {
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
    // Disabled, we want our sprites to work!
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK); 
	glEnable(GL_DEPTH_TEST);
	glAlphaFunc(GL_GREATER, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_CUBE_MAP_EXT);
	glEnable(GL_FRAMEBUFFER_SRGB);

	renderer->depth_func = GL_LESS;
	glDepthFunc(renderer->depth_func);
}

i32 render_state_initialize(Render_state* renderer) {
	opengl_initialize(renderer);
	upload_quad_data();
	Resources* res = &renderer->resources;
	renderer->texture_count = 0;
	renderer->model_count = 0;
	renderer->cube_map_count = 0;

	resources_initialize(res);
	resources_load(res);

	for (u32 i = 0; i < res->image_count; i++) {
		Image* image = &res->images[i];
		u32* texture_id = &renderer->textures[i];
		upload_mipmap_texture(renderer, image, texture_id);
		renderer->texture_count++;
	}

	for (u32 i = 0; i < res->skybox_count / 6; i++) {
		u32* cube_map_id = &renderer->cube_maps[i];
		upload_skybox_texture(renderer, i * 6, cube_map_id);
		renderer->cube_map_count++;
	}

	for (u32 i = 0; i < res->mesh_count; i++) {
		Mesh* mesh = &res->meshes[i];
		Model* model = &renderer->models[i];
		upload_model(model, mesh);
		renderer->model_count++;
	}

    for (int i = 0; i < MAX_SHADER; i++) {
        printf("Compiling shader %s...\n", shader_path[i]);
        shader_compile_from_file(shader_path[i], &renderer->shaders[i]);
    }

	renderer->fbo_count = 0;
	upload_model(&cube_model, cube_vertices, ARR_SIZE(cube_vertices));
	fbos_initialize(renderer, window_width(), window_height());
	return NoError;
}

i32 renderer_initialize() {
	i32 glew_error = glewInit();
	if (glew_error != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(glew_error));
		return Error;
	}
	view = mat4d(1.0f);
	model = mat4d(1.0f);

	render_state_initialize(&render_state);

	/*shader_compile_from_file("resource/shader/textured_phong", &diffuse_shader);
	shader_compile_from_file("resource/shader/skybox", &skybox_shader);
	shader_compile_from_file("resource/shader/texture", &texture_shader);
	shader_compile_from_file("resource/shader/combine", &combine_shader);
	shader_compile_from_file("resource/shader/blur", &blur_shader);
	shader_compile_from_file("resource/shader/flare", &flare_shader);
	shader_compile_from_file("resource/shader/brightness_extract", &brightness_extract_shader);*/

	render_state.use_post_processing = 1;
	render_state.initialized = 1;
	return 0;
}

void renderer_framebuffer_callback(i32 width, i32 height) {
	Render_state* renderer = &render_state;
	if (!renderer->initialized) {
		return;
	}
	fbos_unload(renderer);
	fbos_initialize(renderer, width, height);
}

// Clears the currently bound fbo
void renderer_clear_fbo() {
	Fbo* fbo = current_fbo;
	if (fbo) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_bind_fbo(i32 fbo_id) {
	Render_state* renderer = &render_state;
	if (fbo_id >= 0) {
		Fbo* fbo = &renderer->fbos[fbo_id];
		current_fbo = fbo;
		glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
	}
	else {
		current_fbo = NULL;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void renderer_unbind_fbo() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	current_fbo = NULL;
}

void render_fbo(i32 fbo_id, i32 target_fbo, Fbo_attributes attr) {
	renderer_bind_fbo(target_fbo);

	Render_state* renderer = &render_state;
	Fbo* fbo = &renderer->fbos[fbo_id];

	u32 handle = attr.shader_id;

	glUseProgram(handle);
	u32 texture0 = fbo->texture;

	float width = window_width();
	float height = window_height();

	model = translate(V3(0, 0, 0));
	model = multiply_mat4(model, scale_mat4(V3(width, height, 1)));

	glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), 1, GL_FALSE, (float*)&ortho_projection);
	glUniformMatrix4fv(glGetUniformLocation(handle, "model"), 1, GL_FALSE, (float*)&model);

	glUniform1i(glGetUniformLocation(handle, "texture0"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);

	// Do bindings depending on which fbo we are handling
	switch (fbo_id) {
		case FBO_COMBINE: {
			glUniform1i(glGetUniformLocation(handle, "texture1"), 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, attr.combine.texture1);
			glUniform1f(glGetUniformLocation(handle, "mix"), attr.combine.mix);
			break;
		}
		case FBO_V_BLUR:
		case FBO_H_BLUR: {
			glUniform1i(glGetUniformLocation(handle, "vertical"), attr.blur.vertical);
			break;
		}
		case FBO_BRIGHTNESS_EXTRACT: {
			glUniform1f(glGetUniformLocation(handle, "factor"), attr.extract.factor);
			break;
		}
		default:
			break;
	}

	glEnableVertexAttribArray(0);

	glBindVertexArray(quad_vao);

	glDisable(GL_DEPTH_TEST);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glEnable(GL_DEPTH_TEST);

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);

	glUseProgram(0);
}

void renderer_post_process() {
	Render_state* renderer = &render_state;

	if (!renderer->use_post_processing) {
		render_fbo(FBO_COLOR, FBO_STANDARD_FRAMEBUFFER, (Fbo_attributes) {
			.shader_id = renderer->shaders[TEXTURE_SHADER], //texture_shader,
			{},
		});
		return;
	}

	render_fbo(FBO_COLOR, FBO_BRIGHTNESS_EXTRACT, (Fbo_attributes) {
		.shader_id = renderer->shaders[TEXTURE_SHADER], //texture_shader,
	});

	render_fbo(FBO_BRIGHTNESS_EXTRACT, FBO_V_BLUR, (Fbo_attributes) {
		.shader_id = renderer->shaders[BRIGHTNESS_EXTRACT_SHADER], //brightness_extract_shader,
		{
			.extract = {
				.factor = 0.2f,
				.keep_color = 1,
			},
		}
	});

	render_fbo(FBO_V_BLUR, FBO_H_BLUR, (Fbo_attributes) {
		.shader_id = renderer->shaders[BLUR_SHADER], //blur_shader,
		{
			.blur = {
				.vertical = 1,
			}
		}
	});

	render_fbo(FBO_H_BLUR, FBO_COMBINE, (Fbo_attributes) {
		.shader_id = renderer->shaders[BLUR_SHADER], //blur_shader,
		{
			.blur = {
				.vertical = 0,
			}
		}
	});

	render_fbo(FBO_COMBINE, FBO_STANDARD_FRAMEBUFFER, (Fbo_attributes) {
		.shader_id = renderer->shaders[COMBINE_SHADER], //combine_shader,
		{
			.combine = {
				.texture1 = renderer->fbos[FBO_COLOR].texture,
				.mix = 0.05f,
			},
		}
	});
}

void renderer_toggle_post_processing() {
	render_state.use_post_processing = !render_state.use_post_processing;
}

void renderer_clear_fbos() {
	Render_state* renderer = &render_state;
	for (u32 i = 0; i < renderer->fbo_count; i++) {
		renderer_bind_fbo(i);
		renderer_clear_fbo();	// Clear the bound framebuffer object
	}
	renderer_unbind_fbo();
	renderer_clear_fbo();	// Clear the normal framebuffer
}

void render_flares(v3 flare_source) {
    // Six flares, three from the center to the screen towards the light (0,1,2)
    // and three towards the opposite edge of the screen (-3, -2, -1)
    float flare_sizes[6]     = {0.75, 0.3, 0.12, 0.25,  0.12,  0.15};
    float flare_positions[6] = {0.95, 0.6, 0.33, 0.25, -0.25, -0.60};
    float flare_opacities[6] = {0.75, 0.8, 1.00, 0.85,  0.60,  0.40};
    i32 flare_textures[6] = {
        TEXTURE_LENSFLARE_1,
        TEXTURE_LENSFLARE_3,
        TEXTURE_LENSFLARE_2,
        TEXTURE_LENSFLARE_2,
        TEXTURE_LENSFLARE_2,
        TEXTURE_LENSFLARE_2,
    };
    for (i32 i = 0; i < 6; i++) {
        render_flare(flare_textures[i], flare_positions[i], flare_sizes[i], flare_opacities[i], flare_source);
    }
}

void render_flare(u32 texture_id, float flare_pos, float flare_size, float flare_opacity, v3 flare_source) {
	Render_state* renderer = &render_state;

	u32 handle = renderer->shaders[FLARE_SHADER]; //flare_shader;

	glUseProgram(handle);
	u32 texture0 = renderer->textures[texture_id];

	float width = std::min(window_width(), window_height());

    model = scale_mat4(V3(width, width, 1)); // Make square
	model = multiply_mat4(model, translate(V3(0.5f, 0, 0))); // Center

	glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), 1, GL_FALSE, (float*)&projection);
	glUniformMatrix4fv(glGetUniformLocation(handle, "orthogonal"), 1, GL_FALSE, (float*)&ortho_projection);
	glUniformMatrix4fv(glGetUniformLocation(handle, "view"), 1, GL_FALSE, (float*)&view);
	glUniformMatrix4fv(glGetUniformLocation(handle, "model"), 1, GL_FALSE, (float*)&model);
	glUniform1f(glGetUniformLocation(handle, "flare_position"), flare_pos);
	glUniform1f(glGetUniformLocation(handle, "flare_size"), flare_size);
	glUniform1f(glGetUniformLocation(handle, "flare_opacity_override"), flare_opacity);
    glUniform3fv(glGetUniformLocation(handle, "flare_source"), 1, (float*)&flare_source);

	glUniform1i(glGetUniformLocation(handle, "flare_texture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);

	glEnableVertexAttribArray(0);
	glBindVertexArray(quad_vao);

	glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glUseProgram(0);
}

void render_mesh(mat4 transformation, i32 mesh_id, Material material, Scene* scene) {
	if (mesh_id < 0 || mesh_id >= MAX_MESH) {
		return;
	}
	Render_state* renderer = &render_state;
	u32 texture1 = renderer->textures[material.texture1.id];

    u32 color_map = renderer->textures[material.color_map.id];
    u32 ambient_map = renderer->textures[material.ambient.type == VALUE_MAP_MAP ? material.ambient.value.map.id : 0];
    u32 diffuse_map = renderer->textures[material.diffuse.type == VALUE_MAP_MAP ? material.diffuse.value.map.id : 0];
    u32 specular_map = renderer->textures[material.specular.type == VALUE_MAP_MAP ? material.specular.value.map.id : 0];
    u32 normal_map = renderer->textures[material.normal.type == VALUE_MAP_MAP ? material.normal.value.map.id : 0];
	Model* mesh = &renderer->models[mesh_id];

	//u32 handle = renderer->shaders[DIFFUSE_SHADER];
	u32 handle = renderer->shaders[material.shader_index];
	glUseProgram(handle);

	/*model = translate(position);
	model = multiply_mat4(model, rotate(rotation.y, V3(0.0f, 1.0f, 0.0f)));
	model = multiply_mat4(model, rotate(rotation.z, V3(0.0f, 0.0f, 1.0f)));
	model = multiply_mat4(model, rotate(rotation.x, V3(1.0f, 0.0f, 0.0f)));
	model = multiply_mat4(model, scale_mat4(size));*/
    model = transformation;

    mat4 VM = multiply_mat4(view, model);
    mat4 PVM = multiply_mat4(projection, VM);
    mat4 VM_normal = transpose(inverse(VM));

	glUniformMatrix4fv(glGetUniformLocation(handle, "P"), 1, GL_FALSE, (float*)&projection);
	glUniformMatrix4fv(glGetUniformLocation(handle, "V"), 1, GL_FALSE, (float*)&view);
	glUniformMatrix4fv(glGetUniformLocation(handle, "VM"), 1, GL_FALSE, (float*)&VM);
	glUniformMatrix4fv(glGetUniformLocation(handle, "PVM"), 1, GL_FALSE, (float*)&PVM);
	glUniformMatrix4fv(glGetUniformLocation(handle, "VM_normal"), 1, GL_FALSE, (float*)&VM_normal);

    v2 default_offset = V2(0.0f, 0.0f);
	glUniform2fv(glGetUniformLocation(handle, "color_map_offset"), 1, (float*)&material.color_map.offset);
	glUniform2fv(
        glGetUniformLocation(handle, "ambient_map_offset"), 1,
        material.ambient.type == VALUE_MAP_MAP ? (float*)&material.ambient.value.map.offset : (float*)&default_offset
    );
	glUniform2fv(
        glGetUniformLocation(handle, "diffuse_map_offset"), 1,
        material.diffuse.type == VALUE_MAP_MAP ? (float*)&material.diffuse.value.map.offset : (float*)&default_offset
    );
	glUniform2fv(
        glGetUniformLocation(handle, "specular_map_offset"), 1,
        material.specular.type == VALUE_MAP_MAP ? (float*)&material.specular.value.map.offset : (float*)&default_offset
    );
	glUniform2fv(
        glGetUniformLocation(handle, "normal_map_offset"), 1,
        material.normal.type == VALUE_MAP_MAP ? (float*)&material.normal.value.map.offset : (float*)&default_offset
    );

	glUniform2fv(glGetUniformLocation(handle, "offset1"), 1, (float*)&material.texture1.offset);
	glUniform1f(glGetUniformLocation(handle, "texture_mix"), material.texture_mix);

    // Mappable values
    // If type is not VALUE_MAP_CONST, set their value to -1 (which isn't valid to the shader normally, so should be fine as a flag)
	glUniform1f(glGetUniformLocation(handle, "ambient_amp"), material.ambient.type == VALUE_MAP_CONST ? material.ambient.value.constant : -1.0f);
	glUniform1f(glGetUniformLocation(handle, "diffuse_amp"), material.diffuse.type == VALUE_MAP_CONST ? material.diffuse.value.constant : -1.0f);
	glUniform1f(glGetUniformLocation(handle, "specular_amp"), material.specular.type == VALUE_MAP_CONST ? material.specular.value.constant : -1.0f);
	glUniform1f(glGetUniformLocation(handle, "normal_amp"), material.normal.type == VALUE_MAP_CONST ? material.normal.value.constant : -1.0f);
    // TODO: this -^ is kinda strange and acts like a flag. normals will never be scaled. better solution?
	glUniform1f(glGetUniformLocation(handle, "shininess"), material.shininess);

    for (i32 i = 0; i < scene->num_lights; i++) {
        if (i == MAX_LIGHTS) {
            printf("Warning: too many light sources (max: %d).", MAX_LIGHTS);
            break;
        }
        Point_light light = scene->lights[i];
        std::string uniform_name = "point_lights[" + std::to_string(i) + "]";
        glUniform3fv(glGetUniformLocation(handle, (uniform_name + ".position").c_str()), 1, (float*)&light.position);
        glUniform3fv(glGetUniformLocation(handle, (uniform_name + ".color").c_str()), 1, (float*)&light.color);
        glUniform1f(glGetUniformLocation(handle, (uniform_name + ".falloff_linear").c_str()), light.falloff_linear);
        glUniform1f(glGetUniformLocation(handle, (uniform_name + ".falloff_quadratic").c_str()), light.falloff_quadratic);
        glUniform1f(glGetUniformLocation(handle, (uniform_name + ".ambient").c_str()), light.ambient);
    }
    glUniform1i(glGetUniformLocation(handle, "num_point_lights"), std::min(scene->num_lights, MAX_LIGHTS));

    for (i32 i = 0; i < scene->num_sun_lights; i++) {
        if (i == MAX_LIGHTS) {
            printf("Warning: too many light sources (max: %d).", MAX_LIGHTS);
            break;
        }
        Sun_light light = scene->sun_lights[i];
        std::string uniform_name = "sun_lights[" + std::to_string(i) + "]";

        glUniform3fv(glGetUniformLocation(handle, (uniform_name + ".angle").c_str()), 1, (float*)&light.angle);
        glUniform3fv(glGetUniformLocation(handle, (uniform_name + ".color").c_str()), 1, (float*)&light.color);
        glUniform1f(glGetUniformLocation(handle, (uniform_name + ".falloff_linear").c_str()), light.falloff_linear);
        glUniform1f(glGetUniformLocation(handle, (uniform_name + ".falloff_quadratic").c_str()), light.falloff_quadratic);
        glUniform1f(glGetUniformLocation(handle, (uniform_name + ".ambient").c_str()), light.ambient);
    }
    glUniform1i(glGetUniformLocation(handle, "num_sun_lights"), std::min(scene->num_sun_lights, MAX_LIGHTS));

	glBindVertexArray(mesh->vao);

	glEnableVertexAttribArray(0);	// vertices
	glEnableVertexAttribArray(1);	// uv coordinates
	glEnableVertexAttribArray(2);	// normals

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, color_map);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ambient_map);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, diffuse_map);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, specular_map);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, normal_map);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, texture1);

	glUniform1i(glGetUniformLocation(handle, "color_map"), 0);
	glUniform1i(glGetUniformLocation(handle, "ambient_map"), 1);
	glUniform1i(glGetUniformLocation(handle, "diffuse_map"), 2);
	glUniform1i(glGetUniformLocation(handle, "specular_map"), 3);
	glUniform1i(glGetUniformLocation(handle, "normal_map"), 4);
	glUniform1i(glGetUniformLocation(handle, "obj_texture1"), 5);

	glDrawElements(GL_TRIANGLES, mesh->draw_count, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glBindVertexArray(0);

	glUseProgram(0);
}

void render_skybox(u32 skybox_id, float brightness) {
	Render_state* renderer = &render_state;
	u32 texture = renderer->cube_maps[skybox_id];

	u32 handle = renderer->shaders[SKYBOX_SHADER];//skybox_shader;
	glUseProgram(handle);

	mat4 view_matrix = view;
	view_matrix.elements[3][0] = 0;
	view_matrix.elements[3][1] = 0;
	view_matrix.elements[3][2] = 0;

	glDepthFunc(GL_LEQUAL);

	glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), 1, GL_FALSE, (float*)&projection);
	glUniformMatrix4fv(glGetUniformLocation(handle, "view"), 1, GL_FALSE, (float*)&view_matrix);
	glUniform1f(glGetUniformLocation(handle, "brightness"), brightness);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	glBindVertexArray(cube_model.vao);
	glDrawArrays(GL_TRIANGLES, 0, cube_model.draw_count);
	glBindVertexArray(0);

	glDepthFunc(renderer->depth_func);

	glUseProgram(0);
}

void renderer_destroy() {
	Render_state* renderer = &render_state;

    for (u32 i = 0; i < MAX_SHADER; i++) {
        glDeleteShader(renderer->shaders[i]);
    }
	/*glDeleteShader(diffuse_shader);
	glDeleteShader(skybox_shader);
	glDeleteShader(texture_shader);
	glDeleteShader(combine_shader);
	glDeleteShader(blur_shader);
	glDeleteShader(flare_shader);*/
	glDeleteVertexArrays(1, &quad_vao);
	glDeleteVertexArrays(1, &quad_vbo);

	for (u32 i = 0; i < renderer->texture_count; i++) {
		u32* texture_id = &renderer->textures[i];
		unload_texture(texture_id);
	}
	renderer->texture_count = 0;

	for (u32 i = 0; i < renderer->cube_map_count; i++) {
		u32* cube_map_id = &renderer->cube_maps[i];
		unload_texture(cube_map_id);
	}
	renderer->cube_map_count = 0;

	for (u32 i = 0; i < renderer->model_count; i++) {
		Model* model = &renderer->models[i];
		unload_model(model);
	}
	renderer->model_count = 0;

	resources_unload(&render_state.resources);
	unload_model(&cube_model);
	fbos_unload(renderer);
}
