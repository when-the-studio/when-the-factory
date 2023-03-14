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
typedef SDL_Point coord_t;

/* Entity types possible */
enum EntityType {
	PLAYER,
	ENEMY,
	BUILDING,

	N_ENTITY_TYPE,
};
typedef enum EntityType EntityType;

/* An actual entity on the grid */
struct entity{
	EntityType type;
};
typedef struct Entity Entity;

/* The representation of a map tile */
struct Tile {
	TileType type;
	coord_t pos;
	Entity* entities_on_tile;
};
typedef struct Tile Tile;

/* Global variable : Map grid */
extern Tile* g_grid;

/* Initilises the grid with random tiles */
void init_map(void);

#endif // WHEN_THE_FACTORY_MAP_