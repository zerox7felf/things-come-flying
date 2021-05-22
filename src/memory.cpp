// memory.cpp

#include "common.hpp"
#include "memory.hpp"

struct {
	i64 block_count;
	i64 total;
} memory_info = {
	.block_count = 0,
	.total = 0,
};

#define update_memory_info(add_to_total, add_num_blocks) { \
	memory_info.total += add_to_total; \
	memory_info.block_count += add_num_blocks; \
}

i64 memory_total_allocated() {
	return memory_info.total;
}

i64 memory_num_blocks() {
	return memory_info.block_count;
}

void* m_malloc(const i32 size) {
	void* data = malloc(size);
	if (!data)
		return NULL;
	update_memory_info(size, 1);
	return data;
}

void* m_calloc(const i32 size, const i32 count) {
	void* data = calloc(size, count);
	if (!data)
		return NULL;
	update_memory_info(size * count, 1);
	return data;
}

void* m_realloc(void* data, const i32 old_size, const i32 new_size) {
	i32 size_delta = new_size - old_size;
	void* temporary = realloc(data, new_size);
	if (!temporary)
		return NULL;
	update_memory_info(size_delta, 0);
	return temporary;
}

void m_free(void* data, const i32 size) {
	free(data);
	update_memory_info(-size, -1);
}
