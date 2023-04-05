#include <stdlib.h>
#include <assert.h>
#include "map.h"


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

static void add_entity_to_tile_list(Entity* entity, Tile* tile) {
	tile->entity_count++;
	tile->entities = realloc(tile->entities, tile->entity_count * sizeof(Entity*));
	tile->entities[tile->entity_count-1] = entity;
}

static void remove_entity_from_tile_list(Entity* entity, Tile* tile) {
	int entity_index_in_tile = -1;
	for (int i = 0; i < tile->entity_count; i++) {
		if (tile->entities[i] == entity) {
			entity_index_in_tile = i;
			break;
		}
	}
	assert(entity_index_in_tile != -1);
	for (int i = entity_index_in_tile; i < tile->entity_count - 1; i++) {
		tile->entities[i] = tile->entities[i+1];
	}
	tile->entity_count--;
	tile->entities = realloc(tile->entities, tile->entity_count * sizeof(Entity*));
}


static void add_building_to_tile(Building* building, Tile* tile) {
	tile->building = building;
}

static void remove_building_from_tile(Building* building, Tile* tile) {
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

Entity* new_entity(EntityType type, TileCoords pos) {
	Entity* entity = malloc(sizeof(Entity));
	*entity = (Entity){
		.type = type,
		.pos = pos,
	};
	Tile* tile = get_tile(pos);
	add_entity_to_tile_list(entity, tile);
	return entity;
}

void entity_delete(Entity* entity) {
	Tile* tile = get_tile(entity->pos);
	remove_entity_from_tile_list(entity, tile);
	free(entity);
}

void entity_move(Entity* entity, TileCoords new_pos) {
	TileCoords old_pos = entity->pos;
	Tile* old_tile = &g_grid[old_pos.y * N_TILES_W + old_pos.x];
	Tile* new_tile = &g_grid[new_pos.y * N_TILES_W + new_pos.x];
	remove_entity_from_tile_list(entity, old_tile);
	add_entity_to_tile_list(entity, new_tile);
	entity->pos = new_pos;
}

Building* new_building(BuildingType type, TileCoords pos) {
	Building* building = malloc(sizeof(Building));
	*building = (Building){
		.type = type,
		.pos = pos,
		.powered = false,
	};
	Tile* tile = get_tile(pos);
	add_building_to_tile(building, tile);
	
	return building;
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
			.entities = NULL,
			.building = NULL,
			.flows = NULL,
			.entity_count = 0,
			.flow_count = 0,
		};
	}
}

Tile* get_tile(TileCoords coords) {
	assert(tile_coords_are_valid(coords));
	return &g_grid[coords.y * N_TILES_W + coords.x];
}
