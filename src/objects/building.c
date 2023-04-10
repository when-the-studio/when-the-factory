#include "building.h"
#include "map.h"

static void add_building_to_tile(Building* building, Tile* tile) {
	tile->building = building;
}

Building* new_building(BuildingType type, TileCoords pos) {
	Building* building = malloc(sizeof(Building));
	*building = (Building){
		.type = type,
		.pos = pos,
		.powered = false,
	};
	Tile* tile = get_tile(pos);
	add_building_to_tile(building, tile);
	
	return building;
}
