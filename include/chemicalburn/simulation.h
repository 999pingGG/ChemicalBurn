#pragma once
#ifndef CHEMICALBURN_SIMULATION_H
#define CHEMICALBURN_SIMULATION_H

#include "chemicalburn/cb_math.h"
#include "chemicalburn/connections_map.h"
#include "chemicalburn/node_vec.h"
#include "chemicalburn/package_queue.h"
#include "chemicalburn/package_set.h"
#include "chemicalburn/settings.h"

typedef void (* cb_fill_frect_function)(const cb_frect_t* rect, const cb_color_t* color);

typedef void (* cb_draw_line_function)(
  const cb_vec2_t* from,
  const cb_vec2_t* to,
  float width,
  const cb_color_t* color
);

typedef void (* cb_draw_triangle_function)(const cb_vec2_t vertices[3], const cb_color_t* color);

typedef struct cb_drawing_functions_t {
  cb_fill_frect_function fill_rect;
  cb_draw_line_function draw_line;
  cb_draw_triangle_function draw_triangle;
} cb_drawing_functions_t;

typedef struct cb_simulation_t {
  cb_drawing_functions_t drawing_functions;
  cb_simulation_settings_t settings;
  cb_node_vec nodes, destroy_nodes;
  cb_connection_vec connections;
  cb_connections_map node_connections;
  cb_package_set packages;
  cb_package_queue packages_to_route;
  double package_steps, delivered_packages;
  cb_package_t* package_of_death;
  unsigned step;
} cb_simulation_t;

void cb_simulation_init(
  cb_simulation_t* simulation,
  const cb_drawing_functions_t* drawing_functions,
  const cb_simulation_settings_t* settings
);

cb_connection_t* cb_search_connection(cb_simulation_t* simulation, cb_node_t* src, cb_node_t* dst);

void cb_step(cb_simulation_t* simulation);

void cb_draw_nodes(const cb_simulation_t* simulation);

void cb_draw_connections(const cb_simulation_t* simulation);

void cb_draw_packages(const cb_simulation_t* simulation);

void cb_simulation_release(const cb_simulation_t* simulation);

#endif
