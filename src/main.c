#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <SDL2/SDL.h>

#include "renderer.h"
#include "map.h"
#include "camera.h"
#include "entity.h"

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

int main() {
	renderer_init();
	init_map();
	
	/* Is grid line display enabled? */
	bool render_lines = false;

	/* Selected tile, if any. */
	bool sel_tile_exists = false;
	TileCoords sel_tile_coords = {0, 0};

	/* Game loop iteration time monitoring. */
	Uint64 time = SDL_GetPerformanceCounter();
	Uint64 last_time = 0;

	/* Main game loop. */
	bool running = true;
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
							if (sel_tile_exists) {
								if (rand() % 2 == 0) {
									ent_new_human(sel_tile_coords,
										rand() % 2 == 0 ? FACTION_YELLOW : FACTION_RED);
								} else {
									ent_new_test_block(sel_tile_coords,
										(SDL_Color){rand(), rand(), rand(), 255});
								}
							}
						break;
						case SDLK_m:
							/* Test moving entities from selected tile to adjacent tiles. */
							if (sel_tile_exists) {
								Tile* sel_tile = get_tile(sel_tile_coords);
								for (int i = 0; i < sel_tile->ent_count; i++) {
									ent_move(sel_tile->ents[i],
										(TileCoords){
											sel_tile_coords.x + rand() % 3 - 1,
											sel_tile_coords.y + rand() % 3 - 1});
								}
							}
						break;
						case SDLK_o:
							/* Test deleting entities on selected tile. */
							if (sel_tile_exists) {
								Tile* sel_tile = get_tile(sel_tile_coords);
								for (int i = 0; i < sel_tile->ent_count; i++) {
									ent_delete(sel_tile->ents[i]);
								}
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
							TileCoords tc = window_pixel_to_tile_coords(wc);
							bool sel_tile_is_alrady_tc =
								sel_tile_exists && tile_coords_eq(sel_tile_coords, tc);
							if (tile_coords_are_valid(tc) && !sel_tile_is_alrady_tc) {
								sel_tile_exists = true;
								sel_tile_coords = tc;
							} else {
								sel_tile_exists = false;
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
			TileCoords tc = {.x = i % N_TILES_W, .y = i / N_TILES_W};
			Tile const* tile = get_tile(tc);

			SDL_Rect dst_rect = {
				.x = tc.x * tile_render_size - g_camera.pos.x,
				.y = tc.y * tile_render_size - g_camera.pos.y,
				.w = ceilf(tile_render_size),
				.h = ceilf(tile_render_size)};
			render_tile_ground(tile->type, dst_rect);

			for (int ent_i = 0; ent_i < tile->ent_count; ent_i++) {
				EntId eid = (tile->ents[ent_i]);
				Ent* ent = get_ent(eid);
				if (ent == NULL) continue;

				int ex = (float)(ent_i+1) / (float)(tile->ent_count+1)
					* tile_render_size;
				int ey = (1.0f - (float)(ent_i+1) / (float)(tile->ent_count+1))
					* tile_render_size;

				switch (ent->type) {
					case ENT_HUMAIN: {
						int ew = 0.1f * tile_render_size;
						int eh = 0.3f * tile_render_size;
						SDL_Rect rect = {
							dst_rect.x + ex - ew / 2.0f, dst_rect.y + ey - eh / 2.0f, ew, eh};
						EntDataHuman* data = ent->data;
						switch (data->faction) {
							case FACTION_YELLOW:
								SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255);
							break;
							case FACTION_RED:
								SDL_SetRenderDrawColor(g_renderer, 255,   0, 0, 255);
							break;
							default: assert(false);
						}
						SDL_RenderFillRect(g_renderer, &rect);
					break; }

					case ENT_TEST_BLOCK: {
						int ew = 0.2f * tile_render_size;
						int eh = 0.2f * tile_render_size;
						SDL_Rect rect = {
							dst_rect.x + ex - ew / 2.0f, dst_rect.y + ey - eh / 2.0f, ew, eh};
						EntDataTestBlock* data = ent->data;
						SDL_SetRenderDrawColor(g_renderer,
							data->color.r, data->color.g, data->color.b, 255);
						SDL_RenderFillRect(g_renderer, &rect);
					break; }

					default: assert(false);
				}
			}
		}

		/* Draw grid lines if enabled. */
		if (render_lines) {
			SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
			for (int y = 0; y < N_TILES_W+1; ++y) {
				WinCoords a = tile_coords_to_window_pixel((TileCoords){0, y});
				WinCoords b = tile_coords_to_window_pixel((TileCoords){N_TILES_W, y});
				SDL_RenderDrawLine(g_renderer, a.x, a.y,   b.x, b.y);
				SDL_RenderDrawLine(g_renderer, a.x, a.y-1, b.x, b.y-1);
			}
			for (int x = 0; x < N_TILES_H+1; ++x) {
				WinCoords a = tile_coords_to_window_pixel((TileCoords){x, 0});
				WinCoords b = tile_coords_to_window_pixel((TileCoords){x, N_TILES_H});
				SDL_RenderDrawLine(g_renderer, a.x,   a.y, b.x,   b.y);
				SDL_RenderDrawLine(g_renderer, a.x-1, a.y, b.x-1, b.y);
			}
		}

		if (sel_tile_exists) {
			/* Draw selection rect around the selected tile. */
			SDL_Rect rect = {
				.x = sel_tile_coords.x * tile_render_size - g_camera.pos.x,
				.y = sel_tile_coords.y * tile_render_size - g_camera.pos.y,
				.w = ceilf(tile_render_size),
				.h = ceilf(tile_render_size)};
			SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255);
			SDL_RenderDrawRect(g_renderer, &rect);

			Tile const* sel_tile = 
				&g_grid[sel_tile_coords.y * N_TILES_W + sel_tile_coords.x];

			/* TODO: Redo the following lame UI stuff with the ui-dev's branch widgets. */

			/* Draw the selected tile information in a corner. */
			SDL_Rect ui_rect = {.x = 10, .y = WINDOW_H - 190, .w = 150, .h = 180};
			SDL_SetRenderDrawColor(g_renderer, 200, 200, 200, 255);
			SDL_RenderFillRect(g_renderer, &ui_rect);
			SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
			SDL_RenderDrawRect(g_renderer, &ui_rect);
			rect = (SDL_Rect){.x = 35, .y = WINDOW_H - 135, .w = 100, .h = 100};
			render_tile_ground(sel_tile->type, rect);
			char const* name = g_tile_type_spec_table[sel_tile->type].name;
			render_string(name,
				(WinCoords){10 + 150/2, WINDOW_H - 175}, PP_TOP_CENTER,
				(SDL_Color){0, 0, 0, 255});

			for (int ent_i = 0; ent_i < sel_tile->ent_count; ent_i++) {
				EntId eid = (sel_tile->ents[ent_i]);
				Ent* ent = get_ent(eid);
				if (ent == NULL) continue;

				ui_rect.x += ui_rect.w + 10;
				SDL_SetRenderDrawColor(g_renderer, 200, 200, 200, 255);
				SDL_RenderFillRect(g_renderer, &ui_rect);
				SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
				SDL_RenderDrawRect(g_renderer, &ui_rect);

				switch (ent->type) {
					case ENT_HUMAIN: {
						render_string("Human",
							(WinCoords){ui_rect.x + ui_rect.w/2, WINDOW_H - 175}, PP_TOP_CENTER,
							(SDL_Color){0, 0, 0, 255});
						EntDataHuman* data = ent->data;
						char const* faction_name;
						switch (data->faction) {
							case FACTION_YELLOW: faction_name = "Yellow"; break;
							case FACTION_RED:    faction_name = "Red";    break;
							default: assert(false);
						}
						render_string(faction_name,
							(WinCoords){ui_rect.x + ui_rect.w/2, WINDOW_H - 175 + 40}, PP_TOP_CENTER,
							(SDL_Color){0, 0, 0, 255});
					break; }

					case ENT_TEST_BLOCK: {
						render_string("Test Block",
							(WinCoords){ui_rect.x + ui_rect.w/2, WINDOW_H - 175}, PP_TOP_CENTER,
							(SDL_Color){0, 0, 0, 255});
						EntDataTestBlock* data = ent->data;
						SDL_SetRenderDrawColor(g_renderer,
							data->color.r, data->color.g, data->color.b, 255);
						rect.w = ui_rect.w / 2;
						rect.h = rect.w;
						rect.x = ui_rect.x + ui_rect.w / 2 - rect.w / 2;
						rect.y = ui_rect.y + ui_rect.h / 2 - rect.h / 2;
						SDL_RenderFillRect(g_renderer, &rect);
					break; }

					default: assert(false);
				}
			}
		}
		SDL_RenderPresent(g_renderer);
	}
	return 0;
}
