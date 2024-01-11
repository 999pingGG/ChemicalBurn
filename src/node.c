#include "chemicalburn/cb_math.h"
#include "chemicalburn/node.h"

void cb_node_init(cb_node_t* node, const cb_vec2_t* position) {
  static unsigned next_id = 0;
  *node = (cb_node_t){
    .id = next_id++,
    .position = *position,
  };
}
