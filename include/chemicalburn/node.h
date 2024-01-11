#pragma once
#ifndef CHEMICALBURN_NODE_H
#define CHEMICALBURN_NODE_H

#include "chemicalburn/cb_math.h"

typedef struct cb_node_t {
  cb_vec2_t position;
  struct cb_node_t* previous;
  unsigned id, cost;
} cb_node_t;

void cb_node_init(cb_node_t* node, const cb_vec2_t* position);

#endif
