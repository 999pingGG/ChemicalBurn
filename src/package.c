#include "chemicalburn/connection.h"
#include "chemicalburn/node.h"
#include "chemicalburn/package.h"
#include "chemicalburn/util.h"
#include "chemicalburn/cb_math.h"

#include <stdlib.h>

void cb_package_init(
  cb_package_t* package,
  cb_node_t* source,
  cb_node_t* destination,
  const unsigned start_step
) {
  *package = (cb_package_t){
    .source = source,
    .destination = destination,
    .start_step = start_step,
    .color = {
      .r = rand() % 256,
      .g = rand() % 256,
      .b = rand() % 256,
      .a = 255,
    },
    .speed = cb_random_float_between(0.5f, 2.0f),
    .proportion = 1.0f,
  };
}

void cb_set_connection(cb_package_t* package, cb_connection_t* connection, const bool forward) {
  if (package->current_connection)
    package->current_connection->num_packages--;
  package->current_connection = connection;
  if (connection)
    connection->num_packages++;

  package->forward = forward;
  package->proportion = 0.0f;
}

cb_node_t* cb_get_current_connection_destination(const cb_package_t* package) {
  if (!package->current_connection)
    return NULL;
  return package->forward ? package->current_connection->node2 : package->current_connection->node1;
}

cb_node_t* cb_get_current_node(const cb_package_t* package) {
  if (package->proportion < 1.0f)
    return NULL;

  cb_node_t* node = cb_get_current_connection_destination(package);
  if (!node)
    node = package->source;
  return node;
}

void cb_step_package(cb_package_t* package) {
  CB_ASSERT(package->proportion < 1.0f);

  const float length = cb_get_length(package->current_connection);
  if (length > 0.0f)
    package->proportion += package->speed * cb_get_weight(package->current_connection) / length;
  else
    package->proportion = 1.0f;
  package->proportion = CB_MIN(package->proportion, 1.0f);
}

void cb_set_package_of_death(cb_package_t* package) {
  package->is_package_of_death = true;
  package->color = (cb_color_t){
    .r = 1.0f,
    .g = 0.0f,
    .b = 0.0f,
    .a = 1.0f,
  };
}
