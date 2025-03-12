#include "chemicalburn/cb_math.h"
#include "chemicalburn/node.h"
#include "chemicalburn/node_vec.h"
#include "chemicalburn/package.h"
#include "chemicalburn/simulation.h"
#include "chemicalburn/util.h"
#include "chemicalburn/video.h"

#include <stc/algorithm.h>

#define i_TYPE cb_node_set, cb_node_t*
#include <stc/hset.h>

#include <stdio.h>

#define CB_NODE_CHANGE_INTERVAL 45
#define CB_PACKAGE_GENERATE_INTERVAL 60
#define CB_NODE_SIZE 6.0f
#define CB_NODE_HALF_SIZE 3.0f
#define CB_PACKAGE_SIZE 4.0f
#define CB_PACKAGE_HALF_SIZE 2.0f

typedef struct node_prio {
  cb_node_t* node;
  unsigned priority;
} node_prio;

static int compare_prio(const node_prio* a, const node_prio* b) {
  return a->priority < b->priority;
}

#define i_cmp compare_prio
#define i_TYPE cb_node_pqueue, node_prio
#include <stc/pque.h>

static void generate_node_at(cb_simulation_t* simulation, const cb_vec2_t* point) {
  cb_node_t* new_node = malloc(sizeof(cb_node_t));
  cb_node_init(new_node, point);

  cb_connection_vec array = { 0 };
  c_foreach(node, cb_node_vec, simulation->nodes) {
    cb_connection_t* new_connection = *cb_connection_vec_push_back(&array, malloc(sizeof(cb_connection_t)));
    cb_connection_vec_push_back(&simulation->connections, new_connection);
    cb_connection_init(new_connection, new_node, *node.ref);
    cb_connection_vec_push_back(cb_connections_map_at_mut(&simulation->node_connections, *node.ref), new_connection);
  }
  c_foreach(node, cb_node_vec, simulation->destroy_nodes) {
    cb_connection_t* new_connection = *cb_connection_vec_push_back(&array, malloc(sizeof(cb_connection_t)));
    cb_connection_vec_push_back(&simulation->connections, new_connection);
    cb_connection_init(new_connection, new_node, *node.ref);
    cb_set_will_remove(new_connection);
    cb_connection_vec_push_back(cb_connections_map_at_mut(&simulation->node_connections, *node.ref), new_connection);
  }

  cb_connections_map_insert_or_assign(&simulation->node_connections, new_node, array);

  cb_node_vec_push_back(&simulation->nodes, new_node);
}

static void generate_node(cb_simulation_t* simulation) {
  generate_node_at(
    simulation,
    &(cb_vec2_t){
      .x = cb_random_float_between(0, (float)simulation->settings.bounds.x),
      .y = cb_random_float_between(0, (float)simulation->settings.bounds.y),
    }
  );
}

static void generate_nodes(cb_simulation_t* simulation) {
  for (int i = 0; i < simulation->settings.optimal_node_count; i++)
    generate_node(simulation);
}

static cb_package_t* generate_package(cb_simulation_t* simulation, const bool of_death) {
  cb_node_t* source;
  cb_connection_t* connection;
  do {
    const unsigned source_index = cb_random_int_between(0, (int)cb_node_vec_size(&simulation->nodes));
    source = *cb_node_vec_at(&simulation->nodes, source_index);

    cb_connection_vec* connections = cb_connections_map_at_mut(&simulation->node_connections, source);
    const unsigned connection_index = cb_random_int_between(0, (int)cb_connection_vec_size(connections));
    connection = *cb_connection_vec_at_mut(connections, connection_index);
  } while (connection->will_remove);

  cb_node_t* destination = CB_OTHER_NODE(connection, source);
  cb_package_t* package = malloc(sizeof(cb_package_t));
  cb_package_init(package, source, destination, simulation->step);
  if (of_death)
    cb_set_package_of_death(package);

  cb_package_set_push(&simulation->packages, package);

  return package;
}

void cb_simulation_init(
  cb_simulation_t* simulation,
  const cb_simulation_settings_t* settings
) {
  *simulation = (cb_simulation_t){
    .settings = *settings,
  };

  generate_nodes(simulation);
  if (simulation->settings.create_destroy_nodes && simulation->settings.package_of_death)
    simulation->package_of_death = generate_package(simulation, true);

  cb_set_curve_for_weight(settings->traffic_weight, cb_connection_traffic_weight);
  cb_set_curve_for_weight(settings->distance_weight, cb_connection_distance_weight);
}

static void generate_packages(cb_simulation_t* simulation) {
  const unsigned count = cb_node_vec_size(&simulation->nodes);
  for (int i = 0; i < count; i++)
    if (cb_random_int_between(0, CB_PACKAGE_GENERATE_INTERVAL) == 0)
      generate_package(simulation, false);
}

static void redirect_package_away_from_node(cb_simulation_t* simulation, cb_package_t* package, cb_node_t* node) {
  cb_node_t* new_destination;
  do {
    new_destination = *cb_node_vec_at(
      &simulation->nodes,
      cb_random_int_between(0, (int)cb_node_vec_size(&simulation->nodes))
    );
  } while (new_destination == node || new_destination == cb_get_current_node(package));
  package->destination = new_destination;
}

static void add_node_to_destroy_list(cb_simulation_t* simulation, cb_node_t* node) {
  cb_node_vec_push_back(&simulation->destroy_nodes, node);
  cb_node_vec_iter result;
  c_find_if(cb_node_vec, simulation->nodes, &result, *value == node);
  cb_node_vec_erase_at(&simulation->nodes, result);

  c_foreach(vec, cb_connection_vec, *cb_connections_map_at(&simulation->node_connections, node)) {
    cb_set_will_remove(*vec.ref);
  }

  c_foreach(it, cb_package_set, simulation->packages) {
    // redirect packages whose destination is here but
    // who aren't yet on a direct connection
    cb_package_t* package = *it.ref;
    if (package->destination == node && cb_get_current_connection_destination(package) != node)
      redirect_package_away_from_node(simulation, package, node);
  }
}

static void routing_thread(cb_simulation_t* simulation) {
  while (!cb_package_queue_empty(&simulation->packages_to_route)) {
    cb_package_t* package = cb_package_queue_pull(&simulation->packages_to_route);

    cb_node_t* current_node = cb_get_current_node(package);
    cb_connection_t* next_connection = cb_search_connection(simulation, current_node, package->destination);
    cb_set_connection(package, next_connection, next_connection->node1 == current_node);
  }
}

static void step_packages(cb_simulation_t* simulation) {
  cb_package_set packages_to_erase = { 0 };

  c_foreach(it, cb_package_set, simulation->packages) {
    cb_package_t* package = *it.ref;

    cb_node_t* current_node = cb_get_current_node(package);
    if (!current_node)
      cb_step_package(package);
    else {
      if (package->is_package_of_death) {
        add_node_to_destroy_list(simulation, current_node);
        if (current_node == package->destination)
          redirect_package_away_from_node(simulation, package, current_node);
      }

      cb_increment_weight(package->current_connection);

      if (current_node == package->destination) {
        // Package has arrived to its destination.

        // Packages of death never arrive to a final destination.
        CB_ASSERT(!package->is_package_of_death);

        simulation->package_steps += simulation->step - package->start_step;
        simulation->delivered_packages++;

        cb_set_connection(package, NULL, false);

        cb_package_set_push(&packages_to_erase, package);
      } else
        cb_package_queue_push(&simulation->packages_to_route, package);
    }
  }

  c_foreach(it, cb_package_set, packages_to_erase) {
    cb_package_t* package = *it.ref;
    cb_package_set_erase(&simulation->packages, package);
    free(package);
  }
  cb_package_set_drop(&packages_to_erase);

  routing_thread(simulation);
}

static void degrade_connections(const cb_simulation_t* simulation) {
  c_foreach(it, cb_connection_vec, simulation->connections) {
    cb_decrement_weight(*it.ref);
  }
}

static bool should_destroy_node(const cb_simulation_t* simulation) {
  const intptr_t node_diff = cb_node_vec_size(&simulation->nodes) - simulation->settings.optimal_node_count;
  float factor =
    node_diff <= 0
    ? 1.0f
    : (float)(simulation->settings.optimal_node_count - node_diff) / (float)simulation->settings.optimal_node_count;
  factor = CB_MAX(factor, 0.0f);
  return
    cb_node_vec_size(&simulation->nodes) > 3
    && cb_random_int_between(0, (int)(CB_NODE_CHANGE_INTERVAL * factor)) == 0;
}

static void adjust_package_of_death_speed(cb_simulation_t* simulation) {
  const float factor = (float)cb_node_vec_size(&simulation->nodes) / (float)simulation->settings.optimal_node_count;
  simulation->package_of_death->speed = factor * factor;
}

static void random_add_node_to_destroy_list(cb_simulation_t* simulation) {
  if (should_destroy_node(simulation)) {
    const unsigned index = cb_random_int_between(0, (int)cb_node_vec_size(&simulation->nodes));
    add_node_to_destroy_list(simulation, *cb_node_vec_at_mut(&simulation->nodes, index));
  }
}

// This function is unused, just like in the original code.
// Perhaps it was used instead of adjust_package_of_death_speed(), but right now a segfault happens if we do this.
CB_UNUSED static void random_generate_package_of_death(cb_simulation_t* simulation) {
  if (should_destroy_node(simulation))
    generate_package(simulation, true);
}

static void scan_node_destroy_list(cb_simulation_t* simulation) {
  if (cb_node_vec_size(&simulation->destroy_nodes) <= 0)
    return;

  cb_node_t* death_node = cb_get_current_node(simulation->package_of_death);
  c_foreach(it, cb_node_vec, simulation->destroy_nodes) {
    cb_node_t* node = *it.ref;

    if (death_node == node)
      continue;

    cb_connection_vec* connections = cb_connections_map_at_mut(&simulation->node_connections, node);
    bool connections_have_packages = false;
    c_foreach(it2, cb_connection_vec, *connections) {
      cb_connection_t* connection = *it2.ref;

      if (connection->num_packages > 0) {
        connections_have_packages = true;
        break;
      }
    }

    if (!connections_have_packages) {
      c_foreach(it2, cb_connection_vec, *connections) {
        cb_connection_t* connection = *it2.ref;

        cb_node_t* other = CB_OTHER_NODE(connection, node);
        cb_connection_vec* vec = cb_connections_map_at_mut(&simulation->node_connections, other);
        cb_connection_vec_iter result;
        c_find_if(cb_connection_vec, *vec, &result, *value == connection);
        cb_connection_vec_erase_at(vec, result);
        c_find_if(cb_connection_vec, simulation->connections, &result, *value == connection);
        cb_connection_vec_erase_at(&simulation->connections, result);
        free(connection);
      }

      cb_connection_vec_drop(cb_connections_map_at_mut(&simulation->node_connections, node));
      cb_connections_map_erase(&simulation->node_connections, node);
      cb_node_vec_iter result;
      c_find_if(cb_node_vec, simulation->destroy_nodes, &result, *value == node);
      cb_node_vec_erase_at(&simulation->destroy_nodes, result);
      free(node);
    }
  }
}

static void destroy_nodes(cb_simulation_t* simulation) {
  if (simulation->settings.package_of_death)
    adjust_package_of_death_speed(simulation);
  else
    random_add_node_to_destroy_list(simulation);

  scan_node_destroy_list(simulation);
}

static void create_nodes(cb_simulation_t* simulation) {
  const int node_diff = (int)simulation->settings.optimal_node_count - (int)cb_node_vec_size(&simulation->nodes);
  const float factor =
    node_diff <= 0
    ? 1.0f
    : (float)(simulation->settings.optimal_node_count - node_diff) / (float)simulation->settings.optimal_node_count;
  if (cb_random_int_between(0, CB_NODE_CHANGE_INTERVAL * factor) == 0)
    generate_node(simulation);
}

cb_connection_t* cb_search_connection(cb_simulation_t* simulation, cb_node_t* src, cb_node_t* dst) {
  // initialize costs with infinity
  c_foreach(it, cb_node_vec, simulation->nodes) {
    (*it.ref)->cost = 0xffffffff;
  }
  // except for source
  src->cost = 0;

  cb_node_set hit_nodes = { 0 };
  cb_node_pqueue remaining_nodes = { 0 };
  cb_node_pqueue_push(&remaining_nodes, (node_prio){ .node = src, .priority = 0 });

  while (cb_node_pqueue_size(&remaining_nodes) > 0) {
    cb_node_t* best_node = cb_node_pqueue_top(&remaining_nodes)->node;
    cb_node_pqueue_pop(&remaining_nodes);
    if (cb_node_set_contains(&hit_nodes, best_node))
      // skip stuff we've already seen
      continue;

    cb_node_set_insert(&hit_nodes, best_node);

    const unsigned best_cost = best_node->cost;

    if (best_node == dst)
      break;

    // follow all connections from the smallest node
    c_foreach(it, cb_connection_vec, *cb_connections_map_at_mut(&simulation->node_connections, best_node)) {
      cb_connection_t* connection = *it.ref;

      // find the other node for the connection
      cb_node_t* other_node = CB_OTHER_NODE(connection, best_node);

      // calculate costs
      const unsigned connection_cost = CB_GET_COST(connection);
      const unsigned current_cost = other_node->cost;
      if (best_cost + connection_cost < current_cost) {
        // this route is faster, so reset cost and prev of the other node
        other_node->cost = best_cost + connection_cost;
        other_node->previous = best_node;

        cb_node_pqueue_push(
          &remaining_nodes,
          (node_prio){
            .node = other_node,
            .priority = best_cost + connection_cost,
          }
        );
      }
    }
  }

  cb_node_set_drop(&hit_nodes);
  cb_node_pqueue_drop(&remaining_nodes);

  // we found the fastest path, read it out of the prevs starting from the destination
  cb_node_t* node = NULL, * previous = dst;
  while (previous != src) {
    CB_ASSERT(previous);
    node = previous;
    previous = node->previous;
  }

  // node now contains the first node to follow on the best path away from src
  // so find the connection that matches
  c_foreach(it, cb_connection_vec, *cb_connections_map_at_mut(&simulation->node_connections, src)) {
    cb_connection_t* connection = *it.ref;
    if (cb_contains_node(connection, node))
      return connection;
  }

  // Execution must never reach here.
  CB_ASSERT(false);
  return NULL;
}

void cb_step(cb_simulation_t* simulation) {
  step_packages(simulation);

  if (simulation->settings.create_destroy_nodes) {
    destroy_nodes(simulation);
    create_nodes(simulation);
  }

  degrade_connections(simulation);
  generate_packages(simulation);

  simulation->step++;
}

void cb_draw_nodes(const cb_simulation_t* simulation) {
  const static cb_color_t white = { 1.0f, 1.0f, 1.0f, 1.0f };
  const static cb_color_t red = { 0.5f, 0.0f, 0.0f, 1.0f };

  c_foreach(it, cb_node_vec, simulation->nodes) {
    const cb_node_t* node = *it.ref;
    cb_fill_frect(
      &(cb_frect_t){
        .x = node->position.x - CB_NODE_HALF_SIZE,
        .y = node->position.y - CB_NODE_HALF_SIZE,
        .w = CB_NODE_SIZE,
        .h = CB_NODE_SIZE,
      },
      &white
    );
    if (simulation->settings.show_stats) {
      char id_string[12];
      snprintf(id_string, sizeof(id_string), "%u", node->id);
      cb_draw_text(
        id_string,
        node->position.x,
        node->position.y + CB_NODE_HALF_SIZE,
        cb_upper_middle,
        &white);
    }
  }

  c_foreach(it, cb_node_vec, simulation->destroy_nodes) {
    const cb_node_t* node = *it.ref;
    cb_fill_frect(
      &(cb_frect_t){
        .x = node->position.x - CB_NODE_HALF_SIZE,
        .y = node->position.y - CB_NODE_HALF_SIZE,
        .w = CB_NODE_SIZE,
        .h = CB_NODE_SIZE,
      },
      &red
    );
    if (simulation->settings.show_stats) {
      char id_string[12];
      snprintf(id_string, sizeof(id_string), "%u", node->id);
      cb_draw_text(
        id_string,
        node->position.x,
        node->position.y + CB_NODE_HALF_SIZE,
        cb_upper_middle,
        &red);
    }
  }
}

void cb_draw_connections(const cb_simulation_t* simulation) {
  c_foreach(it, cb_connection_vec, simulation->connections) {
    cb_connection_t* connection = *it.ref;

    const float width = CB_MIN((connection->weight - 1.0f) / 24.0f, 6.0f);

    if (width < 1.0f / 255.0f)
      continue;

    const float real_width = CB_MAX(width, 1.0f);
    const float alpha = CB_MIN(width, 1.0f);

    cb_color_t color =
      connection->will_remove
      ? (cb_color_t){ 1.0f, 0.0f, 0.0f, alpha }
      : (cb_color_t){ 1.0f, 1.0f, 1.0f, alpha };
    cb_draw_line(
      &connection->node1->position,
      &connection->node2->position,
      real_width,
      &color
    );
  }
}

void cb_draw_packages(const cb_simulation_t* simulation) {
  c_foreach(it, cb_package_set, simulation->packages) {
    cb_package_t* package = *it.ref;

    cb_vec2_t point;
    if (package->current_connection) {
      const cb_vec2_t* p1 = &package->current_connection->node1->position;
      const cb_vec2_t* p2 = &package->current_connection->node2->position;

      const float proportion = package->forward ? package->proportion : 1.0f - package->proportion;

      point = (cb_vec2_t){
        .x = p1->x * (1.0f - proportion) + p2->x * proportion,
        .y = p1->y * (1.0f - proportion) + p2->y * proportion,
      };
    } else
      point = package->source->position;

    if (package->is_package_of_death) {
      const cb_vec2_t vertices[3] = {
        {
          .x = point.x - CB_PACKAGE_SIZE,
          .y = point.y + CB_PACKAGE_SIZE,
        },
        {
          .x = point.x,
          .y = point.y - CB_PACKAGE_SIZE,
        },
        {
          .x = point.x + CB_PACKAGE_SIZE,
          .y = point.y + CB_PACKAGE_SIZE,
        },
      };
      cb_draw_triangle(vertices, &package->color);
    } else {
      cb_fill_frect(
        &(cb_frect_t){
          .x = point.x - CB_PACKAGE_HALF_SIZE,
          .y = point.y - CB_PACKAGE_HALF_SIZE,
          .w = CB_PACKAGE_SIZE,
          .h = CB_PACKAGE_SIZE,
        },
        &package->color
      );
    }
  }
}

void cb_simulation_release(const cb_simulation_t* simulation) {
  c_foreach(it, cb_package_set, simulation->packages) {
    free(*it.ref);
  }
  cb_package_set_drop(&simulation->packages);
  cb_package_queue_drop(&simulation->packages_to_route);

  c_foreach(it, cb_node_vec, simulation->nodes) {
    free(*it.ref);
  }
  cb_node_vec_drop(&simulation->nodes);

  c_foreach(it, cb_node_vec, simulation->destroy_nodes) {
    free(*it.ref);
  }
  cb_node_vec_drop(&simulation->destroy_nodes);

  c_foreach(it, cb_connections_map, simulation->node_connections) {
    cb_connection_vec_drop(&it.ref->second);
  }
  cb_connections_map_drop(&simulation->node_connections);

  c_foreach(it, cb_connection_vec, simulation->connections) {
    free(*it.ref);
  }
  cb_connection_vec_drop(&simulation->connections);
}
