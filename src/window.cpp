// window.cpp

#include <GL/glew.h>

#if defined(__APPLE__)
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>

#include "window.h"

typedef struct Window  {
	i32 width;
	i32 height;
	u8 fullscreen;
	void* window;
} Window;

static Window win;

static void framebuffer_callback(GLFWwindow* window, i32 width, i32 height);

void framebuffer_callback(GLFWwindow* window, i32 width, i32 height) {
	glViewport(0, 0, width, height);
	win.width = width;
	win.height = height;
	// TODO(lucas): Update projection matrix here
}

i32 window_open(const char* title, i32 width, i32 height, u8 fullscreen) {
	win.width = width;
	win.height = height;
	win.fullscreen = fullscreen;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_FOCUSED, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
#if defined(__APPLE__)
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	win.window = glfwCreateWindow(win.width, win.height, title, win.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
	if (!win.window) {
		fprintf(stderr, "Failed to create glfw window\n");
		return Error;
	}
	glfwMakeContextCurrent((GLFWwindow*)win.window);
	glfwSetFramebufferSizeCallback((GLFWwindow*)win.window, framebuffer_callback);
	i32 glew_error = glewInit();
	if (glew_error != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(glew_error));
		return Error;
	}
	i32 v_sync = 1;
	glfwSwapInterval(v_sync);
	return NoError;
}

i32 window_width() {
	return win.width;
}

i32 window_height() {
	return win.height;
}

i32 window_poll_events() {
	glfwPollEvents();
	if (glfwWindowShouldClose((GLFWwindow*)win.window)) {
		return -1;
	}
	return 0;
}

void window_clear_buffers(float r, float g, float b) {
	glClearColor(r, g, b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void window_swap_buffers() {
	glfwSwapBuffers((GLFWwindow*)win.window);
}

void window_close() {
	if (win.window) {
		glfwDestroyWindow((GLFWwindow*)win.window);
		glfwTerminate();
	}
}
