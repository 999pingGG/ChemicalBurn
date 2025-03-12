#pragma once
#ifndef CHEMICALBURN_CB_MATH_H
#define CHEMICALBURN_CB_MATH_H

#include <SDL3/SDL.h>

typedef SDL_FPoint cb_vec2_t;

typedef struct cb_ivec2_t {
  int x, y;
} cb_ivec2_t;

typedef SDL_FRect cb_frect_t;

typedef SDL_FColor cb_color_t;

void cb_vec2_plus(const cb_vec2_t* a, const cb_vec2_t* b, cb_vec2_t* result);

void cb_vec2_minus(const cb_vec2_t* a, const cb_vec2_t* b, cb_vec2_t* result);

void cb_vec2_mul_scalar(const cb_vec2_t* vec, float scalar, cb_vec2_t* result);

float cb_vec2_length_squared(const cb_vec2_t* vec);

float cb_vec2_length(const cb_vec2_t* vec);

void cb_vec2_normalize(const cb_vec2_t* vec, cb_vec2_t* result);

#endif
