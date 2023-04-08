#include <assert.h>
#include "widget.h"
#include "renderer.h"

/* TODO: Move to some "utils.c" source file. */
int max(int a, int b) {
	return a < b ? b : a;
}

Wg* g_wg_root = NULL;

/* *** Text Line widget section *** */

struct WgTextLine {
	Wg base;
	char* string;
	SDL_Color fg_color;
};
typedef struct WgTextLine WgTextLine;

Wg* new_wg_text_line(char* string, SDL_Color fg_color) {
	WgTextLine* wg = malloc(sizeof(WgTextLine));
	*wg = (WgTextLine){
		.base = {
			.type = WG_TEXT_LINE,
		},
		.string = string,
		.fg_color = fg_color,
	};
	return (Wg*)wg;
}

static Dims wg_text_line_get_dims(WgTextLine const* wg) {
	Dims dims;
	string_pixel_dims(wg->string, &dims.w, &dims.h);
	return dims;
}

static void wg_text_line_render(WgTextLine const* wg, int x, int y) {
	render_string_pixel(wg->string, (WinCoords){x, y}, PP_TOP_LEFT, wg->fg_color);
}

static bool wg_text_line_click(WgTextLine const* wg, int x, int y, int cx, int cy) {
	Dims dims = wg_text_line_get_dims(wg);
	SDL_Rect r = {x, y, dims.w, dims.h};
	return r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h;
}

static void wg_text_line_delete(WgTextLine* wg) {
	/* TODO: Do something to free `wg->string` if and only if it has to be freed,
	 * or else we are going to leak memory when we generate new strings for these widgets. */
	free(wg);
}

/* *** Multiple Top Left widget section *** */

struct WgMulTopLeft {
	Wg base;
	Wg** sub_wgs;
	int sub_wgs_count;
	int spacing;
	int offset_x, offset_y;
	Orientation orientation;
};
typedef struct WgMulTopLeft WgMulTopLeft;

Wg* new_wg_multopleft(int spacing, int offset_x, int offset_y, Orientation orientation) {
	WgMulTopLeft* wg = malloc(sizeof(WgMulTopLeft));
	*wg = (WgMulTopLeft){
		.base = {
			.type = WG_MULTIPLE_TOP_LEFT,
		},
		.sub_wgs = NULL,
		.sub_wgs_count = 0,
		.spacing = spacing,
		.offset_x = offset_x,
		.offset_y = offset_y,
		.orientation = orientation,
	};
	return (Wg*)wg;
}

void wg_multopleft_add_sub(Wg* wg_, Wg* sub) {
	assert(wg_->type == WG_MULTIPLE_TOP_LEFT);
	WgMulTopLeft* wg = (WgMulTopLeft*)wg_;
	if (wg->sub_wgs_count == 0) {
		assert(wg->sub_wgs == NULL);
		wg->sub_wgs_count = 1;
		wg->sub_wgs = malloc(wg->sub_wgs_count * sizeof(Wg*));
	} else {
		assert(wg->sub_wgs != NULL);
		wg->sub_wgs_count++;
		wg->sub_wgs = realloc(wg->sub_wgs, wg->sub_wgs_count * sizeof(Wg*));
	}
	wg->sub_wgs[wg->sub_wgs_count-1] = sub;
}

void wg_multopleft_empty(Wg* wg_) {
	assert(wg_->type == WG_MULTIPLE_TOP_LEFT);
	WgMulTopLeft* wg = (WgMulTopLeft*)wg_;
	for (int i = 0; i < wg->sub_wgs_count; i++) {
		wg_delete(wg->sub_wgs[i]);
	}
	free(wg->sub_wgs);
	wg->sub_wgs = NULL;
	wg->sub_wgs_count = 0;
}

static Dims wg_multopleft_get_dims(WgMulTopLeft const* wg) {
	Dims dims = {0, 0};
	for (int i = 0; i < wg->sub_wgs_count; i++) {
		Dims sub_dims = wg_get_dims(wg->sub_wgs[i]);
		switch (wg->orientation) {
			case ORIENTATION_TOP_TO_BOTTOM:
				if (i != 0) {
					dims.h += wg->spacing;
				}
				dims.h += sub_dims.h;
				dims.w = max(dims.w, sub_dims.w);
			break;
			case ORIENTATION_LEFT_TO_RIGHT:
				if (i != 0) {
					dims.w += wg->spacing;
				}
				dims.w += sub_dims.w;
				dims.h = max(dims.h, sub_dims.h);
			break;
			default: assert(false);
		}
	}
	return dims;
}

static void wg_multopleft_render(WgMulTopLeft const* wg, int x, int y) {
	x += wg->offset_x;
	y += wg->offset_y;
	for (int i = 0; i < wg->sub_wgs_count; i++) {
		wg_render(wg->sub_wgs[i], x, y);
		Dims sub_dims = wg_get_dims(wg->sub_wgs[i]);
		switch (wg->orientation) {
			case ORIENTATION_TOP_TO_BOTTOM: y += sub_dims.h + wg->spacing; break;
			case ORIENTATION_LEFT_TO_RIGHT: x += sub_dims.w + wg->spacing; break;
			default: assert(false);
		}
	}
}

static bool wg_multopleft_click(WgMulTopLeft const* wg, int x, int y, int cx, int cy) {
	x += wg->offset_x;
	y += wg->offset_y;
	for (int i = 0; i < wg->sub_wgs_count; i++) {
		Dims sub_dims = wg_get_dims(wg->sub_wgs[i]);
		#if 0
		/* TODO: This does not work sometimes for the rightmost button of a left-to-right.
		 * The reason for this bug seem non-obvious enough that it warrants investigation. */
		SDL_Rect r = {x, y, sub_dims.w, sub_dims.h};
		if (r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h) {
			if (wg_click(wg->sub_wgs[i], x, y, cx, cy)) {
				return true;
			}
		}
		#else
		if (wg_click(wg->sub_wgs[i], x, y, cx, cy)) {
			return true;
		}
		#endif
		switch (wg->orientation) {
			case ORIENTATION_TOP_TO_BOTTOM: y += sub_dims.h + wg->spacing; break;
			case ORIENTATION_LEFT_TO_RIGHT: x += sub_dims.w + wg->spacing; break;
			default: assert(false);
		}
	}
	return false;
}

static void wg_multopleft_delete(WgMulTopLeft* wg) {
	wg_multopleft_empty((Wg*)wg);
	free(wg);
}

/* *** Button widget section *** */

struct WgButton {
	Wg base;
	Wg* sub_wg;
	CallbackWithData left_click_callback;
};
typedef struct WgButton WgButton;

Wg* new_wg_button(Wg* sub_wg, CallbackWithData left_click_callback) {
	WgButton* wg = malloc(sizeof(WgButton));
	*wg = (WgButton){
		.base = {
			.type = WG_BUTTON,
		},
		.sub_wg = sub_wg,
		.left_click_callback = left_click_callback,
	};
	return (Wg*)wg;
}

static Dims wg_button_get_dims(WgButton const* wg) {
	return wg_get_dims(wg->sub_wg);
}

static void wg_button_render(WgButton const* wg, int x, int y) {
	wg_render(wg->sub_wg, x, y);
}

static bool wg_button_click(WgButton const* wg, int x, int y, int cx, int cy) {
	Dims sub_dims = wg_get_dims(wg->sub_wg);
	SDL_Rect r = {x, y, sub_dims.w, sub_dims.h};
	if (r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h) {
		wg->left_click_callback.func(wg->left_click_callback.whatever);
		return true;
	}
	return false;
}

static void wg_button_delete(WgButton* wg) {
	/* TODO: Do something to free `wg->whatever` if and only if it has to be freed,
	 * or else we are going to leak memory at some point. */
	wg_delete(wg->sub_wg);
	free(wg);
}

/* *** Box widget section *** */

struct WgBox {
	Wg base;
	Wg* sub_wg;
	int margin_x, margin_y;
	int line_thickness;
	SDL_Color line_color, bg_color;
};
typedef struct WgBox WgBox;

Wg* new_wg_box(Wg* sub_wg, int margin_x, int margin_y, int line_thickness,
	SDL_Color line_color, SDL_Color bg_color
) {
	WgBox* wg = malloc(sizeof(WgBox));
	*wg = (WgBox){
		.base = {
			.type = WG_BOX,
		},
		.sub_wg = sub_wg,
		.margin_x = margin_x,
		.margin_y = margin_y,
		.line_thickness = line_thickness,
		.line_color = line_color,
		.bg_color = bg_color,
	};
	return (Wg*)wg;
}

static Dims wg_box_get_dims(WgBox const* wg) {
	Dims dims = wg_get_dims(wg->sub_wg);
	dims.w += wg->margin_x * 2;
	dims.h += wg->margin_y * 2;
	return dims;
}

static void wg_box_render(WgBox const* wg, int x, int y) {
	Dims dims = wg_box_get_dims(wg);
	SDL_Rect r = {x, y, dims.w, dims.h};
	/* Draw the backgound. */
	SDL_SetRenderDrawColor(g_renderer,
		wg->bg_color.r, wg->bg_color.g, wg->bg_color.b, wg->bg_color.a);
	SDL_RenderFillRect(g_renderer, &r);
	/* Draw the outline (SDL_RenderDrawRect only draws with thickness of 1 pixel
	 * so we call it multiple times). */
	SDL_SetRenderDrawColor(g_renderer,
		wg->line_color.r, wg->line_color.g, wg->line_color.b, wg->line_color.a);
	for (int i = 0; i < wg->line_thickness; i++) {
		SDL_RenderDrawRect(g_renderer, &r);
		r.x++; r.y++; r.w -= 2; r.h -= 2;
	}
	/* Draw the sub widget (at the end so it covers the backgound). */
	wg_render(wg->sub_wg, x + wg->margin_x, y + wg->margin_y);
}

static bool wg_box_click(WgBox const* wg, int x, int y, int cx, int cy) {
	Dims sub_dims = wg_get_dims(wg->sub_wg);
	SDL_Rect sr = {x + wg->margin_x, y + wg->margin_y, sub_dims.w, sub_dims.h};
	Dims dims = wg_box_get_dims(wg);
	SDL_Rect r = {x, y, dims.w, dims.h};
	if (sr.x <= cx && cx < sr.x + sr.w && sr.y <= cy && cy < sr.y + sr.h) {
		return wg_click(wg->sub_wg, x + wg->margin_x, y + wg->margin_y, cx, cy);
	} else if (r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h) {
		return true;
	} else {
		return false;
	}
}

static void wg_box_delete(WgBox* wg) {
	wg_delete(wg->sub_wg);
	free(wg);
}

/* *** Sprite widget section *** */

struct WgSprite {
	Wg base;
	SDL_Rect rect_in_spritesheet;
	int w, h;
};
typedef struct WgSprite WgSprite;

Wg* new_wg_sprite(SDL_Rect rect_in_spritesheet, int w, int h) {
	WgSprite* wg = malloc(sizeof(WgSprite));
	*wg = (WgSprite){
		.base = {
			.type = WG_SPRITE,
		},
		.rect_in_spritesheet = rect_in_spritesheet,
		.w = w,
		.h = h,
	};
	return (Wg*)wg;
}

static Dims wg_sprite_get_dims(WgSprite const* wg) {
	return (Dims){wg->w, wg->h};
}

static void wg_sprite_render(WgSprite const* wg, int x, int y) {
	Dims dims = wg_sprite_get_dims(wg);
	SDL_Rect dst_rect = {x, y, dims.w, dims.h};
	SDL_RenderCopy(g_renderer, g_spritesheet, &wg->rect_in_spritesheet, &dst_rect);
}

static bool wg_sprite_click(WgSprite const* wg, int x, int y, int cx, int cy) {
	Dims dims = wg_sprite_get_dims(wg);
	SDL_Rect r = {x, y, dims.w, dims.h};
	return r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h;
}

static void wg_sprite_delete(WgSprite* wg) {
	free(wg);
}

/* *** Dynamic dispatch section *** */

Dims wg_get_dims(Wg const* wg) {
	switch (wg->type) {
		#define CASE(type_value_, func_, type_) \
			case type_value_: \
				return func_##_get_dims((type_ const*)wg); \
			break
		CASE(WG_TEXT_LINE,         wg_text_line,  WgTextLine);
		CASE(WG_MULTIPLE_TOP_LEFT, wg_multopleft, WgMulTopLeft);
		CASE(WG_BUTTON,            wg_button,     WgButton);
		CASE(WG_BOX,               wg_box,        WgBox);
		CASE(WG_SPRITE,            wg_sprite,     WgSprite);
		#undef CASE
		default: assert(false);
	}
}

void wg_render(Wg const* wg, int x, int y) {
	switch (wg->type) {
		#define CASE(type_value_, func_, type_) \
			case type_value_: \
				func_##_render((type_ const*)wg, x, y); \
			break
		CASE(WG_TEXT_LINE,         wg_text_line,  WgTextLine);
		CASE(WG_MULTIPLE_TOP_LEFT, wg_multopleft, WgMulTopLeft);
		CASE(WG_BUTTON,            wg_button,     WgButton);
		CASE(WG_BOX,               wg_box,        WgBox);
		CASE(WG_SPRITE,            wg_sprite,     WgSprite);
		#undef CASE
		default: assert(false);
	}
}

bool wg_click(Wg const* wg, int x, int y, int cx, int cy) {
	switch (wg->type) {
		#define CASE(type_value_, func_, type_) \
			case type_value_: \
				return func_##_click((type_ const*)wg, x, y, cx, cy); \
			break
		CASE(WG_TEXT_LINE,         wg_text_line,  WgTextLine);
		CASE(WG_MULTIPLE_TOP_LEFT, wg_multopleft, WgMulTopLeft);
		CASE(WG_BUTTON,            wg_button,     WgButton);
		CASE(WG_BOX,               wg_box,        WgBox);
		CASE(WG_SPRITE,            wg_sprite,     WgSprite);
		#undef CASE
		default: assert(false);
	}
}

void wg_delete(Wg* wg) {
	switch (wg->type) {
		#define CASE(type_value_, func_, type_) \
			case type_value_: \
				func_##_delete((type_*)wg); \
			break
		CASE(WG_TEXT_LINE,         wg_text_line,  WgTextLine);
		CASE(WG_MULTIPLE_TOP_LEFT, wg_multopleft, WgMulTopLeft);
		CASE(WG_BUTTON,            wg_button,     WgButton);
		CASE(WG_BOX,               wg_box,        WgBox);
		CASE(WG_SPRITE,            wg_sprite,     WgSprite);
		#undef CASE
		default: assert(false);
	}
}
