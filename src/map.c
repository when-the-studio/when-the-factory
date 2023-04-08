#include <stdlib.h>
#include <assert.h>

#include "map.h"
#include "entity.h"
#include "flow.h"
#include "utils.h"


FlowTypeSpec g_flow_type_spec_table[FLOW_TX_NUM] = {
	[ELECTRICITY_STRAIGHT] = {
		.rect_in_spritesheet = {16, 16, 8, 8},
		.name = "Electric cable (straight)",
	},
	[ELECTRICITY_TURN] = {
		.rect_in_spritesheet = {24, 16, 8, 8},
		.name = "Electric cable (turn)",
	},
};


TileTypeSpec g_tile_type_spec_table[TILE_TYPE_NUM] = {
	[TILE_PLAIN] = {
		.rect_in_spritesheet = {0, 0, 8, 8},
		.name = "Plain",
	},
	[TILE_MOUTAIN] = {
		.rect_in_spritesheet = {16, 0, 8, 8},
		.name = "Mountain",
	},
	[TILE_RIVER] = {
		.rect_in_spritesheet = {8, 0, 8, 8},
		.name = "River",
	},
	[TILE_FOREST] = {
		.rect_in_spritesheet = {24, 0, 8, 8},
		.name = "Forest",
	},
};

BuildingTypeSpec g_building_type_spec_table[BUILDING_TX_NUM] = {
	[BUILDING_TX_EMITTER] = {
		.rect_in_spritesheet = {0, 24, 8, 8},
		.name = "Power Plant",
	},
	[BUILDING_TX_RECEIVER_OFF] = {
		.rect_in_spritesheet = {8, 24, 8, 8},
		.name = "Shield off",
	},
	[BUILDING_TX_RECEIVER_ON] = {
		.rect_in_spritesheet = {16, 24, 8, 8},
		.name = "Shield on",
	},
};


bool tile_coords_are_valid(TileCoords coords) {
	return
		0 <= coords.x && coords.x < N_TILES_W &&
		0 <= coords.y && coords.y < N_TILES_H;
}

bool tile_coords_eq(TileCoords a, TileCoords b) {
	return a.x == b.x && a.y == b.y;
}

void add_eid_to_tile_list(EntId eid, Tile* tile) {
	for (int i = 0; i < tile->ent_count; i++) {
		if (eid_null(tile->ents[i])) {
			tile->ents[i] = eid;
			return;
		}
	}
	tile->ent_count++;
	tile->ents = realloc(tile->ents, tile->ent_count * sizeof(Ent*));
	tile->ents[tile->ent_count-1] = eid;
}

void remove_eid_from_tile_list(EntId eid, Tile* tile) {
	for (int i = 0; i < tile->ent_count; i++) {
		if (eid_eq(tile->ents[i], eid)) {
			tile->ents[i] = EID_NULL;
			break;
		}
	}
	while (eid_null(tile->ents[tile->ent_count-1])) {
		tile->ent_count--;
	}
	tile->ents = realloc(tile->ents, tile->ent_count * sizeof(Ent*));
}

static void remove_building_from_tile(Tile* tile) {
	tile->building = NULL;
}

static void add_flow_to_tile_list(Flow* flow, Tile* tile) {
	tile->flow_count++;
	tile->flows = realloc(tile->flows, tile->flow_count * sizeof(Flow*));
	tile->flows[tile->flow_count-1] = flow;
}

static void remove_flow_from_tile_list(Flow* flow, Tile* tile) {
	int flow_index_in_tile = -1;
	for (int i = 0; i < tile->flow_count; i++) {
		if (tile->flows[i] == flow) {
			flow_index_in_tile = i;
			break;
		}
	}
	assert(flow_index_in_tile != -1);
	for (int i = flow_index_in_tile; i < tile->flow_count - 1; i++) {
		tile->flows[i] = tile->flows[i+1];
	}
	tile->flow_count--;
	tile->flows = realloc(tile->flows, tile->flow_count * sizeof(Flow*));
}

Flow* new_flow(FlowType type, TileCoords pos, CardinalType entry, CardinalType exit) {
	Flow* flow = malloc(sizeof(Flow));
	*flow = (Flow){
		.type = type,
		.pos = pos,
		.connections = {entry, exit},
		.capacity = 10,
		.powered = false,
	};
	qsort(flow->connections, 2, sizeof(int), cmpInt);
	Tile* tile = get_tile(pos);
	add_flow_to_tile_list(flow, tile);
	
	return flow;
}

Tile* g_grid = NULL;

void init_map(void) {
	g_grid = malloc(N_TILES * sizeof(Tile));
	for(int y = 0; y < N_TILES_H; ++y) for(int x = 0; x < N_TILES_W; ++x) {
		TileCoords tc = {x, y};
		Tile* tile = get_tile(tc);
		*tile = (Tile){
			.type = rand() % TILE_TYPE_NUM,
			.ents = NULL,
			.ent_count = 0,
			.building = NULL,
			.flows = NULL,
			.flow_count = 0,
		};
	}
}

Tile* get_tile(TileCoords coords) {
	assert(tile_coords_are_valid(coords));
	return &g_grid[coords.y * N_TILES_W + coords.x];
}
