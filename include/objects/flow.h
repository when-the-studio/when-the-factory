#ifndef WHEN_THE_FACTORY_FLOW_
#define WHEN_THE_FACTORY_FLOW_

#include "utils.h"
#include <stdbool.h>
#include <SDL2/SDL.h>

enum CardinalType {
	WEST,
	SOUTH,
	EAST,
	NORTH,

	CARDINAL_TYPE_NUM
};
typedef enum CardinalType CardinalType;

enum FlowType {
	ELECTRIC_CABLE,
	FLUID,
	CONVEYOR,

	FLOW_TYPE_NUM
};
typedef enum FlowType FlowType;

enum CableTexture {
	ELECTRICITY_STRAIGHT,
	ELECTRICITY_TURN,
	ELECTRICITY_STRAIGHT_ON,
	ELECTRICITY_TURN_ON,

	CABLE_TX_NUM
};
typedef enum CableTexture CableTexture;

/* Describes a type of a flow. */
struct CableTypeSpec {
	SDL_Rect rect_in_spritesheet;
	char const* name;
};
typedef struct CableTypeSpec CableTypeSpec;

extern CableTypeSpec g_cable_type_spec_table[CABLE_TX_NUM];

/* An actual flow */
struct Cable {
	TileCoords pos;
	CardinalType connections[2];
	int maxFlow;
	int capacity;	
	bool powered;
};
typedef struct Cable Cable;

struct Fluid {
	TileCoords pos;
	CardinalType connections[2];
	int maxFlow;
	int capacity;
};
typedef struct Fluid Fluid;

struct Conveyor {
	TileCoords pos;
	CardinalType input;
	CardinalType output;
	int maxFlow;
	int capacity;
};
typedef struct Conveyor Conveyor;

CardinalType getOpposedDirection(CardinalType direction);
void update_cable_network(TileCoords tc);
void update_surroundings(TileCoords tc);

#endif