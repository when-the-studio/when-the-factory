#include "gameplay.h"
#include "map.h"


bool ent_is_playing(EntId eid) {
	Ent* ent = get_ent(eid);
	return
		ent != NULL &&
		ent->type == ENT_HUMAN &&
		ent->human.faction == g_faction_currently_playing;
}

bool ent_can_move(EntId eid) {
	Ent* ent = get_ent(eid);
	return
		ent_is_playing(eid) &&
		(!ent->human.already_moved_this_turn);
}

void move_human(EntId eid, TileCoords dst_pos) {
	Ent* ent = get_ent(eid);
	assert(ent != NULL);
	assert(ent->type == ENT_HUMAN);

	TileCoords src_pos = ent->pos;
	ent_move(eid, dst_pos);

	if (ent->anim != NULL) {
		free(ent->anim);
	}
	ent->anim = malloc(sizeof(Anim));
	*ent->anim = (Anim){
		.time_beginning = g_time_ms,
		.time_end = g_time_ms + 80,
		.offset_beginning_x = dst_pos.x - src_pos.x,
		.offset_beginning_y = dst_pos.y - src_pos.y,
	};

	/* Mark it as having already moved. */
	ent->human.already_moved_this_turn = true;

	refresh_selected_tile_ui();
	DA_EMPTY_LEAK(&g_action_da_on_tcs);
	action_menu_refresh();
}

/* The human (given by its entity ID `eid`) will build a building (of the given type)
 * on the tile at the given coords. */
void have_human_to_build(EntId eid, BuildingType building_type, TileCoords tc) {
	new_building(building_type, tc);
	Ent* ent = get_ent(eid);
	assert(ent != NULL && ent->type == ENT_HUMAN);
	ent->human.already_moved_this_turn = true;
	refresh_selected_tile_ui();
	DA_EMPTY_LEAK(&g_action_da_on_tcs);
	action_menu_refresh();
}

/* The human (given by its entity ID `eid`) will perform the given action
 * on the tile at the given coords. */
void have_human_to_act(EntId eid, Action const* action, TileCoords tc) {
	Ent* ent = get_ent(eid);
	assert(ent != NULL && ent->type == ENT_HUMAN);
	assert(action != NULL);
	switch (action->type) {
		case ACTION_MOVE:
			move_human(g_sel_ent_id, tc);
		break;
		case ACTION_BUILD: {
			have_human_to_build(g_sel_ent_id, action->build.building_type, tc);
		break; }
		default: assert(false);
	}
}

void click(WinCoords wc) {
	/* The user clicked on the pixel at `wc`.
	 * First we forward this click to the UI in case it lands on a widget. */
	bool ui_used_the_click = ui_click(wc);
	if (ui_used_the_click) {
		return;
	}

	/* The UI said the click was not for them, so it lands on the map. */
	TileCoords tc = window_pixel_to_tile_coords(wc);
	if (tile_is_available(tc)) {
		/* A movable human is selected and the click landed on a tile to which
		 * the human can move or act. */
		assert(g_sel_ent_exists);
		Action const* action = action_menu_selection();
		have_human_to_act(g_sel_ent_id, action, tc);
	} else if (g_sel_tile_exists && tile_coords_eq(g_sel_tile_coords, tc)) {
		/* The click landed on the selected tile. */
		Tile const* tile = get_tile(g_sel_tile_coords);
		if (get_tile_real_ent_count(tile) > 0) {
			cycle_ent_sel_through_ents_in_tile();
		} else {
			ui_unselect_tile();
		}
	} else if (tile_coords_are_valid(tc)) {
		ui_unselect_ent();
		ui_select_tile(tc);
	} else {
		ui_unselect_tile();
	}
}
