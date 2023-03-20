#include <assert.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "renderer.h"

SDL_Window*   g_window      = NULL;
SDL_Renderer* g_renderer    = NULL;
SDL_Texture*  g_spritesheet = NULL;

static TTF_Font* s_font = NULL;
static SDL_Texture* s_spritesheet_font = NULL;

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

	surface = IMG_Load("../assets/images/spritesheet-font-01.png");
	assert(surface != NULL);
	s_spritesheet_font = SDL_CreateTextureFromSurface(g_renderer, surface);
	SDL_FreeSurface(surface);
	assert(s_spritesheet_font != NULL);

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

static int char_pixel_width(char c) {
	if (c == ' ') {
		return 2;
	} else if ('A' <= c && c <= 'Z') {
		return 3;
	} else if ('a' <= c && c <= 'z') {
		return 3;
	} else if ('0' <= c && c <= '9') {
		return 3;
	} else if (c == '.' || c == ':' || c == '!' || c == '\'' || c == '|') {
		return 1;
	} else if (c == '(' || c == ')' || c == '[' || c == ']' || c == ';' || c == ',' || c == '`') {
		return 2;
	} else {
		return 3;
	}
}

static int string_pixel_width(char const* string, int scale, int spacing) {
	int w = 0;
	for (int i = 0; string[i] != '\0'; i++) {
		if (i != 0) {
			w += spacing;
		}
		char c = string[i];
		w += char_pixel_width(c) * scale;
	}
	return w;
}

void render_string_pixel(char const* string, WinCoords wc, PinPoint pp, SDL_Color color) {
	/* TODO: Use the given color instead of just black. */
	/* TODO: Take scale and spacing as parameters, in a nice way. */

	int scale = 3;
	int spacing = 2;
	int w = string_pixel_width(string, scale, spacing);
	SDL_Rect dst_full_rect = {wc.x, wc.y, w, 5 * scale};
	adjust_rect_for_pin_point(&dst_full_rect, pp);
	int x = dst_full_rect.x;
	int y = dst_full_rect.y;

	for (int i = 0; string[i] != '\0'; i++) {
		char c = string[i];
		if (c == ' ') {
			x += char_pixel_width(c) * scale + spacing;
			continue;
		}
		SDL_Rect rect_in_spritesheet = {.w = char_pixel_width(c), .h = 5};
		if ('a' <= c && c <= 'z') {
			rect_in_spritesheet.y = 1;
			rect_in_spritesheet.x = 1 + (c - 'a') * 4;
		} else if ('A' <= c && c <= 'Z') {
			rect_in_spritesheet.y = 1;
			rect_in_spritesheet.x = 1 + (c - 'A') * 4;
		} else if ('0' <= c && c <= '9') {
			rect_in_spritesheet.y = 7;
			rect_in_spritesheet.x = 1 + (c - '0') * 4;
		} else {
			rect_in_spritesheet.y = 13;
			char const* third_line = "+-.%()[]{}<>:;,?!/*#\"\'~_|`\\^=";
			rect_in_spritesheet.x = 1;
			for (int tl_i = 0; third_line[tl_i] != '\0'; tl_i++) {
				if (c == third_line[tl_i]) {
					break;
				}
				rect_in_spritesheet.x += 4;
			}
		}
		SDL_Rect dst_glyph_rect = {x, y, char_pixel_width(c) * scale, 5 * scale};
		SDL_RenderCopy(g_renderer, s_spritesheet_font, &rect_in_spritesheet, &dst_glyph_rect);
		x += dst_glyph_rect.w + spacing;
	}
}

void render_string_ttf(char const* string, WinCoords wc, PinPoint pp, SDL_Color color) {
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
