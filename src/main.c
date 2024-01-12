#include "chemicalburn/cb_math.h"
#include "chemicalburn/settings.h"
#include "chemicalburn/simulation.h"
#include "chemicalburn/util.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define CB_TARGET_FPS 30
#define CB_FRAME_BUDGET (1000000000 / CB_TARGET_FPS)

static SDL_Renderer* renderer = NULL;

static void cb_fill_frect(const cb_frect_t* rect, const cb_color_t* color) {
  SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
  SDL_RenderFillRect(renderer, rect);
}

static void cb_draw_line(const cb_vec2_t* from, const cb_vec2_t* to, const float width, const cb_color_t* color) {
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

static void cb_draw_triangle(const cb_vec2_t vertices[3], const cb_color_t* color) {
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

int main(int argc, char* argv[]) {
  int result = 0;
  SDL_Window* window = NULL;
  cb_simulation_t simulation = { 0 };

  srand(time(NULL));

  cb_simulation_settings_t simulation_settings;
  cb_video_settings_t video_settings;
  cb_settings_read_from_storage(&simulation_settings, &video_settings);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    fprintf(stderr, "SDL initialization error: %s\n", SDL_GetError());
    result = 1;
    goto exit;
  }

  if (video_settings.fullscreen) {
    SDL_DisplayID* displays = SDL_GetDisplays(NULL);
    if (!displays) {
      fprintf(stderr, "SDL_GetDisplays() error: %s\n", SDL_GetError());
      result = 2;
      goto exit;
    }

    const SDL_DisplayMode* display_mode = SDL_GetDesktopDisplayMode(*displays);
    SDL_free(displays);
    if (!display_mode) {
      fprintf(stderr, "SDL_GetDesktopDisplayMode() error: %s\n", SDL_GetError());
      result = 3;
      goto exit;
    }

    simulation_settings.bounds = video_settings.viewport_size = (cb_ivec2_t){
      .x = display_mode->w,
      .y = display_mode->h,
    };
  }

  if (!(window = SDL_CreateWindow(
    CB_APP,
    video_settings.viewport_size.x,
    video_settings.viewport_size.y,
    video_settings.fullscreen ? SDL_WINDOW_FULLSCREEN : 0
  ))) {
    fprintf(stderr, "SDL_CreateWindow() error: %s\n", SDL_GetError());
    result = 4;
    goto exit;
  }

  if (!(renderer = SDL_CreateRenderer(
    window,
    NULL,
    SDL_RENDERER_ACCELERATED | (video_settings.vsync ? SDL_RENDERER_PRESENTVSYNC : 0)
  ))) {
    fprintf(stderr, "SDL_CreateRenderer() error: %s\n", SDL_GetError());
    result = 5;
    goto exit;
  }

  if (SDL_GetCurrentRenderOutputSize(renderer, &simulation_settings.bounds.x, &simulation_settings.bounds.y) < 0) {
    fprintf(stderr, "SDL_GetCurrentRenderOutputSize() error: %s\n", SDL_GetError());
    result = 6;
    goto exit;
  }

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  cb_simulation_init(
    &simulation,
    &(cb_drawing_functions_t){
      .fill_rect = cb_fill_frect,
      .draw_line = cb_draw_line,
      .draw_triangle = cb_draw_triangle,
    },
    &simulation_settings
  );

  bool quit = false;
  Uint64 time_at_last_frame = SDL_GetTicksNS();
  SDL_Event event;
  while (!quit) {
    while (SDL_PollEvent(&event))
      switch (event.type) {
        case SDL_EVENT_QUIT:
          quit = true;
          break;
      }

    cb_step(&simulation);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    cb_draw_nodes(&simulation);
    cb_draw_connections(&simulation);
    cb_draw_packages(&simulation);

    SDL_RenderPresent(renderer);

    const Uint64 current_time = SDL_GetTicksNS();
    const Uint64 delta_time = current_time - time_at_last_frame;
    const Sint64 time_left = CB_FRAME_BUDGET - delta_time;
    if (time_left > 0)
      SDL_DelayNS(time_left);

    const char* sdl_error = SDL_GetError();
    if (sdl_error[0]) {
      CB_DEBUG_LOG("SDL error: %s\n", sdl_error);
      SDL_ClearError();
    }

    time_at_last_frame = SDL_GetTicksNS();
  }

  exit:
  cb_simulation_release(&simulation);
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);

  return result;
}
