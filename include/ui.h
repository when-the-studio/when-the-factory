#ifndef WHEN_THE_FACTORY_UI_
#define WHEN_THE_FACTORY_UI_

#include <stdbool.h>
#include "map.h"

/* Initializes the widget tree. */
void init_wg_tree(void);

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

#endif /* WHEN_THE_FACTORY_UI_ */
