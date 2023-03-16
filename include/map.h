#ifndef WHEN_THE_FACTORY_MAP_
#define WHEN_THE_FACTORY_MAP_
#include <SDL2/SDL.h>

#define TILE_SIZE 100

//#include "renderer.h"

// #define N_TILES_H (WINDOW_H / TILE_SIZE)
// #define N_TILES_W (WINDOW_W / TILE_SIZE)

#define N_TILES_H 50
#define N_TILES_W 50

#define N_TILES N_TILES_H * N_TILES_W
/* Type of terrain for each entity on the g_grid */
enum TileType {
	TILE_PLAIN,
	TILE_MOUTAIN,
	TILE_RIVER,
	TILE_FOREST,

	TILE_TYPE_NUM,
};
typedef enum TileType TileType;

/* Coords of something on the map */
typedef SDL_Point Coord;

/* Entity types possible */
enum EntityType {
	ENTITY_HUMAIN,
	ENTITY_BUILDING,

	N_ENTITY_TYPE,
};
typedef enum EntityType EntityType;

enum FactionIdent {
	FACTION_YELLOW,
	FACTION_RED,

	FACTION_IDENT_NUM
};
typedef enum FactionIdent FactionIdent;

/* An actual entity on the grid */
struct Entity {
	EntityType type;
	Coord pos;
	/* DISCUSS (about the `pos`):
	 * We may not need that here if we do end up storing the entities directly in the tiles, 
	 * but we might also end up with the entities in some global table. */
	FactionIdent faction;
};
typedef struct Entity Entity;

/* The representation of a map tile */
struct Tile {
	TileType type;
	Coord pos;
	Entity** entities;
	int entity_count;
};
typedef struct Tile Tile;

/* Global variable : Map grid */
extern Tile* g_grid;

/* Initilises the grid with random tiles */
void init_map(void);

Entity* new_entity(EntityType type, Coord pos);
void entity_delete(Entity* entity);
void entity_move(Entity* entity, Coord new_pos);

#endif // WHEN_THE_FACTORY_MAP_