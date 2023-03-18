#include <stdlib.h>
#include "entity.h"

/* An entry in the global (static) entity table. */
struct Entry {
	/* Generation number.
	 * If there is an entity in this entry, then this is the generation of this entity,
	 * else this is the generation of the next enity that will be created in this entry. */
	uint32_t gen;
	Ent* ent;
};
typedef struct Entry Entry;

/* Here it is, the entity table. */
static Entry* s_entry_arr = NULL;
static int s_entry_len = 0;

static Entry* get_unused_entry(void) {
	for (int i = 0; i < s_entry_len; i++) {
		if (s_entry_arr[i].ent == NULL) {
			return &s_entry_arr[i];
		}
	}
	/* All entries are occupied by an entity, so we have to add new entries. */
	int old_len = s_entry_len;
	s_entry_len = s_entry_len == 0 ? 24 : s_entry_len * 2;
	s_entry_arr = realloc(s_entry_arr, s_entry_len * sizeof(Entry));
	memset(&s_entry_arr[old_len], 0, (s_entry_len - old_len) * sizeof(Entry));
	return &s_entry_arr[old_len];
}

Ent* new_ent(EntType type, TileCoords pos) {
	Entry* entry = get_unused_entry();
	entry->ent = h
	#error TODO
}

void ent_delete(Ent* ent) {

}

void ent_move(Ent* ent, TileCoords new_pos) {

}
