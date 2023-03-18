#ifndef WHEN_THE_FACTORY_MAP_
#define WHEN_THE_FACTORY_MAP_

#include <stdbool.h>

#include <SDL2/SDL.h>

#define TILE_SIZE 100

#define N_TILES_H 50
#define N_TILES_W 50
#define N_TILES (N_TILES_H * N_TILES_W)

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

/* Coords of a tile on the map. */
struct TileCoords {
	int x, y;
};
typedef struct TileCoords TileCoords;

bool tile_coords_are_valid(TileCoords coords);
bool tile_coords_eq(TileCoords a, TileCoords b);

/* Entity types. */
enum EntityType {
	ENTITY_HUMAIN,
	ENTITY_BUILDING,

	ENTITY_TYPE_NUM,
};
typedef enum EntityType EntityType;

/* Entity, something that is not a tile terrain but rather ON a tile. */
enum FactionIdent {
	FACTION_YELLOW,
	FACTION_RED,

	FACTION_IDENT_NUM
};
typedef enum FactionIdent FactionIdent;

/* An actual entity on the grid */
struct Entity {
	EntityType type;
	TileCoords pos;
	FactionIdent faction;
};
typedef struct Entity Entity;

Entity* new_entity(EntityType type, TileCoords pos);
void entity_delete(Entity* entity);
void entity_move(Entity* entity, TileCoords new_pos);

/* The representation of a map tile. */
struct Tile {
	TileType type;
	Entity** entities;
	int entity_count;
};
typedef struct Tile Tile;

/* This is the map, which is a grid of tiles. */
extern Tile* g_grid;

/* Initilises the grid with random tiles. */
void init_map(void);

Tile* get_tile(TileCoords coords);

#endif // WHEN_THE_FACTORY_MAP_
