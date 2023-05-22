#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include <SDL2/SDL.h>
// #include <SDL2/SDL_ttf.h>

#include "renderer.h"
#include "map.h"
#include "camera.h"
#include "widget.h"
#include "entity.h"
#include "ui.h"
#include "gameplay.h"


int main(int argc, char const** argv) {
	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--map-size") == 0) {
			i++;
			assert(i < argc);
			int map_size = atoi(argv[i]);
			g_map_w = map_size;
			g_map_h = map_size;
		} else {
			printf("Unknown command line parameter \"%s\"\n", argv[i]);
		}
	}

	renderer_init();
	init_map();
	init_wg_tree();

	/* Game loop iteration time monitoring. */
	Uint64 time = SDL_GetPerformanceCounter();
	Uint64 last_time = 0;

	/* Main game loop. */
	bool running = true;
	int cable_orientation = NORTH;
	while (running) {
		last_time = time;
		time = SDL_GetPerformanceCounter();
		/* Delta time in ms. */
		double dt = (time - last_time) * 1000 / (double)SDL_GetPerformanceFrequency();
		g_time_ms = (double)time * 1000.0 / (double)SDL_GetPerformanceFrequency();

		/* Handle events. */
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN && event.key.repeat) {
				if (event.key.keysym.sym == SDLK_KP_DIVIDE) {
					/* Allow skipping tunrs fast to see the AI play fast. */
					end_turn();
				}

				/* SDL will spam KEYDOWN events when a key is just pressed for long enough,
				 * which will confuse whatever relies on matching KEYDOWN and KEYUP events.
				 * So we just filter out these 'fake' KEYDOWN events. */
				continue;
			}

			switch (event.type) {
				case SDL_QUIT:
					running = false;
				break;
				case SDL_KEYDOWN:
					if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL]) {
						if (keycode_is_arrow(event.key.keysym.sym) &&
							g_sel_ent_exists && ent_can_move(g_sel_ent_id)
						) {
							EntId eid = g_sel_ent_id;
							Ent* ent = get_ent(eid);
							if (ent != NULL &&
								ent->type == ENT_HUMAN &&
								ent_can_move(eid)
							) {
								DxDy dxdy = arrow_keycode_to_dxdy(event.key.keysym.sym);
								TileCoords dst_pos = tc_add_dxdy(ent->pos, dxdy);
								if (tile_coords_are_valid(dst_pos)) {
									Tile const* dst_tile = get_tile(dst_pos);
									if (tile_is_walkable(dst_tile)) {
										move_human(eid, dst_pos);
										ui_unselect_ent();
										break;
									}
								}
							}
						} else if (keycode_is_arrow(event.key.keysym.sym) &&
							(!g_sel_ent_exists) && g_sel_tile_exists
						) {
							DxDy dxdy = arrow_keycode_to_dxdy(event.key.keysym.sym);
							TileCoords new_sel_tc = tc_add_dxdy(g_sel_tile_coords, dxdy);
							if (tile_coords_are_valid(new_sel_tc)) {
								ui_select_tile(new_sel_tc);
								break;
							}
						}
					}
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
							running = false;
						break;
						case SDLK_DOWN:  g_camera.speed.y = +1.0f; break;
						case SDLK_UP:    g_camera.speed.y = -1.0f; break;
						case SDLK_RIGHT: g_camera.speed.x = +1.0f; break;
						case SDLK_LEFT:  g_camera.speed.x = -1.0f; break;
						case SDLK_l:
							g_render_lines = !g_render_lines;
						break;
						case SDLK_p:
							/* Test spawing entity on selected tile. */
							if (g_sel_tile_exists) {
								ent_new_test_block(g_sel_tile_coords,
									(SDL_Color){rand(), rand(), rand(), 255});
								refresh_selected_tile_ui();
							}
						break;
						case SDLK_n:
							/* Test spawing building on selected tile. */
							if (g_sel_tile_exists) {
								new_building(BUILDING_EMITTER, g_sel_tile_coords);
								update_surroundings(g_sel_tile_coords);
								refresh_selected_tile_ui();
							}
						break;
						case SDLK_b:
							/* Test spawing building on selected tile. */
							if (g_sel_tile_exists) {
								new_building(BUILDING_RECEIVER, g_sel_tile_coords);
								update_surroundings(g_sel_tile_coords);
								refresh_selected_tile_ui();
							}
						break;
						case SDLK_g:
							/* Test spawing cable on selected tile. */
							if (g_sel_tile_exists) {
								new_cable(g_sel_tile_coords, cable_orientation, (cable_orientation+2)%4);
								update_surroundings(g_sel_tile_coords);
							}
						break;
						case SDLK_h:
							/* Test spawing cable on selected tile. */
							if (g_sel_tile_exists) {
								new_cable(g_sel_tile_coords, cable_orientation, (cable_orientation+1)%4);
								update_surroundings(g_sel_tile_coords);
							}
						break;
						case SDLK_j:
							/* Test rotating cable to be placed on selected tile. */
							cable_orientation = (cable_orientation+1)%4;
						break;
						case SDLK_m:
							/* Test moving entities from selected tile to adjacent tiles. */
							if (g_sel_tile_exists) {
								Tile* sel_tile = get_tile(g_sel_tile_coords);
								for (int i = 0; i < sel_tile->ents.len; i++) {
									ent_move(sel_tile->ents.arr[i], tc_add_dxdy(g_sel_tile_coords,
										(DxDy){rand() % 3 - 1, rand() % 3 - 1}));
								}
								refresh_selected_tile_ui();
							}
						break;
						case SDLK_o:
							/* Test deleting entities on selected tile. */
							if (g_sel_tile_exists) {
								Tile* sel_tile = get_tile(g_sel_tile_coords);
								for (int i = 0; i < sel_tile->ents.len; i++) {
									ent_delete(sel_tile->ents.arr[i]);
								}
								refresh_selected_tile_ui();
							}
						break;
						case SDLK_i:
							if (g_sel_tile_exists) {
								EntId eid = ent_new_human(g_sel_tile_coords, FACTION_YELLOW);
								/* TEST: For now, humans spawn with wood in their inventory
								 * so that we can see that the inventory works. */
								ItemStack test_wood_stack =
									(ItemStack){.item = {.type = ITEM_WOOD_LOG}, .count = 1};
								DA_PUSH(&get_ent(eid)->human.inventory.stacks, test_wood_stack);
								refresh_selected_tile_ui();
							}
						break;
						case SDLK_u:
							if (g_sel_tile_exists) {
								ent_new_human(g_sel_tile_coords, FACTION_RED);
								refresh_selected_tile_ui();
							}
						break;
						case SDLK_TAB:
							cycle_ent_sel_through_ents_in_tile();
						break;
						case SDLK_SPACE:
							if (g_sel_ent_exists) {
								ui_unselect_ent();
							} else if (g_sel_tile_exists) {
								ui_unselect_tile();
							}
						break;
						case SDLK_RETURN:
							end_turn();
						break;
					}
				break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym) {
						/* Stop moving the camera when a key that
						 * was moving the camera is released. */
						case SDLK_DOWN:  if (g_camera.speed.y > 0) {g_camera.speed.y = 0.0f;}; break;
						case SDLK_UP:    if (g_camera.speed.y < 0) {g_camera.speed.y = 0.0f;}; break;
						case SDLK_RIGHT: if (g_camera.speed.x > 0) {g_camera.speed.x = 0.0f;}; break;
						case SDLK_LEFT:  if (g_camera.speed.x < 0) {g_camera.speed.x = 0.0f;}; break;
					}
				break;
				case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button) {
						case SDL_BUTTON_LEFT:
							click((WinCoords){event.button.x, event.button.y});
						break;
					}
				break;
				case SDL_MOUSEMOTION:
					action_menu_refresh();
					if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_RMASK) {
						/* The map is grabbed and the mouse is moving,
						 * so the map has to look like it follows the mouse
						 * (by moving the camera in the other direction). */
						g_camera.target_pos.x -= event.motion.xrel;
						g_camera.target_pos.y -= event.motion.yrel;
						g_camera.pos.x -= event.motion.xrel;
						g_camera.pos.y -= event.motion.yrel;
					}
				break;
				case SDL_MOUSEWHEEL:
					if (g_sel_ent_exists) {
						SDL_Point mouse;
						SDL_GetMouseState(&mouse.x, &mouse.y);
						TileCoords tc = window_pixel_to_tile_coords((WinCoords){mouse.x, mouse.y});
						if (tile_coords_are_valid(tc) && tile_is_available(tc)) {
							action_menu_scroll(-event.wheel.y);
							break;
						}
					}
					if (event.wheel.y > 0 && g_camera.target_zoom < ZOOM_MAX){
						g_camera.target_zoom /= WHEEL_ZOOM_FACTOR;
					} else if (event.wheel.y < 0 && g_camera.target_zoom > ZOOM_MIN) {
						g_camera.target_zoom *= WHEEL_ZOOM_FACTOR;
					}
				break;
			}
		}

		camera_update(dt);

		/* Background. */
		SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
		SDL_RenderClear(g_renderer);

		render_map();
		render_wg_tree();

		SDL_RenderPresent(g_renderer);
	}
	return 0;
}
