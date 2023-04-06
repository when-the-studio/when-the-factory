#ifndef WHEN_THE_FACTORY_FLOW_
#define WHEN_THE_FACTORY_FLOW_

#include "utils.h"

enum CardinalType {
	NORTH,
	EAST,
	SOUTH,
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

Flow* new_flow(FlowType type, TileCoords pos, CardinalType entry, CardinalType exit);

#endif