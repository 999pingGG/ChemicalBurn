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
  bool create_destroy_nodes, package_of_death;
} cb_simulation_settings_t;

#endif
