#ifndef WHEN_THE_FACTORY_UI_
#define WHEN_THE_FACTORY_UI_

#include <stdbool.h>
#include "map.h"
#include "utils.h"
#include "renderer.h"

/* Initializes the UI widget tree. */
void init_wg_tree(void);

/* Renders the UI widget tree. */
void render_wg_tree(void);

/* Inform the UI that a click occured.
 * Returns `true` iff the UI used a click (when it lands on a widget). */
bool ui_click(WinCoords wc);

/* Selected tile, if any.
 * These should be modified via `ui_select_tile` and `ui_unselect_tile` to ensure
 * that the UI is consistent with the tile that appears selected on the map. */
extern bool g_sel_tile_exists;
extern TileCoords g_sel_tile_coords;
void ui_select_tile(TileCoords tc);
void ui_unselect_tile(void);

/* When changes are made to the selected tile and what its content,
 * calling this ensures that the UI is updated. */
void refresh_selected_tile_ui(void);

/* Selected entity, if any.
 * These should be modified via `ui_select_ent` and `ui_unselect_ent` to ensure
 * that the UI is consistent with the entity that appears selected on the map. */
extern bool g_sel_ent_exists;
extern EntId g_sel_ent_id;
void ui_select_ent(EntId eid);
void ui_unselect_ent(void);

typedef DA(TileCoords) DA_TileCoords; 
extern DA_TileCoords g_available_tcs;
bool tile_is_available(TileCoords tc);

enum ActionType {
	ACTION_MOVE,
	ACTION_BUILD,
};
typedef enum ActionType ActionType;
struct Action {
	ActionType type;
	union {
		struct ActionMove {
			TileCoords dst_pos;
		} move;
		struct ActionBuild {
			BuildingType building_type;
			TileCoords dst_pos;
		} build;
	};
};
typedef struct Action Action;

void action_menu_scroll(int dy);

#endif /* WHEN_THE_FACTORY_UI_ */
