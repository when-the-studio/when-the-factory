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
