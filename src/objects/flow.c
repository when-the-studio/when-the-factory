#include "flow.h"
#include "map.h"

CardinalType getOpposedDirection(CardinalType direction){
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

	int size_buildings = 10;
	int size_network = 10;
	int size_scanned = 10;
	int size_new_tiles = 10;

	int ind_buildings = -1;
	int ind_network = -1;
	int ind_scanned = -1;
	int ind_new_tiles = -1;

	Building ** buildings = malloc(size_buildings * sizeof(*buildings));
	Cable ** network = malloc(size_network * sizeof(*network));
	Cable ** scanned = malloc(size_scanned * sizeof(*scanned));
	
	Tile ** tiles;
	Tile ** new_tiles = malloc(size_new_tiles * sizeof(*new_tiles));
	Tile * t;
	bool power = false;
	Cable * flow;
	bool scan_network = true;

	for (int i=0; i<tile->flow_count; i++){
		ind_scanned++;
		if (ind_scanned == size_scanned){
			size_scanned += 10;
			scanned = realloc(scanned, size_scanned);
		}
		scanned[ind_scanned] = tile->flows[i];
	}
	while (scan_network){
		scan_network = false;
		ind_new_tiles = -1;
		for (int scan_i=0; scan_i < ind_scanned+1; scan_i++){
			tiles = get_tiles_from_cable(scanned[scan_i]);
			for (int i=0; i<2; i++){

				if (!is_tile_in((tiles)[i], new_tiles, ind_new_tiles)){
					ind_new_tiles++;
					if (ind_new_tiles == size_new_tiles){
						size_new_tiles += 10;
						new_tiles = realloc(new_tiles, size_new_tiles);
					}
					new_tiles[ind_new_tiles] = (tiles)[i];
				}				
			}

			ind_network++;
			if (ind_network == size_network){
				size_network += 10;
				network = realloc(network, size_network);
			}
			network[ind_network] = scanned[scan_i];
			free(tiles);
		}

		ind_scanned = -1;

		for (int new_i = 0; new_i<ind_new_tiles+1; new_i++){
			t = (new_tiles)[new_i];
			if (t->building != NULL && !is_building_in(t->building, buildings, ind_buildings)){
				ind_buildings++;
				if (ind_buildings == size_buildings){
					size_buildings += 10;
					buildings = realloc(buildings, size_buildings);
				}
				buildings[ind_buildings] = t->building;
			}

			for (int flow_i=0; flow_i<t->flow_count; flow_i++){
				if (!is_cable_in(t->flows[flow_i], scanned, ind_scanned) && !is_cable_in(t->flows[flow_i], network, ind_network)){
					ind_scanned++;
					if (ind_scanned == size_scanned){
						size_scanned += 10;
						scanned = realloc(scanned, size_scanned);
					}
					scanned[ind_scanned] = t->flows[flow_i];
					scan_network = true;
				}
			}
		}
	}

	for (int build_i=0; build_i < ind_buildings+1; build_i++){
		if (buildings[build_i]->type == BUILDING_EMITTER){
			power = true;
		}
	}

	for (int net_i=0; net_i<ind_network+1; net_i++){
		network[net_i]->powered = power;
	}

	for (int build_i=0; build_i < ind_buildings+1; build_i++){
		// TODO implement direction of cable
		bool build_powered = false;
		Tile ** build_tiles = get_adjacent_tiles(buildings[build_i]->pos);
		for (int tile_adj_i=0; tile_adj_i<4; tile_adj_i++){
			for (int flow_adj_i=0; flow_adj_i<build_tiles[tile_adj_i]->flow_count; flow_adj_i++){
				if (build_tiles[tile_adj_i]->flows[flow_adj_i]->powered){
					build_powered = true;
				}
			}
		}
		buildings[build_i]->powered = build_powered;
		free(build_tiles);
	}

	free(buildings);
	free(new_tiles);
	free(scanned);
	free(network);	
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