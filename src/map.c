#include <stdlib.h>
#include <assert.h>
#include "map.h"

Tile* g_grid;

void init_map(void) {
	g_grid = malloc(N_TILES * sizeof(Tile));
	for(int i = 0; i < N_TILES; ++i) {
		g_grid[i].type = rand() % TILE_TYPE_NUM;
		g_grid[i].pos = (Coord){i % N_TILES_W, i % N_TILES_W};
		g_grid[i].entities = NULL;
		g_grid[i].entity_count = 0;
	}
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

Entity* new_entity(EntityType type, Coord pos) {
	Entity* entity = malloc(sizeof(Entity));
	Tile* tile = &g_grid[pos.y * N_TILES_W + pos.x];
	add_entity_to_tile_list(entity, tile);
	entity->pos = pos;
	entity->type = type;
	return entity;
}

void entity_delete(Entity* entity) {
	Tile* tile = &g_grid[entity->pos.y * N_TILES_W + entity->pos.x];
	remove_entity_from_tile_list(entity, tile);
	free(entity);
}

void entity_move(Entity* entity, Coord new_pos) {
	Coord old_pos = entity->pos;
	Tile* old_tile = &g_grid[old_pos.y * N_TILES_W + old_pos.x];
	Tile* new_tile = &g_grid[new_pos.y * N_TILES_W + new_pos.x];
	remove_entity_from_tile_list(entity, old_tile);
	add_entity_to_tile_list(entity, new_tile);
	entity->pos = new_pos;
}
