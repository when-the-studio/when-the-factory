#include "flow.h"
#include "map.h"

DxDy cardinal_to_dxdy(CardinalType card) {
	switch (card) {
		case SOUTH: return (DxDy){0, +1};
		case NORTH: return (DxDy){0, -1};
		case WEST:  return (DxDy){+1, 0};
		case EAST:  return (DxDy){-1, 0};
		default: assert(false); exit(EXIT_FAILURE);
	}
}

CardinalType get_opposed_direction(CardinalType direction){
	/* We could also do
		return (CardinalType)((direction+2)%4)
		(performance ?)
	 */
	switch (direction) {
		case NORTH: return SOUTH;
		case SOUTH: return NORTH;
		case WEST:	return EAST;
		case EAST:	return WEST;
		default:
			assert(false);
			exit(EXIT_FAILURE);
	}
}

TileCoords get_offset_from_cardinal(CardinalType direction){
	TileCoords tc ={ 0,0 };
	switch (direction) {
		case NORTH: tc.y = -1; break;
		case SOUTH: tc.y = 1;  break;
		case EAST:  tc.x = 1;  break;
		case WEST:  tc.x = -1; break;
		default:
			assert(false);
			exit(EXIT_FAILURE);
	}
	return tc;
}


bool is_building_in_da(Building* building, DA(Building*)* buildings){
	for (int i=0; i<buildings->len; i++){
		if (building == buildings->arr[i]){
			return true;
		}
	}
	return false;
}

bool is_cable_in_da(Cable* cable, DA(Cable*)* cables){
	for (int i=0; i<cables->len; i++){
		if (cable == cables->arr[i]){
			return true;
		}
	}
	return false;
}

/**
 * Return an array with the 4 adjacent tiles.
 * The order of the tiles is :
 * WEST, SOUTH, EAST, NORTH
*/
Tile** get_adjacent_tiles(TileCoords tc){
	assert(tile_coords_are_valid(tc));
	Tile** tiles = malloc(4 * sizeof *tiles);
	TileCoords tc_neigh;
	Tile* neigh;
	for (int i=0; i<4; i++){
		tc_neigh = get_offset_from_cardinal(WEST+i);
		tc_neigh.x += tc.x;
		tc_neigh.y += tc.y;
		neigh = get_tile(tc_neigh);
		tiles[i] = neigh;
	}
	return tiles;
}

bool is_cable_facing_pos(Cable* c, TileCoords pos) {
	for (int i=0; i<2; i++) {
		TileCoords offset = get_offset_from_cardinal(c->connections[i]);
		TileCoords c_plus_offset = {c->pos.x + offset.x, c->pos.y + offset.y};
		if (tile_coords_eq(c_plus_offset, pos)) return true;
	}
	return false;
}

bool are_cables_facing(Cable* c1, Cable* c2) {
	return is_cable_facing_pos(c1, c2->pos) && is_cable_facing_pos(c2, c1->pos);
}

/**
 * Return the tiles that are adjacent to the ends of the cable.
 * @param cable
*/
Tile** get_tiles_from_cable(Cable* cable){
	TileCoords tc = cable->pos;
	Tile* neigh;
	Tile** tiles = malloc(2 * sizeof(Tile*));
	TileCoords tc_neigh;
	for (int i=0; i<2; i++){
		tc_neigh = get_offset_from_cardinal(cable->connections[i]);
		tc_neigh.x += tc.x;
		tc_neigh.y += tc.y;
		neigh = get_tile(tc_neigh);
		tiles[i] = neigh;
	}
	return tiles;
}

/**
 * Update the power state of a dynamic array of buildings.
 * @param buildings
*/
void update_buildings_da(DA(Building*)* buildings){
	for (int build_i=0; build_i < buildings->len; build_i++){
		bool build_powered = false;
		Tile** build_tiles = get_adjacent_tiles(buildings->arr[build_i]->pos);
		for (int tile_adj_i=0; tile_adj_i<4; tile_adj_i++){
			Tile* build_tile = build_tiles[tile_adj_i];
			for (int cable_adj_i=0; cable_adj_i<build_tile->cable_count; cable_adj_i++){
				Cable* cable_adj = build_tile->cables[cable_adj_i];
				bool cable_powered = cable_adj->powered;
				bool facing = false;
				for (int cable_card_i=0; cable_card_i<2; cable_card_i++){
					if (abs((int)cable_adj->connections[cable_card_i]-(int)tile_adj_i)==2){
						facing = true;
					}
				}
				if (cable_powered && facing){
					build_powered = true;
				}
			}
		}
		buildings->arr[build_i]->powered = build_powered;
		free(build_tiles);
	}
}

/**
 * Update all the networks of cables connected to a tile.
 * @param tc Coordinates of the tile to update
*/
void update_cable_network(TileCoords tc){
	Tile* tile = get_tile(tc);
	/*
	Each individual cable of the root tile causes an update.
	This is mainly to allow an intersection of two networks to be updated correctly.
	*/
	for (int i=0; i<tile->cable_count; i++){

		/* Array of buildings that will be updated at the end */
		DA(Building*) buildings = {0};

		/* Represent the whole network that have already been processed completely */
		DA(Cable*) network = {0};

		/* The cables scanned during the last iteration */
		DA(Cable*) scanned = {0};

		/* Temporary array used during an iteration */
		DA(Cable*) new_cables = {0};

		Tile** tiles;
		bool power = false;
		bool scan_network = true;

		/* Root cable */
		DA_PUSH(&scanned, tile->cables[i]);

		while (scan_network){
			scan_network = false;
			/* The neighboring tiles of each cable in 'scanned' are examined */
			for (int scan_i=0; scan_i < scanned.len; scan_i++){
				tiles = get_tiles_from_cable(scanned.arr[scan_i]);
				for (int i=0; i<2; i++){
					Tile* t = tiles[i];
					for (int j=0; j<t->cable_count; j++){
						if (!is_cable_in_da(t->cables[j], &new_cables) && are_cables_facing(scanned.arr[scan_i], t->cables[j])){
							DA_PUSH(&new_cables, t->cables[j]);
						}
					}
					if ((tiles)[i]->building != NULL && !is_building_in_da((tiles)[i]->building, &buildings)){
						DA_PUSH(&buildings, (tiles)[i]->building);
					}
				}
				DA_PUSH(&network, scanned.arr[scan_i]);
				free(tiles);
			}

			scanned.len = 0;

			/* If a new cable is present, it's added to scanned and a new iteration is planned */
			for (int cable_i=0; cable_i<new_cables.len; cable_i++){
				if (!is_cable_in_da(new_cables.arr[cable_i], &scanned) && !is_cable_in_da(new_cables.arr[cable_i], &network)){
					DA_PUSH(&scanned, new_cables.arr[cable_i]);
					scan_network = true;
				}
			}
		}

		/* Check if a power source is present in the network */
		for (int build_i=0; build_i < buildings.len; build_i++){
			if (buildings.arr[build_i]->type == BUILDING_EMITTER){
				power = true;
			}
		}
		/* Update the cables */
		for (int net_i=0; net_i<network.len; net_i++){
			network.arr[net_i]->powered = power;
		}

		/* Update buildings, regardless of the state of this particular network.
			This allows multiple networks to be connected to the same building, each from a different side.
		*/
		update_buildings_da(&buildings);

		free(buildings.arr);
		free(new_cables.arr);
		free(scanned.arr);
		free(network.arr);
	}
}

void update_surroundings(TileCoords tc){
	TileCoords tc_neigh;
	for (int i=0; i<4; i++){
		tc_neigh = get_offset_from_cardinal(WEST+i);
		tc_neigh.x += tc.x;
		tc_neigh.y += tc.y;
		update_cable_network(tc_neigh);
	}
}
