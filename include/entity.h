#ifndef WHEN_THE_FACTORY_ENTITY_
#define WHEN_THE_FACTORY_ENTITY_

#include <stdbool.h>
#include <stdint.h>

#include "map.h"

/* Entity types. */
enum EntType {
	ENT_HUMAIN,
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

/* Entity, something that is not a tile terrain but rather ON a tile. */
struct Ent {
	EntType type;
	/* TODO: Add support for an entity being "inside" an other entity
	 * instead of directly on a tile. */
	TileCoords pos;
	FactionIdent faction;
};
typedef struct Ent Ent;

/* Entity ID. This refers to an entity (that may not exist anymore, it's ok though).
 * Prefer using `EntId` instead of `Ent*` to refer to entities when in doubt. */
struct EntId {
	uint32_t index;
	uint32_t gen;
};
typedef struct EntId EntId;



Ent* new_ent(EntType type, TileCoords pos);
void ent_delete(Ent* ent);
void ent_move(Ent* ent, TileCoords new_pos);

#endif // WHEN_THE_FACTORY_ENTITY_
