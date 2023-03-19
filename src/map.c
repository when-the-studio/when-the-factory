#include <stdlib.h>
#include <assert.h>
#include "map.h"
#include "entity.h"

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
		};
	}
}

Tile* get_tile(TileCoords coords) {
	assert(tile_coords_are_valid(coords));
	return &g_grid[coords.y * N_TILES_W + coords.x];
}
