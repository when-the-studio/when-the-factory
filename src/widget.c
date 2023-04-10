#include <assert.h>
#include "widget.h"
#include "renderer.h"

Wg* g_wg_root = NULL;

/* *** Text Line widget section *** */

Wg* new_wg_text_line(char* string, SDL_Color fg_color) {
	Wg* wg = malloc(sizeof(Wg));
	*wg = (Wg){
		.type = WG_TEXT_LINE,
		.text_line = {
			.string = string,
			.fg_color = fg_color,
		}
	};
	return wg;
}

static Dims wg_text_line_get_dims(Wg const* wg) {
	assert(wg->type == WG_TEXT_LINE);
	Dims dims;
	string_pixel_dims(wg->text_line.string, &dims.w, &dims.h);
	return dims;
}

static void wg_text_line_render(Wg const* wg, int x, int y) {
	assert(wg->type == WG_TEXT_LINE);
	render_string_pixel(wg->text_line.string, (WinCoords){x, y}, PP_TOP_LEFT, wg->text_line.fg_color);
}

static bool wg_text_line_click(Wg const* wg, int x, int y, int cx, int cy) {
	assert(wg->type == WG_TEXT_LINE);
	Dims dims = wg_text_line_get_dims(wg);
	SDL_Rect r = {x, y, dims.w, dims.h};
	return r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h;
}

static void wg_text_line_delete(Wg* wg) {
	assert(wg->type == WG_TEXT_LINE);
	/* TODO: Do something to free `wg->string` if and only if it has to be freed,
	 * or else we are going to leak memory when we generate new strings for these widgets. */
	free(wg);
}

/* *** Multiple Top Left widget section *** */

Wg* new_wg_multopleft(int spacing, int offset_x, int offset_y, Orientation orientation) {
	Wg* wg = malloc(sizeof(Wg));
	*wg = (Wg){
		.type = WG_MULTIPLE_TOP_LEFT,
		.multl = {
			.sub_wgs = NULL,
			.sub_wgs_count = 0,
			.spacing = spacing,
			.offset_x = offset_x,
			.offset_y = offset_y,
			.orientation = orientation,
		}
	};
	return wg;
}

void wg_multopleft_add_sub(Wg* wg, Wg* sub) {
	assert(wg->type == WG_MULTIPLE_TOP_LEFT);
	if (wg->multl.sub_wgs_count == 0) {
		assert(wg->multl.sub_wgs == NULL);
		wg->multl.sub_wgs_count = 1;
		wg->multl.sub_wgs = malloc(wg->multl.sub_wgs_count * sizeof(Wg*));
	} else {
		assert(wg->multl.sub_wgs != NULL);
		wg->multl.sub_wgs_count++;
		wg->multl.sub_wgs = realloc(wg->multl.sub_wgs, wg->multl.sub_wgs_count * sizeof(Wg*));
	}
	wg->multl.sub_wgs[wg->multl.sub_wgs_count-1] = sub;
}

void wg_multopleft_empty(Wg* wg) {
	assert(wg->type == WG_MULTIPLE_TOP_LEFT);
	for (int i = 0; i < wg->multl.sub_wgs_count; i++) {
		wg_delete(wg->multl.sub_wgs[i]);
	}
	free(wg->multl.sub_wgs);
	wg->multl.sub_wgs = NULL;
	wg->multl.sub_wgs_count = 0;
}

static Dims wg_multopleft_get_dims(Wg const* wg) {
	assert(wg->type == WG_MULTIPLE_TOP_LEFT);
	Dims dims = {0, 0};
	for (int i = 0; i < wg->multl.sub_wgs_count; i++) {
		Dims sub_dims = wg_get_dims(wg->multl.sub_wgs[i]);
		switch (wg->multl.orientation) {
			case ORIENTATION_TOP_TO_BOTTOM:
				if (i != 0) {
					dims.h += wg->multl.spacing;
				}
				dims.h += sub_dims.h;
				dims.w = max(dims.w, sub_dims.w);
			break;
			case ORIENTATION_LEFT_TO_RIGHT:
				if (i != 0) {
					dims.w += wg->multl.spacing;
				}
				dims.w += sub_dims.w;
				dims.h = max(dims.h, sub_dims.h);
			break;
			default: assert(false);
		}
	}
	return dims;
}

static void wg_multopleft_render(Wg const* wg, int x, int y) {
	assert(wg->type == WG_MULTIPLE_TOP_LEFT);
	x += wg->multl.offset_x;
	y += wg->multl.offset_y;
	for (int i = 0; i < wg->multl.sub_wgs_count; i++) {
		wg_render(wg->multl.sub_wgs[i], x, y);
		Dims sub_dims = wg_get_dims(wg->multl.sub_wgs[i]);
		switch (wg->multl.orientation) {
			case ORIENTATION_TOP_TO_BOTTOM: y += sub_dims.h + wg->multl.spacing; break;
			case ORIENTATION_LEFT_TO_RIGHT: x += sub_dims.w + wg->multl.spacing; break;
			default: assert(false);
		}
	}
}

static bool wg_multopleft_click(Wg const* wg, int x, int y, int cx, int cy) {
	assert(wg->type == WG_MULTIPLE_TOP_LEFT);
	x += wg->multl.offset_x;
	y += wg->multl.offset_y;
	for (int i = 0; i < wg->multl.sub_wgs_count; i++) {
		Dims sub_dims = wg_get_dims(wg->multl.sub_wgs[i]);
		#if 0
		/* TODO: This does not work sometimes for the rightmost button of a left-to-right.
		 * The reason for this bug seem non-obvious enough that it warrants investigation. */
		SDL_Rect r = {x, y, sub_dims.w, sub_dims.h};
		if (r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h) {
			if (wg_click(wg->multl.sub_wgs[i], x, y, cx, cy)) {
				return true;
			}
		}
		#else
		if (wg_click(wg->multl.sub_wgs[i], x, y, cx, cy)) {
			return true;
		}
		#endif
		switch (wg->multl.orientation) {
			case ORIENTATION_TOP_TO_BOTTOM: y += sub_dims.h + wg->multl.spacing; break;
			case ORIENTATION_LEFT_TO_RIGHT: x += sub_dims.w + wg->multl.spacing; break;
			default: assert(false);
		}
	}
	return false;
}

static void wg_multopleft_delete(Wg* wg) {
	assert(wg->type == WG_MULTIPLE_TOP_LEFT);
	wg_multopleft_empty(wg);
	free(wg);
}

/* *** Button widget section *** */

Wg* new_wg_button(Wg* sub_wg, CallbackWithData left_click_callback) {
	Wg* wg = malloc(sizeof(Wg));
	*wg = (Wg){
		.type = WG_BUTTON,
		.button = {
			.sub_wg = sub_wg,
			.left_click_callback = left_click_callback,
		}
	};
	return wg;
}

static Dims wg_button_get_dims(Wg const* wg) {
	assert(wg->type == WG_BUTTON);
	return wg_get_dims(wg->button.sub_wg);
}

static void wg_button_render(Wg const* wg, int x, int y) {
	assert(wg->type == WG_BUTTON);
	wg_render(wg->button.sub_wg, x, y);
}

static bool wg_button_click(Wg const* wg, int x, int y, int cx, int cy) {
	assert(wg->type == WG_BUTTON);
	Dims sub_dims = wg_get_dims(wg->button.sub_wg);
	SDL_Rect r = {x, y, sub_dims.w, sub_dims.h};
	if (r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h) {
		call_callback(wg->button.left_click_callback);
		return true;
	}
	return false;
}

static void wg_button_delete(Wg* wg) {
	assert(wg->type == WG_BUTTON);
	/* TODO: Do something to free `wg->button.left_click_callback.whatever`
	 * if and only if it has to be freed,
	 * or else we are going to leak memory at some point. */
	wg_delete(wg->button.sub_wg);
	free(wg);
}

/* *** Box widget section *** */

Wg* new_wg_box(Wg* sub_wg, int margin_x, int margin_y, int line_thickness,
	SDL_Color line_color, SDL_Color bg_color
) {
	Wg* wg = malloc(sizeof(Wg));
	*wg = (Wg){
		.type = WG_BOX,
		.box = {
			.sub_wg = sub_wg,
			.margin_x = margin_x,
			.margin_y = margin_y,
			.line_thickness = line_thickness,
			.line_color = line_color,
			.bg_color = bg_color,
		}
	};
	return wg;
}

static Dims wg_box_get_dims(Wg const* wg) {
	assert(wg->type == WG_BOX);
	Dims dims = wg_get_dims(wg->box.sub_wg);
	dims.w += wg->box.margin_x * 2;
	dims.h += wg->box.margin_y * 2;
	return dims;
}

static void wg_box_render(Wg const* wg, int x, int y) {
	assert(wg->type == WG_BOX);
	Dims dims = wg_box_get_dims(wg);
	SDL_Rect r = {x, y, dims.w, dims.h};
	/* Draw the backgound. */
	SDL_SetRenderDrawColor(g_renderer,
		wg->box.bg_color.r, wg->box.bg_color.g, wg->box.bg_color.b, wg->box.bg_color.a);
	SDL_RenderFillRect(g_renderer, &r);
	/* Draw the outline (SDL_RenderDrawRect only draws with thickness of 1 pixel
	 * so we call it multiple times). */
	SDL_SetRenderDrawColor(g_renderer,
		wg->box.line_color.r, wg->box.line_color.g, wg->box.line_color.b, wg->box.line_color.a);
	for (int i = 0; i < wg->box.line_thickness; i++) {
		SDL_RenderDrawRect(g_renderer, &r);
		r.x++; r.y++; r.w -= 2; r.h -= 2;
	}
	/* Draw the sub widget (at the end so it covers the backgound). */
	wg_render(wg->box.sub_wg, x + wg->box.margin_x, y + wg->box.margin_y);
}

static bool wg_box_click(Wg const* wg, int x, int y, int cx, int cy) {
	assert(wg->type == WG_BOX);
	Dims sub_dims = wg_get_dims(wg->box.sub_wg);
	SDL_Rect sr = {x + wg->box.margin_x, y + wg->box.margin_y, sub_dims.w, sub_dims.h};
	Dims dims = wg_box_get_dims(wg);
	SDL_Rect r = {x, y, dims.w, dims.h};
	if (sr.x <= cx && cx < sr.x + sr.w && sr.y <= cy && cy < sr.y + sr.h) {
		return wg_click(wg->box.sub_wg, x + wg->box.margin_x, y + wg->box.margin_y, cx, cy);
	} else if (r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h) {
		return true;
	} else {
		return false;
	}
}

static void wg_box_delete(Wg* wg) {
	assert(wg->type == WG_BOX);
	wg_delete(wg->box.sub_wg);
	free(wg);
}

/* *** Sprite widget section *** */

Wg* new_wg_sprite(SDL_Rect rect_in_spritesheet, int w, int h) {
	Wg* wg = malloc(sizeof(Wg));
	*wg = (Wg){
		.type = WG_SPRITE,
		.sprite = {
			.rect_in_spritesheet = rect_in_spritesheet,
			.w = w,
			.h = h,
		}
	};
	return wg;
}

static Dims wg_sprite_get_dims(Wg const* wg) {
	assert(wg->type == WG_SPRITE);
	return (Dims){wg->sprite.w, wg->sprite.h};
}

static void wg_sprite_render(Wg const* wg, int x, int y) {
	assert(wg->type == WG_SPRITE);
	Dims dims = wg_sprite_get_dims(wg);
	SDL_Rect dst_rect = {x, y, dims.w, dims.h};
	SDL_RenderCopy(g_renderer, g_spritesheet, &wg->sprite.rect_in_spritesheet, &dst_rect);
}

static bool wg_sprite_click(Wg const* wg, int x, int y, int cx, int cy) {
	assert(wg->type == WG_SPRITE);
	Dims dims = wg_sprite_get_dims(wg);
	SDL_Rect r = {x, y, dims.w, dims.h};
	return r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h;
}

static void wg_sprite_delete(Wg* wg) {
	assert(wg->type == WG_SPRITE);
	free(wg);
}

/* *** Dynamic dispatch section *** */

Dims wg_get_dims(Wg const* wg) {
	switch (wg->type) {
		#define CASE(type_value_, func_) \
			case type_value_: \
				return func_##_get_dims(wg); \
			break
		CASE(WG_TEXT_LINE,         wg_text_line);
		CASE(WG_MULTIPLE_TOP_LEFT, wg_multopleft);
		CASE(WG_BUTTON,            wg_button);
		CASE(WG_BOX,               wg_box);
		CASE(WG_SPRITE,            wg_sprite);
		#undef CASE
		default: assert(false);
	}
}

void wg_render(Wg const* wg, int x, int y) {
	switch (wg->type) {
		#define CASE(type_value_, func_) \
			case type_value_: \
				func_##_render(wg, x, y); \
			break
		CASE(WG_TEXT_LINE,         wg_text_line);
		CASE(WG_MULTIPLE_TOP_LEFT, wg_multopleft);
		CASE(WG_BUTTON,            wg_button);
		CASE(WG_BOX,               wg_box);
		CASE(WG_SPRITE,            wg_sprite);
		#undef CASE
		default: assert(false);
	}
}

bool wg_click(Wg const* wg, int x, int y, int cx, int cy) {
	switch (wg->type) {
		#define CASE(type_value_, func_) \
			case type_value_: \
				return func_##_click(wg, x, y, cx, cy); \
			break
		CASE(WG_TEXT_LINE,         wg_text_line);
		CASE(WG_MULTIPLE_TOP_LEFT, wg_multopleft);
		CASE(WG_BUTTON,            wg_button);
		CASE(WG_BOX,               wg_box);
		CASE(WG_SPRITE,            wg_sprite);
		#undef CASE
		default: assert(false);
	}
}

void wg_delete(Wg* wg) {
	switch (wg->type) {
		#define CASE(type_value_, func_) \
			case type_value_: \
				func_##_delete(wg); \
			break
		CASE(WG_TEXT_LINE,         wg_text_line);
		CASE(WG_MULTIPLE_TOP_LEFT, wg_multopleft);
		CASE(WG_BUTTON,            wg_button);
		CASE(WG_BOX,               wg_box);
		CASE(WG_SPRITE,            wg_sprite);
		#undef CASE
		default: assert(false);
	}
}
