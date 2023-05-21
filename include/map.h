#ifndef WHEN_THE_FACTORY_MAP_
#define WHEN_THE_FACTORY_MAP_

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "utils.h"
#include "flow.h"
#include "building.h"



extern int g_map_w;
extern int g_map_h;
#define N_TILES (g_map_w * g_map_h)

/* Type of terrain for each tile of the map. */
enum TileType {
	TILE_PLAIN,
	TILE_MOUTAIN,
	TILE_RIVER,
	TILE_FOREST,

	TILE_TYPE_NUM,
};
typedef enum TileType TileType;

/* Describes a type of tile. */
struct TileTypeSpec {
	SDL_Rect rect_in_spritesheet;
	char const* name;
};
typedef struct TileTypeSpec TileTypeSpec;

extern TileTypeSpec g_tile_type_spec_table[TILE_TYPE_NUM];

bool tile_coords_are_valid(TileCoords coords);
bool tile_coords_eq(TileCoords a, TileCoords b);

typedef struct EntId EntId;

/* The representation of a map tile. */
struct Tile {
	TileType type;
	DA(EntId) ents;
	Building* building;
	Cable** cables;
	int cable_count;
};
typedef struct Tile Tile;

/* This is the map, which is a grid of tiles. */
extern Tile* g_grid;


Cable* new_cable(TileCoords pos, CardinalType entry, CardinalType exit);

/* Initilises the grid with random tiles. */
void init_map(void);

Tile* get_tile(TileCoords coords);
bool tile_is_walkable(Tile const* tile);

#endif // WHEN_THE_FACTORY_MAP_
