#include <stdlib.h>
#include <assert.h>
#include "map.h"

bool tile_coords_are_valid(TileCoords coords) {
	return
		0 <= coords.x && coords.x < N_TILES_W &&
		0 <= coords.y && coords.y < N_TILES_H;
}

bool tile_coords_eq(TileCoords a, TileCoords b) {
	return a.x == b.x && a.y == b.y;
}

Tile* g_grid = NULL;

void init_map(void) {
	g_grid = malloc(N_TILES * sizeof(Tile));
	for(int i = 0; i < N_TILES; ++i) {
		g_grid[i] = (Tile){
			.type = rand() % TILE_TYPE_NUM,
			.entities = NULL,
		};
	}
}

Tile* get_tile(TileCoords coords) {
	assert(tile_coords_are_valid(coords));
	return &g_grid[coords.y * N_TILES_W + coords.x];
}
