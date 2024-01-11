#include "chemicalburn/cb_math.h"

#include <math.h>

void cb_vec2_plus(const cb_vec2_t* a, const cb_vec2_t* b, cb_vec2_t* result) {
  *result = (cb_vec2_t){
    .x = a->x + b->x,
    .y = a->y + b->y,
  };
}

void cb_vec2_minus(const cb_vec2_t* a, const cb_vec2_t* b, cb_vec2_t* result) {
  *result = (cb_vec2_t){
    .x = a->x - b->x,
    .y = a->y - b->y,
  };
}

void cb_vec2_mul_scalar(const cb_vec2_t* vec, const float scalar, cb_vec2_t* result) {
  *result = (cb_vec2_t){
    .x = vec->x * scalar,
    .y = vec->y * scalar,
  };
}

float cb_vec2_length_squared(const cb_vec2_t* vec) {
  return vec->x * vec->x + vec->y * vec->y;
}

float cb_vec2_length(const cb_vec2_t* vec) {
  return sqrtf(cb_vec2_length_squared(vec));
}

void cb_vec2_normalize(const cb_vec2_t* vec, cb_vec2_t* result) {
  const float length = cb_vec2_length(vec);
  *result = (cb_vec2_t){
    .x = vec->x / length,
    .y = vec->y / length,
  };
}
