#pragma once
#ifndef CHEMICALBURN_CONNECTIONS_MAP_H
#define CHEMICALBURN_CONNECTIONS_MAP_H

#include "chemicalburn/connection_vec.h"

#define i_TYPE cb_connections_map, cb_node_t*, cb_connection_vec
#define i_header
#ifdef CB_IMPLEMENT_CONNECTIONS_MAP
#undef CB_IMPLEMENT_CONNECTIONS_MAP
#define i_implement
#endif

#include <stc/hmap.h>

#endif
