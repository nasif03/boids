#ifndef BASE_H
#define BASE_H

#include <cstdint>

// types

typedef int8_t    i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// constants

const i32 SCR_SCALE  = 120;
const i32 SCR_WIDTH  = 16 * SCR_SCALE;
const i32 SCR_HEIGHT = 9 * SCR_SCALE;

// boids

const i32 NUM_BOIDS = 20000;

// timing

float delta_time = 0.0f;
float last_frame = 0.0f;

// input

float last_x = SCR_WIDTH / 2.0f, last_y = SCR_HEIGHT / 2.0f;
bool first_mouse = true;

#endif