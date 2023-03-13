#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "utils.h"

#include "renderer.h"

#define TILE_H 100
#define TILE_W 100

#define N_TILES_H 8
#define N_TILES_W 8

#define N_TILES N_TILES_H * N_TILES_W

SDL_Window* g_window = NULL;
SDL_Renderer* g_renderer = NULL;

enum tile_type_t {
    TILE_PLAIN,
    TILE_MOUTAIN,
    TILE_RIVER,
    TILE_FOREST,
	TILE_TYPE_NUM
};
typedef enum tile_type_t tile_type_t;

struct entity{};
typedef struct entity entity;

struct tile_t {
	tile_type_t type;
	entity* entities_on_tile;
};
typedef struct tile_t tile_t;

tile_t* grid;

int main() {
    /* Renderer initialisation*/
    renderer_init(&g_window, &g_renderer);

	/* Game variables init*/
	grid = malloc(N_TILES * sizeof(tile_t));
	for(int i = 0; i < N_TILES; ++i) {
		grid[i].type = rand() % TILE_TYPE_NUM;
	}
    /* Main game loop*/
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
					}
				break;
			}
		}
        SDL_SetRenderDrawColor(g_renderer, 0, 200, 200, 255);
		SDL_RenderClear(g_renderer);
		/* Draw tiles*/
		for (int i = 0; i < N_TILES; ++i) {
			int x = (i % N_TILES_H) * TILE_W;
			int y = (i / N_TILES_W) * TILE_H;
			SDL_Rect rect = {x, y, TILE_H, TILE_W};
			printf("i = %d, (x, y) = (%d, %d), grid type : %d\n", i, x, y, grid[i].type);
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
        SDL_RenderPresent(g_renderer);
    }
    printf("Yo!\n");
    printf("tilen %d\n",N_TILES);
	// for (int i = 0; i < N_TILES; i++) {
	// 	printf("%d", grid[i].type);
	// 	if (i%8 == 7) {printf("\n");}
	// }
	// printf("\n");
	int x = 800/100;
	int y = 35 % 10;
	int z = 43 / 4;
	printf("%d, %d, %d\n", x, y ,z);
    return 0;
}