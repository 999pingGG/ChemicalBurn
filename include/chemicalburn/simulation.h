#pragma once
#ifndef CHEMICALBURN_SIMULATION_H
#define CHEMICALBURN_SIMULATION_H

#include "chemicalburn/cb_math.h"
#include "chemicalburn/connections_map.h"
#include "chemicalburn/node_vec.h"
#include "chemicalburn/package_queue.h"
#include "chemicalburn/package_set.h"
#include "chemicalburn/settings.h"
#include "chemicalburn/video.h"

typedef struct cb_simulation_t {
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
  const cb_simulation_settings_t* settings
);

cb_connection_t* cb_search_connection(cb_simulation_t* simulation, cb_node_t* src, cb_node_t* dst);

void cb_step(cb_simulation_t* simulation);

void cb_draw_nodes(const cb_simulation_t* simulation);

void cb_draw_connections(const cb_simulation_t* simulation);

void cb_draw_packages(const cb_simulation_t* simulation);

void cb_simulation_release(const cb_simulation_t* simulation);

#endif
