// window.h

#ifndef _WINDOW_H
#define _WINDOW_H

#include "common.h"

i32 window_open(const char* title, i32 width, i32 height, u8 fullscreen);

i32 window_width();

i32 window_height();

i32 window_poll_events();

void window_clear_buffers(float r, float g, float b);

void window_swap_buffers();

void window_close();

#endif
