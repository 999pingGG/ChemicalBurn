#pragma once
#ifndef CHEMICALBURN_PACKAGE_H
#define CHEMICALBURN_PACKAGE_H

#include "chemicalburn/connection.h"
#include "chemicalburn/node.h"

#include <stdbool.h>

typedef struct cb_package_t {
  float speed, proportion;
  cb_node_t* source, *destination;
  cb_connection_t* current_connection;
  unsigned start_step;
  cb_color_t color;
  bool forward, is_package_of_death;
} cb_package_t;

void cb_package_init(cb_package_t* package, cb_node_t* source, cb_node_t* destination, unsigned start_step);

void cb_set_connection(cb_package_t* package, cb_connection_t* connection, bool forward);

cb_node_t* cb_get_current_connection_destination(const cb_package_t* package);

cb_node_t* cb_get_current_node(const cb_package_t* package);

void cb_step_package(cb_package_t* package);

void cb_set_package_of_death(cb_package_t* package);

#endif
