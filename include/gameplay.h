#ifndef WHEN_THE_FACTORY_GAMEPLAY_
#define WHEN_THE_FACTORY_GAMEPLAY_

#include "entity.h"
#include "renderer.h"
#include "ui.h"
#include "objects/building.h"

bool ent_is_playing(EntId eid);

bool ent_can_move(EntId eid);

void move_human(EntId eid, TileCoords dst_pos);

/* The human (given by its entity ID `eid`) will build a building (of the given type)
 * on the tile at the given coords. */
void have_human_to_build(EntId eid, BuildingType building_type, TileCoords tc);

/* The human (given by its entity ID `eid`) will perform the given action
 * on the tile at the given coords. */
void have_human_to_act(EntId eid, Action const* action, TileCoords tc);

void click(WinCoords wc);

#endif // WHEN_THE_FACTORY_GAMEPLAY_
