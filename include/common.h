// common.h

#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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

typedef union v3 {
  struct {
    float x, y, z;
  };
} v3;

typedef union v2 {
  struct {
    float x, y;
  };
} v2;

#define V2(X, Y) ((v2) { .x = X, .y = Y })
#define V3(X, Y, Z) ((v3) { .x = X, .y = Y, .z = Z })

typedef enum Status_code {
  NoError = 0,
  Error = -1,
} Status_code;

#endif
