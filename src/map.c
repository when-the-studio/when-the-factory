#include <stdlib.h>
#include <assert.h>
#include "map.h"

int g_map_w = 250;
int g_map_h = 250;

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
		0 <= coords.x && coords.x < g_map_w &&
		0 <= coords.y && coords.y < g_map_h;
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
	Tile* old_tile = &g_grid[old_pos.y * g_map_w + old_pos.x];
	Tile* new_tile = &g_grid[new_pos.y * g_map_w + new_pos.x];
	remove_entity_from_tile_list(entity, old_tile);
	add_entity_to_tile_list(entity, new_tile);
	entity->pos = new_pos;
}

Tile* g_grid = NULL;

void init_map(void) {
	/* Terrain generation on tile types.
	 * We use two grids so that each generation step can read from one grid and write to the next,
	 * so that there is no problems araising from reading what we are modifying (which can introduce
	 * asymmetry due to the order in which we process each tile). */
	TileType* tt_grid_src = malloc(N_TILES * sizeof(TileType));
	TileType* tt_grid_dst = malloc(N_TILES * sizeof(TileType));
	for(int y = 0; y < g_map_h; ++y) for(int x = 0; x < g_map_w; ++x) {
		TileType* tt = &tt_grid_src[y * g_map_w + x];
		*tt = rand() % TILE_TYPE_NUM;

		/* Put some water at the edges. */
		#define MARGIN 2
		if (x < MARGIN || x >= g_map_w-MARGIN || y < MARGIN || y >= g_map_h-MARGIN) {
			*tt = TILE_RIVER;
		}
	}

	for (int gen_i = 0; gen_i < 6; gen_i++) {
		for(int y = 0; y < g_map_h; ++y) for(int x = 0; x < g_map_w; ++x) {
			TileType tt_src = tt_grid_src[y * g_map_w + x];
			TileType* tt_dst = &tt_grid_dst[y * g_map_w + x];
			*tt_dst = tt_src;

			/* Count neighbors, both in the 3x3 square around the tile (`neighbors_8`)
			 * and in the '+' shape of tiles adjacent to the tile (`neighbors_4`). */
			typedef struct Dir {int dx, dy;} Dir;
			#define DIR_ADJACENT(dir_) (abs(dir_.dx) + abs(dir_.dy) == 1)
			Dir dirs[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
			int neighbors_8[TILE_TYPE_NUM] = {0};
			int neighbors_4[TILE_TYPE_NUM] = {0};
			for (int dir_i = 0; dir_i < 8; dir_i++) {
				int neighbor_x = x + dirs[dir_i].dx;
				int neighbor_y = y + dirs[dir_i].dy;
				if (0 <= neighbor_x && neighbor_x < g_map_w && 0 <= neighbor_y && neighbor_y < g_map_h) {
					TileType neighbor_tt = tt_grid_src[neighbor_y * g_map_w + neighbor_x];
					neighbors_8[neighbor_tt]++;
					if (DIR_ADJACENT(dirs[dir_i])) {
						neighbors_4[neighbor_tt]++;
					}
				}
			}

			/* Modify the terrain. */
			switch (tt_src) {
				case TILE_FOREST:
					if (neighbors_8[TILE_FOREST] <= 1 || neighbors_8[TILE_RIVER] <= 1) {
						*tt_dst = TILE_PLAIN;
					}
					if (neighbors_8[TILE_FOREST] >= 4 && neighbors_8[TILE_RIVER] <= 3) {
						*tt_dst = TILE_PLAIN;
					}
				break;
				case TILE_MOUTAIN:
					if (neighbors_8[TILE_MOUTAIN] <= 1) {
						*tt_dst = TILE_PLAIN;
					}
				break;
				case TILE_RIVER:
					if (neighbors_8[TILE_RIVER] <= 1 || neighbors_8[TILE_RIVER] >= 6) {
						*tt_dst = TILE_PLAIN;
					}
				break;
				case TILE_PLAIN:
					if (neighbors_8[TILE_RIVER] >= 2) {
						*tt_dst = TILE_FOREST;
					}
					if (neighbors_8[TILE_MOUTAIN] >= 7) {
						*tt_dst = TILE_MOUTAIN;
					}
					if (neighbors_8[TILE_PLAIN] >= 8) {
						*tt_dst = (rand() % 4) ? TILE_FOREST : TILE_RIVER;
					}
				break;
			}
		}

		/* Swap src and dst for next round. */
		TileType* tmp = tt_grid_src;
		tt_grid_src = tt_grid_dst;
		tt_grid_dst = tmp;
	}

	g_grid = malloc(N_TILES * sizeof(Tile));
	for(int y = 0; y < g_map_h; ++y) for(int x = 0; x < g_map_w; ++x) {
		TileCoords tc = {x, y};
		Tile* tile = get_tile(tc);
		*tile = (Tile){
			.type = tt_grid_src[y * g_map_w + x],
			.entities = NULL,
			.entity_count = 0,
		};
	}

	free(tt_grid_src);
	free(tt_grid_dst);
}

Tile* get_tile(TileCoords coords) {
	assert(tile_coords_are_valid(coords));
	return &g_grid[coords.y * g_map_w + coords.x];
}
