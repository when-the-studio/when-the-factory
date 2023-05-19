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

void render_tile_building(Building* building, SDL_Rect dst_rect) {
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
		SDL_RenderCopy(g_renderer, g_spritesheet_buildings, &rect_in_spritesheet, &dst_rect);
	} else {
		printf("[ERROR] Missing building !");
	}
}

void render_tile_cable(Cable* cable, SDL_Rect dst_rect) {
	if (cable != NULL){
		SDL_Rect rect_in_spritesheet;
		int angle = 0;
		bool straight = cable->connections[1] - cable->connections[0] == 2;
			// Cringe af but is just to test. The true way of storing and evaluating directions must be discussed.
			if (straight){
				rect_in_spritesheet = g_cable_type_spec_table[ELECTRICITY_STRAIGHT].rect_in_spritesheet;
				if (cable->powered){
					rect_in_spritesheet = g_cable_type_spec_table[ELECTRICITY_STRAIGHT_ON].rect_in_spritesheet;
				}

				angle = 90 * (cable->connections[0] == SOUTH);
			} else {
				rect_in_spritesheet = g_cable_type_spec_table[ELECTRICITY_TURN].rect_in_spritesheet;
				if (cable->powered){
					rect_in_spritesheet = g_cable_type_spec_table[ELECTRICITY_TURN_ON].rect_in_spritesheet;
				}
				angle = 90 * (4-cable->connections[1]) * !(cable->connections[0] == WEST && cable->connections[1] == NORTH);
			}
			SDL_RenderCopyEx(g_renderer, g_spritesheet_buildings, &rect_in_spritesheet, &dst_rect, angle, NULL, SDL_FLIP_NONE);
	}
}

void end_turn(void);
void move_human(EntId eid, TileCoords dst_pos);

void cycle_ent_sel_through_ents_in_tile(void);

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

bool keycode_is_arrow(SDL_Keycode keycode) {
	return
		keycode == SDLK_DOWN ||
		keycode == SDLK_UP ||
		keycode == SDLK_RIGHT ||
		keycode == SDLK_LEFT;
}

typedef struct DxDy {int dx, dy;} DxDy;

TileCoords tc_add_dxdy(TileCoords tc, DxDy dxdy) {
	return (TileCoords){tc.x + dxdy.dx, tc.y + dxdy.dy};
}

DxDy arrow_keycode_to_dxdy(SDL_Keycode keycode) {
	switch (keycode) {
		case SDLK_DOWN:  return (DxDy){0, +1};
		case SDLK_UP:    return (DxDy){0, -1};
		case SDLK_RIGHT: return (DxDy){+1, 0};
		case SDLK_LEFT:  return (DxDy){-1, 0};
		default: assert(false); exit(EXIT_FAILURE);
	}
}

DxDy cardinal_to_dxdy(CardinalType card) {
	switch (card) {
		case SOUTH: return (DxDy){0, +1};
		case NORTH: return (DxDy){0, -1};
		case WEST:  return (DxDy){+1, 0};
		case EAST:  return (DxDy){-1, 0};
		default: assert(false); exit(EXIT_FAILURE);
	}
}

void cycle_ent_sel_through_ents_in_tile(void) {
	if (!g_sel_tile_exists) return;
	Tile* sel_tile = get_tile(g_sel_tile_coords);

	/* Here we only want to select entities that belong to the currently playing faction.
	 * `first` is the first of such entities (if any).
	 * `first_after_sel` is the first of such that follows the currently selected entity (if any). */
	EntId first = EID_NULL;
	bool sel_ent_is_in_sel_tile = false;
	EntId first_after_sel = EID_NULL;
	for (int i = 0; i < sel_tile->ents.len; i++) {
		EntId eid = sel_tile->ents.arr[i];
		if (!ent_is_playing(eid)) continue;
		if (eid_null(first)) {
			first = eid;
		}
		if (eid_eq(g_sel_ent_id, eid)) {
			sel_ent_is_in_sel_tile = true;
		} else if (sel_ent_is_in_sel_tile &&
			eid_null(first_after_sel)
		) {
			first_after_sel = eid;
		}
	}

	/* Now we just select the next of such entities if that makes sense,
	 * else we just select the first (if any). */
	if (sel_ent_is_in_sel_tile && eid_null(first_after_sel)) {
		ui_select_ent(first);
	} else if (sel_ent_is_in_sel_tile) {
		ui_select_ent(first_after_sel);
	} else if (!eid_null(first)) {
		ui_select_ent(first);
	} else {
		ui_unselect_ent();
	}
}

SDL_Rect tile_rect(TileCoords tc) {
	float tile_render_size = TILE_SIZE * g_camera.zoom;
	return (SDL_Rect){
		.x = tc.x * tile_render_size - g_camera.pos.x,
		.y = tc.y * tile_render_size - g_camera.pos.y,
		.w = ceilf(tile_render_size),
		.h = ceilf(tile_render_size)};
}

int get_tile_real_ent_count(Tile const* tile) {
	int true_ent_count = 0;
	for (int ent_i = 0; ent_i < tile->ents.len; ent_i++) {
		if (get_ent(tile->ents.arr[ent_i]) != NULL) {
			true_ent_count++;
		}
	}
	return true_ent_count;
}

/* Is grid line display enabled? */
bool g_render_lines = false;

Uint64 g_time_ms = 0;


void render_map(void) {
	float tile_render_size = TILE_SIZE * g_camera.zoom;

	/* Draw tile terrain. */
	for (int i = 0; i < N_TILES; ++i) {
		TileCoords tc = {.x = i % g_map_w, .y = i / g_map_w};
		Tile const* tile = get_tile(tc);
		SDL_Rect rect = tile_rect(tc);
		render_tile_ground(tile->type, rect);
	}

	if (g_render_lines) {
		/* Draw grid lines. */
		SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
		for (int y = 0; y < g_map_w+1; ++y) {
			WinCoords a = tile_coords_to_window_pixel((TileCoords){0, y});
			WinCoords b = tile_coords_to_window_pixel((TileCoords){g_map_w, y});
			for (int i = 0; i < 2; i++) {
				SDL_RenderDrawLine(g_renderer, a.x, a.y - i, b.x, b.y - i);
			}
		}
		for (int x = 0; x < g_map_h+1; ++x) {
			WinCoords a = tile_coords_to_window_pixel((TileCoords){x, 0});
			WinCoords b = tile_coords_to_window_pixel((TileCoords){x, g_map_h});
			for (int i = 0; i < 2; i++) {
				SDL_RenderDrawLine(g_renderer, a.x - i, a.y, b.x - i, b.y);
			}
		}
	}

	/* Draw selection rect around the selected tile (if any).
	 * We do this after all the tile terrains so that terrain drawn after the
	 * selection rectangle won't cover a part of it. */
	if (g_sel_tile_exists) {
		SDL_Rect rect = tile_rect(g_sel_tile_coords);
		SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255);
		for (int i = 0; i < 3; i++) {
			SDL_RenderDrawRect(g_renderer, &rect);
			rect.x += 1; rect.y += 1; rect.w -= 2; rect.h -= 2;
		}
	}

	for (int i = 0; i < g_action_da_on_tcs.len; i++) {
		SDL_Rect rect = tile_rect(g_action_da_on_tcs.arr[i].tc);
		SDL_Point mouse;
		SDL_GetMouseState(&mouse.x, &mouse.y);
		if (SDL_PointInRect(&mouse, &rect)) {
			SDL_SetRenderDrawColor(g_renderer, 0, 255, 255, 255);
		} else {
			SDL_SetRenderDrawColor(g_renderer, 0, 0, 255, 255);
		}
		for (int i = 0; i < 2; i++) {
			rect.x += 1; rect.y += 1; rect.w -= 2; rect.h -= 2;
			SDL_RenderDrawRect(g_renderer, &rect);
		}
	}

	/* Draw entities, buildings and flows.
	 * We do this after the tile backgrounds so that these can cover a part
	 * of neighboring tiles. */
	for (int i = 0; i < N_TILES; ++i) {
		TileCoords tc = {.x = i % g_map_w, .y = i / g_map_w};
		Tile const* tile = get_tile(tc);
		SDL_Rect rect = tile_rect(tc);

		/* Draw flows. */
		for (int cable_i = 0; cable_i < tile->cable_count; cable_i++){
			Cable* cable = tile->cables[cable_i];
			assert(cable != NULL);
			render_tile_cable(cable, rect);
		}

		/* Draw building. */
		if (tile->building != NULL){
			render_tile_building(tile->building, rect);
		}

		/* Draw entities.
		 * Note that `tile->ents.arr` can contain empty cells, so we cannot count entities
		 * with this dynamic array and must use `real_ent_count` and `real_ent_i` instead. */
		int real_ent_count = get_tile_real_ent_count(tile);
		int real_ent_i = -1;
		for (int ent_i = 0; ent_i < tile->ents.len; ent_i++) {
			EntId eid = (tile->ents.arr[ent_i]);
			Ent* ent = get_ent(eid);
			if (ent == NULL) continue;
			real_ent_i++;

			/* Entity position on screen. */
			int ex = (float)(real_ent_i+1) / (float)(real_ent_count+1)
				* tile_render_size;
			int ey = (1.0f - (float)(real_ent_i+1) / (float)(real_ent_count+1))
				* tile_render_size;

			/* Account for an ongoing animation (if any). */
			if (ent->anim != NULL &&
				ent->anim->time_beginning <= g_time_ms && g_time_ms < ent->anim->time_end
			) {
				float interpolation =
					(float)(g_time_ms - ent->anim->time_beginning) /
					(float)(ent->anim->time_end - ent->anim->time_beginning);
				/* The offset is maximal at the beginning of the animation (when `interpolation`
				 * is zero) and tends toward zero as `interpolation` tends toward one. */
				ex -= ent->anim->offset_beginning_x * tile_render_size * (1.0f - interpolation);
				ey -= ent->anim->offset_beginning_y * tile_render_size * (1.0f - interpolation);
			}

			switch (ent->type) {
				case ENT_HUMAN: {
					int ew = 0.3f * tile_render_size;
					int eh = 0.4f * tile_render_size;

					/* Draw human sprite. */
					SDL_Rect ent_rect = {
						rect.x + ex - ew / 2.0f, rect.y + ey - eh / 2.0f, ew, eh};
					if (g_sel_ent_exists && eid_eq(g_sel_ent_id, eid)) {
						/* If the entity is selected, then make it a bit bigger
						 * its fixed point is its bottom middle point as its looks better that way. */
						ent_rect.x -= 2; ent_rect.w += 4;
						ent_rect.y -= 4; ent_rect.h += 4;
					}
					render_human(ent_rect);

					/* Draw faction color square. */
					SDL_Color color = g_faction_spec_table[ent->human.faction].color;
					SDL_SetRenderDrawColor(g_renderer, color.r, color.g, color.b, 255);
					#define FACTION_SIDE 8
					SDL_Rect faction_rect = {
						ent_rect.x + ent_rect.w/2 - FACTION_SIDE/2,
						ent_rect.y - FACTION_SIDE/2 - FACTION_SIDE,
						FACTION_SIDE, FACTION_SIDE};
					#undef FACTION_SIDE
					if (g_sel_ent_exists && eid_eq(g_sel_ent_id, eid)) {
						faction_rect.y -= 2;
					}
					SDL_RenderFillRect(g_renderer, &faction_rect);

					/* Draw some black and white boarders around the faction color square of the
					 * selected entity (if any). */
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
					SDL_Rect ent_rect = {rect.x + ex - ew / 2.0f, rect.y + ey - eh / 2.0f, ew, eh};
					SDL_Color color = ent->test_block.color;
					SDL_SetRenderDrawColor(g_renderer, color.r, color.g, color.b, 255);
					SDL_RenderFillRect(g_renderer, &ent_rect);
				break; }

				default: assert(false);
			}
		}
	}
}

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
