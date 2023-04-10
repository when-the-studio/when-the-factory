#ifndef WHEN_THE_FACTORY_UTILS_
#define WHEN_THE_FACTORY_UTILS_

/* Coords of a tile on the map. */
struct TileCoords {
	int x, y;
};
typedef struct TileCoords TileCoords;

/* Function used by qsort */
int cmpInt (const void * a, const void * b);

struct Dims {
	int w, h;
};
typedef struct Dims Dims;

/* Just a callback function returning `void` bundled with optional generic data. */
struct CallbackWithData {
	void (*func)(void* whatever);
	void* whatever;
};
typedef struct CallbackWithData CallbackWithData;

void call_callback(CallbackWithData cb);

int max(int a, int b);

#endif
