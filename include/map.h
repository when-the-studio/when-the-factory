#ifndef WHEN_THE_FACTORY_MAP_
#define WHEN_THE_FACTORY_MAP_


#define TILE_SIZE 100

//#include "renderer.h"

// #define N_TILES_H (WINDOW_H / TILE_SIZE)
// #define N_TILES_W (WINDOW_W / TILE_SIZE)

#define N_TILES_H 50
#define N_TILES_W 50

#define N_TILES N_TILES_H * N_TILES_W
/* Type of terrain for each entity on the grid */
enum tile_type_t {
	TILE_PLAIN,
	TILE_MOUTAIN,
	TILE_RIVER,
	TILE_FOREST,

	TILE_TYPE_NUM,
};
typedef enum tile_type_t tile_type_t;

/* Coords of something on the map */
struct coord_t {
	int x, y;
};
typedef struct coord_t coord_t;

/* Entity types possible */
enum entity_type_t {
	PLAYER,
	ENEMY,
	BUILDING,

	N_ENTITY_TYPE,
};
typedef enum entity_type_t entity_type_t;

/* An actual entity on the grid */
struct entity{
	entity_type_t type;
};
typedef struct entity entity;

/* The representation of a map tile */
struct tile_t {
	tile_type_t type;
	coord_t pos;
	entity* entities_on_tile;
};
typedef struct tile_t tile_t;

/* Global variable : Map grid */
extern tile_t* grid;

/* Initilises the grid with random tiles */
void init_map(void);

#endif // WHEN_THE_FACTORY_MAP_