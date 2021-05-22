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
#include "renderer.hpp"

mat4 projection;
mat4 view;
mat4 model;

u32 basic_shader = 0;
u32 diffuse_shader = 0;
Model cube_model = {0};
Mesh sphere_mesh = {0};
Model sphere_model = {0};

#define SHADER_ERROR_BUFFER_SIZE 512

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
	"}\n"
	;
static const char* frag_source_code =
	"#version 330 core\n"
	"\n"
	"out vec4 out_color;\n"
	"\n"
	"void main() {\n"
	"	out_color = vec4(1, 0, 0, 1);\n"
	"}\n"
	;

static void opengl_initialize();
static i32 shader_compile_from_source(const char* vert_source, const char* frag_source, u32* program_out);
static i32 shader_compile_from_file(const char* path, u32* program_out);
static i32 upload_model(Model* model, float* vertices, u32 vertex_count);
static i32 upload_model(Model* model, Mesh* mesh);
static void unload_model(Model* model);
static void store_attribute(Model* model, u32 attribute_index, u32 count, u32 size, void* data);

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
	if (!compile_report) {
		glGetProgramInfoLog(program, SHADER_ERROR_BUFFER_SIZE, NULL, err_log);
		fprintf(stderr, "shader compile error: %s\n", err_log);
		goto done;
	}

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

void store_attribute(Model* model, u32 attribute_index, u32 count, u32 size, void* data) {
	glEnableVertexAttribArray(attribute_index);
	glGenBuffers(1, &model->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	glVertexAttribPointer(attribute_index, count, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void opengl_initialize() {
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glAlphaFunc(GL_GREATER, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

i32 renderer_initialize() {
	i32 glew_error = glewInit();
	if (glew_error != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(glew_error));
		return Error;
	}
	opengl_initialize();
	view = mat4d(1.0f);
	model = mat4d(1.0f);

	shader_compile_from_source(vert_source_code, frag_source_code, &basic_shader);
	shader_compile_from_file("resource/shader/diffuse", &diffuse_shader);
	load_mesh("resource/mesh/sphere.obj", &sphere_mesh);
	upload_model(&sphere_model, &sphere_mesh);
	return 0;
}

void render_cube(v3 position, v3 rotation, v3 size) {
	u32 handle = diffuse_shader;
	glUseProgram(handle);

	model = translate(position);

	model = multiply_mat4(model, rotate(rotation.y, V3(0.0f, 1.0f, 0.0f)));
	model = multiply_mat4(model, rotate(rotation.z, V3(0.0f, 0.0f, 1.0f)));
	model = multiply_mat4(model, rotate(rotation.x, V3(1.0f, 0.0f, 0.0f)));

	model = multiply_mat4(model, scale_mat4(size));

	glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), 1, GL_FALSE, (float*)&projection);
	glUniformMatrix4fv(glGetUniformLocation(handle, "view"), 1, GL_FALSE, (float*)&view);
	glUniformMatrix4fv(glGetUniformLocation(handle, "model"), 1, GL_FALSE, (float*)&model);

	Model* model = &cube_model;

	glBindVertexArray(model->vao);

	glDrawArrays(GL_TRIANGLES, 0, model->draw_count);

	glBindVertexArray(0);

	glUseProgram(0);
}

void render_mesh(v3 position, v3 rotation, v3 size) {
	u32 handle = diffuse_shader;
	glUseProgram(handle);

	model = translate(position);

	model = multiply_mat4(model, rotate(rotation.y, V3(0.0f, 1.0f, 0.0f)));
	model = multiply_mat4(model, rotate(rotation.z, V3(0.0f, 0.0f, 1.0f)));
	model = multiply_mat4(model, rotate(rotation.x, V3(1.0f, 0.0f, 0.0f)));

	model = multiply_mat4(model, scale_mat4(size));

	glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), 1, GL_FALSE, (float*)&projection);
	glUniformMatrix4fv(glGetUniformLocation(handle, "view"), 1, GL_FALSE, (float*)&view);
	glUniformMatrix4fv(glGetUniformLocation(handle, "model"), 1, GL_FALSE, (float*)&model);

	Model* model = &sphere_model;

	glBindVertexArray(model->vao);

	glEnableVertexAttribArray(0);

	glDrawElements(GL_TRIANGLES, model->draw_count, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);

	glBindVertexArray(0);

	glUseProgram(0);
}

void renderer_destroy() {
	unload_model(&sphere_model);
	unload_mesh(&sphere_mesh);
	glDeleteVertexArrays(1, &cube_model.vao);
	glDeleteVertexArrays(1, &cube_model.vbo);
	glDeleteShader(basic_shader);
	glDeleteShader(diffuse_shader);
}
