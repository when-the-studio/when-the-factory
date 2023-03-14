#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <time.h>
#include "utils.h"

#include "renderer.h"
#include "map.h"
#include "camera.h"

int main() {
	/* Renderer initialisation*/
	renderer_init();
	/* Game variables init*/
	init_map();
	
	/* Position of the top left corner of the grid in the window */
	// coord_t offset = {-1000, -1000}; // Replaced with camera.pos
	Camera camera = {{-1000,-1000}, {-1000,-1000}, {0,0}, 1};

	bool render_lines = true;
	/* Main game loop */
	bool running = true;

	clock_t timer = clock();
	float dt = 0;
	while (running) {
		clock_t clockDt = clock() - timer;
		timer = clock();
		dt = clockDt * 1000 / (float)CLOCKS_PER_SEC;
		
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
						case SDLK_DOWN:  cam_speed(&camera, camera.speed.x, -1); break;
						case SDLK_UP:    cam_speed(&camera, camera.speed.x, 1); break;
						case SDLK_RIGHT: cam_speed(&camera, -1, camera.speed.y); break;
						case SDLK_LEFT:  cam_speed(&camera, 1, camera.speed.y); break;
						case SDLK_l:
							render_lines = !render_lines;
						break;
					}
				break;
				// case SDL_MOUSEBUTTONUP:
				// break;
				// case SDL_MOUSEBUTTONDOWN:
				// break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym) {
						case SDLK_DOWN:  if (camera.speed.y < 0) {cam_speed(&camera, camera.speed.x, 0);}; break;
						case SDLK_UP:    if (camera.speed.y > 0) {cam_speed(&camera, camera.speed.x, 0);}; break;
						case SDLK_RIGHT: if (camera.speed.x < 0) {cam_speed(&camera, 0, camera.speed.y);}; break;
						case SDLK_LEFT:  if (camera.speed.x > 0) {cam_speed(&camera, 0, camera.speed.y);}; break;
						break;
					}
				break;
				case SDL_MOUSEMOTION:
					if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) {
						camera.target_pos.x += event.motion.xrel;
						camera.target_pos.y += event.motion.yrel;
					}
				break;
			}
		}
		cam_update(&camera, dt);

		/* Background*/
		SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
		SDL_RenderClear(g_renderer);
		/* Draw tiles */
		for (int i = 0; i < N_TILES; ++i) {
			int x = (i % N_TILES_W) * TILE_SIZE + camera.pos.x;
			int y = (i / N_TILES_W) * TILE_SIZE + camera.pos.y;
			SDL_Rect rect = {x, y, TILE_SIZE, TILE_SIZE};
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
				int xi = row * TILE_SIZE + camera.pos.x;
				SDL_RenderDrawLine(g_renderer, xi, y0, xi, yf);
			}
			for (int col = 1; col < N_TILES_H; ++col) {
				int yi = col * TILE_SIZE + camera.pos.y;
				SDL_RenderDrawLine(g_renderer, x0, yi, xf, yi);
			}
		}
		SDL_RenderPresent(g_renderer);
	}
	return 0;
}