// memory.hpp
// basic memory tracker

#ifndef _MEMORY_H
#define _MEMORY_H

#include "common.hpp"

i64 memory_total_allocated();

i64 memory_num_blocks();

void* m_malloc(const i32 size);

void* m_calloc(const i32 size, const i32 count);

void* m_realloc(void* data, const i32 old_size, const i32 new_size);

void m_free(void* data, const i32 size);

#endif
