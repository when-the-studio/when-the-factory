#ifndef WHEN_THE_FACTORY_ENTITY_
#define WHEN_THE_FACTORY_ENTITY_

#include <stdbool.h>
#include <stdint.h>

#include "map.h"

/* Entity types. */
enum EntType {
	ENT_UNTYPED = 0,
	ENT_HUMAIN,
	ENT_TEST_BLOCK,
	ENT_BUILDING,

	ENT_TYPE_NUM,
};
typedef enum EntType EntType;

enum FactionIdent {
	FACTION_YELLOW,
	FACTION_RED,

	FACTION_IDENT_NUM
};
typedef enum FactionIdent FactionIdent;

struct FactionSpec {
	bool is_player;
	char* name;
	SDL_Color color;
};
typedef struct FactionSpec FactionSpec;

extern FactionSpec g_faction_spec_table[FACTION_IDENT_NUM];
extern FactionIdent g_faction_currently_playing;

/* Animation for an entity. */
struct Anim {
	int time_beginning; // ms
	int time_end; // ms
	float offset_beginning_x, offset_beginning_y;
};
typedef struct Anim Anim;

/* Entity, something that is not a tile terrain but rather ON a tile. */
struct Ent {
	EntType type;
	/* TODO: Add support for an entity being "inside" an other entity
	 * instead of directly on a tile. */
	TileCoords pos;
	Anim* anim;
	union {
		struct EntDataHuman {
			FactionIdent faction;
			bool already_moved_this_turn;
		} human;
		struct EntDataTestBlock {
			SDL_Color color;
		} test_block;
	};
};
typedef struct Ent Ent;

/* Entity ID. This refers to an entity (that may not exist anymore, it's ok though).
 * Prefer using `EntId` instead of `Ent*` to refer to entities when in doubt. */
struct EntId {
	uint32_t index;
	uint32_t gen;
};
typedef struct EntId EntId;

/* Magic `EntId` value that references no entity. */
#define EID_NULL ((EntId){.index = UINT32_MAX, .gen = UINT32_MAX})

bool eid_null(EntId eid);
bool eid_eq(EntId a, EntId b);

/* Returns the entity referenced by the given ID,
 * or `NULL` if the referenced entity does not exist anymore or eid is `ENT_ID_NULL`. */
Ent* get_ent(EntId eid);

EntId ent_new(TileCoords pos);
void ent_delete(EntId eid);
void ent_move(EntId eid, TileCoords new_pos);

EntId ent_new_human(TileCoords pos, FactionIdent faction);

EntId ent_new_test_block(TileCoords pos, SDL_Color color);

#endif // WHEN_THE_FACTORY_ENTITY_
