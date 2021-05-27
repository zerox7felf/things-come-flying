// common.hpp

#ifndef _COMMON_HPP
#define _COMMON_HPP

#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_PATH_SIZE 512
#define PI32 3.14159265359f
#define ARR_SIZE(ARRAY) ((sizeof(ARRAY)) / (sizeof(ARRAY[0])))

typedef int64_t i64;
typedef uint64_t u64;
typedef int32_t i32;
typedef uint32_t u32;
typedef int16_t i16;
typedef uint16_t u16;
typedef int8_t i8;
typedef uint8_t u8;
typedef float r32;
typedef double r64;

typedef union v4 {
    struct {
        float x, y, z, w;
    };
    struct {
        float r, g, b, a;
    };
} v4;

typedef union v3 {
	struct {
		float x, y, z;
	};
	struct {
		float r, g, b;
	};
} v3;

typedef union v2 {
	struct {
		float x, y;
	};
	struct {
		float w, h;
	};
} v2;

typedef struct Buffer {
	char* data = NULL;
	i32 size = 0;
} Buffer;

#define V2(X, Y) ((v2) {{.x = X, .y = Y, }})
#define V3(X, Y, Z) ((v3) {{ .x = X, .y = Y, .z = Z, }})
#define V4(X, Y, Z, W) ((v4) {{ .x = X, .y = Y, .z = Z, .w = W, }})

#define clamp(Value, Min, Max) (Value < Min ? Min : (Value > Max ? Max : Value))

typedef enum Status_code {
	NoError = 0,
	Error = -1,
} Status_code;

#define list_push(List, Count, Element) do { \
	if (List == NULL) { \
    List = (typeof(Element)*)list_initialize(sizeof(Element), 1); List[0] = Element; Count = 1; break; \
  } \
	void* NewList = m_realloc(List, Count * sizeof(*List), (1 + Count) * (sizeof(Element))); \
	if (NewList) { \
		List = (typeof(Element)*)NewList; \
		List[(Count)++] = Element; \
	} \
} while (0); \

#define list_realloc(List, Count, NewSize) do { \
  if (List == NULL) break; \
  if (NewSize == 0) { list_free(List, Count); break; } \
	void* NewList = m_realloc(List, Count * sizeof(*List), (NewSize) * (sizeof(*List))); \
  List = NewList; \
  Count = NewSize; \
} while(0); \

#define list_shrink(List, Count, Num) { \
  if ((Count - Num) >= 0) { \
	  list_realloc(List, Count, Count - Num); \
  } \
}
 
#define list_assign(List, Count, Index, Element) { \
	assert(List != NULL); \
	if (Index < Count) { \
		List[Index] = Element; \
	} else { \
		assert(0); \
	} \
} \

#define list_free(List, Count) { \
	if ((List) != NULL && Count > 0) { \
		m_free(List, Count * sizeof(*List)); \
		Count = 0; \
		List = NULL; \
	}\
}

void buffer_free(Buffer* buffer);

void* list_initialize(const i32 size, const i32 count);

i32 read_file(const char* path, Buffer* buffer);

i32 read_and_null_terminate_file(const char* path, Buffer* buffer);

#endif
