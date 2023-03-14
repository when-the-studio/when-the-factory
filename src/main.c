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
	Uint64 timer = SDL_GetPerformanceCounter();
	Uint64 last_timer = 0;

	/* Main game loop. */
	bool running = true;
	while (running) {
		last_timer = timer;
		timer = SDL_GetPerformanceCounter();
		/* Delta time in ms. */
		double dt = (timer - last_timer) * 1000 / (double)SDL_GetPerformanceFrequency();

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
						case SDLK_DOWN:  cam_speed(g_camera.speed.x, +1); break;
						case SDLK_UP:    cam_speed(g_camera.speed.x, -1); break;
						case SDLK_RIGHT: cam_speed(+1, g_camera.speed.y); break;
						case SDLK_LEFT:  cam_speed(-1, g_camera.speed.y); break;
						case SDLK_l:
							render_lines = !render_lines;
						break;
					}
				break;
				case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button) {
						case SDL_BUTTON_LEFT: {
							WinCoords wc = {event.button.x, event.button.y};
							TileCoords tc = window_pixel_to_tile_coords(wc);
							if (tile_coords_are_valid(tc) && !(
								sel_tile_exists && coords_eq(sel_tile_coords, tc)
							)) {
								sel_tile_exists = true;
								sel_tile_coords = tc;
							} else {
								sel_tile_exists = false;
							}
						} break;
					}
				break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym) {
						case SDLK_DOWN:  if (g_camera.speed.y > 0) {cam_speed(g_camera.speed.x, 0);}; break;
						case SDLK_UP:    if (g_camera.speed.y < 0) {cam_speed(g_camera.speed.x, 0);}; break;
						case SDLK_RIGHT: if (g_camera.speed.x > 0) {cam_speed(0, g_camera.speed.y);}; break;
						case SDLK_LEFT:  if (g_camera.speed.x < 0) {cam_speed(0, g_camera.speed.y);}; break;
						break;
					}
				break;
				case SDL_MOUSEMOTION:
					if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_RMASK) {
						g_camera.target_pos.x -= event.motion.xrel;
						g_camera.target_pos.y -= event.motion.yrel;
						g_camera.pos.x -= event.motion.xrel;
						g_camera.pos.y -= event.motion.yrel;
					}
					break;
				case SDL_MOUSEWHEEL:
					if (event.wheel.y > 0 && g_camera.target_zoom < 4 ){
						g_camera.target_zoom /= 0.8;
						
					} else if (event.wheel.y < 0 && g_camera.target_zoom > 0.1) {
						g_camera.target_zoom *= 0.8;
					}
					break;
				break;
			}
		}

		cam_update(dt);
		
		/* Background. */
		SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
		SDL_RenderClear(g_renderer);

		/* TODO: Do a little formatting on what follows. */

		/* Draw tiles. */
		float tile_render_size = TILE_SIZE * g_camera.zoom;
		for (int i = 0; i < N_TILES; ++i) {
			int x = (i % N_TILES_W) * tile_render_size - g_camera.pos.x;
			int y = (i / N_TILES_W) * tile_render_size - g_camera.pos.y;
			SDL_Rect rect = {x, y, ceilf(tile_render_size), ceilf(tile_render_size)};
			SDL_Rect rect_in_spritesheet = {.x = 0, .y = 0, .w = 8, .h = 8};
			switch (g_grid[i].type) {
			case TILE_PLAIN:
				rect_in_spritesheet.x = 0;
				break;
			case TILE_MOUTAIN:
				rect_in_spritesheet.x = 16;
				break;
			case TILE_RIVER:
				rect_in_spritesheet.x = 8;
				break;
			case TILE_FOREST:
				rect_in_spritesheet.x = 24;
				break;
			default:
				break;
			}
			SDL_RenderCopy(g_renderer, g_spritesheet, &rect_in_spritesheet, &rect);
		}
		/* Draw lines */
		if (render_lines) {
			SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
			int x0, y0, xf, yf;
			x0 = 0; xf = WINDOW_W;
			y0 = 0; yf = WINDOW_H;
			for (int row = 1; row < N_TILES_W; ++row) {
				int xi = row * tile_render_size - g_camera.pos.x;
				SDL_RenderDrawLine(g_renderer, xi, y0, xi, yf);
				SDL_RenderDrawLine(g_renderer, xi-1, y0, xi-1, yf);
			}
			for (int col = 1; col < N_TILES_H; ++col) {
				int yi = col * tile_render_size - g_camera.pos.y;
				SDL_RenderDrawLine(g_renderer, x0, yi, xf, yi);
				SDL_RenderDrawLine(g_renderer, x0, yi-1, xf, yi-1);
			}
		}
		if (sel_tile_exists) {
			SDL_Rect rect = {
				.x = sel_tile_coords.x * tile_render_size - g_camera.pos.x,
				.y = sel_tile_coords.y * tile_render_size - g_camera.pos.y,
				.w = ceilf(tile_render_size),
				.h = ceilf(tile_render_size)};
			SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255);
			SDL_RenderDrawRect(g_renderer, &rect);

			rect = (SDL_Rect){.x = 10, .y = WINDOW_H - 190, .w = 150, .h = 180};
			SDL_SetRenderDrawColor(g_renderer, 200, 200, 200, 255);
			SDL_RenderFillRect(g_renderer, &rect);
			SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
			SDL_RenderDrawRect(g_renderer, &rect);

			Tile const* sel_tile = 
				&g_grid[sel_tile_coords.y * N_TILES_W + sel_tile_coords.x];

			rect = (SDL_Rect){.x = 35, .y = WINDOW_H - 135, .w = 100, .h = 100};
			SDL_Rect rect_in_spritesheet = {.x = 0, .y = 0, .w = 8, .h = 8};
			switch (sel_tile->type) {
				case TILE_PLAIN:
					rect_in_spritesheet.x = 0;
				break;
				case TILE_MOUTAIN:
					rect_in_spritesheet.x = 16;
				break;
				case TILE_RIVER:
					rect_in_spritesheet.x = 8;
				break;
				case TILE_FOREST:
					rect_in_spritesheet.x = 24;
				break;
				default: assert(false);
			}
			SDL_RenderCopy(g_renderer, g_spritesheet, &rect_in_spritesheet, &rect);

			char const* name;
			switch (sel_tile->type) {
				case TILE_PLAIN:
					name = "Plain";
				break;
				case TILE_MOUTAIN:
					name = "Mountain";
				break;
				case TILE_RIVER:
					name = "River";
				break;
				case TILE_FOREST:
					name = "Forest";
				break;
				default: assert(false);
			}
			render_text(name,
				10 + 150/2, WINDOW_H - 175, (SDL_Color){0, 0, 0, 255},
				PP_TOP_CENTER);
		}
		SDL_RenderPresent(g_renderer);
	}
	return 0;
}
