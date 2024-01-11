#pragma once
#ifndef CHEMICALBURN_CONNECTION_H
#define CHEMICALBURN_CONNECTION_H

#include "chemicalburn/node.h"
#include "chemicalburn/settings.h"

#include <stdbool.h>

#define CB_GET_COST(c) ((c)->cached_cost == 0xffffffff ? cb_get_cost((c)) : (c)->cached_cost)
#define CB_OTHER_NODE(connection, node) (node) == (connection)->node1 ? (connection)->node2 : (connection)->node1

typedef struct cb_connection_t {
  cb_node_t* node1, *node2;
  float weight;
  int num_packages;
  unsigned cached_cost;
  bool will_remove;
} cb_connection_t;

void cb_set_curve_for_weight(cb_curve curve, cb_weighting weighting);

void cb_connection_init(cb_connection_t* connection, cb_node_t* node1, cb_node_t* node2);

bool cb_contains_node(const cb_connection_t* connection, const cb_node_t* node);

void cb_increment_weight(cb_connection_t* connection);

void cb_decrement_weight(cb_connection_t* connection);

void cb_set_will_remove(cb_connection_t* connection);

float cb_get_weight(const cb_connection_t* connection);

unsigned cb_get_cost(cb_connection_t* connection);

float cb_get_length(const cb_connection_t* connection);

#endif
