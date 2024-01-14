#include "chemicalburn/settings.h"
#include "chemicalburn/simulation.h"
#include "chemicalburn/video.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define CB_TARGET_FPS 30
#define CB_FRAME_BUDGET (1000000000 / CB_TARGET_FPS)

int main(int argc, char* argv[]) {
  srand(time(NULL));

  cb_simulation_settings_t simulation_settings;
  cb_video_settings_t video_settings;
  cb_settings_read_from_storage(&simulation_settings, &video_settings);

  int result;
  if ((result = cb_video_init(&video_settings)) != 0)
    return result;
  simulation_settings.bounds = video_settings.viewport_size;

  cb_simulation_t simulation = { 0 };
  cb_simulation_init(&simulation, &simulation_settings);

  bool quit = false;
  const Uint64 time_at_start = SDL_GetTicksNS();
  Uint64 time_at_last_frame = SDL_GetTicksNS();
  SDL_Event event;
  static Uint64 delta_time = 0;
  while (!quit) {
    while (SDL_PollEvent(&event))
      switch (event.type) {
        case SDL_EVENT_QUIT:
          quit = true;
          break;
      }

    cb_step(&simulation);

    cb_begin_frame();

    cb_draw_nodes(&simulation);
    cb_draw_connections(&simulation);
    cb_draw_packages(&simulation);

    if (simulation_settings.show_stats) {
      const intptr_t nodes = cb_node_vec_size(&simulation.nodes);
      const intptr_t destroy_nodes = cb_node_vec_size(&simulation.destroy_nodes);

      double current_time = (double)(SDL_GetTicksNS() - time_at_start) / 1e9;
      char text_buffer[1000];
      snprintf(
        text_buffer,
        sizeof(text_buffer),
        "%.2f FPS, %.2f ms\n"
        "%.2f seconds elapsed, step %u, %.2f steps per second so far\n"
        "%" PRIiPTR " nodes, %" PRIiPTR " nodes to destroy, %" PRIiPTR " total nodes\n"
        "%" PRIiPTR " packages, %" PRIuMAX " package steps, %" PRIuMAX " delivered packages",
        1e9 / (double)delta_time,
        (double)delta_time / 1e6,
        current_time,
        simulation.step,
        simulation.step / current_time,
        nodes,
        destroy_nodes,
        nodes + destroy_nodes,
        cb_package_set_size(&simulation.packages),
        simulation.package_steps,
        simulation.delivered_packages
      );
      cb_draw_text(text_buffer, 0.0f, 0.0f, cb_upper_left, &(cb_color_t){ 255, 0, 255, 255 });
    }

    cb_end_frame();

    const Uint64 current_time = SDL_GetTicksNS();
    delta_time = current_time - time_at_last_frame;
    const Sint64 time_left = CB_FRAME_BUDGET - delta_time;
    if (time_left > 0)
      SDL_DelayNS(time_left);

    time_at_last_frame = SDL_GetTicksNS();
    delta_time += time_at_last_frame - current_time;
  }

  cb_simulation_release(&simulation);
  cb_video_release();

  return 0;
}
