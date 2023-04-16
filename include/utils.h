#ifndef WHEN_THE_FACTORY_UTILS_
#define WHEN_THE_FACTORY_UTILS_

/* Coords of a tile on the map. */
struct TileCoords {
	int x, y;
};
typedef struct TileCoords TileCoords;

/*  */
struct Array {
	int size;
	void * arr;
};
typedef struct Array Array;

/* Function used by qsort */
int cmpInt (const void * a, const void * b);

#endif