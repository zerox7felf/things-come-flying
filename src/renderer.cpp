// renderer.cpp

#include <GL/glew.h>

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

Render_state render_state = {0};

u32 basic_shader = 0,
	diffuse_shader = 0,
	skybox_shader = 0,
	texture_combine_shader = 0;

Model cube_model;
Fbo current_fbo;

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

static const char* vert_source_code =
	"#version 330 core\n"
	"\n"
	"in vec3 position;\n"
	"\n"
	"uniform mat4 projection;\n"
	"uniform mat4 view;\n"
	"uniform mat4 model;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = projection * view * model * vec4(position, 1);\n"
	"}\n";

static const char* frag_source_code =
	"#version 330 core\n"
	"\n"
	"out vec4 out_color;\n"
	"\n"
	"void main() {\n"
	"	out_color = vec4(1, 0, 0, 1);\n"
	"}\n";

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
static void fbo_initialize(Fbo* fbo, i32 width, i32 height);
static void fbo_unload(Fbo* fbo);

i32 shader_compile_from_source(const char* vert_source, const char* frag_source, u32* program_out) {
	i32 result = NoError;
	i32 compile_report = 0;
	u32 program = 0;
	char err_log[SHADER_ERROR_BUFFER_SIZE] = {0};
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

	Buffer vert_source = {0};
	Buffer frag_source = {0};
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

void fbo_initialize(Fbo* fbo, i32 width, i32 height) {
	fbo->width = width;
	fbo->height = height;

	glGenFramebuffers(1, &fbo->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);

	glGenTextures(1, &fbo->texture);
	glBindTexture(GL_TEXTURE_2D, fbo->texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &fbo->depth);
	glBindTexture(GL_TEXTURE_2D, fbo->depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glAlphaFunc(GL_GREATER, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_CUBE_MAP_EXT);
	// glEnable(GL_FRAMEBUFFER_SRGB);

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

	upload_model(&cube_model, cube_vertices, ARR_SIZE(cube_vertices));
	fbo_initialize(&current_fbo, window_width(), window_height());
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
	shader_compile_from_source(vert_source_code, frag_source_code, &basic_shader);
	shader_compile_from_file("resource/shader/textured_phong", &diffuse_shader);
	shader_compile_from_file("resource/shader/skybox", &skybox_shader);
	shader_compile_from_file("resource/shader/texture_combine", &texture_combine_shader);
	render_state.initialized = 1;
	return 0;
}

void renderer_framebuffer_callback(i32 width, i32 height) {
	if (!render_state.initialized)
		return;
	Fbo* fbo = &current_fbo;
	if (fbo->texture > 0) {
		fbo_unload(fbo);
	}
	fbo_initialize(fbo, width, height);
}

void renderer_clear_fbo() {
	Fbo* fbo = &current_fbo;
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_bind_fbo() {
	Fbo* fbo = &current_fbo;
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
}

void renderer_unbind_fbo() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_mesh(v3 position, v3 rotation, v3 size, u32 mesh_id, Material material) {
	Render_state* renderer = &render_state;
	//u32 texture0 = renderer->textures[material.texture0.id];
	u32 texture1 = renderer->textures[material.texture1.id];

    u32 color_map = renderer->textures[material.color_map.id];
    u32 ambient_map = renderer->textures[material.ambient.type == VALUE_MAP_MAP ? material.ambient.value.map.id : 0];
	Model* mesh = &renderer->models[mesh_id];

	u32 handle = diffuse_shader;
	glUseProgram(handle);

	model = translate(position);
	model = multiply_mat4(model, rotate(rotation.y, V3(0.0f, 1.0f, 0.0f)));
	model = multiply_mat4(model, rotate(rotation.z, V3(0.0f, 0.0f, 1.0f)));
	model = multiply_mat4(model, rotate(rotation.x, V3(1.0f, 0.0f, 0.0f)));
	model = multiply_mat4(model, scale_mat4(size));

    mat4 VM = multiply_mat4(view, model);
    mat4 PVM = multiply_mat4(projection, VM);
    mat4 VM_normal = transpose(inverse(VM));

	glUniformMatrix4fv(glGetUniformLocation(handle, "VM"), 1, GL_FALSE, (float*)&VM);
	glUniformMatrix4fv(glGetUniformLocation(handle, "PVM"), 1, GL_FALSE, (float*)&PVM);
	glUniformMatrix4fv(glGetUniformLocation(handle, "VM_normal"), 1, GL_FALSE, (float*)&VM_normal);

    v2 default_offset = V2(0.0f, 0.0f);
	glUniform2fv(glGetUniformLocation(handle, "color_map_offset"), 1, (float*)&material.color_map.offset);
	glUniform2fv(
        glGetUniformLocation(handle, "ambient_map_offset"), 1,
        material.ambient.type == VALUE_MAP_MAP ? (float*)&material.ambient.value.map.offset : (float*)&default_offset
    );

	glUniform2fv(glGetUniformLocation(handle, "offset1"), 1, (float*)&material.texture1.offset);
	glUniform1f(glGetUniformLocation(handle, "texture_mix"), material.texture_mix);

    // Mappable values
    // If type is not VALUE_MAP_CONST, set their value to -1 (which isn't valid to the shader normally, so should be fine as a flag)
	glUniform1f(glGetUniformLocation(handle, "ambient_amp"), material.ambient.type == VALUE_MAP_CONST ? material.ambient.value.constant : -1.0f);
	glUniform1f(glGetUniformLocation(handle, "diffuse_amp"), material.diffuse);
	glUniform1f(glGetUniformLocation(handle, "specular_amp"), material.specular);
	glUniform1f(glGetUniformLocation(handle, "shininess"), material.shininess);

	v4 light_position = multiply_mat4_v4(view, V4(0.0f, 0.0f, 0.0f, 1.0f));
    glUniform3fv(glGetUniformLocation(handle, "light_position"), 1, (float*)&light_position);
    glUniform3f(glGetUniformLocation(handle, "light_color"), 1.0f, 1.0f, 1.0f);

	glBindVertexArray(mesh->vao);

	glEnableVertexAttribArray(0);	// vertices
	glEnableVertexAttribArray(1);	// uv coordinates
	glEnableVertexAttribArray(2);	// normals

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, color_map);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ambient_map);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture1);

	glUniform1i(glGetUniformLocation(handle, "color_map"), 0);
	glUniform1i(glGetUniformLocation(handle, "ambient_map"), 1);
	glUniform1i(glGetUniformLocation(handle, "obj_texture1"), 2);

	glDrawElements(GL_TRIANGLES, mesh->draw_count, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glBindVertexArray(0);

	glUseProgram(0);
}

void render_fbo() {
	Fbo* fbo = &current_fbo;
	u32 handle = texture_combine_shader;
	glUseProgram(handle);
	u32 texture = fbo->texture;

	model = translate(V3(0, 0, 0));

	model = multiply_mat4(model, scale_mat4(V3((float)window_width(), (float)window_height(), 0)));

	glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), 1, GL_FALSE, (float*)&ortho_projection);
	glUniformMatrix4fv(glGetUniformLocation(handle, "model"), 1, GL_FALSE, (float*)&model);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture);

	glUniform1i(glGetUniformLocation(handle, "texture0"), 0);
	glUniform1i(glGetUniformLocation(handle, "texture1"), 1);

	glEnableVertexAttribArray(0);

	glBindVertexArray(quad_vao);

	glDisable(GL_DEPTH_TEST);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glEnable(GL_DEPTH_TEST);

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);

	glUseProgram(0);
}


void render_skybox(u32 skybox_id, float brightness) {
	Render_state* renderer = &render_state;
	u32 texture = renderer->cube_maps[skybox_id];

	u32 handle = skybox_shader;
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

	glDeleteShader(basic_shader);
	glDeleteShader(diffuse_shader);
	glDeleteShader(skybox_shader);
	glDeleteShader(texture_combine_shader);
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
	fbo_unload(&current_fbo);
}
