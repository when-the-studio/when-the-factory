#include "flow.h"
#include "map.h"

CardinalType get_opposed_direction(CardinalType direction){
	/* We could also do 
		return (CardinalType)((direction+2)%4)
		(performance ?)
	 */
	switch (direction){
		case NORTH: return SOUTH;
		case SOUTH: return NORTH;
		case WEST:	return EAST;
		case EAST:	return WEST;
		default:		return NORTH;
	}
}

TileCoords get_offset_from_cardinal(CardinalType direction){
	TileCoords tc = {0,0};
	switch (direction){
		case NORTH:
			tc.y = -1;
		break;
		case SOUTH:
			tc.y = 1;
		break;
		case EAST:
			tc.x = 1;
		break;
		case WEST:
			tc.x = -1;
		break;
		default:
		break;
	}
	return tc;
}

bool is_tile_in(Tile * tile, Tile ** tiles, int size){
	for (int i=0; i<size; i++){
		if (tile == tiles[i]){
			return true;
		}
	}
	return false;
}

bool is_building_in(Building * building, Building ** buildings, int size){
	for (int i=0; i<size; i++){
		if (building == buildings[i]){
			return true;
		}
	}
	return false;
}

bool is_cable_in(Cable * cable, Cable ** cables, int size){
	for (int i=0; i<size; i++){
		if (cable == cables[i]){
			return true;
		}
	}
	return false;
}

bool is_building_in_da(Building * building, DA(Building*) * buildings){
	for (int i=0; i<buildings->len; i++){
		if (building == buildings->arr[i]){
			return true;
		}
	}
	return false;
}

bool is_cable_in_da(Cable * cable, DA(Cable*) * cables){
	for (int i=0; i<cables->len; i++){
		if (cable == cables->arr[i]){
			return true;
		}
	}
	return false;
}

Tile ** get_adjacent_tiles(TileCoords tc){
	Tile ** tiles = malloc(4*sizeof(*tiles));
	TileCoords tc_neigh;
	Tile * neigh;
	for (int i=0; i<4; i++){
		tc_neigh = get_offset_from_cardinal(WEST+i);
		tc_neigh.x += tc.x;
		tc_neigh.y += tc.y;
		neigh = get_tile(tc_neigh);
		tiles[i] = neigh;
	}
	return tiles;
}

bool areCablesFacing(Cable * c1, Cable * c2){
	for (int i=0; i<2; i++){
		for (int j=0; j<2; j++){
			if (abs(c1->connections[i]-c2->connections[j])==2){
				return true;
			}
		}
	}
	return false;
}

/**
 * Return the tiles that are adjacent to the ends of the cable.
 * @param cable
*/
Tile ** get_tiles_from_cable(Cable * cable){
	TileCoords tc = cable->pos;
	Tile * neigh;
	Tile ** tiles = malloc(2 * sizeof(Tile));
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

void update_cable_network(TileCoords tc){
	Tile * tile = get_tile(tc);
	for (int i=0; i<tile->cable_count; i++){
		DA(Building*) buildings = {0};
		DA(Cable*) network = {0};
		DA(Cable*) scanned = {0};
		DA(Cable*) new_cables = {0};
		
		Tile ** tiles;
		Tile * t;
		bool power = false;
		bool scan_network = true;

		
		DA_PUSH(&scanned, tile->cables[i]);
		while (scan_network){
			scan_network = false;
			for (int scan_i=0; scan_i < scanned.len; scan_i++){
				tiles = get_tiles_from_cable(scanned.arr[scan_i]);
				for (int i=0; i<2; i++){
					Tile * t = tiles[i];
					for (int j=0; j<t->cable_count; j++){
						if (!is_cable_in_da(t->cables[j], &new_cables) && areCablesFacing(scanned.arr[scan_i], t->cables[j])){
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
			for (int cable_i=0; cable_i<new_cables.len; cable_i++){
				if (!is_cable_in_da(new_cables.arr[cable_i], &scanned) && !is_cable_in_da(new_cables.arr[cable_i], &network)){
					DA_PUSH(&scanned, new_cables.arr[cable_i]);
					scan_network = true;
				}
			}
		}

		for (int build_i=0; build_i < buildings.len; build_i++){
			if (buildings.arr[build_i]->type == BUILDING_EMITTER){
				power = true;
			}
		}

		for (int net_i=0; net_i<network.len; net_i++){
			network.arr[net_i]->powered = power;
		}

		for (int build_i=0; build_i < buildings.len; build_i++){
			// TODO implement direction of cable
			bool build_powered = false;
			Tile ** build_tiles = get_adjacent_tiles(buildings.arr[build_i]->pos);
			for (int tile_adj_i=0; tile_adj_i<4; tile_adj_i++){
				for (int cable_adj_i=0; cable_adj_i<build_tiles[tile_adj_i]->cable_count; cable_adj_i++){
					if (build_tiles[tile_adj_i]->cables[cable_adj_i]->powered){
						build_powered = true;
					}
				}
			}
			buildings.arr[build_i]->powered = build_powered;
			free(build_tiles);
		}

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