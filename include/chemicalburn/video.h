#pragma once
#ifndef CHEMICALBURN_VIDEO_H
#define CHEMICALBURN_VIDEO_H

#include "chemicalburn/settings.h"

#include <SDL3/SDL.h>

typedef enum cb_text_anchor {
  cb_upper_left = 1,
  cb_upper_middle,
  cb_upper_right,
  cb_middle_right,
  cb_lower_right,
  cb_lower_middle,
  cb_lower_left,
  cb_middle_left,
  cb_center,
} cb_text_anchor;

void cb_fill_frect(const cb_frect_t* rect, const cb_color_t* color);

void cb_draw_line(const cb_vec2_t* from, const cb_vec2_t* to, float width, const cb_color_t* color);

void cb_draw_triangle(const cb_vec2_t vertices[3], const cb_color_t* color);

void cb_draw_text(const char* text, float x, float y, cb_text_anchor anchor, const cb_color_t* color);

void cb_begin_frame(void);

void cb_end_frame(void);

int cb_video_init(cb_video_settings_t* settings);

void cb_video_release(void);

#endif
