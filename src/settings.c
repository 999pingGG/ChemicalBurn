#include "chemicalburn/settings.h"
#include "chemicalburn/util.h"

#include <SDL3/SDL.h>

#include <stdio.h>
#include <string.h>

#define CB_SETTINGS_FILE_NAME "settings.conf"

#define CB_SETTINGS_FILE_MAX_SIZE 10000

#define CB_DEFAULT_SETTINGS \
  "#### Simulation settings ####\n" \
  "\n" \
  "# Possible values are linear, sqrt, sqr, exp, log, bell\n" \
  "# Default: sqrt\n" \
  "traffic_weighting=sqrt\n" \
  "\n" \
  "# Possible values are linear, sqrt, sqr, exp, log\n" \
  "# Default: linear\n" \
  "distance_weighting=linear\n" \
  "\n" \
  "# Possible values are 0 and 1\n" \
  "# Default: 1\n" \
  "create_and_destroy_nodes_at_random=1\n" \
  "\n" \
  "# Possible values are 0 and 1\n" \
  "# Default: 1\n" \
  "package_of_death=1\n" \
  "\n" \
  "# Possible values are 2 and higher.\n" \
  "# Default: 100\n" \
  "number_of_nodes=100\n" \
  "\n" \
  "\n" \
  "#### Video settings ####\n" \
  "\n" \
  "# Possible values are 1 and higher.\n" \
  "# Default: 1\n" \
  "vsync=1\n" \
  "\n" \
  "# Possible values are 0 and 1.\n" \
  "# Default: 1\n" \
  "fullscreen=1\n" \
  "\n" \
  "# Possible values depend on your system.\n" \
  "# Default: 1024\n" \
  "width=1024\n" \
  "# Default: 768\n" \
  "height=768\n"

#define CB_DEFAULT_SETTINGS_LENGTH (sizeof(CB_DEFAULT_SETTINGS) - 1)

#define CB_PARSE_WEIGHT(curve, weight) \
  if (strcmp(buffer, #curve) == 0) \
    simulation_settings->weight##_weight = cb_connection_weight_##curve

#define CB_PARSE_WEIGHTS(weight) do { \
  CB_PARSE_WEIGHT(linear, weight); \
  else CB_PARSE_WEIGHT(sqrt, weight); \
  else CB_PARSE_WEIGHT(sqr, weight); \
  else CB_PARSE_WEIGHT(exp, weight); \
  else CB_PARSE_WEIGHT(log, weight); \
  else CB_PARSE_WEIGHT(bell, weight); \
} while (0)

void cb_parse_settings(
  char* settings,
  cb_simulation_settings_t* simulation_settings,
  cb_video_settings_t* video_settings
) {
  char* token = strtok(settings, "\n");
  do {
    if (!token)
      continue;

    char buffer[CB_SETTINGS_FILE_MAX_SIZE];
    *buffer = '\0';
    int int_buffer;

    if (sscanf(token, " %s", buffer) == EOF)
      // Ignore blank lines.
      continue;

    if (sscanf(token, " %[#]", buffer) == 1)
      // Ignore comments.
      continue;

    if (sscanf(token, " traffic_weighting = %s", buffer) == 1)
      CB_PARSE_WEIGHTS(traffic);
    else if (sscanf(token, " distance_weighting = %s", buffer) == 1)
      CB_PARSE_WEIGHTS(distance);
    else if (sscanf(token, " create_and_destroy_nodes_at_random = %i", &int_buffer) == 1)
      simulation_settings->create_destroy_nodes = int_buffer;
    else if (sscanf(token, " package_of_death = %i", &int_buffer) == 1)
      simulation_settings->package_of_death = int_buffer;
    else if (sscanf(token, " number_of_nodes = %i", &int_buffer) == 1)
      simulation_settings->optimal_node_count = CB_MAX(int_buffer, 2);
    else if (sscanf(token, " vsync = %i", &int_buffer) == 1)
      video_settings->vsync = int_buffer;
    else if (sscanf(token, " fullscreen = %i", &int_buffer) == 1)
      video_settings->fullscreen = int_buffer;
    else if (sscanf(token, " width = %i", &int_buffer) == 1)
      simulation_settings->bounds.x = video_settings->viewport_size.x = int_buffer;
    else if (sscanf(token, " height = %i", &int_buffer) == 1)
      simulation_settings->bounds.y = video_settings->viewport_size.y = int_buffer;
  } while ((token = strtok(NULL, "\n")));

  if (simulation_settings->distance_weight == cb_connection_weight_bell)
    simulation_settings->distance_weight = cb_connection_weight_linear;
}

void cb_settings_read_from_storage(cb_simulation_settings_t* simulation_settings, cb_video_settings_t* video_settings) {
  *simulation_settings = (cb_simulation_settings_t){
    .bounds = {
      .x = 1024,
      .y = 768,
    },
    .traffic_weight = cb_connection_weight_sqrt,
    .distance_weight = cb_connection_weight_linear,
    .optimal_node_count = 100u,
    .create_destroy_nodes = true,
    .package_of_death = true,
  };

  *video_settings = (cb_video_settings_t){
    .viewport_size = simulation_settings->bounds,
    .vsync = true,
    .fullscreen = true,
  };

  char* pref_path = SDL_GetPrefPath(CB_ORGANIZATION, CB_APP);
  if (!pref_path)
    return;

  char path[500];
  snprintf(path, sizeof(path), "%s%s", pref_path, CB_SETTINGS_FILE_NAME);
  SDL_free(pref_path);

  SDL_RWops* file = SDL_RWFromFile(path, "rt");
  if (!file) {
    printf("%s\nTrying to create it...\n", SDL_GetError());
    SDL_ClearError();

    file = SDL_RWFromFile(path, "wt");
    if (!file) {
      fprintf(stderr, "%s\n", SDL_GetError());
      SDL_ClearError();
      return;
    }

    if (SDL_RWwrite(file, CB_DEFAULT_SETTINGS, CB_DEFAULT_SETTINGS_LENGTH) < CB_DEFAULT_SETTINGS_LENGTH) {
      fprintf(stderr, "SDL_RWwrite() error: %s\n", SDL_GetError());
      SDL_ClearError();
    }
  } else {
    char contents[CB_SETTINGS_FILE_MAX_SIZE] = { 0 };
    if (SDL_RWread(file, contents, sizeof(contents)) == -1) {
      fprintf(stderr, "SDL_RWread() failed: %s\n", SDL_GetError());
      SDL_ClearError();
      goto exit;
    }

    cb_parse_settings(contents, simulation_settings, video_settings);
  }

  exit:
  if (SDL_RWclose(file) < 0) {
    fprintf(stderr, "SDL_RWclose() failed: %s\n", SDL_GetError());
    SDL_ClearError();
  }
}

#undef CB_PARSE_WEIGHT
#undef CB_PARSE_WEIGHTS
