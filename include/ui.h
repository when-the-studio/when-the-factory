#ifndef WHEN_THE_FACTORY_UI_
#define WHEN_THE_FACTORY_UI_

#include <stdbool.h>
#include "map.h"

/* Initializes the widget tree. */
void init_wg_tree(void);

/* Selected tile, if any. */
extern bool g_sel_tile_exists;
extern TileCoords g_sel_tile_coords;

void ui_select_tile(TileCoords tc);
void ui_unselect_tile(void);

void refresh_selected_tile_ui(void);

#endif /* WHEN_THE_FACTORY_UI_ */
