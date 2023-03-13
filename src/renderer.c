#include <assert.h>
#include "renderer.h"

SDL_Window* g_window = NULL;
SDL_Renderer* g_renderer = NULL;

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
}