// window.cpp

#include "renderer.hpp"
#include "window.hpp"

i8 mouse_state = 0;
i8 key_down[GLFW_KEY_LAST] = {0};
i8 key_pressed[GLFW_KEY_LAST] = {0};

double scroll_x = 0;
double scroll_y = 0;

typedef struct Window  {
	i32 width;
	i32 height;
	u8 fullscreen;
	u8 cursor_hidden;
	framebuffer_change_cb framebuffer_cb;
	void* window;
} Window;

static Window win;

static void framebuffer_callback(GLFWwindow* window, i32 width, i32 height);
static void scroll_callback(GLFWwindow* window, double x, double y);

void framebuffer_callback(GLFWwindow* window, i32 width, i32 height) {
	glViewport(0, 0, width, height);
	win.width = width;
	win.height = height;
	projection = perspective(
		65, // fov
		(float)width / height,	// aspect ratio
		0.02f,	// z near clipping
		2000.0f // z far clipping
	);
	ortho_projection = orthographic(0.0f, win.width, win.height, 0.0f, -1.0f, 1.0f);
	if (win.framebuffer_cb) {
		win.framebuffer_cb(width, height);
	}
}

void scroll_callback(GLFWwindow* window, double x, double y) {
	scroll_x = x;
	scroll_y = y;
}

i32 window_open(const char* title, i32 width, i32 height, u8 fullscreen, u8 vsync, framebuffer_change_cb framebuffer_cb) {
	win.width = width;
	win.height = height;
	win.fullscreen = fullscreen;
	win.cursor_hidden = 1;
	win.framebuffer_cb = framebuffer_cb;

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
	glfwSetScrollCallback((GLFWwindow*)win.window, scroll_callback);
	glfwSwapInterval(vsync);
	framebuffer_callback((GLFWwindow*)win.window, win.width, win.height);
	glfwSetInputMode((GLFWwindow*)win.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	return NoError;
}

i32 window_width() {
	return win.width;
}

i32 window_height() {
	return win.height;
}

void window_set_title(const char* title) {
	glfwSetWindowTitle((GLFWwindow*)win.window, title);
}

i32 window_poll_events() {
	glfwPollEvents();

	for (u16 i = 0; i < GLFW_KEY_LAST; i++) {
		i32 key_state = glfwGetKey((GLFWwindow*)win.window, i);
		if (key_state == GLFW_PRESS) {
			key_pressed[i] = !key_down[i];
			key_down[i] = 1;
		}
		else {
			key_down[i] = 0;
			key_pressed[i] = 0;
		}
	}

	i32 left_mouse_state = glfwGetMouseButton((GLFWwindow*)win.window, 0);
	i32 right_mouse_state = glfwGetMouseButton((GLFWwindow*)win.window, 1);
	i32 middle_mouse_state = glfwGetMouseButton((GLFWwindow*)win.window, 2);

    // Arcane. Do not touch.
	(left_mouse_state && !(mouse_state & (1 << 7))) ? mouse_state |= (1 << 6) : (mouse_state &= ~(1 << 6));
	left_mouse_state ? mouse_state |= (1 << 7) : (mouse_state &= ~(1 << 7));

	(middle_mouse_state && !(mouse_state & (1 << 3))) ? mouse_state |= (1 << 2) : (mouse_state &= ~(1 << 2));
	middle_mouse_state ? mouse_state |= (1 << 3) : (mouse_state &= ~(1 << 3));

	(right_mouse_state && !(mouse_state & (1 << 5))) ? mouse_state |= (1 << 4) : (mouse_state &= ~(1 << 4));
	right_mouse_state ? mouse_state |= (1 << 5) : (mouse_state &= ~(1 << 5));

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

void window_get_cursor(double* x, double* y) {
	glfwGetCursorPos((GLFWwindow*)win.window, x, y);
}

void window_get_scroll(double* x, double* y) {
	if (x) {
		*x = scroll_x;
	}
	if (y) {
		*y = scroll_y;
	}
	scroll_x = 0;
	scroll_y = 0;
}

void window_toggle_cursor_visibility() {
	win.cursor_hidden = !win.cursor_hidden;
	if (win.cursor_hidden) {
		glfwSetInputMode((GLFWwindow*)win.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else {
		glfwSetInputMode((GLFWwindow*)win.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void window_toggle_fullscreen() {
  win.fullscreen = !win.fullscreen;
  if (win.fullscreen) {
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	win.width = mode->width;
	win.height = mode->height;
  }
  else {
  	win.width = 800;
  	win.height = 600;
  }
  glfwSetWindowSize((GLFWwindow*)win.window, win.width, win.height);
}

void window_close() {
	if (win.window) {
		glfwDestroyWindow((GLFWwindow*)win.window);
		glfwTerminate();
	}
}
