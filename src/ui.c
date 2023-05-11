#include <assert.h>
#include "ui.h"
#include "widget.h"
#include "map.h"
#include "entity.h"
#include "objects/building.h"

#define RGB(r_, g_, b_) ((SDL_Color){(r_), (g_), (b_), 255})

static void next_faction_to_play(void) {
	g_faction_currently_playing = (g_faction_currently_playing + 1) % FACTION_IDENT_NUM;

	/* Reset the "already moved this turn" flags on entities. */
	for (int y = 0; y < g_map_h; y++) for (int x = 0; x < g_map_w; x++) {
		Tile* tile = get_tile((TileCoords){x, y});
		for (int i = 0; i < tile->ents.len; i++) {
			Ent* ent = get_ent(tile->ents.arr[i]);
			if (ent == NULL) continue;
			if (ent->type == ENT_HUMAIN) {
				ent->human.already_moved_this_turn = false;
			}
		}
	}
}

extern Uint64 g_time_ms;

void move_human(EntId eid, TileCoords dst_pos) {
	Ent* ent = get_ent(eid);
	assert(ent != NULL);
	assert(ent->type == ENT_HUMAIN);

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

static void random_ai_play(void) {
	assert(!g_faction_spec_table[g_faction_currently_playing].is_player);

	for (int y = 0; y < g_map_h; y++) for (int x = 0; x < g_map_w; x++) {
		TileCoords tc = (TileCoords){x, y};
		Tile* tile = get_tile((TileCoords){x, y});
		for (int i = 0; i < tile->ents.len; i++) {
			EntId eid = tile->ents.arr[i];
			Ent* ent = get_ent(eid);
			if (ent == NULL) continue;
			if (ent->type == ENT_HUMAIN) {
				if (ent->human.faction == g_faction_currently_playing
					&& !ent->human.already_moved_this_turn
				) {
					if (rand() % 7 != 0) {
						TileCoords dst_pos = tc;
						*(rand() % 2 == 0 ? &dst_pos.x : &dst_pos.y) += (rand() % 2) * 2 - 1;
						if (tile_coords_are_valid(dst_pos)) {
							Tile const* dst_tile = get_tile(dst_pos);
							if (tile_is_walkable(dst_tile)) {
								move_human(eid, dst_pos);
							}
						}
					}
				}
			}
		}
	}
}

void end_turn(void) {
	while (next_faction_to_play(), !g_faction_spec_table[g_faction_currently_playing].is_player) {
		random_ai_play();
	}
	if (g_sel_tile_exists) {
		ui_select_tile(g_sel_tile_coords);
	}
}

static void callback_end_turn(void* whatever) {
	(void)whatever;
	end_turn();
}

static Wg* s_wg_action_menu = NULL;
static int s_action_index = 0;

/* The root of the main widget tree. */
static Wg* s_wg_root = NULL;

/* Somewhere in `s_wg_root`, reserved for info on the selected tile (if any). */
static Wg* s_wg_tile_info = NULL;

void init_wg_tree(void) {
	s_wg_root = new_wg_multopleft(10, 10, 10, ORIENTATION_TOP_TO_BOTTOM);
	wg_multopleft_add_sub(s_wg_root,
		s_wg_tile_info = new_wg_multopleft(5, 0, 0, ORIENTATION_TOP_TO_BOTTOM)
	);
	wg_multopleft_add_sub(s_wg_root,
		new_wg_button(
			new_wg_box(
				new_wg_text_line("end turn", RGB(0, 0, 0)),
				5, 5, 3,
				RGB(0, 0, 0), RGB(255, 255, 255)
			),
			(CallbackWithData){.func = callback_end_turn, .whatever = NULL}
		)
	);

	s_wg_action_menu = new_wg_multopleft(2, 0, 0, ORIENTATION_TOP_TO_BOTTOM);
}

SDL_Rect tile_rect(TileCoords tc);
TileCoords window_pixel_to_tile_coords(WinCoords wc);

void render_wg_tree(void) {
	wg_render(s_wg_root, 0, 0);

	if (g_sel_ent_exists) {
		SDL_Point mouse;
		SDL_GetMouseState(&mouse.x, &mouse.y);
		TileCoords tc = window_pixel_to_tile_coords((WinCoords){mouse.x, mouse.y});
		if (tile_coords_are_valid(tc) && tile_is_available(tc)) {
			SDL_Rect rect = tile_rect(tc);
			wg_render(s_wg_action_menu, rect.x + rect.w, rect.y);
		}
	}
}

bool ui_click(WinCoords wc) {
	return wg_click(s_wg_root, 0, 0, wc.x, wc.y);
}

bool g_sel_tile_exists = false;
TileCoords g_sel_tile_coords = {0, 0};

struct CallbackMoveEntityData {
	EntId eid;
	TileCoords dst_pos;
};
typedef struct CallbackMoveEntityData CallbackMoveEntityData;

void test_callback_move_entity(void* whatever) {
	/* Move the entity. */
	CallbackMoveEntityData* data = whatever;
	move_human(data->eid, data->dst_pos);
}

void ui_select_tile(TileCoords tc) {
	ui_unselect_tile();
	g_sel_tile_exists = true;
	g_sel_tile_coords = tc;

	Tile* tile = get_tile(tc);
	char const* name = g_tile_type_spec_table[tile->type].name;
	Wg* wg_terrain_info = new_wg_multopleft(6, 0, 0, ORIENTATION_LEFT_TO_RIGHT);
	wg_multopleft_add_sub(s_wg_tile_info,
		new_wg_box(
			wg_terrain_info,
			7, 7, 3,
			RGB(0, 0, 0), RGB(200, 200, 255)
		)
	);
	int ui_terrain_sprite_side = 5 * 3 * 3;
	wg_multopleft_add_sub(wg_terrain_info,
		new_wg_sprite(
			g_spritesheet,
			g_tile_type_spec_table[tile->type].rect_in_spritesheet,
			ui_terrain_sprite_side, ui_terrain_sprite_side
		)
	);
	wg_multopleft_add_sub(wg_terrain_info,
		new_wg_text_line((char*)name, RGB(0, 0, 0))
	);

	// Building widget
	// Supposes that there one building at most on a tile
	int ui_building_sprite_side = 5 * 3 * 3;
	if (tile->building != NULL) {
		Wg* wg_building_info = new_wg_multopleft(6, 0, 0, ORIENTATION_LEFT_TO_RIGHT);
		BuildingType b_type = tile->building->type + tile->building->powered;
		char const* b_name = g_building_type_spec_table[b_type].name;
		SDL_Rect const b_rect = g_building_type_spec_table[b_type].rect_in_spritesheet;

		Wg* wg_building_sprite = new_wg_sprite(
			g_spritesheet_buildings,
			b_rect,
			ui_building_sprite_side, ui_building_sprite_side
		);
		wg_multopleft_add_sub(wg_building_info, wg_building_sprite);

		Wg* wg_building_text = new_wg_text_line(
			(char*) b_name,
			RGB(0, 0, 0)
		);
		wg_multopleft_add_sub(wg_building_info, wg_building_text);

		wg_multopleft_add_sub(s_wg_tile_info,
			new_wg_box(
				wg_building_info,
				7, 7, 3,
				RGB(0, 0, 0), RGB(200, 200, 255)
			)
		);
	}

	for (int i = 0; i < tile->ents.len; i++) {
		EntId eid = tile->ents.arr[i];
		Ent* ent = get_ent(eid);
		if (ent == NULL) continue;
		Wg* wg_ent = NULL;
		switch (ent->type) {
			case ENT_HUMAIN:;
				wg_ent = new_wg_multopleft(6, 0, 0, ORIENTATION_TOP_TO_BOTTOM);
				Wg* wg_ent_info = new_wg_multopleft(6, 0, 0, ORIENTATION_LEFT_TO_RIGHT);
				wg_multopleft_add_sub(wg_ent, wg_ent_info);
				wg_multopleft_add_sub(wg_ent_info,
					new_wg_text_line("Human", RGB(0, 0, 0))
				);
				char* faction_name = g_faction_spec_table[ent->human.faction].name;
				SDL_Color faction_color = g_faction_spec_table[ent->human.faction].color;
				wg_multopleft_add_sub(wg_ent_info,
					new_wg_text_line(faction_name, faction_color)
				);
				if (ent->human.faction == g_faction_currently_playing) {
					Wg* wg_ent_buttons = new_wg_multopleft(4, 0, 0, ORIENTATION_LEFT_TO_RIGHT);
					wg_multopleft_add_sub(wg_ent, wg_ent_buttons);
					if (ent->human.already_moved_this_turn) {
						wg_multopleft_add_sub(wg_ent_buttons,
							new_wg_text_line("already moved", RGB(150, 150, 150))
						);
					} else {
						/* The human can move and has (temporary, TODO better) buttons to do so. */
						typedef struct { int dx, dy; char* name; } Dir;
						Dir dirs[4] = {{1, 0, "Right"}, {0, 1, "Down"}, {-1, 0, "Left"}, {0, -1, "Up"}};
						for (int dir_i = 0; dir_i < 4; dir_i++) {
							Dir dir = dirs[dir_i];
							TileCoords dst_pos = {tc.x + dir.dx, tc.y + dir.dy};
							if (!tile_coords_are_valid(dst_pos)) continue;
							Tile const* dst_tile = get_tile(dst_pos);
							if (!tile_is_walkable(dst_tile)) continue;
							CallbackMoveEntityData* data = malloc(sizeof(CallbackMoveEntityData));
							*data = (CallbackMoveEntityData){
								.eid = eid,
								.dst_pos = dst_pos,
							};
							wg_multopleft_add_sub(wg_ent_buttons,
								new_wg_button(
									new_wg_box(
										new_wg_text_line(dir.name, RGB(0, 0, 255)),
										6, 6, 3,
										RGB(0, 0, 0), RGB(255, 255, 255)
									),
									(CallbackWithData){.func = test_callback_move_entity, .whatever = data}
								)
							);
						}
					}
				}
			break;
			case ENT_TEST_BLOCK:;
				wg_ent = new_wg_multopleft(10, 0, 0, ORIENTATION_LEFT_TO_RIGHT);
				wg_multopleft_add_sub(wg_ent,
					new_wg_text_line("Test block", ent->test_block.color)
				);
			break;
			default: assert(false);
		}
		assert(wg_ent != NULL);
		Wg* wg_ent_box = new_wg_box(
			wg_ent,
			6, 6, 3,
			RGB(0, 0, 0), RGB(200, 200, 255)
		);
		if (eid_eq(g_sel_ent_id, eid)) {
			wg_multopleft_add_sub(s_wg_tile_info,
				new_wg_box(
					wg_ent_box,
					10, 0, 0,
					(SDL_Color){0, 0, 0, 0}, (SDL_Color){0, 0, 0, 0}
				)
			);
		} else {
			wg_multopleft_add_sub(s_wg_tile_info, wg_ent_box);
		}
	}
}

void ui_unselect_tile(void) {
	g_sel_tile_exists = false;
	wg_multopleft_empty(s_wg_tile_info);
}

void refresh_selected_tile_ui(void) {
	wg_multopleft_empty(s_wg_tile_info);
	if (g_sel_tile_exists) {
		ui_select_tile(g_sel_tile_coords);
	}
}

bool g_sel_ent_exists = false;
EntId g_sel_ent_id = EID_NULL;

void ui_select_ent(EntId eid) {
	g_sel_ent_exists = !eid_null(eid);
	g_sel_ent_id = eid;
	refresh_selected_tile_ui();

	DA_EMPTY_LEAK(&g_action_da_on_tcs);

	if (g_sel_ent_exists) {
		EntId eid = g_sel_ent_id;
		Ent* ent = get_ent(eid);
		if (ent != NULL &&
			ent->type == ENT_HUMAIN &&
			ent->human.faction == g_faction_currently_playing &&
			(!ent->human.already_moved_this_turn)
		) {
			struct {int dx, dy;} const dirs[4] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
			for (int i = 0; i < 4; i++) {
				TileCoords dst_pos = ent->pos;
				dst_pos.x += dirs[i].dx;
				dst_pos.y += dirs[i].dy;
				if (tile_coords_are_valid(dst_pos)) {
					Tile const* dst_tile = get_tile(dst_pos);
					if (tile_is_walkable(dst_tile)) {
						ActionDaOnTc action_da_on_tc = {
							.tc = dst_pos,
							.actions = {0}
						};
						/* Move. */
						{
							Action action = {
								.type = ACTION_MOVE,
								.move = {0},
							};
							DA_PUSH(&action_da_on_tc.actions, action);
						}
						/* Build. */
						for (int i = 0; i < BUILDING_TYPE_NUM; i++) {
							Action action = {
								.type = ACTION_BUILD,
								.build = {
									.building_type = i,
								}
							};
							DA_PUSH(&action_da_on_tc.actions, action);
						}
						DA_PUSH(&g_action_da_on_tcs, action_da_on_tc);
					}
				}
			}
			action_menu_refresh();
		}
	}
}

void ui_unselect_ent(void) {
	g_sel_ent_exists = false;
	g_sel_ent_id = EID_NULL;
	refresh_selected_tile_ui();

	DA_EMPTY_LEAK(&g_action_da_on_tcs);
}

DA_ActionDaOnTc g_action_da_on_tcs = {0};

bool tile_is_available(TileCoords tc) {
	for (int i = 0; i < g_action_da_on_tcs.len; i++) {
		if (tile_coords_eq(g_action_da_on_tcs.arr[i].tc, tc)) {
			return true;
		}
	}
	return false;
}

bool ent_can_move(EntId eid);

void action_menu_refresh(void) {
	wg_multopleft_empty(s_wg_action_menu);
	if (g_action_da_on_tcs.len == 0) return;

	SDL_Point mouse;
	SDL_GetMouseState(&mouse.x, &mouse.y);
	TileCoords tc = window_pixel_to_tile_coords((WinCoords){mouse.x, mouse.y});

	ActionDaOnTc const* action_da_on_tc = NULL;
	for (int i = 0; i < g_action_da_on_tcs.len; i++) {
		if (tile_coords_eq(g_action_da_on_tcs.arr[i].tc, tc)) {
			action_da_on_tc = &g_action_da_on_tcs.arr[i];
			break;
		}
	}
	if (action_da_on_tc == NULL) {
		s_action_index = 0;
		return;
	}

	for (int i = 0; i < action_da_on_tc->actions.len; i++) {
		Action const* action = &action_da_on_tc->actions.arr[i];
		switch (action->type) {
			case ACTION_MOVE:
				wg_multopleft_add_sub(s_wg_action_menu,
					new_wg_box(
						new_wg_text_line("move", RGB(0, 0, 0)),
						10, 10, 3,
						RGB(0, 0, 0), RGB(200, 200, 200)
					)
				);
			break;
			case ACTION_BUILD: {
				BuildingTextureType building_texture_index =
					(BuildingTextureType[]){
						[BUILDING_EMITTER] = BUILDING_TX_EMITTER,
						[BUILDING_RECEIVER] = BUILDING_TX_RECEIVER_OFF,
					}[action->build.building_type];
				wg_multopleft_add_sub(s_wg_action_menu,
					new_wg_box(
						new_wg_sprite(
							g_spritesheet_buildings,
							g_building_type_spec_table[building_texture_index].rect_in_spritesheet,
							24, 24
						),
						10, 10, 3,
						RGB(0, 0, 0), RGB(200, 200, 200)
					)
				);
			break; }
			default: assert(false);
		}
	}

	/* Hilight of the selected action. */
	assert(0 <= s_action_index && s_action_index < s_wg_action_menu->multl.sub_wgs.len);
	Wg* wg_action = s_wg_action_menu->multl.sub_wgs.arr[s_action_index];
	assert(wg_action->type == WG_BOX);
	wg_action->box.line_color = RGB(255, 0, 0);
}

void action_menu_scroll(int dy) {
	int len = s_wg_action_menu->multl.sub_wgs.len;
	if (len == 0) return;

	s_action_index += dy;
	if (s_action_index < 0) {
		s_action_index = len - 1;
	} else if (len <= s_action_index) {
		s_action_index = 0;
	}

	action_menu_refresh();
}

Action const* action_menu_selection(void) {
	if (g_action_da_on_tcs.len == 0) return NULL;

	SDL_Point mouse;
	SDL_GetMouseState(&mouse.x, &mouse.y);
	TileCoords tc = window_pixel_to_tile_coords((WinCoords){mouse.x, mouse.y});

	ActionDaOnTc const* action_da_on_tc = NULL;
	for (int i = 0; i < g_action_da_on_tcs.len; i++) {
		if (tile_coords_eq(g_action_da_on_tcs.arr[i].tc, tc)) {
			action_da_on_tc = &g_action_da_on_tcs.arr[i];
			break;
		}
	}
	if (action_da_on_tc == NULL) {
		return NULL;
	}

	assert(0 <= s_action_index && s_action_index < action_da_on_tc->actions.len);
	return &action_da_on_tc->actions.arr[s_action_index];
}
