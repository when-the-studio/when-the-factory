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

/* *** Multiple Top Left Top To Bottom widget section *** */

struct WgMTLTTB {
	Wg base;
	Wg** sub_wgs;
	int sub_wgs_count;
	int spacing;
	int offset_x, offset_y;
};
typedef struct WgMTLTTB WgMTLTTB;

Wg* new_wg_mtlttb(int spacing, int offset_x, int offset_y) {
	WgMTLTTB* wg = malloc(sizeof(WgMTLTTB));
	*wg = (WgMTLTTB){
		.base = {
			.type = WG_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM,
		},
		.sub_wgs = NULL,
		.sub_wgs_count = 0,
		.spacing = spacing,
		.offset_x = offset_x,
		.offset_y = offset_y,
	};
	return (Wg*)wg;
}

void wg_mtlttb_add_sub(Wg* wg_, Wg* sub) {
	assert(wg_->type == WG_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM);
	WgMTLTTB* wg = (WgMTLTTB*)wg_;
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

void wg_mtlttb_empty(Wg* wg_) {
	assert(wg_->type == WG_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM);
	WgMTLTTB* wg = (WgMTLTTB*)wg_;
	for (int i = 0; i < wg->sub_wgs_count; i++) {
		wg_delete(wg->sub_wgs[i]);
	}
	free(wg->sub_wgs);
	wg->sub_wgs = NULL;
	wg->sub_wgs_count = 0;
}

static Dims wg_mtlttb_get_dims(WgMTLTTB const* wg) {
	Dims dims = {0, 0};
	for (int i = 0; i < wg->sub_wgs_count; i++) {
		Dims sub_dims = wg_get_dims(wg->sub_wgs[i]);
		dims.h += sub_dims.h;
		if (i != 0) {
			dims.h += wg->spacing;
		}
		dims.w = max(dims.w, sub_dims.w);
	}
	return dims;
}

static void wg_mtlttb_render(WgMTLTTB const* wg, int x, int y) {
	x += wg->offset_x;
	y += wg->offset_y;
	for (int i = 0; i < wg->sub_wgs_count; i++) {
		wg_render(wg->sub_wgs[i], x, y);
		Dims sub_dims = wg_get_dims(wg->sub_wgs[i]);
		y += sub_dims.h + wg->spacing;
	}
}

static bool wg_mtlttb_click(WgMTLTTB const* wg, int x, int y, int cx, int cy) {
	x += wg->offset_x;
	y += wg->offset_y;
	for (int i = 0; i < wg->sub_wgs_count; i++) {
		Dims sub_dims = wg_get_dims(wg->sub_wgs[i]);
		SDL_Rect r = {x, y, sub_dims.w, sub_dims.h};
		if (r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h) {
			return wg_click(wg->sub_wgs[i], x, y, cx, cy);
		}
		y += sub_dims.h + wg->spacing;
	}
	return false;
}

static void wg_mtlttb_delete(WgMTLTTB* wg) {
	wg_mtlttb_empty((Wg*)wg);
	free(wg);
}

/* *** Button widget section *** */

struct WgButton {
	Wg base;
	Wg* sub_wg;
	void* whatever;
	void (*left_click_callback)(void* whatever);
};
typedef struct WgButton WgButton;

Wg* new_wg_button(Wg* sub_wg, void* whatever, void (*left_click_callback)(void* whatever)) {
	WgButton* wg = malloc(sizeof(WgButton));
	*wg = (WgButton){
		.base = {
			.type = WG_BUTTON,
		},
		.sub_wg = sub_wg,
		.whatever = whatever,
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
		wg->left_click_callback(wg->whatever);
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

/* *** Dynamic dispatch section *** */

Dims wg_get_dims(Wg const* wg) {
	switch (wg->type) {
		case WG_TEXT_LINE:
			return wg_text_line_get_dims((WgTextLine const*)wg);
		break;
		case WG_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM:
			return wg_mtlttb_get_dims((WgMTLTTB const*)wg);
		break;
		case WG_BUTTON:
			return wg_button_get_dims((WgButton const*)wg);
		break;
		default: assert(false);
	}
}

void wg_render(Wg const* wg, int x, int y) {
	switch (wg->type) {
		case WG_TEXT_LINE:
			wg_text_line_render((WgTextLine const*)wg, x, y);
		break;
		case WG_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM:
			wg_mtlttb_render((WgMTLTTB const*)wg, x, y);
		break;
		case WG_BUTTON:
			wg_button_render((WgButton const*)wg, x, y);
		break;
		default: assert(false);
	}
}

bool wg_click(Wg const* wg, int x, int y, int cx, int cy) {
	switch (wg->type) {
		case WG_TEXT_LINE:
			return wg_text_line_click((WgTextLine const*)wg, x, y, cx, cy);
		break;
		case WG_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM:
			return wg_mtlttb_click((WgMTLTTB const*)wg, x, y, cx, cy);
		break;
		case WG_BUTTON:
			return wg_button_click((WgButton const*)wg, x, y, cx, cy);
		break;
		default: assert(false);
	}
}

void wg_delete(Wg* wg) {
	switch (wg->type) {
		case WG_TEXT_LINE:
			wg_text_line_delete((WgTextLine*)wg);
		break;
		case WG_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM:
			wg_mtlttb_delete((WgMTLTTB*)wg);
		break;
		case WG_BUTTON:
			wg_button_delete((WgButton*)wg);
		break;
		default: assert(false);
	}
}
