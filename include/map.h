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

/* Building types. */
enum BuildingType {
	BUILDING_EMITTER,
	BUILDING_RECEIVER,

	BUILDING_TYPE_NUM
};
typedef enum BuildingType BuildingType;

/* Building textures types. */
enum BuildingTextureType {
	BUILDING_TX_EMITTER,
	BUILDING_TX_RECEIVER_ON,
	BUILDING_TX_RECEIVER_OFF,

	BUILDING_TX_NUM
};
typedef enum BuildingTextureType BuildingTextureType;

/* An actual building on the grid */
struct Building {
	BuildingType type;
	bool powered;
	TileCoords pos;
	FactionIdent faction;
};
typedef struct Building Building;

/* Describes a type of tile. */
struct BuildingTypeSpec {
	SDL_Rect rect_in_spritesheet;
	char const* name;
};
typedef struct BuildingTypeSpec BuildingTypeSpec;

extern BuildingTypeSpec g_building_type_spec_table[BUILDING_TX_NUM];

enum CardinalType {
	NORTH,
	SOUTH,
	EAST,
	WEST,

	CARDINAL_TYPE_NUM
};
typedef enum CardinalType CardinalType;

enum FlowType {
	ELECTRICITY,
	CONVEYER,
	FLUID,

	FLOW_TYPE_NUM
};
typedef enum FlowType FlowType;

enum FlowTextureType {
	ELECTRICITY_STRAIGHT,
	ELECTRICITY_TURN,

	FLOW_TX_NUM
};
typedef enum FlowType FlowType;

/* Describes a type of a flow. */
struct FlowTypeSpec {
	SDL_Rect rect_in_spritesheet;
	char const* name;
};
typedef struct FlowTypeSpec FlowTypeSpec;

extern FlowTypeSpec g_flow_type_spec_table[FLOW_TX_NUM];

/* An actual flow */
struct Flow {
	FlowType type;
	int capacity;
	TileCoords pos;
	CardinalType connections[2];
	bool powered;
};
typedef struct Flow Flow;

Entity* new_entity(EntityType type, TileCoords pos);
void entity_delete(Entity* entity);
void entity_move(Entity* entity, TileCoords new_pos);

Building* new_building(BuildingType type, TileCoords pos);

Flow* new_flow(FlowType type, TileCoords pos, CardinalType entry, CardinalType exit);

/* The representation of a map tile. */
struct Tile {
	TileType type;
	Entity** entities;
	Building* building;
	Flow** flows;
	int entity_count;
	int flow_count;
};
typedef struct Tile Tile;

/* This is the map, which is a grid of tiles. */
extern Tile* g_grid;

/* Initilises the grid with random tiles. */
void init_map(void);

Tile* get_tile(TileCoords coords);

#endif // WHEN_THE_FACTORY_MAP_
