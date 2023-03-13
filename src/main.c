#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "utils.h"

#include "renderer.h"
#include "map.h"


int main() {
	/* Renderer initialisation*/
	renderer_init();
	/* Game variables init*/
	init_map();

	coord_t offset;
	/* Main game loop */
	bool running = true;
	while (running) {
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
						case SDLK_DOWN:  offset.y += 10; break;
						case SDLK_UP:    offset.y -= 10; break;
						case SDLK_RIGHT: offset.x += 10; break;
						case SDLK_LEFT:  offset.x -= 10; break;
					}
				break;
			}
		}

		/* Draw tiles */
		for (int i = 0; i < N_TILES; ++i) {
			int x = (i % N_TILES_W) * TILE_SIZE + offset.x;
			int y = (i / N_TILES_W) * TILE_SIZE + offset.y;
			SDL_Rect rect = {x, y, TILE_SIZE, TILE_SIZE};
			switch (grid[i].type) {
			case TILE_PLAIN:
				SDL_SetRenderDrawColor(g_renderer, 103, 184,  74, 255);
				break;
			case TILE_MOUTAIN:
				SDL_SetRenderDrawColor(g_renderer, 144, 144, 144, 255);
				break;
			case TILE_RIVER:
				SDL_SetRenderDrawColor(g_renderer,  38,  53, 176, 255);
				break;
			case TILE_FOREST:
				SDL_SetRenderDrawColor(g_renderer,  61, 101,  40, 255);
				break;
			default:
				break;
			}
			SDL_RenderFillRect(g_renderer, &rect);
		}
		/* Draw lines */
		SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
		int x0, y0, xf, yf;
		x0 = 0; xf = WINDOW_W;
		y0 = 0; yf = WINDOW_H;
		for (int row = 1; row < N_TILES_W; ++row) {
			int xi = row * TILE_SIZE + offset.x;
			SDL_RenderDrawLine(g_renderer, xi, y0, xi, yf);
		}
		for (int col = 1; col < N_TILES_H; ++col) {
			int yi = col * TILE_SIZE + offset.y;
			SDL_RenderDrawLine(g_renderer, x0, yi, xf, yi);
		}
		SDL_RenderPresent(g_renderer);
	}
	return 0;
}