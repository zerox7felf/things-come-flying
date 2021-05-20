// window.h

#ifndef _WINDOW_H
#define _WINDOW_H

#include <GLFW/glfw3.h>

#include "common.h"

extern i8 mouse_state;
extern i8 key_down[];
extern i8 key_pressed[];

i32 window_open(const char* title, i32 width, i32 height, u8 fullscreen, u8 vsync);

i32 window_width();

i32 window_height();

i32 window_poll_events();

void window_clear_buffers(float r, float g, float b);

void window_swap_buffers();

void window_get_cursor(double* x, double* y);

void window_close();

#endif
