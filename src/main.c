#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "renderer.h"
#include "map.h"
#include "camera.h"
#include "widget.h"
#include "entity.h"
#include "ui.h"

TileCoords window_pixel_to_tile_coords(WinCoords wc) {
	float tile_render_size = TILE_SIZE * g_camera.zoom;
	return (TileCoords){
		.x = floorf(((float)wc.x + g_camera.pos.x) / tile_render_size),
		.y = floorf(((float)wc.y + g_camera.pos.y) / tile_render_size),
	};
}

WinCoords tile_coords_to_window_pixel(TileCoords tc) {
	float tile_render_size = TILE_SIZE * g_camera.zoom;
	return (WinCoords){
		.x = tc.x * tile_render_size - g_camera.pos.x,
		.y = tc.y * tile_render_size - g_camera.pos.y,
	};
}

void render_tile_ground(TileType tile_type, SDL_Rect dst_rect) {
	SDL_Rect rect_in_spritesheet = g_tile_type_spec_table[tile_type].rect_in_spritesheet;
	SDL_RenderCopy(g_renderer, g_spritesheet, &rect_in_spritesheet, &dst_rect);
}

void render_human(SDL_Rect dst_rect) {
	SDL_Rect rect_in_spritesheet = {0, 16, 3, 4};
	SDL_RenderCopy(g_renderer, g_spritesheet, &rect_in_spritesheet, &dst_rect);
}

void render_tile_building(Building * building, SDL_Rect dst_rect) {
	if (building != NULL){
		SDL_Rect rect_in_spritesheet;
		switch (building->type)
		{
		case BUILDING_EMITTER:
			rect_in_spritesheet = g_building_type_spec_table[BUILDING_TX_EMITTER].rect_in_spritesheet;
			break;
		case BUILDING_RECEIVER:
			if (building->powered){
				rect_in_spritesheet = g_building_type_spec_table[BUILDING_TX_RECEIVER_ON].rect_in_spritesheet;
			} else {
				rect_in_spritesheet = g_building_type_spec_table[BUILDING_TX_RECEIVER_OFF].rect_in_spritesheet;
			}
			break;
		default:
			break;
		}
		SDL_RenderCopy(g_renderer, g_spritesheet, &rect_in_spritesheet, &dst_rect);
	} else {
		printf("[ERROR] Missing building !");
	}
}

void setFlow(Tile * tile, bool powered, CardinalType entry) {
	assert(tile != NULL);
	Flow * flow;
	for (int i=0; i<tile->flow_count; i++){
		flow = tile->flows[i];
		if (flow->connections[0] == entry || flow->connections[1] == entry){
			flow->powered = powered;
		}
	}
	if (tile->building != NULL && tile->building->type == BUILDING_RECEIVER) {
		tile->building->powered = powered;
	}
}

void render_tile_flow(Flow * flow, SDL_Rect dst_rect) {
	if (flow != NULL){
		SDL_Rect rect_in_spritesheet;
		int angle = 0;
		bool straight = flow->connections[1] - flow->connections[0] == 2;
		switch (flow->type)
		{
		case ELECTRICITY:
			// Cringe af but is just to test. The true way of storing and evaluating directions must be discussed.
			if (straight){
				rect_in_spritesheet = g_flow_type_spec_table[ELECTRICITY_STRAIGHT].rect_in_spritesheet;
				angle = 90 * (flow->connections[0] == SOUTH);
			} else {
				rect_in_spritesheet = g_flow_type_spec_table[ELECTRICITY_TURN].rect_in_spritesheet;
				angle = 90 * (4-flow->connections[1]) * !(flow->connections[0] == WEST && flow->connections[1] == NORTH);
			}
			SDL_RenderCopyEx(g_renderer, g_spritesheet, &rect_in_spritesheet, &dst_rect, angle, NULL, SDL_FLIP_NONE);
			break;
		default:
			break;
		}
	} else {
		printf("[ERROR] Missing flow !");
	}
}

int main(int argc, char const** argv) {
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

	/* Is grid line display enabled? */
	bool render_lines = false;

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

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN && event.key.repeat) {
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
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
							running = false;
						break;
						case SDLK_DOWN:  g_camera.speed.y = +1; break;
						case SDLK_UP:    g_camera.speed.y = -1; break;
						case SDLK_RIGHT: g_camera.speed.x = +1; break;
						case SDLK_LEFT:  g_camera.speed.x = -1; break;
						case SDLK_l:
							render_lines = !render_lines;
						break;
						case SDLK_p:
							/* Test spawing entity on selected tile. */
							if (g_sel_tile_exists) {
								if (rand() % 2 == 0) {
									ent_new_human(g_sel_tile_coords,
										rand() % 2 == 0 ? FACTION_YELLOW : FACTION_RED);
								} else {
									ent_new_test_block(g_sel_tile_coords,
										(SDL_Color){rand(), rand(), rand(), 255});
								}
								refresh_selected_tile_ui();
							}
						break;
						case SDLK_n:
							/* Test spawing building on selected tile. */
							if (g_sel_tile_exists) {
								new_building(BUILDING_EMITTER, g_sel_tile_coords);
							}
						break;
						case SDLK_b:
							/* Test spawing building on selected tile. */
							if (g_sel_tile_exists) {
								new_building(BUILDING_RECEIVER, g_sel_tile_coords);
							}
						break;
						case SDLK_g:
							/* Test spawing cable on selected tile. */
							if (g_sel_tile_exists) {
								new_flow(ELECTRICITY, g_sel_tile_coords, cable_orientation, (cable_orientation+2)%4);
							}
						break;
						case SDLK_h:
							/* Test spawing cable on selected tile. */
							if (g_sel_tile_exists) {
								new_flow(ELECTRICITY, g_sel_tile_coords, cable_orientation, (cable_orientation+1)%4);
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
								for (int i = 0; i < sel_tile->ent_count; i++) {
									ent_move(sel_tile->ents[i],
										(TileCoords){
											g_sel_tile_coords.x + rand() % 3 - 1,
											g_sel_tile_coords.y + rand() % 3 - 1});
								}
								refresh_selected_tile_ui();
							}
						break;
						case SDLK_o:
							/* Test deleting entities on selected tile. */
							if (g_sel_tile_exists) {
								Tile* sel_tile = get_tile(g_sel_tile_coords);
								for (int i = 0; i < sel_tile->ent_count; i++) {
									ent_delete(sel_tile->ents[i]);
								}
								refresh_selected_tile_ui();
							}
						break;
						case SDLK_i:
							if (g_sel_tile_exists) {
								ent_new_human(g_sel_tile_coords, FACTION_YELLOW);
							}
						break;
						case SDLK_u:
							if (g_sel_tile_exists) {
								ent_new_human(g_sel_tile_coords, FACTION_RED);
							}
						break;
						case SDLK_TAB:
							/* Select entity in selected tile or something. */
							if (g_sel_tile_exists) {
								Tile* sel_tile = get_tile(g_sel_tile_coords);
								bool sel_eid_is_in_sel_tile = false;
								int sel_eid_in_sel_tile_index = -1;
								for (int i = 0; i < sel_tile->ent_count; i++) {
									EntId eid = sel_tile->ents[i];
									if (eid_eq(g_sel_ent_id, eid)) {
										sel_eid_is_in_sel_tile = true;
										sel_eid_in_sel_tile_index = i;
										break;
									}
								}
								int start_index = (sel_eid_in_sel_tile_index + 1) % sel_tile->ent_count;
								int i = start_index;
								do {
									EntId eid = sel_tile->ents[i];
									Ent* ent = get_ent(eid);
									if (ent == NULL) continue;
									if (ent->type != ENT_HUMAIN) continue;
									if (ent->human.faction != g_faction_currently_playing) continue;
									ui_select_ent(eid);
									break;
									i = (i + 1) % sel_tile->ent_count;
								} while (i != start_index);
							}
						break;
					}
				break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym) {
						/* Stop moving the camera when a key that
						* was moving the camera is released. */
						case SDLK_DOWN:  if (g_camera.speed.y > 0) {g_camera.speed.y = 0;}; break;
						case SDLK_UP:    if (g_camera.speed.y < 0) {g_camera.speed.y = 0;}; break;
						case SDLK_RIGHT: if (g_camera.speed.x > 0) {g_camera.speed.x = 0;}; break;
						case SDLK_LEFT:  if (g_camera.speed.x < 0) {g_camera.speed.x = 0;}; break;
					}
				break;
				case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button) {
						case SDL_BUTTON_LEFT:;
							WinCoords wc = {event.button.x, event.button.y};
							bool some_wg_got_the_click = wg_click(g_wg_root, 0, 0, wc.x, wc.y);
							if (!some_wg_got_the_click) {
								TileCoords tc = window_pixel_to_tile_coords(wc);
								bool sel_tile_is_alrady_tc =
									g_sel_tile_exists && tile_coords_eq(g_sel_tile_coords, tc);
								if (tile_coords_are_valid(tc) && !sel_tile_is_alrady_tc) {
									ui_select_tile(tc);
								} else {
									ui_unselect_tile();
								}
							}
						break;
					}
				break;
				case SDL_MOUSEMOTION:
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
					if (event.wheel.y > 0 && g_camera.target_zoom < ZOOM_MAX){
						g_camera.target_zoom /= 0.8f;
					} else if (event.wheel.y < 0 && g_camera.target_zoom > ZOOM_MIN) {
						g_camera.target_zoom *= 0.8f;
					}
				break;
			}
		}

		camera_update(dt);
		
		/* Background. */
		SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
		SDL_RenderClear(g_renderer);

		/* Draw tiles. */
		float tile_render_size = TILE_SIZE * g_camera.zoom;
		for (int i = 0; i < N_TILES; ++i) {
			TileCoords tc = {.x = i % g_map_w, .y = i / g_map_w};
			Tile const* tile = get_tile(tc);

			SDL_Rect dst_rect = {
				.x = tc.x * tile_render_size - g_camera.pos.x,
				.y = tc.y * tile_render_size - g_camera.pos.y,
				.w = ceilf(tile_render_size),
				.h = ceilf(tile_render_size)};
			render_tile_ground(tile->type, dst_rect);

			if (g_sel_tile_exists && tile_coords_eq(g_sel_tile_coords, tc)) {
				/* Draw selection rect around the selected tile. */
				SDL_Rect rect = dst_rect;
				SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255);
				for (int i = 0; i < 2; i++) {
					SDL_RenderDrawRect(g_renderer, &rect);
					rect.x += 1; rect.y += 1; rect.w -= 2; rect.h -= 2;
				}
			}
			
			for (int flow_i = 0; flow_i < tile->flow_count; flow_i++){
				Flow* flow = tile->flows[flow_i];
				assert(flow != NULL);
				render_tile_flow(flow, dst_rect);
				if(flow->powered){
					
					TileCoords neighPosFLow;
					Tile * neighTile;
					for (int connection_i=0; connection_i < 2; connection_i++){
						neighPosFLow.x = tc.x;
						neighPosFLow.y = tc.y;
						switch (flow->connections[connection_i]){
							case NORTH:
								neighPosFLow.x += 0;
								neighPosFLow.y += -1;

							break;
							case SOUTH:
								neighPosFLow.x += 0;
								neighPosFLow.y += 1;
							break;
							case EAST:
								neighPosFLow.x += 1;
								neighPosFLow.y += 0;
							break;
							case WEST:
								neighPosFLow.x += -1;
								neighPosFLow.y += 0;
							break;
							
							default:
							break;
						}
						neighTile = get_tile(neighPosFLow);
						setFlow(neighTile, true, getOpposedDirection(flow->connections[connection_i]));
					}
				}
			}

			if (tile->building != NULL){
				render_tile_building(tile->building, dst_rect);
				if (tile->building->type == BUILDING_EMITTER){
					int offsets[] = {1, 0, -1, 0, 1};
					/* Uses the array 'offsets' to get the four adjacent tiles.
						The order is EAST (1,0), NORTH (0,-1), WEST (-1, 0) and SOUTH (0, 1)
					*/
					for (int tile_i=0; tile_i<4; tile_i++){
						TileCoords neighPos = {tc.x+offsets[tile_i], tc.y+offsets[tile_i+1]};							
						Tile * neighTile = get_tile(neighPos);
						setFlow(neighTile, true, (CardinalType)(tile_i));
						
					}
				}
			}

			int true_ent_count = 0;
			for (int ent_i = 0; ent_i < tile->ent_count; ent_i++) {
				if (get_ent(tile->ents[ent_i]) != NULL) {
					true_ent_count++;
				}
			}

			int true_ent_i = -1;
			for (int ent_i = 0; ent_i < tile->ent_count; ent_i++) {
				EntId eid = (tile->ents[ent_i]);
				Ent* ent = get_ent(eid);
				if (ent == NULL) continue;
				true_ent_i++;

				int ex = (float)(true_ent_i+1) / (float)(true_ent_count+1)
					* tile_render_size;
				int ey = (1.0f - (float)(true_ent_i+1) / (float)(true_ent_count+1))
					* tile_render_size;

				switch (ent->type) {
					case ENT_HUMAIN: {
						int ew = 0.3f * tile_render_size;
						int eh = 0.4f * tile_render_size;
						/* Draw human sprite. */
						int ex = (float)(ent_i+1) / (float)(tile->ent_count+1)
							* tile_render_size;
						int ey = (1.0f - (float)(ent_i+1) / (float)(tile->ent_count+1))
							* tile_render_size;
						SDL_Rect rect = {
							dst_rect.x + ex - ew / 2.0f, dst_rect.y + ey - eh / 2.0f, ew, eh};
						render_human(rect);
						/* Draw faction color. */
						switch (ent->human.faction) {
							case FACTION_YELLOW:
								SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255);
							break;
							case FACTION_RED:
								SDL_SetRenderDrawColor(g_renderer, 255,   0, 0, 255);
							break;
							default: assert(false);
						}
						#define FACTION_SIDE 8
						SDL_Rect faction_rect = {
							rect.x + rect.w/2 - FACTION_SIDE/2, rect.y - FACTION_SIDE/2 - FACTION_SIDE,
							FACTION_SIDE, FACTION_SIDE};
						if (g_sel_ent_exists && eid_eq(g_sel_ent_id, eid)) {
							faction_rect.y -= 2;
						}
						SDL_RenderFillRect(g_renderer, &faction_rect);
						if (g_sel_ent_exists && eid_eq(g_sel_ent_id, eid)) {
							SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
							for (int i = 0; i < 2; i++) {
								faction_rect.x-=1; faction_rect.y-=1; faction_rect.w+=2; faction_rect.h+=2;
								SDL_RenderDrawRect(g_renderer, &faction_rect);
							}
							SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
							for (int i = 0; i < 2; i++) {
								faction_rect.x-=1; faction_rect.y-=1; faction_rect.w+=2; faction_rect.h+=2;
								SDL_RenderDrawRect(g_renderer, &faction_rect);
							}
						}
					break; }

					case ENT_TEST_BLOCK: {
						int ew = 0.2f * tile_render_size;
						int eh = 0.2f * tile_render_size;
						SDL_Rect rect = {
							dst_rect.x + ex - ew / 2.0f, dst_rect.y + ey - eh / 2.0f, ew, eh};
						SDL_Color color = ent->test_block.color;
						SDL_SetRenderDrawColor(g_renderer, color.r, color.g, color.b, 255);
						SDL_RenderFillRect(g_renderer, &rect);
					break; }

					default: assert(false);
				}
			}
		}

		if (render_lines) {
			/* Draw grid lines. */
			SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
			for (int y = 0; y < g_map_w+1; ++y) {
				WinCoords a = tile_coords_to_window_pixel((TileCoords){0, y});
				WinCoords b = tile_coords_to_window_pixel((TileCoords){g_map_w, y});
				SDL_RenderDrawLine(g_renderer, a.x, a.y,   b.x, b.y);
				SDL_RenderDrawLine(g_renderer, a.x, a.y-1, b.x, b.y-1);
			}
			for (int x = 0; x < g_map_h+1; ++x) {
				WinCoords a = tile_coords_to_window_pixel((TileCoords){x, 0});
				WinCoords b = tile_coords_to_window_pixel((TileCoords){x, g_map_h});
				SDL_RenderDrawLine(g_renderer, a.x,   a.y, b.x,   b.y);
				SDL_RenderDrawLine(g_renderer, a.x-1, a.y, b.x-1, b.y);
			}
		}

		wg_render(g_wg_root, 0, 0);

		SDL_RenderPresent(g_renderer);
	}
	return 0;
}
