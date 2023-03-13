#include <stdlib.h>
#include "map.h"

tile_t* g_grid;

void init_map(void) {
	g_grid = malloc(N_TILES * sizeof(tile_t));
	for(int i = 0; i < N_TILES; ++i) {
		g_grid[i].type = rand() % TILE_TYPE_NUM;
		g_grid[i].pos = (coord_t){i % N_TILES_W, i % N_TILES_W};
	}
}