#include <stdlib.h>
#include <assert.h>

#include "map.h"
#include "entity.h"
#include "flow.h"
#include "utils.h"


CableTypeSpec g_cable_type_spec_table[CABLE_TX_NUM] = {
	[ELECTRICITY_STRAIGHT] = {
		.rect_in_spritesheet = {32, 0, 8, 8},
		.name = "Electric cable (straight)",
	},
	[ELECTRICITY_TURN] = {
		.rect_in_spritesheet = {40, 0, 8, 8},
		.name = "Electric cable (turn)",
	},
	[ELECTRICITY_STRAIGHT_ON] = {
		.rect_in_spritesheet = {32, 8, 8, 8},
		.name = "Electric cable (straight)",
	},
	[ELECTRICITY_TURN_ON] = {
		.rect_in_spritesheet = {40, 8, 8, 8},
		.name = "Electric cable (turn)",
	},
};


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
	[TILE_WATER] = {
		.rect_in_spritesheet = {8, 0, 8, 8},
		.name = "Water",
	},
	[TILE_FOREST] = {
		.rect_in_spritesheet = {24, 0, 8, 8},
		.name = "Forest",
	},
};

BuildingTypeSpec g_building_type_spec_table[BUILDING_TX_NUM] = {
	[BUILDING_TX_EMITTER] = {
		.rect_in_spritesheet = {0, 0, 8, 8},
		.name = "Power Plant",
	},
	[BUILDING_TX_RECEIVER_OFF] = {
		.rect_in_spritesheet = {8, 0, 8, 8},
		.name = "Shield off",
	},
	[BUILDING_TX_RECEIVER_ON] = {
		.rect_in_spritesheet = {16, 0, 8, 8},
		.name = "Shield on",
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

void add_eid_to_tile_list(EntId eid, Tile* tile) {
	for (int i = 0; i < tile->ents.len; i++) {
		if (eid_null(tile->ents.arr[i])) {
			tile->ents.arr[i] = eid;
			return;
		}
	}
	DA_PUSH(&tile->ents, eid);
}

void remove_eid_from_tile_list(EntId eid, Tile* tile) {
	for (int i = 0; i < tile->ents.len; i++) {
		if (eid_eq(tile->ents.arr[i], eid)) {
			tile->ents.arr[i] = EID_NULL;
			break;
		}
	}
	/* Remove trailing `EID_NULL`s. */
	while (eid_null(tile->ents.arr[tile->ents.len-1])) {
		tile->ents.len--;
	}
	//tile->ents.arr = realloc(tile->ents.arr, tile->ents.len * sizeof(Ent*));
}

static void remove_building_from_tile(Tile* tile) {
	free(tile->building);
	tile->building = NULL;
}

static void add_cable_to_tile_list(Cable* cable, Tile* tile) {
	tile->cable_count++;
	tile->cables = realloc(tile->cables, tile->cable_count * sizeof(Cable*));
	tile->cables[tile->cable_count-1] = cable;
}

static void remove_cable_from_tile_list(Cable* cable, Tile* tile) {
	int cable_index_in_tile = -1;
	for (int i = 0; i < tile->cable_count; i++) {
		if (tile->cables[i] == cable) {
			cable_index_in_tile = i;
			break;
		}
	}
	assert(cable_index_in_tile != -1);
	for (int i = cable_index_in_tile; i < tile->cable_count - 1; i++) {
		tile->cables[i] = tile->cables[i+1];
	}
	tile->cable_count--;
	tile->cables = realloc(tile->cables, tile->cable_count * sizeof(Cable*));
}

Cable* new_cable(TileCoords pos, CardinalType entry, CardinalType exit) {
	Cable* cable = malloc(sizeof(Cable));
	*cable = (Cable){
		.pos = pos,
		.connections = {entry, exit},
		.capacity = 10,
		.powered = false,
	};
	if (entry > exit){
		cable->connections[0] = exit;
		cable->connections[1] = entry;
	}
	Tile* tile = get_tile(pos);
	add_cable_to_tile_list(cable, tile);

	return cable;
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
			*tt = TILE_WATER;
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
					if (neighbors_8[TILE_FOREST] <= 1 || neighbors_8[TILE_WATER] <= 1) {
						*tt_dst = TILE_PLAIN;
					}
					if (neighbors_8[TILE_FOREST] >= 4 && neighbors_8[TILE_WATER] <= 3) {
						*tt_dst = TILE_PLAIN;
					}
				break;
				case TILE_MOUTAIN:
					if (neighbors_8[TILE_MOUTAIN] <= 1) {
						*tt_dst = TILE_PLAIN;
					}
				break;
				case TILE_WATER:
					if (neighbors_8[TILE_WATER] <= 1 || neighbors_8[TILE_WATER] >= 6) {
						*tt_dst = TILE_PLAIN;
					}
				break;
				case TILE_PLAIN:
					if (neighbors_8[TILE_WATER] >= 2) {
						*tt_dst = TILE_FOREST;
					}
					if (neighbors_8[TILE_MOUTAIN] >= 7) {
						*tt_dst = TILE_MOUTAIN;
					}
					if (neighbors_8[TILE_PLAIN] >= 8) {
						*tt_dst = (rand() % 4) ? TILE_FOREST : TILE_WATER;
					}
				break;
				default: break;
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
			.ents = {0},
			.building = NULL,
			.cables = NULL,
			.cable_count = 0,
		};
	}

	free(tt_grid_src);
	free(tt_grid_dst);
}

Tile* get_tile(TileCoords coords) {
	assert(tile_coords_are_valid(coords));
	return &g_grid[coords.y * g_map_w + coords.x];
}

bool tile_is_walkable(Tile const* tile) {
	if (tile->type == TILE_MOUTAIN || tile->type == TILE_WATER) {
		return false;
	}
	if (tile->building != NULL) {
		return false;
	}
	return true;
}
