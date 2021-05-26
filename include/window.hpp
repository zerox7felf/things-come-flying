// window.hpp

#ifndef _WINDOW_HPP
#define _WINDOW_HPP

#include <GLFW/glfw3.h>

#include "common.hpp"

extern i8 mouse_state;
extern i8 key_down[];
extern i8 key_pressed[];

typedef void (*framebuffer_change_cb)(i32 width, i32 height);

i32 window_open(const char* title, i32 width, i32 height, u8 fullscreen, u8 vsync, framebuffer_change_cb framebuffer_cb);

i32 window_width();

i32 window_height();

void window_set_title(const char* title);

i32 window_poll_events();

void window_clear_buffers(float r, float g, float b);

void window_swap_buffers();

void window_get_cursor(double* x, double* y);

void window_toggle_cursor_visibility();

void window_toggle_fullscreen();

void window_close();

#endif
