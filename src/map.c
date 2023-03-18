#include <stdlib.h>
#include <assert.h>
#include "map.h"

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

Tile* g_grid = NULL;

void init_map(void) {
	g_grid = malloc(N_TILES * sizeof(Tile));
	for(int y = 0; y < N_TILES_H; ++y) for(int x = 0; x < N_TILES_W; ++x) {
		TileCoords tc = {x, y};
		Tile* tile = get_tile(tc);
		*tile = (Tile){
			.type = rand() % TILE_TYPE_NUM,
			.entities = NULL,
			.entity_count = 0,
		};
	}
}

Tile* get_tile(TileCoords coords) {
	assert(tile_coords_are_valid(coords));
	return &g_grid[coords.y * N_TILES_W + coords.x];
}
