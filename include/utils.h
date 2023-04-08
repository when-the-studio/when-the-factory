#ifndef WHEN_THE_FACTORY_UTILS_
#define WHEN_THE_FACTORY_UTILS_

/* Coords of a tile on the map. */
struct TileCoords {
	int x, y;
};
typedef struct TileCoords TileCoords;

/* Function used by qsort */
int cmpInt (const void * a, const void * b);

#endif