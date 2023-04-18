#include <assert.h>
#include "ui.h"
#include "widget.h"
#include "map.h"
#include "entity.h"

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

void move_human(EntId eid, TileCoords dst_pos) {
	ent_move(eid, dst_pos);

	/* Mark it as having already moved. */
	Ent* ent = get_ent(eid);
	assert(ent->type == ENT_HUMAIN);
	ent->human.already_moved_this_turn = true;

	refresh_selected_tile_ui();
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

static void callback_end_turn(void* whatever) {
	(void)whatever;
	while (next_faction_to_play(), !g_faction_spec_table[g_faction_currently_playing].is_player) {
		random_ai_play();
	}
	if (g_sel_tile_exists) {
		ui_select_tile(g_sel_tile_coords);
	}
}

static Wg* s_wg_tile_info = NULL;

void init_wg_tree(void) {
	g_wg_root = new_wg_multopleft(10, 10, 10, ORIENTATION_TOP_TO_BOTTOM);
	wg_multopleft_add_sub(g_wg_root,
		s_wg_tile_info = new_wg_multopleft(5, 0, 0, ORIENTATION_TOP_TO_BOTTOM)
	);
	wg_multopleft_add_sub(g_wg_root,
		new_wg_button(
			new_wg_box(
				new_wg_text_line("end turn", RGB(0, 0, 0)),
				5, 5, 3,
				RGB(0, 0, 0), RGB(255, 255, 255)
			),
			(CallbackWithData){.func = callback_end_turn, .whatever = NULL}
		)
	);
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
			g_tile_type_spec_table[tile->type].rect_in_spritesheet,
			ui_terrain_sprite_side, ui_terrain_sprite_side
		)
	);
	wg_multopleft_add_sub(wg_terrain_info, 
		new_wg_text_line((char*)name, RGB(0, 0, 0))
	);
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
		wg_multopleft_add_sub(s_wg_tile_info, 
			new_wg_box(
				wg_ent,
				6, 6, 3,
				RGB(0, 0, 0), RGB(200, 200, 255)
			)
		);
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
