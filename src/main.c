#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "renderer.h"
#include "map.h"
#include "camera.h"

int max(int a, int b) {
	return a < b ? b : a;
}

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

struct Dims {
	int w, h;
};
typedef struct Dims Dims;

enum WidgetType {
	WIDGET_TEXT_LINE,
	WIDGET_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM,
};
typedef enum WidgetType WidgetType;

struct Widget {
	WidgetType type;
};
typedef struct Widget Widget;

Dims widget_get_dims(Widget const* widget);
void widget_render(Widget const* widget, int x, int y);

/* The root of the whole widget tree. */
Widget* g_wg_root = NULL;

struct WidgetTextLine {
	Widget base;
	char* string;
	SDL_Color fg_color;
};
typedef struct WidgetTextLine WidgetTextLine;

Dims widget_text_line_get_dims(WidgetTextLine const* widget) {
	Dims dims;
	TTF_SizeText(g_font, widget->string, &dims.w, &dims.h);
	return dims;
}

void widget_text_line_render(WidgetTextLine const* widget, int x, int y) {
	render_text(widget->string, x, y, widget->fg_color, PP_TOP_LEFT);
}

struct WidgetMultipleTopLeftTopToBottom {
	Widget base;
	Widget** sub_wgs;
	int sub_wgs_count;
};
typedef struct WidgetMultipleTopLeftTopToBottom WidgetMultipleTopLeftTopToBottom;

Dims widget_multiple_top_left_top_to_bottom_get_dims(WidgetMultipleTopLeftTopToBottom const* widget) {
	Dims dims = {0, 0};
	for (int i = 0; i < widget->sub_wgs_count; i++) {
		Dims sub_dims = widget_get_dims(widget->sub_wgs[i]);
		dims.h += sub_dims.h;
		dims.w = max(dims.w, sub_dims.w);
	}
	return dims;
}

void widget_multiple_top_left_top_to_bottom_render(WidgetMultipleTopLeftTopToBottom const* widget, int x, int y) {
	for (int i = 0; i < widget->sub_wgs_count; i++) {
		widget_render(widget->sub_wgs[i], x, y);
		Dims sub_dims = widget_get_dims(widget->sub_wgs[i]);
		y += sub_dims.h;
	}
}

Dims widget_get_dims(Widget const* widget) {
	switch (widget->type) {
		case WIDGET_TEXT_LINE:
			return widget_text_line_get_dims((WidgetTextLine const*)widget);
		break;
		case WIDGET_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM:
			return widget_multiple_top_left_top_to_bottom_get_dims((WidgetMultipleTopLeftTopToBottom const*)widget);
		break;
		default: assert(false);
	}
}

void widget_render(Widget const* widget, int x, int y) {
	switch (widget->type) {
		case WIDGET_TEXT_LINE:
			widget_text_line_render((WidgetTextLine const*)widget, x, y);
		break;
		case WIDGET_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM:
			widget_multiple_top_left_top_to_bottom_render((WidgetMultipleTopLeftTopToBottom const*)widget, x, y);
		break;
		default: assert(false);
	}
}

void init_widget_tree(void) {
	WidgetTextLine* widget_a = malloc(sizeof(WidgetTextLine));
	*widget_a = (WidgetTextLine){
		.base = {
			.type = WIDGET_TEXT_LINE,
		},
		.string = "test xd",
		.fg_color = {255, 0, 0, 255},
	};
	WidgetTextLine* widget_b = malloc(sizeof(WidgetTextLine));
	*widget_b = (WidgetTextLine){
		.base = {
			.type = WIDGET_TEXT_LINE,
		},
		.string = "gaming",
		.fg_color = {255, 0, 0, 255},
	};
	Widget** h = malloc(2 * sizeof(WidgetTextLine));
	h[0] = (Widget*)widget_a;
	h[1] = (Widget*)widget_b;
	WidgetMultipleTopLeftTopToBottom* widget_root = malloc(sizeof(WidgetMultipleTopLeftTopToBottom));
	*widget_root = (WidgetMultipleTopLeftTopToBottom){
		.base = {
			.type = WIDGET_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM,
		},
		.sub_wgs = h,
		.sub_wgs_count = 2,
	};
	g_wg_root = (Widget*)widget_root;
}

int main() {
	/* Renderer initialisation*/
	renderer_init();
	/* Game variables init*/
	init_map();
	
	/* Position of the top left corner of the grid in the window */
	// coord_t offset = {-1000, -1000}; // Replaced with camera.pos
	Camera camera = {{1000,1000}, {1000,1000}, {0,0}, 1, 1};

	bool render_lines = false;

	bool selected_tile_exists = false;
	Coord selected_tile_coords = {0, 0};

	init_widget_tree();

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

		widget_render(g_wg_root, 0, 0);

		SDL_RenderPresent(g_renderer);
	}
	return 0;
}
