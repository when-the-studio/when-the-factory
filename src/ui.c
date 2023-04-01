#include <assert.h>
#include "ui.h"
#include "widget.h"
#include "map.h"
#include "entity.h"

#define RGB(r_, g_, b_) ((SDL_Color){(r_), (g_), (b_), 255})

static void test_callback_print(void* whatever) {
	(void)whatever;
	printf("test uwu\n");
}

static void test_callback_add(void* whatever) {
	(void)whatever;
	int r = rand() % 256;
	wg_multopleft_add_sub(g_wg_root,
		new_wg_text_line("owo", RGB(255, r, 255 - r))
	);
}

static void test_callback_clear(void* whatever) {
	(void)whatever;
	wg_multopleft_empty(g_wg_root);
}

static Wg* s_wg_tile_info = NULL;

void init_wg_tree(void) {
	g_wg_root = new_wg_multopleft(10, 10, 10, ORIENTATION_TOP_TO_BOTTOM);
	wg_multopleft_add_sub(g_wg_root,
		new_wg_button(
			new_wg_text_line("test xd", RGB(0, 0, 255)),
			(CallbackWithData){.func = test_callback_print, .whatever = NULL}
		)
	);
	wg_multopleft_add_sub(g_wg_root,
		new_wg_text_line("test uwu !!! ballz ``sus amogus -1 +8 1000 gaming", RGB(0, 0, 0))
	);
	wg_multopleft_add_sub(g_wg_root,
		new_wg_button(
			new_wg_text_line("add", RGB(0, 0, 255)),
			(CallbackWithData){.func = test_callback_add, .whatever = NULL}
		)
	);
	wg_multopleft_add_sub(g_wg_root,
		new_wg_button(
			new_wg_text_line("clear (will cause crash when selecting tile xd)", RGB(255, 0, 0)),
			(CallbackWithData){.func = test_callback_clear, .whatever = NULL}
		)
	);
	wg_multopleft_add_sub(g_wg_root,
		new_wg_box(
			new_wg_text_line("box", RGB(0, 0, 0)),
			5, 5, 3,
			RGB(0, 0, 0), RGB(255, 255, 0)
		)
	);
	wg_multopleft_add_sub(g_wg_root,
		s_wg_tile_info = new_wg_multopleft(10, 10, 0, ORIENTATION_TOP_TO_BOTTOM)
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
	CallbackMoveEntityData* data = whatever;
	ent_move(data->eid, data->dst_pos);
	refresh_selected_tile_ui();
}

void ui_select_tile(TileCoords tc) {
	ui_unselect_tile();
	g_sel_tile_exists = true;
	g_sel_tile_coords = tc;

	Tile* tile = get_tile(tc);
	char const* name = g_tile_type_spec_table[tile->type].name;
	wg_multopleft_add_sub(s_wg_tile_info,
		new_wg_text_line((char*)name, RGB(0, 0, 0))
	);
	for (int i = 0; i < tile->ent_count; i++) {
		EntId eid = tile->ents[i];
		Ent* ent = get_ent(eid);
		if (ent == NULL) continue;
		Wg* wg_ent = new_wg_multopleft(10, 10, 0, ORIENTATION_LEFT_TO_RIGHT);
		wg_multopleft_add_sub(s_wg_tile_info, wg_ent);
		switch (ent->type) {
			case ENT_HUMAIN:;
				EntDataHuman* data_human = ent->data;
				wg_multopleft_add_sub(wg_ent,
					new_wg_text_line("Human", RGB(0, 0, 0))
				);
				char* name;
				SDL_Color color;
				switch (data_human->faction) {
					case FACTION_YELLOW: name = "Yellow"; color = (SDL_Color){255, 255, 0, 255}; break;
					case FACTION_RED:    name = "Red";    color = (SDL_Color){255, 0,   0, 255}; break;
					default: assert(false);
				}
				wg_multopleft_add_sub(wg_ent,
					new_wg_text_line(name, color)
				);
				typedef struct { int dx, dy; char* name; } Dir;
				Dir dirs[4] = {{1, 0, "Right"}, {0, 1, "Down"}, {-1, 0, "Left"}, {0, -1, "Up"}};
				for (int dir_i = 0; dir_i < 4; dir_i++) {
					Dir dir = dirs[dir_i];
					CallbackMoveEntityData* data = malloc(sizeof(CallbackMoveEntityData));
					*data = (CallbackMoveEntityData){
						.eid = eid,
						.dst_pos = {tc.x + dir.dx, tc.y + dir.dy},
					};
					wg_multopleft_add_sub(wg_ent,
						new_wg_button(
							new_wg_text_line(dir.name, RGB(0, 0, 255)),
							(CallbackWithData){.func = test_callback_move_entity, .whatever = data}
						)
					);
				}
			break;
			case ENT_TEST_BLOCK:;
				EntDataTestBlock* data_block = ent->data;
				wg_multopleft_add_sub(wg_ent,
					new_wg_text_line("Test block", data_block->color)
				);
			break;
			default: assert(false);
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
