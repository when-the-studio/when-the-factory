#ifndef WHEN_THE_FACTORY_BLUIDING_
#define WHEN_THE_FACTORY_BLUIDING_

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "utils.h"

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
	BUILDING_TX_RECEIVER_OFF,
	BUILDING_TX_RECEIVER_ON,

	BUILDING_TX_NUM
};
typedef enum BuildingTextureType BuildingTextureType;

/* An actual building on the grid */
struct Building {
	BuildingType type;
	bool powered;
	TileCoords pos;
};
typedef struct Building Building;

typedef DA(Building*) DA_Building_ptr;

/* Describes a type of tile. */
struct BuildingTypeSpec {
	SDL_Rect rect_in_spritesheet;
	char const* name;
};
typedef struct BuildingTypeSpec BuildingTypeSpec;

extern BuildingTypeSpec g_building_type_spec_table[BUILDING_TX_NUM];

Building* new_building(BuildingType type, TileCoords pos);

#endif
