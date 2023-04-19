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

#include <stdlib.h>
#include <assert.h>

/* Dynamic array of some generic type `T_`. */
#define DA(T_) struct { T_* arr; int len, cap; }


/* Internal stuff used by macros and functions dealing with `DA(T_)`. */
struct DaVoid { void* arr; int len, cap; };
void da_void_reserve_at_least_one(struct DaVoid* da, int elem_size);
void da_void_empty_leak(struct DaVoid* da);

/* Pushes `elem_` at the end of the given dynamic array (passed by pointer).
 * Beware: `da_ptr_` is evaluated multiple times. */
#define DA_PUSH(da_ptr_, elem_) \
	do { \
		da_void_reserve_at_least_one((struct DaVoid*)(da_ptr_), sizeof *(da_ptr_)->arr); \
		(da_ptr_)->arr[(da_ptr_)->len++] = elem_; \
	} while (0)

/* Empties the given dynamic array (passed by pointer).
 * If it conrtained resources that should be freed, then
 * this should be done manually just before emptying the dynamic array. */
#define DA_EMPTY_LEAK(da_ptr_) da_void_empty_leak((struct DaVoid*)(da_ptr_))

#endif
