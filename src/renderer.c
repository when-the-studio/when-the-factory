#include <assert.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "renderer.h"

SDL_Window* g_window = NULL;
SDL_Renderer* g_renderer = NULL;
SDL_Texture* g_spritesheet = NULL;
TTF_Font* g_font = NULL;

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

	if (TTF_Init() != 0) {
		assert(false);
	}

	g_font = TTF_OpenFont("../assets/fonts/bitstream_vera_sans/Vera.ttf", 22);
	assert(g_font != NULL);
}

void render_text(char const* text, int x, int y, SDL_Color color, PinPoint pin_point) {
	/* Note that this is extremely wasteful if the same string is rendered every frame
	 * to create and destroy a surface and a texture representing this string every frame.
	 * TODO: Optimize (but later). */
	SDL_Surface* surface = TTF_RenderText_Blended(g_font, text, color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, surface);
	SDL_Rect rect = {
		.x = x - pin_point.x * (float)surface->w,
		.y = y - pin_point.y * (float)surface->h,
		.w = surface->w,
		.h = surface->h};
	SDL_FreeSurface(surface);
	SDL_RenderCopy(g_renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
}
