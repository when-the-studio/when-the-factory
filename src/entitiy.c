#include <stdlib.h>
#include <assert.h>
#include "entity.h"
#include "map.h"

FactionSpec g_faction_spec_table[FACTION_IDENT_NUM] = {
	[FACTION_YELLOW] = {
		.is_player = true,
		.name = "Yellow",
		.color = {255, 255, 0},
	},
	[FACTION_RED] = {
		.is_player = false,
		.name = "Red",
		.color = {255, 0, 0},
	},
};

FactionIdent g_faction_currently_playing = FACTION_YELLOW;

bool eid_null(EntId eid) {
	return eid_eq(eid, EID_NULL);
}

bool eid_eq(EntId a, EntId b) {
	return a.index == b.index && a.gen == b.gen;
}

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
static Entry* s_entry_table = NULL;
static uint32_t s_entry_count = 0;

Ent* get_ent(EntId eid) {
	if (eid_null(eid)) return NULL;
	assert(eid.index < s_entry_count);
	Entry* entry = &s_entry_table[eid.index];
	if (entry->ent != NULL && entry->gen == eid.gen) {
		return entry->ent;
	} else if (entry->gen < eid.gen) {
		/* The referenced entity is deleted (but has existed in the past). */
		return NULL;
	} else {
		/* The entry's gen can only increase (on entity deletion),
		 * so if the ID's gen is higher than the entry's,
		 * then it means that it references an entity that was not created yet (bug)
		 * or the index is wrong (bug). */
		assert(false);
	}
}

static uint32_t get_unused_entry_index(void) {
	for (uint32_t i = 0; i < s_entry_count; i++) {
		if (s_entry_table[i].ent == NULL) {
			return i;
		}
	}
	/* All entries are occupied by an entity, so we have to add new entries. */
	uint32_t old_count = s_entry_count;
	s_entry_count = s_entry_count == 0 ? 24 : s_entry_count * 2;
	s_entry_table = realloc(s_entry_table, s_entry_count * sizeof(Entry));
	memset(&s_entry_table[old_count], 0, (s_entry_count - old_count) * sizeof(Entry));
	return old_count;
}

void add_eid_to_tile_list(EntId eid, Tile* tile);
void remove_eid_from_tile_list(EntId eid, Tile* tile);

EntId ent_new(TileCoords pos) {
	uint32_t index = get_unused_entry_index();
	Entry* entry = &s_entry_table[index];
	entry->ent = malloc(sizeof(Ent));
	*entry->ent = (Ent){
		.type = ENT_UNTYPED,
		.pos = pos,
	};
	EntId eid = {.index = index, .gen = entry->gen};
	Tile* tile = get_tile(pos);
	add_eid_to_tile_list(eid, tile);
	return eid;
}

void ent_delete(EntId eid) {
	if (eid_null(eid)) return;
	assert(eid.index < s_entry_count);
	Entry* entry = &s_entry_table[eid.index];
	if (entry->ent != NULL && entry->gen == eid.gen) {
		Tile* tile = get_tile(entry->ent->pos);
		remove_eid_from_tile_list(eid, tile);
		/* TODO: Do some more adaptative freeing of whatever is pointed to by the entity
		* or else we are going to leak memory evesntually. */
		free(entry->ent);
		entry->ent = NULL;
		entry->gen++;
	} else if (entry->gen < eid.gen) {
		/* The referenced entity is deleted (but has existed in the past). */
	} else {
		/* The entry's gen can only increase (on entity deletion),
		 * so if the ID's gen is higher than the entry's,
		 * then it means that it references an entity that was not created yet (bug)
		 * or the index is wrong (bug). */
		assert(false);
	}
}

void ent_move(EntId eid, TileCoords new_pos) {
	Ent* ent = get_ent(eid);
	if (ent == NULL) return;
	TileCoords old_pos = ent->pos;
	Tile* old_tile = get_tile(old_pos);
	Tile* new_tile = get_tile(new_pos);
	remove_eid_from_tile_list(eid, old_tile);
	add_eid_to_tile_list(eid, new_tile);
	ent->pos = new_pos;
}

EntId ent_new_human(TileCoords pos, FactionIdent faction) {
	EntId eid = ent_new(pos);
	Ent* ent = get_ent(eid);
	ent->type = ENT_HUMAIN;
	ent->human = (struct EntDataHuman){
		.faction = faction,
		.already_moved_this_turn = false,
	};
	return eid;
}

EntId ent_new_test_block(TileCoords pos, SDL_Color color) {
	EntId eid = ent_new(pos);
	Ent* ent = get_ent(eid);
	ent->type = ENT_TEST_BLOCK;
	ent->test_block = (struct EntDataTestBlock){
		.color = color,
	};
	return eid;
}
