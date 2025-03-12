#include "chemicalburn/video.h"

#include "chemicalburn/util.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CB_FONT_ATLAS_SIZE 64
#define CB_GLYPH_WIDTH 5.0f
#define CB_GLYPH_HEIGHT 8.0f
#define CB_GLYPHS_PER_ROW 12
#define CB_GLYPH_ROWS 8
#define CB_GLYPH_WIDTH_IN_TEXTURE (CB_GLYPH_WIDTH / CB_FONT_ATLAS_SIZE)
#define CB_GLYPH_HEIGHT_IN_TEXTURE (CB_GLYPH_HEIGHT / CB_FONT_ATLAS_SIZE)

static const uint8_t spleen_font_texture[CB_FONT_ATLAS_SIZE * CB_FONT_ATLAS_SIZE / 8] = {
  0x01, 0x14, 0x02, 0x08, 0x84, 0x12, 0x00, 0x00, 0x01, 0x14, 0xA7, 0x49, 0x44, 0x21, 0x00, 0x00,
  0x01, 0x15, 0xFA, 0x51, 0x44, 0x40, 0xA4, 0x40, 0x01, 0x00, 0xA6, 0x11, 0x80, 0x40, 0x98, 0x40,
  0x01, 0x00, 0xA3, 0x22, 0xA0, 0x40, 0xBD, 0xF0, 0x00, 0x01, 0xF3, 0x2A, 0x40, 0x40, 0x98, 0x40,
  0x01, 0x00, 0xAE, 0x49, 0xA0, 0x21, 0x24, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x12, 0x00, 0x00,
  0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x11, 0x8C, 0x87, 0x99, 0xE0,
  0x00, 0x00, 0x49, 0x32, 0x52, 0xA4, 0x21, 0x20, 0x00, 0x00, 0x4B, 0x10, 0x44, 0xA7, 0x38, 0x20,
  0x07, 0x80, 0x8D, 0x11, 0x82, 0xF0, 0xA4, 0x40, 0x20, 0x00, 0x89, 0x12, 0x12, 0x20, 0xA4, 0x80,
  0x20, 0x09, 0x06, 0x3B, 0xCC, 0x27, 0x18, 0x80, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x63, 0x00, 0x01, 0x01, 0x12, 0x63, 0x38, 0xE0,
  0x94, 0x80, 0x02, 0x00, 0x82, 0x94, 0xA5, 0x00, 0x64, 0x88, 0x44, 0x78, 0x44, 0xB4, 0xB9, 0x00,
  0x93, 0x80, 0x04, 0x00, 0x48, 0xB7, 0xA5, 0x00, 0x90, 0x80, 0x42, 0x78, 0x80, 0x84, 0xA5, 0x00,
  0x63, 0x08, 0x41, 0x01, 0x08, 0x74, 0xB8, 0xE0, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE3, 0x9C, 0xE9, 0x39, 0xD2, 0x84, 0xA4, 0xC0,
  0x94, 0x21, 0x09, 0x10, 0x92, 0x87, 0xB5, 0x20, 0x97, 0x21, 0x6F, 0x10, 0x9C, 0x87, 0xB5, 0x20,
  0x94, 0x39, 0x29, 0x10, 0x92, 0x84, 0xAD, 0x20, 0x94, 0x21, 0x29, 0x10, 0x92, 0x84, 0xAD, 0x20,
  0xE3, 0xA0, 0xE9, 0x3B, 0x12, 0x74, 0xA4, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xE3, 0x38, 0xEF, 0xCA, 0x52, 0x94, 0xBC, 0x80,
  0x94, 0xA5, 0x02, 0x4A, 0x52, 0x94, 0x84, 0x80, 0x94, 0xA4, 0xC2, 0x4A, 0x52, 0x64, 0x88, 0x80,
  0xE4, 0xB8, 0x22, 0x4A, 0x5E, 0x63, 0x90, 0x80, 0x84, 0xA4, 0x22, 0x49, 0x9E, 0x90, 0xA0, 0x80,
  0x83, 0x25, 0xC2, 0x39, 0x92, 0x97, 0x3C, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0,
  0x83, 0x80, 0x04, 0x02, 0x00, 0x10, 0x0C, 0x00, 0x80, 0x88, 0x02, 0x02, 0x00, 0x10, 0x10, 0x00,
  0x40, 0x94, 0x00, 0x33, 0x8E, 0x73, 0x90, 0xE0, 0x40, 0xA2, 0x00, 0x0A, 0x50, 0x94, 0xB9, 0x20,
  0x20, 0x80, 0x00, 0x3A, 0x50, 0x97, 0x91, 0x20, 0x20, 0x80, 0x00, 0x4A, 0x50, 0x94, 0x10, 0xC0,
  0x10, 0x80, 0x00, 0x3B, 0x8E, 0x73, 0x90, 0x20, 0x13, 0x81, 0xE0, 0x00, 0x00, 0x00, 0x01, 0xC0,
  0x80, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x09, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xE0, 0x01, 0x24, 0x4B, 0x8C, 0xE3, 0x9C, 0xE0, 0x93, 0x09, 0x44, 0x7A, 0x52, 0x94, 0xA5, 0x00,
  0x91, 0x09, 0x84, 0x7A, 0x52, 0x94, 0xA0, 0xC0, 0x91, 0x09, 0x44, 0x4A, 0x52, 0xE3, 0xA0, 0x20,
  0x91, 0x89, 0x23, 0x4A, 0x4C, 0x80, 0xA1, 0xC0, 0x00, 0x30, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00,
  0x40, 0x00, 0x00, 0x00, 0x06, 0x26, 0x01, 0xF0, 0x40, 0x00, 0x00, 0x00, 0x08, 0x21, 0x01, 0xF0,
  0xE4, 0xA5, 0x29, 0x4B, 0xC8, 0x21, 0x01, 0xF0, 0x44, 0xA5, 0x26, 0x48, 0x58, 0x21, 0x93, 0xF0,
  0x44, 0xA5, 0xE6, 0x48, 0x98, 0x21, 0xAD, 0xF0, 0x44, 0x99, 0xE9, 0x39, 0x08, 0x21, 0x01, 0xF0,
  0x33, 0x99, 0x29, 0x0B, 0xC8, 0x21, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x70, 0x06, 0x26, 0x01, 0xF0,
};

static SDL_FPoint glyph_locations[CB_GLYPH_ROWS * CB_GLYPHS_PER_ROW] = { 0 };
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* font = NULL;
static float text_scale = 1.0f;

void cb_fill_frect(const cb_frect_t* rect, const cb_color_t* color) {
  SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
  SDL_RenderFillRect(renderer, rect);
}

void cb_draw_line(const cb_vec2_t* from, const cb_vec2_t* to, const float width, const cb_color_t* color) {
  cb_vec2_t a_perpendicular, b_perpendicular;
  cb_vec2_minus(from, to, &a_perpendicular);
  cb_vec2_normalize(&a_perpendicular, &a_perpendicular);
  cb_vec2_mul_scalar(&a_perpendicular, width * 0.5f, &a_perpendicular);
  a_perpendicular = (cb_vec2_t){
    .x = a_perpendicular.y,
    .y = -a_perpendicular.x,
  };
  b_perpendicular = (cb_vec2_t){
    .x = -a_perpendicular.x,
    .y = -a_perpendicular.y,
  };

  SDL_Vertex vertices[] = {
    {
      .position = *from,
      .color = *color,
    },
    {
      .position = *from,
      .color = *color,
    },
    {
      .position = *to,
      .color = *color,
    },
    {
      .position = *to,
      .color = *color,
    },
  };

  cb_vec2_plus(&vertices[0].position, &a_perpendicular, &vertices[0].position);
  cb_vec2_plus(&vertices[1].position, &b_perpendicular, &vertices[1].position);
  cb_vec2_plus(&vertices[2].position, &b_perpendicular, &vertices[2].position);
  cb_vec2_plus(&vertices[3].position, &a_perpendicular, &vertices[3].position);

  SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);

  static const int indices[] = { 0, 1, 2, 2, 3, 0 };
  SDL_RenderGeometry(renderer, NULL, vertices, CB_COUNTOF(vertices), indices, CB_COUNTOF(indices));
}

void cb_draw_triangle(const cb_vec2_t vertices[3], const cb_color_t* color) {
  const SDL_Vertex sdl_vertices[3] = {
    {
      .position = vertices[0],
      .color = *color,
    },
    {
      .position = vertices[1],
      .color = *color,
    },
    {
      .position = vertices[2],
      .color = *color,
    },
  };
  SDL_RenderGeometry(renderer, NULL, sdl_vertices, CB_COUNTOF(sdl_vertices), NULL, 0);
}

void cb_draw_text(const char* text, float x, float y, const cb_text_anchor anchor, const cb_color_t* color) {
  const char* newline = strchr(text, '\n');
  if (newline) {
    // If the text contains newlines, draw all the lines separately and with increasing y coord.
    while (newline) {
      const size_t length = newline - text;
      char* string_copy = malloc(length + 1);
      strncpy(string_copy, text, length);
      string_copy[length] = '\0';

      cb_draw_text(string_copy, x, y, anchor, color);

      free(string_copy);

      text += length + 1;
      newline = strchr(text, '\n');
      y += CB_GLYPH_HEIGHT * text_scale;
    }

    // Draw the last token.
    cb_draw_text(text, x, y, anchor, color);
  } else {
    x /= text_scale;
    y /= text_scale;
    const float x_stride = CB_GLYPH_WIDTH * text_scale, y_stride = CB_GLYPH_HEIGHT * text_scale;

    const int length = (int)strlen(text);

    switch (anchor) {
      case cb_upper_left:
        // The calculations ahead assume this, nothing to do.
        break;
      case cb_upper_middle:
        x -= (float)length * x_stride * 0.5f;
        break;
      case cb_upper_right:
        x -= (float)length * x_stride;
        break;
      case cb_middle_right:
        x -= (float)length * x_stride;
        y -= y_stride * 0.5f;
        break;
      case cb_lower_right:
        x -= (float)length * x_stride;
        y -= y_stride;
        break;
      case cb_lower_middle:
        x -= (float)length * x_stride * 0.5f;
        y -= y_stride;
        break;
      case cb_lower_left:
        y -= y_stride;
        break;
      case cb_middle_left:
        y -= y_stride * 0.5f;
        break;
      case cb_center:
        x -= (float)length * x_stride * 0.5f;
        y -= y_stride * 0.5f;
        break;
      default:
        CB_ASSERT(false);
    }

    const int vertices_total = length * 4;
    const int indices_total = length * 6;
    SDL_Vertex* vertices = malloc(vertices_total * sizeof(SDL_Vertex));
    int* indices = malloc(indices_total * sizeof(int));

    for (int i = 0; i < length; i++) {
      const int first_vertex = i * 4, first_index = i * 6;

      char c = text[i];
      if (c < 32 || c > 126)
        c = 127;
      const int char_index = c - 32;

      const SDL_FPoint base_location = glyph_locations[char_index];
      vertices[first_vertex] = (SDL_Vertex){
        .position = { .x = x, .y = y },
        .color = *color,
        .tex_coord = base_location,
      };
      vertices[first_vertex + 1] = (SDL_Vertex){
        .position = { .x = x + x_stride, .y = y, },
        .color = *color,
        .tex_coord = {
          .x = base_location.x + CB_GLYPH_WIDTH_IN_TEXTURE,
          .y = base_location.y,
        },
      };
      vertices[first_vertex + 2] = (SDL_Vertex){
        .position = { .x = x + x_stride, .y = y + y_stride, },
        .color = *color,
        .tex_coord = {
          .x = base_location.x + CB_GLYPH_WIDTH_IN_TEXTURE,
          .y = base_location.y + CB_GLYPH_HEIGHT_IN_TEXTURE,
        },
      };
      vertices[first_vertex + 3] = (SDL_Vertex){
        .position = { .x = x, .y = y + y_stride, },
        .color = *color,
        .tex_coord = {
          .x = base_location.x,
          .y = base_location.y + CB_GLYPH_HEIGHT_IN_TEXTURE,
        },
      };

      indices[first_index] = first_vertex;
      indices[first_index + 1] = first_vertex + 1;
      indices[first_index + 2] = first_vertex + 2;
      indices[first_index + 3] = first_vertex + 2;
      indices[first_index + 4] = first_vertex + 3;
      indices[first_index + 5] = first_vertex;

      x += x_stride;
    }

    SDL_SetRenderScale(renderer, text_scale, text_scale);
    SDL_RenderGeometry(renderer, font, vertices, vertices_total, indices, indices_total);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);

    free(indices);
    free(vertices);
  }
}

void cb_begin_frame(void) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
}

void cb_end_frame(void) {
  SDL_RenderPresent(renderer);

  const char* sdl_error = SDL_GetError();
  if (sdl_error[0]) {
    fprintf(stderr, "SDL error: %s\n", sdl_error);
    SDL_ClearError();
  }
}

int cb_video_init(cb_video_settings_t* settings) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    fprintf(stderr, "SDL initialization error: %s\n", SDL_GetError());
    cb_video_release();
    return 1;
  }

  if (settings->fullscreen) {
    SDL_DisplayID* displays = SDL_GetDisplays(NULL);
    if (!displays) {
      fprintf(stderr, "SDL_GetDisplays() error: %s\n", SDL_GetError());
      cb_video_release();
      return 2;
    }

    const SDL_DisplayMode* display_mode = SDL_GetDesktopDisplayMode(*displays);
    SDL_free(displays);
    if (!display_mode) {
      fprintf(stderr, "SDL_GetDesktopDisplayMode() error: %s\n", SDL_GetError());
      cb_video_release();
      return 3;
    }

    settings->viewport_size = (cb_ivec2_t){
      .x = display_mode->w,
      .y = display_mode->h,
    };
  }

  if (!(window = SDL_CreateWindow(
    CB_APP,
    settings->viewport_size.x,
    settings->viewport_size.y,
    settings->fullscreen ? SDL_WINDOW_FULLSCREEN : 0
  ))) {
    fprintf(stderr, "SDL_CreateWindow() error: %s\n", SDL_GetError());
    cb_video_release();
    return 4;
  }

  printf("Available video renderers: ");
  for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
    if (i > 0)
      printf(", ");
    printf("%s", SDL_GetRenderDriver(i));
  }
  printf("\nUsing %s.\n", SDL_GetRenderDriver(0));

  if (!(renderer = SDL_CreateRenderer(window, NULL))) {
    fprintf(stderr, "SDL_CreateRenderer() error: %s\n", SDL_GetError());
    cb_video_release();
    return 5;
  }

  if (SDL_GetCurrentRenderOutputSize(renderer, &settings->viewport_size.x, &settings->viewport_size.y) < 0) {
    fprintf(stderr, "SDL_GetCurrentRenderOutputSize() error: %s\n", SDL_GetError());
    cb_video_release();
    return 6;
  }

  if (!(font = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA4444, SDL_TEXTUREACCESS_STATIC, 64, 64))) {
    fprintf(stderr, "SDL_CreateTexture() error: %s\n", SDL_GetError());
    cb_video_release();
    return 7;
  }

  SDL_SetTextureBlendMode(font, SDL_BLENDMODE_BLEND);

  {
    // Expand our 1-bit per pixel, monochrome font texture into a 16-bit per pixel RGBA4444 texture.
    uint8_t rgba4444_font[sizeof(spleen_font_texture) * 16];
    int pixel_index = 0;
    for (int i = 0; i < sizeof(spleen_font_texture); i++) {
      int mask = 128;
      do {
        rgba4444_font[pixel_index] = rgba4444_font[pixel_index + 1] = spleen_font_texture[i] & mask ? 255 : 0;

        pixel_index += 2;
        mask >>= 1;
      } while (mask);
    }

    if (SDL_UpdateTexture(font, NULL, rgba4444_font, 128) < 0) {
      fprintf(stderr, "SDL_UpdateTexture() error: %s\n", SDL_GetError());
      cb_video_release();
      return 8;
    }
  }

  for (int row = 0; row < CB_GLYPH_ROWS; row++)
    for (int column = 0; column < CB_GLYPHS_PER_ROW; column++)
      glyph_locations[row * CB_GLYPHS_PER_ROW + column] = (SDL_FPoint){
        .x = ((float)column * CB_GLYPH_WIDTH) / CB_FONT_ATLAS_SIZE,
        .y = ((float)row * CB_GLYPH_HEIGHT) / CB_FONT_ATLAS_SIZE,
      };

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  // Because the scale is being squared by SDL??
  text_scale = sqrtf(settings->text_scale);

  return 0;
}

void cb_video_release(void) {
  if (font) {
    SDL_DestroyTexture(font);
    font = NULL;
  }

  if (renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
  }

  if (window) {
    SDL_DestroyWindow(window);
    window = NULL;
  }
}
