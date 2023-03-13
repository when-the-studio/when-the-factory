#include <stdlib.h>
#include "map.h"

tile_t* grid;

void init_map(void) {
	grid = malloc(N_TILES * sizeof(tile_t));
	for(int i = 0; i < N_TILES; ++i) {
		grid[i].type = rand() % TILE_TYPE_NUM;
		grid[i].pos = (coord_t){i % N_TILES_W, i % N_TILES_W};
	}
}