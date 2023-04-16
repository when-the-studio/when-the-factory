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

TileCoords getOffsetFromCardinal(CardinalType direction){
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

bool isTileIn(Tile * tile, Tile ** tiles, int size){
	for (int i=0; i<size; i++){
		printf("i : %d \n", i);
		if (tile == tiles[i]){
			return true;
		}
	}
	return false;
}

bool isBuildingIn(Building * building, Building ** buildings, int size){
	for (int i=0; i<size; i++){
		if (building == buildings[i]){
			return true;
		}
	}
	return false;
}

bool isCableIn(Cable * cable, Cable ** cables, int size){
	for (int i=0; i<size; i++){
		if (cable == cables[i]){
			return true;
		}
	}
	return false;
}

Array getConnectedTiles(TileCoords tc, FlowType type){
	Tile * tile = get_tile(tc);
	Tile * neigh;
	Array arr;
	Tile ** tiles = (Tile**)malloc(4 * sizeof(Tile*));
	int index = 0;
	TileCoords tcNeigh;
	if (tile->building != NULL){
		for (int i=0; i<4; i++){
			tcNeigh = getOffsetFromCardinal(WEST+i);
			tcNeigh.x += tc.x;
			tcNeigh.y += tc.y;
			printf("getTILES ADD %d ", tcNeigh.x);
			printf("%d", tcNeigh.y);
			neigh = get_tile(tcNeigh);
			printf(" %d\n", neigh->type);
			tiles[i] = neigh;
		}
		index = 4;
	} else {
		switch (type){
		case ELECTRIC_CABLE:
			for (int i=0; i<tile->flow_count; i++){
				for (int j=0; j<2; j++){
					tcNeigh = getOffsetFromCardinal(tile->flows[i]->connections[j]);
					tcNeigh.x += tc.x;
					tcNeigh.y += tc.y;
					neigh = get_tile(tcNeigh);
					if (!isTileIn(neigh, tiles, 4)){
						tiles[index] = neigh;
						index++;
					}
				}
			}
			break;
		}
	}
	
	printf("ind : %d \n", index);
	Tile ** test = realloc(tiles, index * sizeof(Tile*));
	arr.size = index;
	arr.arr = test;
	return arr;
}

Tile ** getAdjacentTiles(TileCoords tc){
	Tile ** tiles = malloc(4*sizeof(*tiles));
	TileCoords tcNeigh;
	Tile * neigh;
	for (int i=0; i<4; i++){
		tcNeigh = getOffsetFromCardinal(WEST+i);
		tcNeigh.x += tc.x;
		tcNeigh.y += tc.y;
		neigh = get_tile(tcNeigh);
		tiles[i] = neigh;
	}
	return tiles;
}

Tile ** getTilesFromCable(Cable * cable){
	printf("OwO 1\n");
	printf("11 %d \n", cable->capacity);
			fflush(stdout);
	TileCoords tc = cable->pos;
	printf("UwU\n");
		fflush(stdout);
	Tile * neigh;
	Tile ** tiles = malloc(2 * sizeof(Tile));
	TileCoords tcNeigh;
	for (int i=0; i<2; i++){
		printf("OwO\n");
		fflush(stdout);
		tcNeigh = getOffsetFromCardinal(cable->connections[i]);
		tcNeigh.x += tc.x;
		tcNeigh.y += tc.y;
		neigh = get_tile(tcNeigh);
		printf("neigh scan\n");
			fflush(stdout);
		tiles[i] = neigh;
	}
	return tiles;
}

void updateCableNetwork(TileCoords tc){
	Tile * tile = get_tile(tc);
	printf("%d ", tc.x);
	printf("%d\n", tc.y);

	int sizeBuildings = 10;
	int sizeNetwork = 10;
	int sizeScanned = 10;
	int sizeNewTiles = 10;

	int indBuildings = -1;
	int indNetwork = -1;
	int indScanned = -1;
	int indNewTiles = -1;

	Building ** buildings = malloc(sizeBuildings * sizeof(*buildings));
	Cable ** network = malloc(sizeNetwork * sizeof(*network));
	Cable ** scanned = malloc(sizeScanned * sizeof(*scanned));
	
	Tile ** tiles;
	Tile ** newTiles = malloc(sizeNewTiles * sizeof(*newTiles));
	Tile * t;
	bool power = false;
	Cable * flow;
	bool scanNetwork = true;

	for (int i=0; i<tile->flow_count; i++){
		indScanned++;
		if (indScanned == sizeScanned){
			sizeScanned += 10;
			scanned = realloc(scanned, sizeScanned);
		}
		scanned[indScanned] = tile->flows[i];
		printf("Added initial flow\n");
		fflush(stdout);
	}
	printf("Starting network scan\n");
	printf("initial flows / %d\n", tile->flow_count);
	fflush(stdout);
	while (scanNetwork){
		scanNetwork = false;
		indNewTiles = -1;
		for (int scan_i=0; scan_i < indScanned+1; scan_i++){

			printf("ALED / %d\n", scanned[0]->capacity);
			fflush(stdout);

			tiles = getTilesFromCable(scanned[scan_i]);
			printf("Starting neigh scan\n");
			fflush(stdout);
			for (int i=0; i<2; i++){

				if (!isTileIn((tiles)[i], newTiles, indNewTiles)){
					indNewTiles++;
					if (indNewTiles == sizeNewTiles){
						sizeNewTiles += 10;
						newTiles = realloc(newTiles, sizeNewTiles);
					}
					newTiles[indNewTiles] = (tiles)[i];
				}

				printf("%d ", i);
				printf("%d", (tiles)[i]->type);
				fflush(stdout);
				if ((tiles)[i]->flow_count>0){
					printf(" : %d has cable", i);
					fflush(stdout);
				}
				printf("\n");
				
			}

			indNetwork++;
			if (indNetwork == sizeNetwork){
				sizeNetwork += 10;
				network = realloc(network, sizeNetwork);
			}
			network[indNetwork] = scanned[scan_i];
			free(tiles);
		}

		indScanned = -1;

		for (int new_i = 0; new_i<indNewTiles+1; new_i++){
			t = (newTiles)[new_i];
			if (t->building != NULL && !isBuildingIn(t->building, buildings, indBuildings)){
				indBuildings++;
				if (indBuildings == sizeBuildings){
					sizeBuildings += 10;
					buildings = realloc(buildings, sizeBuildings);
				}
				buildings[indBuildings] = t->building;
			}

			for (int flow_i=0; flow_i<t->flow_count; flow_i++){
				if (!isCableIn(t->flows[flow_i], scanned, indScanned) && !isCableIn(t->flows[flow_i], network, indNetwork)){
					indScanned++;
					if (indScanned == sizeScanned){
						sizeScanned += 10;
						scanned = realloc(scanned, sizeScanned);
					}
					scanned[indScanned] = t->flows[flow_i];
					scanNetwork = true;
				}
			}
		}
		printf("\n=============\n");
	}

	for (int build_i=0; build_i < indBuildings+1; build_i++){
		if (buildings[build_i]->type == BUILDING_EMITTER){
			power = true;
		}
	}

	for (int net_i=0; net_i<indNetwork+1; net_i++){
		network[net_i]->powered = power;
	}

	for (int build_i=0; build_i < indBuildings+1; build_i++){
		// TODO implement direction of cable
		bool buildPowered = false;
		Tile ** buildTiles = getAdjacentTiles(buildings[build_i]->pos);
		for (int tile_adj_i=0; tile_adj_i<4; tile_adj_i++){
			for (int flow_adj_i=0; flow_adj_i<buildTiles[tile_adj_i]->flow_count; flow_adj_i++){
				if (buildTiles[tile_adj_i]->flows[flow_adj_i]->powered){
					buildPowered = true;
				}
			}
		}
		buildings[build_i]->powered = buildPowered;
		free(buildTiles);
	}

	free(buildings);
	free(newTiles);
	free(scanned);
	free(network);	
}

void updateSurroundings(TileCoords tc){
	TileCoords tcNeigh;
	for (int i=0; i<4; i++){
		tcNeigh = getOffsetFromCardinal(WEST+i);
		tcNeigh.x += tc.x;
		tcNeigh.y += tc.y;
		updateCableNetwork(tcNeigh);
	}
}