#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <SDL2/SDL.h>

#include "renderer.h"
#include "map.h"
#include "camera.h"

TileCoords window_pixel_to_tile_coords(WinCoords wc) {
	float tile_render_size = TILE_SIZE * g_camera.zoom;
	return (TileCoords){
		.x = floorf(((float)wc.x + g_camera.pos.x) / tile_render_size),
		.y = floorf(((float)wc.y + g_camera.pos.y) / tile_render_size),
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
	/* DISCUSS: Abreviations use and conventions and all. */
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
						case SDL_BUTTON_LEFT: {
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
						} break;
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

		cam_update(dt);
		
		/* Background. */
		SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
		SDL_RenderClear(g_renderer);

		/* Draw tiles. */
		float tile_render_size = TILE_SIZE * g_camera.zoom;
		for (int i = 0; i < N_TILES; ++i) {
			int x = i % N_TILES_W;
			int y = i / N_TILES_W;
			SDL_Rect dst_rect = {
				.x = x * tile_render_size - g_camera.pos.x,
				.y = y * tile_render_size - g_camera.pos.y,
				.w = ceilf(tile_render_size),
				.h = ceilf(tile_render_size)};
			render_tile_ground(g_grid[i].type, dst_rect);
		}

		/* Draw grid lines if enabled. */
		if (render_lines) {
			SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
			for (int row = 1; row < N_TILES_W; ++row) {
				int x = row * tile_render_size - g_camera.pos.x;
				SDL_RenderDrawLine(g_renderer, x,   0, x,   WINDOW_H);
				SDL_RenderDrawLine(g_renderer, x-1, 0, x-1, WINDOW_H);
			}
			for (int col = 1; col < N_TILES_H; ++col) {
				int y = col * tile_render_size - g_camera.pos.y;
				SDL_RenderDrawLine(g_renderer, 0, y,   WINDOW_W, y);
				SDL_RenderDrawLine(g_renderer, 0, y-1, WINDOW_W, y-1);
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

			/* Draw the selected tile information in a corner. */
			{
				rect = (SDL_Rect){.x = 10, .y = WINDOW_H - 190, .w = 150, .h = 180};
				SDL_SetRenderDrawColor(g_renderer, 200, 200, 200, 255);
				SDL_RenderFillRect(g_renderer, &rect);
				SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
				SDL_RenderDrawRect(g_renderer, &rect);

				Tile const* sel_tile = 
					&g_grid[sel_tile_coords.y * N_TILES_W + sel_tile_coords.x];

				rect = (SDL_Rect){.x = 35, .y = WINDOW_H - 135, .w = 100, .h = 100};
				render_tile_ground(sel_tile->type, rect);

				char const* name = g_tile_type_spec_table[sel_tile->type].name;
				render_string(name,
					(WinCoords){10 + 150/2, WINDOW_H - 175}, PP_TOP_CENTER,
					(SDL_Color){0, 0, 0, 255});
			}
		}
		SDL_RenderPresent(g_renderer);
	}
	return 0;
}
