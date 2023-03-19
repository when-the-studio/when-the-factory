#include <assert.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "renderer.h"

SDL_Window*   g_window      = NULL;
SDL_Renderer* g_renderer    = NULL;
SDL_Texture*  g_spritesheet = NULL;

static TTF_Font* s_font = NULL;

void renderer_init(void) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		assert(false);
	}

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

	s_font = TTF_OpenFont("../assets/fonts/bitstream_vera_sans/Vera.ttf", 22);
	assert(s_font != NULL);
}

static void adjust_rect_for_pin_point(SDL_Rect* rect, PinPoint pp) {
	rect->x -= pp.x * (float)rect->w;
	rect->y -= pp.y * (float)rect->h;
}

void render_string(char const* string, WinCoords wc, PinPoint pp, SDL_Color color) {
	/* Note that this is extremely wasteful if the same string is rendered every frame
	 * to create and destroy a surface and a texture representing this string every frame.
	 * TODO: Optimize (but later). */
	SDL_Surface* surface = TTF_RenderText_Blended(s_font, string, color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, surface);
	SDL_Rect dst_rect = {wc.x, wc.y, surface->w, surface->h};
	adjust_rect_for_pin_point(&dst_rect, pp);
	SDL_FreeSurface(surface);
	SDL_RenderCopy(g_renderer, texture, NULL, &dst_rect);
	SDL_DestroyTexture(texture);
}
