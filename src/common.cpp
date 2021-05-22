// common.cpp

#include "memory.hpp"
#include "common.hpp"

void buffer_free(Buffer* buffer) {
	list_free(buffer->data, buffer->size);
}

void* list_initialize(const i32 size, const i32 count) {
	void* list = m_calloc(size, count);
	if (!list) {
		fprintf(stderr, "Failed to allocate memory for list\n");
		return NULL;
	}
	return list;
}

i32 read_file(const char* path, Buffer* buffer) {
	i32 result = NoError;
	u32 num_bytes_read = 0;

	FILE* fp = fopen(path, "rb");
	if (!fp) {
		fprintf(stderr, "No such file '%s'\n", path);
		return Error;
	}

	fseek(fp, 0, SEEK_END);
	u32 size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buffer->data = (char*)m_malloc(size * sizeof(char));
	buffer->size = size;
	if (!buffer->data) {
		buffer->size = 0;
		result = Error;
		goto done;
	}

	num_bytes_read = fread(buffer->data, 1, size, fp);
	if (num_bytes_read != size) {
		fprintf(stderr, "Failed to read file '%s'\n", path);
		result = Error;
		goto done;
	}

done:
	fclose(fp);
	return result;
}

i32 read_and_null_terminate_file(const char* path, Buffer* buffer) {
	i32 result = NoError;

	if ((result = read_file(path, buffer)) == NoError) {
		buffer->data[buffer->size - 1] = '\0';
	}
	return result;
}
