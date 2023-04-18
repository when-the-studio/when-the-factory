#include "utils.h"

int cmpInt (const void * a, const void * b) {
	return ( *(int*)a - *(int*)b );
}

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
