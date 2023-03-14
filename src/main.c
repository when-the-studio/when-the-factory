#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <SDL2/SDL.h>

#include "renderer.h"
#include "map.h"
#include "camera.h"

Coord window_pixel_to_tile_coords(Camera const* camera, int x, int y) {
	float tileRenderSize = TILE_SIZE * camera->zoom;
	return (Coord){
		.x = floorf(((float)x + camera->pos.x) / tileRenderSize),
		.y = floorf(((float)y + camera->pos.y) / tileRenderSize),
	};
}

bool tile_coords_are_valid(Coord coords) {
	return
		0 <= coords.x && coords.x < N_TILES_W &&
		0 <= coords.y && coords.y < N_TILES_H;
}

bool coords_eq(Coord a, Coord b) {
	return a.x == b.x && a.y == b.y;
}

int main() {
	/* Renderer initialisation*/
	renderer_init();
	/* Game variables init*/
	init_map();
	
	/* Position of the top left corner of the grid in the window */
	// coord_t offset = {-1000, -1000}; // Replaced with camera.pos
	Camera camera = {{1000,1000}, {1000,1000}, {0,0}, 1, 1};

	bool render_lines = true;

	bool selected_tile_exists = false;
	Coord selected_tile_coords = {0, 0};

	/* Main game loop */
	bool running = true;

	double dt = 0;  /* delta time in ms */
	Uint64 timer = SDL_GetPerformanceCounter();
	Uint64 lastTimer = 0;
	while (running) {
		lastTimer = timer;
		timer = SDL_GetPerformanceCounter();
		dt = (double)((timer-lastTimer)*1000/(double)SDL_GetPerformanceFrequency());
		cam_update(&camera, dt);
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat) {
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
						case SDLK_DOWN:  cam_speed(&camera, camera.speed.x, 1); break;
						case SDLK_UP:    cam_speed(&camera, camera.speed.x, -1); break;
						case SDLK_RIGHT: cam_speed(&camera, 1, camera.speed.y); break;
						case SDLK_LEFT:  cam_speed(&camera, -1, camera.speed.y); break;
						case SDLK_l:
							render_lines = !render_lines;
						break;
					}
				break;
				// case SDL_MOUSEBUTTONUP:
				// break;
				case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button) {
						case SDL_BUTTON_LEFT: {
							Coord tile_coords = window_pixel_to_tile_coords(&camera,
								event.button.x, event.button.y);
							if (tile_coords_are_valid(tile_coords) && !(
								selected_tile_exists && coords_eq(selected_tile_coords, tile_coords)
							)) {
								selected_tile_exists = true;
								selected_tile_coords = tile_coords;
							} else {
								selected_tile_exists = false;
							}
						} break;
					}
				break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym) {
						case SDLK_DOWN:  if (camera.speed.y > 0) {cam_speed(&camera, camera.speed.x, 0);}; break;
						case SDLK_UP:    if (camera.speed.y < 0) {cam_speed(&camera, camera.speed.x, 0);}; break;
						case SDLK_RIGHT: if (camera.speed.x > 0) {cam_speed(&camera, 0, camera.speed.y);}; break;
						case SDLK_LEFT:  if (camera.speed.x < 0) {cam_speed(&camera, 0, camera.speed.y);}; break;
						break;
					}
				break;
				case SDL_MOUSEMOTION:
					if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_RMASK) {
						camera.target_pos.x -= event.motion.xrel;
						camera.target_pos.y -= event.motion.yrel;
						camera.pos.x -= event.motion.xrel;
						camera.pos.y -= event.motion.yrel;
					}
					break;
				case SDL_MOUSEWHEEL:
					if (event.wheel.y > 0 && camera.target_zoom < 4 ){
						camera.target_zoom /= 0.8;
						
					} else if (event.wheel.y < 0 && camera.target_zoom > 0.1) {
						camera.target_zoom *= 0.8;
					}
					break;
				break;
			}
		}
		
		/* Background*/
		SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
		SDL_RenderClear(g_renderer);
		/* Draw tiles */
		float tileRenderSize = TILE_SIZE * camera.zoom;
		for (int i = 0; i < N_TILES; ++i) {
			int x = (i % N_TILES_W) * tileRenderSize - camera.pos.x;
			int y = (i / N_TILES_W) * tileRenderSize - camera.pos.y;
			SDL_Rect rect = {x, y, ceilf(tileRenderSize), ceilf(tileRenderSize)};
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
				int xi = row * tileRenderSize - camera.pos.x;
				SDL_RenderDrawLine(g_renderer, xi, y0, xi, yf);
				SDL_RenderDrawLine(g_renderer, xi-1, y0, xi-1, yf);
			}
			for (int col = 1; col < N_TILES_H; ++col) {
				int yi = col * tileRenderSize - camera.pos.y;
				SDL_RenderDrawLine(g_renderer, x0, yi, xf, yi);
				SDL_RenderDrawLine(g_renderer, x0, yi-1, xf, yi-1);
			}
		}
		if (selected_tile_exists) {
			SDL_Rect rect = {
				.x = selected_tile_coords.x * tileRenderSize - camera.pos.x,
				.y = selected_tile_coords.y * tileRenderSize - camera.pos.y,
				.w = ceilf(tileRenderSize),
				.h = ceilf(tileRenderSize)};
			SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255);
			SDL_RenderDrawRect(g_renderer, &rect);

			rect = (SDL_Rect){.x = 10, .y = WINDOW_H - 190, .w = 150, .h = 180};
			SDL_SetRenderDrawColor(g_renderer, 200, 200, 200, 255);
			SDL_RenderFillRect(g_renderer, &rect);
			SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
			SDL_RenderDrawRect(g_renderer, &rect);

			Tile const* selected_tile = 
				&g_grid[selected_tile_coords.y * N_TILES_W + selected_tile_coords.x];

			rect = (SDL_Rect){.x = 35, .y = WINDOW_H - 135, .w = 100, .h = 100};
			SDL_Rect rect_in_spritesheet = {.x = 0, .y = 0, .w = 8, .h = 8};
			switch (selected_tile->type) {
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
			switch (selected_tile->type) {
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
