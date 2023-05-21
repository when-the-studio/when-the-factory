#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils.h"

void call_callback(CallbackWithData cb) {
	cb.func(cb.whatever);
}

int max(int a, int b) {
	return a < b ? b : a;
}

void da_void_reserve_at_least_one(struct DaVoid* da, int elem_size) {
	assert(da != NULL);
	assert(elem_size >= 1);
	assert(da->len <= da->cap);
	if (da->len == da->cap) {
		if (da->cap == 0) {
			assert(da->arr == NULL);
			assert(da->len == 0);
			da->cap = 4;
		} else {
			assert(da->arr != NULL);
			da->cap *= 2;
		}
		da->arr = realloc(da->arr, da->cap * elem_size);
	}
}

void da_void_empty_leak(struct DaVoid* da) {
	assert(da != NULL);
	assert(da->len <= da->cap);
	assert((da->cap == 0 && da->arr == NULL) || (da->cap > 0 && da->arr != NULL));
	free(da->arr);
	*da = (struct DaVoid){0};
}

TileCoords tc_add_dxdy(TileCoords tc, DxDy dxdy) {
	return (TileCoords){tc.x + dxdy.dx, tc.y + dxdy.dy};
}

DxDy arrow_keycode_to_dxdy(SDL_Keycode keycode) {
	switch (keycode) {
		case SDLK_DOWN:  return (DxDy){0, +1};
		case SDLK_UP:    return (DxDy){0, -1};
		case SDLK_RIGHT: return (DxDy){+1, 0};
		case SDLK_LEFT:  return (DxDy){-1, 0};
		default: assert(false); exit(EXIT_FAILURE);
	}
}

bool keycode_is_arrow(SDL_Keycode keycode) {
	return
		keycode == SDLK_DOWN ||
		keycode == SDLK_UP ||
		keycode == SDLK_RIGHT ||
		keycode == SDLK_LEFT;
}
