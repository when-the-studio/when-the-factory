#include <assert.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>
#include "renderer.h"

SDL_Window* g_window = NULL;
SDL_Renderer* g_renderer = NULL;
SDL_Texture* g_spritesheet = NULL;

/* Initialises the rendering, maybe a ugly to put pointers to renderers?
 * TODO: Discuss ?*/
void renderer_init(void) {
    int init_res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	assert(init_res == 0);
	g_window = SDL_CreateWindow("When the Factory", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WINDOW_W, WINDOW_H,
		0);
	assert(g_window != NULL);
	g_renderer = SDL_CreateRenderer(g_window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	assert(g_renderer != NULL);

	if (IMG_Init(IMG_INIT_PNG) == 0) {
		assert(false);
	}
	
	SDL_Surface* surface = IMG_Load("../assets/images/spritesheet-test-01.png");
	assert(surface != NULL);
	g_spritesheet = SDL_CreateTextureFromSurface(g_renderer, surface);
	SDL_FreeSurface(surface);
	assert(g_spritesheet != NULL);

	IMG_Quit();
}