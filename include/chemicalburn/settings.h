#pragma once
#ifndef CHEMICALBURN_SETTINGS_H
#define CHEMICALBURN_SETTINGS_H

#include "chemicalburn/cb_math.h"

#include <stdbool.h>

typedef enum cb_weighting {
  cb_connection_traffic_weight = 1,
  cb_connection_distance_weight,
} cb_weighting;

typedef enum cb_curve {
  cb_connection_weight_linear = 1,
  cb_connection_weight_sqrt,
  cb_connection_weight_sqr,
  cb_connection_weight_exp,
  cb_connection_weight_log,
  cb_connection_weight_bell, // applies to traffic only
} cb_curve;

typedef struct cb_simulation_settings_t {
  cb_ivec2_t bounds;
  cb_curve traffic_weight, distance_weight;
  unsigned optimal_node_count;
  bool create_destroy_nodes, package_of_death, show_stats;
} cb_simulation_settings_t;

typedef struct cb_video_settings_t {
  cb_ivec2_t viewport_size;
  float text_scale;
  bool vsync, fullscreen;
} cb_video_settings_t;

void cb_parse_settings(
  char* settings,
  cb_simulation_settings_t* simulation_settings,
  cb_video_settings_t* video_settings
);

void cb_settings_read_from_storage(cb_simulation_settings_t* simulation_settings, cb_video_settings_t* video_settings);

#endif
