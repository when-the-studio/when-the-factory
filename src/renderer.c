#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "renderer.h"
#include "utils.h"
#include "map.h"
#include "camera.h"
#include "ui.h"
#include "entity.h"

SDL_Window*   g_window      = NULL;
SDL_Renderer* g_renderer    = NULL;
SDL_Texture*  g_spritesheet = NULL;
SDL_Texture*  g_spritesheet_buildings = NULL;

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
	SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);

	if (IMG_Init(IMG_INIT_PNG) == 0) {
		assert(false);
	}

	/* Load the spritesheet for tiles and stuff. */
	SDL_Surface* surface = IMG_Load("../assets/images/spritesheet-test-01.png");
	assert(surface != NULL);
	g_spritesheet = SDL_CreateTextureFromSurface(g_renderer, surface);
	SDL_FreeSurface(surface);
	assert(g_spritesheet != NULL);

	surface = IMG_Load("../assets/images/spritesheet-buildings-01.png");
	assert(surface != NULL);
	g_spritesheet_buildings = SDL_CreateTextureFromSurface(g_renderer, surface);
	SDL_FreeSurface(surface);
	assert(g_spritesheet_buildings != NULL);

	/* Load the spritesheet for glyphs of the font.
	 * The image uses opaque black on a fully transparent background
	 * (these colors make it a bit nicer to edit in a pixel-art editor),
	 * but in order to change the color of the glyphs when rendering them, it is better to
	 * have the glyphs opaque white, so we convert opaque black to opaque white here. */
	surface = IMG_Load("../assets/images/spritesheet-font-01.png");
	assert(surface != NULL);
	SDL_Surface* surface_tmp = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(surface);
	surface = surface_tmp;
	assert(surface != NULL);
	assert(surface->format->format == SDL_PIXELFORMAT_RGBA32);
	typedef struct PixelRGBA32 {uint8_t r, g, b, a;} PixelRGBA32;
	assert(sizeof(PixelRGBA32) * 8 == surface->format->BitsPerPixel);
	SDL_LockSurface(surface);
	PixelRGBA32* src_pixels = surface->pixels;
	PixelRGBA32* dst_pixels = calloc(surface->w * surface->h, sizeof(PixelRGBA32));
	for (int y = 0; y < surface->h; y++) for (int x = 0; x < surface->w; x++) {
		PixelRGBA32 src = src_pixels[y * surface->w + x];
		PixelRGBA32* dst = &dst_pixels[y * surface->w + x];
		if (src.r == 0 && src.g == 0 && src.b == 0 && src.a == 255) {
			/* Map opaque black to opaque white. */
			*dst = (PixelRGBA32){255, 255, 255, 255};
		} else if (src.a == 0) {
			/* Ignore the fully transparent pixels. */
			*dst = src;
		} else {
			/* The font glyph spritesheet contains a pixel which is neither fully transparent
			 * nor fully opaque black, this should be handled here explicitely (like should
			 * we ignore it? what about the coloring of text? etc.). */
			assert(false);
		}
	}
	SDL_UnlockSurface(surface);
	SDL_Surface* surface_modified = SDL_CreateRGBSurfaceWithFormatFrom(dst_pixels,
		surface->w, surface->h, 32, surface->pitch, SDL_PIXELFORMAT_RGBA32);
	SDL_FreeSurface(surface);
	#if 0
		/* Can be used to see how we modify the glyph spritesheet for debugging. */
		SDL_SaveBMP(surface_modified, "modified-spritesheet-font.bmp");
	#endif
	s_spritesheet_font = SDL_CreateTextureFromSurface(g_renderer, surface_modified);
	SDL_FreeSurface(surface_modified);
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

#define SCALE 3
#define SPACING 2

void string_pixel_dims(char const* string, int* out_w, int* out_h) {
	int w = string_pixel_width(string, SCALE, SPACING);
	if (out_w != NULL) *out_w = w;
	if (out_h != NULL) *out_h = 5 * SCALE;
}

void render_string_pixel(char const* string, WinCoords wc, PinPoint pp, SDL_Color color) {
	/* TODO: Take scale and spacing as parameters, in a nice way. */

	int color_mod_is_supported_if_zero = SDL_SetTextureColorMod(s_spritesheet_font,
		color.r, color.g, color.b);
	assert(color_mod_is_supported_if_zero == 0);

	int w = string_pixel_width(string, SCALE, SPACING);
	SDL_Rect dst_full_rect = {wc.x, wc.y, w, 5 * SCALE};
	adjust_rect_for_pin_point(&dst_full_rect, pp);
	int x = dst_full_rect.x;
	int y = dst_full_rect.y;

	for (int i = 0; string[i] != '\0'; i++) {
		char c = string[i];
		if (c == ' ') {
			x += char_pixel_width(c) * SCALE + SPACING;
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
		SDL_Rect dst_glyph_rect = {x, y, char_pixel_width(c) * SCALE, 5 * SCALE};
		SDL_RenderCopy(g_renderer, s_spritesheet_font, &rect_in_spritesheet, &dst_glyph_rect);
		x += dst_glyph_rect.w + SPACING;
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

SDL_Rect tile_rect(TileCoords tc) {
	float tile_render_size = TILE_SIZE * g_camera.zoom;
	return (SDL_Rect){
		.x = tc.x * tile_render_size - g_camera.pos.x,
		.y = tc.y * tile_render_size - g_camera.pos.y,
		.w = ceilf(tile_render_size),
		.h = ceilf(tile_render_size)};
}

void render_tile_ground(TileType tile_type, SDL_Rect dst_rect) {
	SDL_Rect rect_in_spritesheet = g_tile_type_spec_table[tile_type].rect_in_spritesheet;
	SDL_RenderCopy(g_renderer, g_spritesheet, &rect_in_spritesheet, &dst_rect);
}

void render_human(SDL_Rect dst_rect) {
	SDL_Rect rect_in_spritesheet = {0, 16, 3, 4};
	SDL_RenderCopy(g_renderer, g_spritesheet, &rect_in_spritesheet, &dst_rect);
}

void render_tile_building(Building* building, SDL_Rect dst_rect) {
	if (building != NULL){
		SDL_Rect rect_in_spritesheet;
		switch (building->type)
		{
		case BUILDING_EMITTER:
			rect_in_spritesheet = g_building_type_spec_table[BUILDING_TX_EMITTER].rect_in_spritesheet;
			break;
		case BUILDING_RECEIVER:
			if (building->powered){
				rect_in_spritesheet = g_building_type_spec_table[BUILDING_TX_RECEIVER_ON].rect_in_spritesheet;
			} else {
				rect_in_spritesheet = g_building_type_spec_table[BUILDING_TX_RECEIVER_OFF].rect_in_spritesheet;
			}
			break;
		default:
			break;
		}
		SDL_RenderCopy(g_renderer, g_spritesheet_buildings, &rect_in_spritesheet, &dst_rect);
	} else {
		printf("[ERROR] Missing building !");
	}
}

void render_tile_cable(Cable* cable, SDL_Rect dst_rect) {
	if (cable != NULL){
		SDL_Rect rect_in_spritesheet;
		int angle = 0;
		bool straight = cable->connections[1] - cable->connections[0] == 2;
			// Cringe af but is just to test. The true way of storing and evaluating directions must be discussed.
			if (straight){
				rect_in_spritesheet = g_cable_type_spec_table[ELECTRICITY_STRAIGHT].rect_in_spritesheet;
				if (cable->powered){
					rect_in_spritesheet = g_cable_type_spec_table[ELECTRICITY_STRAIGHT_ON].rect_in_spritesheet;
				}

				angle = 90 * (cable->connections[0] == SOUTH);
			} else {
				rect_in_spritesheet = g_cable_type_spec_table[ELECTRICITY_TURN].rect_in_spritesheet;
				if (cable->powered){
					rect_in_spritesheet = g_cable_type_spec_table[ELECTRICITY_TURN_ON].rect_in_spritesheet;
				}
				angle = 90 * (4-cable->connections[1]) * !(cable->connections[0] == WEST && cable->connections[1] == NORTH);
			}
			SDL_RenderCopyEx(g_renderer, g_spritesheet_buildings, &rect_in_spritesheet, &dst_rect, angle, NULL, SDL_FLIP_NONE);
	}
}

/* Is grid line display enabled? */
bool g_render_lines = false;

void render_map(void) {
	float tile_render_size = TILE_SIZE * g_camera.zoom;

	/* Draw tile terrain. */
	for (int i = 0; i < N_TILES; ++i) {
		TileCoords tc = {.x = i % g_map_w, .y = i / g_map_w};
		Tile const* tile = get_tile(tc);
		SDL_Rect rect = tile_rect(tc);
		render_tile_ground(tile->type, rect);
	}

	if (g_render_lines) {
		/* Draw grid lines. */
		SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
		for (int y = 0; y < g_map_w+1; ++y) {
			WinCoords a = tile_coords_to_window_pixel((TileCoords){0, y});
			WinCoords b = tile_coords_to_window_pixel((TileCoords){g_map_w, y});
			for (int i = 0; i < 2; i++) {
				SDL_RenderDrawLine(g_renderer, a.x, a.y - i, b.x, b.y - i);
			}
		}
		for (int x = 0; x < g_map_h+1; ++x) {
			WinCoords a = tile_coords_to_window_pixel((TileCoords){x, 0});
			WinCoords b = tile_coords_to_window_pixel((TileCoords){x, g_map_h});
			for (int i = 0; i < 2; i++) {
				SDL_RenderDrawLine(g_renderer, a.x - i, a.y, b.x - i, b.y);
			}
		}
	}

	/* Draw selection rect around the selected tile (if any).
	 * We do this after all the tile terrains so that terrain drawn after the
	 * selection rectangle won't cover a part of it. */
	if (g_sel_tile_exists) {
		SDL_Rect rect = tile_rect(g_sel_tile_coords);
		SDL_SetRenderDrawColor(g_renderer, 255, 255, 0, 255);
		for (int i = 0; i < 3; i++) {
			SDL_RenderDrawRect(g_renderer, &rect);
			rect.x += 1; rect.y += 1; rect.w -= 2; rect.h -= 2;
		}
	}

	for (int i = 0; i < g_action_da_on_tcs.len; i++) {
		SDL_Rect rect = tile_rect(g_action_da_on_tcs.arr[i].tc);
		SDL_Point mouse;
		SDL_GetMouseState(&mouse.x, &mouse.y);
		if (SDL_PointInRect(&mouse, &rect)) {
			SDL_SetRenderDrawColor(g_renderer, 0, 255, 255, 255);
		} else {
			SDL_SetRenderDrawColor(g_renderer, 0, 0, 255, 255);
		}
		for (int i = 0; i < 2; i++) {
			rect.x += 1; rect.y += 1; rect.w -= 2; rect.h -= 2;
			SDL_RenderDrawRect(g_renderer, &rect);
		}
	}

	/* Draw entities, buildings and flows.
	 * We do this after the tile backgrounds so that these can cover a part
	 * of neighboring tiles. */
	for (int i = 0; i < N_TILES; ++i) {
		TileCoords tc = {.x = i % g_map_w, .y = i / g_map_w};
		Tile const* tile = get_tile(tc);
		SDL_Rect rect = tile_rect(tc);

		/* Draw flows. */
		for (int cable_i = 0; cable_i < tile->cable_count; cable_i++){
			Cable* cable = tile->cables[cable_i];
			assert(cable != NULL);
			render_tile_cable(cable, rect);
		}

		/* Draw building. */
		if (tile->building != NULL){
			render_tile_building(tile->building, rect);
		}

		/* Draw entities.
		 * Note that `tile->ents.arr` can contain empty cells, so we cannot count entities
		 * with this dynamic array and must use `real_ent_count` and `real_ent_i` instead. */
		int real_ent_count = get_tile_real_ent_count(tile);
		int real_ent_i = -1;
		for (int ent_i = 0; ent_i < tile->ents.len; ent_i++) {
			EntId eid = (tile->ents.arr[ent_i]);
			Ent* ent = get_ent(eid);
			if (ent == NULL) continue;
			real_ent_i++;

			/* Entity position on screen. */
			int ex = (float)(real_ent_i+1) / (float)(real_ent_count+1)
				* tile_render_size;
			int ey = (1.0f - (float)(real_ent_i+1) / (float)(real_ent_count+1))
				* tile_render_size;

			/* Account for an ongoing animation (if any). */
			if (ent->anim != NULL &&
				ent->anim->time_beginning <= g_time_ms && g_time_ms < ent->anim->time_end
			) {
				float interpolation =
					(float)(g_time_ms - ent->anim->time_beginning) /
					(float)(ent->anim->time_end - ent->anim->time_beginning);
				/* The offset is maximal at the beginning of the animation (when `interpolation`
				 * is zero) and tends toward zero as `interpolation` tends toward one. */
				ex -= ent->anim->offset_beginning_x * tile_render_size * (1.0f - interpolation);
				ey -= ent->anim->offset_beginning_y * tile_render_size * (1.0f - interpolation);
			}

			switch (ent->type) {
				case ENT_HUMAN: {
					int ew = 0.3f * tile_render_size;
					int eh = 0.4f * tile_render_size;

					/* Draw human sprite. */
					SDL_Rect ent_rect = {
						rect.x + ex - ew / 2.0f, rect.y + ey - eh / 2.0f, ew, eh};
					if (g_sel_ent_exists && eid_eq(g_sel_ent_id, eid)) {
						/* If the entity is selected, then make it a bit bigger
						 * its fixed point is its bottom middle point as its looks better that way. */
						ent_rect.x -= 2; ent_rect.w += 4;
						ent_rect.y -= 4; ent_rect.h += 4;
					}
					render_human(ent_rect);

					/* Draw faction color square. */
					SDL_Color color = g_faction_spec_table[ent->human.faction].color;
					SDL_SetRenderDrawColor(g_renderer, color.r, color.g, color.b, 255);
					#define FACTION_SIDE 8
					SDL_Rect faction_rect = {
						ent_rect.x + ent_rect.w/2 - FACTION_SIDE/2,
						ent_rect.y - FACTION_SIDE/2 - FACTION_SIDE,
						FACTION_SIDE, FACTION_SIDE};
					#undef FACTION_SIDE
					if (g_sel_ent_exists && eid_eq(g_sel_ent_id, eid)) {
						faction_rect.y -= 2;
					}
					SDL_RenderFillRect(g_renderer, &faction_rect);

					/* Draw some black and white boarders around the faction color square of the
					 * selected entity (if any). */
					if (g_sel_ent_exists && eid_eq(g_sel_ent_id, eid)) {
						SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
						for (int i = 0; i < 2; i++) {
							faction_rect.x-=1; faction_rect.y-=1; faction_rect.w+=2; faction_rect.h+=2;
							SDL_RenderDrawRect(g_renderer, &faction_rect);
						}
						SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
						for (int i = 0; i < 2; i++) {
							faction_rect.x-=1; faction_rect.y-=1; faction_rect.w+=2; faction_rect.h+=2;
							SDL_RenderDrawRect(g_renderer, &faction_rect);
						}
					}
				break; }

				case ENT_TEST_BLOCK: {
					int ew = 0.2f * tile_render_size;
					int eh = 0.2f * tile_render_size;
					SDL_Rect ent_rect = {rect.x + ex - ew / 2.0f, rect.y + ey - eh / 2.0f, ew, eh};
					SDL_Color color = ent->test_block.color;
					SDL_SetRenderDrawColor(g_renderer, color.r, color.g, color.b, 255);
					SDL_RenderFillRect(g_renderer, &ent_rect);
				break; }

				default: assert(false);
			}
		}
	}
}
