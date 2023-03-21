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

TileCoords window_pixel_to_tile_coords(WinCoords wc) {
	float tile_render_size = TILE_SIZE * g_camera.zoom;
	return (TileCoords){
		.x = floorf(((float)wc.x + g_camera.pos.x) / tile_render_size),
		.y = floorf(((float)wc.y + g_camera.pos.y) / tile_render_size),
	};
}

WinCoords tile_coords_to_window_pixel(TileCoords tc) {
	float tile_render_size = TILE_SIZE * g_camera.zoom;
	return (WinCoords){
		.x = tc.x * tile_render_size - g_camera.pos.x,
		.y = tc.y * tile_render_size - g_camera.pos.y,
	};
}

void render_tile_ground(TileType tile_type, SDL_Rect dst_rect) {
	SDL_Rect rect_in_spritesheet = g_tile_type_spec_table[tile_type].rect_in_spritesheet;
	SDL_RenderCopy(g_renderer, g_spritesheet, &rect_in_spritesheet, &dst_rect);
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
	string_pixel_dims(widget->string, &dims.w, &dims.h);
	return dims;
}

void widget_text_line_render(WidgetTextLine const* widget, int x, int y) {
	render_string_pixel(widget->string, (WinCoords){x, y}, PP_TOP_LEFT, widget->fg_color);
}

struct WidgetMultipleTopLeftTopToBottom {
	Widget base;
	Widget** sub_wgs;
	int sub_wgs_count;
	int spacing;
};
typedef struct WidgetMultipleTopLeftTopToBottom WidgetMultipleTopLeftTopToBottom;

Dims widget_multiple_top_left_top_to_bottom_get_dims(WidgetMultipleTopLeftTopToBottom const* widget) {
	Dims dims = {0, 0};
	for (int i = 0; i < widget->sub_wgs_count; i++) {
		Dims sub_dims = widget_get_dims(widget->sub_wgs[i]);
		dims.h += sub_dims.h;
		if (i != 0) {
			dims.h += widget->spacing;
		}
		dims.w = max(dims.w, sub_dims.w);
	}
	return dims;
}

void widget_multiple_top_left_top_to_bottom_render(WidgetMultipleTopLeftTopToBottom const* widget, int x, int y) {
	for (int i = 0; i < widget->sub_wgs_count; i++) {
		widget_render(widget->sub_wgs[i], x, y);
		Dims sub_dims = widget_get_dims(widget->sub_wgs[i]);
		y += sub_dims.h + widget->spacing;
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
		.string = "test uwu !!! ballz ``sus amogus -1 +8 1000 gaming",
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
		.spacing = 5,
	};
	g_wg_root = (Widget*)widget_root;
}

int main() {
	renderer_init();
	init_map();
	
	/* Is grid line display enabled? */
	bool render_lines = false;

	/* Selected tile, if any. */
	bool sel_tile_exists = false;
	TileCoords sel_tile_coords = {0, 0};

	init_widget_tree();

	/* Game loop iteration time monitoring. */
	Uint64 time = SDL_GetPerformanceCounter();
	Uint64 last_time = 0;

	/* Main game loop. */
	bool running = true;
	while (running) {
		last_time = time;
		time = SDL_GetPerformanceCounter();
		/* Delta time in ms. */
		double dt = (time - last_time) * 1000 / (double)SDL_GetPerformanceFrequency();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN && event.key.repeat) {
				/* SDL will spam KEYDOWN events when a key is just pressed for long enough,
				 * which will confuse whatever relies on matching KEYDOWN and KEYUP events.
				 * So we just filter out these 'fake' KEYDOWN events. */
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
						case SDLK_DOWN:  g_camera.speed.y = +1; break;
						case SDLK_UP:    g_camera.speed.y = -1; break;
						case SDLK_RIGHT: g_camera.speed.x = +1; break;
						case SDLK_LEFT:  g_camera.speed.x = -1; break;
						case SDLK_l:
							render_lines = !render_lines;
						break;
						case SDLK_p:
							/* Test spawing entity on selected tile. */
							if (sel_tile_exists) {
								Entity* entity = new_entity(ENTITY_HUMAIN, sel_tile_coords);
								entity->faction = FACTION_YELLOW;
							}
						break;
						case SDLK_m:
							/* Test moving entity from selected tile to the right. */
							if (sel_tile_exists) {
								Tile* sel_tile = 
									&g_grid[sel_tile_coords.y * N_TILES_W + sel_tile_coords.x];
								if (1 <= sel_tile->entity_count) {
									entity_move(sel_tile->entities[0],
										(TileCoords){sel_tile_coords.x + 1, sel_tile_coords.y});
								}
							}
						break;
					}
				break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym) {
						/* Stop moving the camera when a key that
						* was moving the camera is released. */
						case SDLK_DOWN:  if (g_camera.speed.y > 0) {g_camera.speed.y = 0;}; break;
						case SDLK_UP:    if (g_camera.speed.y < 0) {g_camera.speed.y = 0;}; break;
						case SDLK_RIGHT: if (g_camera.speed.x > 0) {g_camera.speed.x = 0;}; break;
						case SDLK_LEFT:  if (g_camera.speed.x < 0) {g_camera.speed.x = 0;}; break;
					}
				break;
				case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button) {
						case SDL_BUTTON_LEFT:;
							WinCoords wc = {event.button.x, event.button.y};
							TileCoords tc = window_pixel_to_tile_coords(wc);
							bool sel_tile_is_alrady_tc =
								sel_tile_exists && tile_coords_eq(sel_tile_coords, tc);
							if (tile_coords_are_valid(tc) && !sel_tile_is_alrady_tc) {
								sel_tile_exists = true;
								sel_tile_coords = tc;
							} else {
								sel_tile_exists = false;
							}
						break;
					}
				break;
				case SDL_MOUSEMOTION:
					if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_RMASK) {
						/* The map is grabbed and the mouse is moving,
						* so the map has to look like it follows the mouse
						* (by moving the camera in the other direction). */
						g_camera.target_pos.x -= event.motion.xrel;
						g_camera.target_pos.y -= event.motion.yrel;
						g_camera.pos.x -= event.motion.xrel;
						g_camera.pos.y -= event.motion.yrel;
					}
				break;
				case SDL_MOUSEWHEEL:
					if (event.wheel.y > 0 && g_camera.target_zoom < ZOOM_MAX){
						g_camera.target_zoom /= 0.8f;
					} else if (event.wheel.y < 0 && g_camera.target_zoom > ZOOM_MIN) {
						g_camera.target_zoom *= 0.8f;
					}
				break;
			}
		}

		camera_update(dt);
		
		/* Background. */
		SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
		SDL_RenderClear(g_renderer);

		/* Draw tiles. */
		float tile_render_size = TILE_SIZE * g_camera.zoom;
		for (int i = 0; i < N_TILES; ++i) {
			TileCoords tc = {.x = i % N_TILES_W, .y = i / N_TILES_W};
			Tile const* tile = get_tile(tc);

			SDL_Rect dst_rect = {
				.x = tc.x * tile_render_size - g_camera.pos.x,
				.y = tc.y * tile_render_size - g_camera.pos.y,
				.w = ceilf(tile_render_size),
				.h = ceilf(tile_render_size)};
			render_tile_ground(tile->type, dst_rect);

			for (int entity_i = 0; entity_i < tile->entity_count; entity_i++) {
				Entity* entity = tile->entities[entity_i];
				assert(entity != NULL);
				switch (entity->type) {
					case ENTITY_HUMAIN:;
						int ex = (float)(entity_i+1) / (float)(tile->entity_count+1)
							* tile_render_size;
						int ey = (1.0f - (float)(entity_i+1) / (float)(tile->entity_count+1))
							* tile_render_size;
						int ew = 0.1f * tile_render_size;
						int eh = 0.3f * tile_render_size;
						SDL_Rect rect = {
							dst_rect.x + ex - ew / 2.0f, dst_rect.y + ey - eh / 2.0f, ew, eh};
						switch (entity->faction) {
							case FACTION_YELLOW:
								SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255);
							break;
							case FACTION_RED:
								SDL_SetRenderDrawColor(g_renderer, 255,   0, 0, 255);
							break;
							default: assert(false);
						}
						SDL_RenderFillRect(g_renderer, &rect);
					break;
					default: assert(false);
				}
			}
		}

		/* Draw grid lines if enabled. */
		if (render_lines) {
			SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
			for (int y = 0; y < N_TILES_W+1; ++y) {
				WinCoords a = tile_coords_to_window_pixel((TileCoords){0, y});
				WinCoords b = tile_coords_to_window_pixel((TileCoords){N_TILES_W, y});
				SDL_RenderDrawLine(g_renderer, a.x, a.y,   b.x, b.y);
				SDL_RenderDrawLine(g_renderer, a.x, a.y-1, b.x, b.y-1);
			}
			for (int x = 0; x < N_TILES_H+1; ++x) {
				WinCoords a = tile_coords_to_window_pixel((TileCoords){x, 0});
				WinCoords b = tile_coords_to_window_pixel((TileCoords){x, N_TILES_H});
				SDL_RenderDrawLine(g_renderer, a.x,   a.y, b.x,   b.y);
				SDL_RenderDrawLine(g_renderer, a.x-1, a.y, b.x-1, b.y);
			}
		}

		if (sel_tile_exists) {
			/* Draw selection rect around the selected tile. */
			SDL_Rect rect = {
				.x = sel_tile_coords.x * tile_render_size - g_camera.pos.x,
				.y = sel_tile_coords.y * tile_render_size - g_camera.pos.y,
				.w = ceilf(tile_render_size),
				.h = ceilf(tile_render_size)};
			SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255);
			SDL_RenderDrawRect(g_renderer, &rect);
			rect.x -= 1; rect.y -= 1; rect.w += 2; rect.h += 2;
			SDL_RenderDrawRect(g_renderer, &rect);

			Tile const* sel_tile = 
				&g_grid[sel_tile_coords.y * N_TILES_W + sel_tile_coords.x];

			/* TODO: Redo the following lame UI stuff with the ui-dev's branch widgets. */

			/* Draw the selected tile information in a corner. */
			SDL_Rect ui_rect = {.x = 10, .y = WINDOW_H - 190, .w = 150, .h = 180};
			SDL_SetRenderDrawColor(g_renderer, 200, 200, 200, 255);
			SDL_RenderFillRect(g_renderer, &ui_rect);
			SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
			SDL_RenderDrawRect(g_renderer, &ui_rect);
			ui_rect.x -= 1; ui_rect.y -= 1; ui_rect.w += 2; ui_rect.h += 2;
			SDL_RenderDrawRect(g_renderer, &ui_rect);
			ui_rect.x += 1; ui_rect.y += 1; ui_rect.w -= 2; ui_rect.h -= 2;
			
			rect = (SDL_Rect){.x = 35, .y = WINDOW_H - 135, .w = 100, .h = 100};
			render_tile_ground(sel_tile->type, rect);
			char const* name = g_tile_type_spec_table[sel_tile->type].name;
			render_string_pixel(name,
				(WinCoords){10 + 150/2, WINDOW_H - 175}, PP_TOP_CENTER,
				(SDL_Color){0, 0, 0, 255});

			for (int entity_i = 0; entity_i < sel_tile->entity_count; entity_i++) {
				Entity* entity = sel_tile->entities[entity_i];
				assert(entity != NULL);

				ui_rect.x += ui_rect.w + 10;
				SDL_SetRenderDrawColor(g_renderer, 200, 200, 200, 255);
				SDL_RenderFillRect(g_renderer, &ui_rect);
				SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
				SDL_RenderDrawRect(g_renderer, &ui_rect);
				ui_rect.x -= 1; ui_rect.y -= 1; ui_rect.w += 2; ui_rect.h += 2;
				SDL_RenderDrawRect(g_renderer, &ui_rect);
				ui_rect.x += 1; ui_rect.y += 1; ui_rect.w -= 2; ui_rect.h -= 2;

				char const* name;
				switch (entity->type) {
					case ENTITY_HUMAIN: name = "Human"; break;
					default: assert(false);
				}
				render_string_pixel(name,
					(WinCoords){ui_rect.x + ui_rect.w/2, WINDOW_H - 175}, PP_TOP_CENTER,
					(SDL_Color){0, 0, 0, 255});

				char const* faction_name;
				switch (entity->faction) {
					case FACTION_YELLOW: faction_name = "Yellow"; break;
					case FACTION_RED:    faction_name = "Red";    break;
					default: assert(false);
				}
				render_string_pixel(faction_name,
					(WinCoords){ui_rect.x + ui_rect.w/2, WINDOW_H - 175 + 40}, PP_TOP_CENTER,
					(SDL_Color){0, 0, 0, 255});
			}
		}

		widget_render(g_wg_root, 0, 0);

		SDL_RenderPresent(g_renderer);
	}
	return 0;
}
