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
	/* Terrain generation on tile types.
	 * We use two grids so that each generation step can read from one grid and write to the next,
	 * so that there is no problems araising from reading what we are modifying (which can introduce
	 * asymmetry due to the order in which we process each tile). */
	TileType* tt_grid_src = malloc(N_TILES * sizeof(TileType));
	TileType* tt_grid_dst = malloc(N_TILES * sizeof(TileType));
	for(int y = 0; y < N_TILES_H; ++y) for(int x = 0; x < N_TILES_W; ++x) {
		TileType* tt = &tt_grid_src[y * N_TILES_W + x];
		*tt = rand() % TILE_TYPE_NUM;

		/* Put some water at the edges. */
		#define MARGIN 2
		if (x < MARGIN || x >= N_TILES_W-MARGIN || y < MARGIN || y >= N_TILES_H-MARGIN) {
			*tt = TILE_RIVER;
		}
	}

	for (int gen_i = 0; gen_i < 6; gen_i++) {
		for(int y = 0; y < N_TILES_H; ++y) for(int x = 0; x < N_TILES_W; ++x) {
			TileType tt_src = tt_grid_src[y * N_TILES_W + x];
			TileType* tt_dst = &tt_grid_dst[y * N_TILES_W + x];
			*tt_dst = tt_src;

			/* Count neighbors. */
			typedef struct Dir {int dx, dy;} Dir;
			Dir dirs[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
			int neighbors_forest = 0;
			int neighbors_mountain = 0;
			int neighbors_river = 0;
			int neighbors_plain = 0;
			for (int dir_i = 0; dir_i < 8; dir_i++) {
				int neighbor_x = x + dirs[dir_i].dx;
				int neighbor_y = y + dirs[dir_i].dy;
				if (0 <= neighbor_x && neighbor_x < N_TILES_W && 0 <= neighbor_y && neighbor_y < N_TILES_H) {
					TileType neighbor = tt_grid_src[neighbor_y * N_TILES_W + neighbor_x];
					switch (neighbor) {
						case TILE_FOREST:  neighbors_forest++;   break;
						case TILE_MOUTAIN: neighbors_mountain++; break;
						case TILE_RIVER:   neighbors_river++;    break;
						case TILE_PLAIN:   neighbors_plain++;    break;
					}
				}
			}

			/* Modify the terrain. */
			switch (tt_src) {
				case TILE_FOREST:
					if (neighbors_forest <= 1 || neighbors_river <= 1) {
						*tt_dst = TILE_PLAIN;
					}
					if (neighbors_forest >= 4 && neighbors_river <= 3) {
						*tt_dst = TILE_PLAIN;
					}
				break;
				case TILE_MOUTAIN:
					if (neighbors_mountain <= 1) {
						*tt_dst = TILE_PLAIN;
					}
				break;
				case TILE_RIVER:
					if (neighbors_river <= 1 || neighbors_river >= 6) {
						*tt_dst = TILE_PLAIN;
					}
				break;
				case TILE_PLAIN:
					if (neighbors_river >= 2) {
						*tt_dst = TILE_FOREST;
					}
					if (neighbors_mountain >= 7) {
						*tt_dst = TILE_MOUTAIN;
					}
					if (neighbors_plain >= 8) {
						*tt_dst =(rand() % 4) ? TILE_FOREST : TILE_RIVER;
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
	for(int y = 0; y < N_TILES_H; ++y) for(int x = 0; x < N_TILES_W; ++x) {
		TileCoords tc = {x, y};
		Tile* tile = get_tile(tc);
		*tile = (Tile){
			.type = tt_grid_src[y * N_TILES_W + x],
			.entities = NULL,
			.entity_count = 0,
		};
	}

	free(tt_grid_src);
	free(tt_grid_dst);
}

Tile* get_tile(TileCoords coords) {
	assert(tile_coords_are_valid(coords));
	return &g_grid[coords.y * N_TILES_W + coords.x];
}
