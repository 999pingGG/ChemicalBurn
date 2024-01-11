#include "chemicalburn/connection.h"
#include "chemicalburn/node.h"
#include "chemicalburn/util.h"
#include "chemicalburn/cb_math.h"

#include <math.h>

static cb_curve traffic_weight;
static cb_curve distance_weight;

void cb_set_curve_for_weight(const cb_curve curve, const cb_weighting weighting) {
  switch (weighting) {
    case cb_connection_traffic_weight:
      traffic_weight = curve;
      break;
    case cb_connection_distance_weight:
      distance_weight = curve;
      break;
    default:
      CB_DEBUG_LOG("Unknown weighting: %i\n", weighting);
      break;
  }
}

void cb_connection_init(cb_connection_t* connection, cb_node_t* node1, cb_node_t* node2) {
  CB_ASSERT(node1 != node2);

  *connection = (cb_connection_t){
    .weight = 1.0f,
    .cached_cost = 0xffffffff,
  };

  if (node1->id < node2->id) {
    connection->node1 = node1;
    connection->node2 = node2;
  } else {
    connection->node1 = node2;
    connection->node2 = node1;
  }
}

bool cb_contains_node(const cb_connection_t* connection, const cb_node_t* node) {
  return node == connection->node1 || node == connection->node2;
}

void cb_increment_weight(cb_connection_t* connection) {
  if (!connection)
    return;

  connection->cached_cost = 0xffffffff;
  connection->weight++;
}

void cb_decrement_weight(cb_connection_t* connection) {
  connection->cached_cost = 0xffffffff;
  connection->weight = (connection->weight - 1.0f) * 0.99f + 1.0f;
}

void cb_set_will_remove(cb_connection_t* connection) {
  connection->cached_cost = 0xffffffff;
  connection->will_remove = true;
}

float cb_get_weight(const cb_connection_t* connection) {
  switch (traffic_weight) {
    case cb_connection_weight_linear:
      return connection->weight;
    case cb_connection_weight_sqrt:
      return sqrtf(connection->weight);
    case cb_connection_weight_sqr:
      return connection->weight * connection->weight;
    case cb_connection_weight_exp:
      return CB_MIN(expf(connection->weight / 3.0f), 1000000.0f);
    case cb_connection_weight_log:
      return logf(connection->weight) + 1.0f;
    case cb_connection_weight_bell: {
      const float adjustment = connection->weight / 3.0f - 2.0f;
      return CB_MAX(0.01f, expf(adjustment - (adjustment * adjustment) / 2.0f) * 25.0f);
    }
    default:
      CB_DEBUG_LOG("Unknown curve: %i\n", traffic_weight);
      return connection->weight;
  }
}

unsigned cb_get_cost(cb_connection_t* connection) {
  if (connection->cached_cost == 0xffffffff) {
    // for willremove connections, cost needs to be
    // low enough that you can pathfind through it as a last resort
    // for packages with no alternative

    connection->cached_cost = (unsigned)ceilf(cb_get_length(connection) / cb_get_weight(connection)) + 1u;
    if (connection->will_remove)
      connection->cached_cost += 0x00ffffff;
  }

  return connection->cached_cost;
}

float cb_get_length(const cb_connection_t* connection) {
  cb_vec2_t diff;
  cb_vec2_minus(&connection->node1->position, &connection->node2->position, &diff);

  const float length_squared = cb_vec2_length_squared(&diff);

  switch (distance_weight) {
    case cb_connection_weight_linear:
      return sqrtf(length_squared);
    case cb_connection_weight_sqrt:
      return powf(length_squared, 0.25f) * 5.0f;
    case cb_connection_weight_sqr:
      return length_squared / 25.0f;
    case cb_connection_weight_exp:
      return CB_MIN(expf(sqrtf(length_squared) / 10.0f) / 3.0f, 1000000.0f);
    case cb_connection_weight_log:
      return CB_MAX((logf(length_squared) / 2.0f + 1.0f) * 25.0f, 1.0f);
    case cb_connection_weight_bell:
      CB_DEBUG_LOG("Bell curve doesn't make sense for distance.\n");
      return sqrtf(length_squared);
    default:
      CB_DEBUG_LOG("Unknown curve: %i\n", distance_weight);
      return sqrtf(length_squared);
  }
}
