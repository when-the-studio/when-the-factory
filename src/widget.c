#include <assert.h>
#include "widget.h"
#include "renderer.h"

/* TODO: Move to some "utils.c" source file. */
int max(int a, int b) {
	return a < b ? b : a;
}

Widget* g_wg_root = NULL;

/* *** Text Line widget section *** */

struct WidgetTextLine {
	Widget base;
	char* string;
	SDL_Color fg_color;
};
typedef struct WidgetTextLine WidgetTextLine;

Widget* new_widget_text_line(char* string, SDL_Color fg_color) {
	WidgetTextLine* widget = malloc(sizeof(WidgetTextLine));
	*widget = (WidgetTextLine){
		.base = {
			.type = WIDGET_TEXT_LINE,
		},
		.string = string,
		.fg_color = fg_color,
	};
	return (Widget*)widget;
}

static Dims widget_text_line_get_dims(WidgetTextLine const* widget) {
	Dims dims;
	string_pixel_dims(widget->string, &dims.w, &dims.h);
	return dims;
}

static void widget_text_line_render(WidgetTextLine const* widget, int x, int y) {
	render_string_pixel(widget->string, (WinCoords){x, y}, PP_TOP_LEFT, widget->fg_color);
}

static bool widget_text_line_click(WidgetTextLine const* widget, int x, int y, int cx, int cy) {
	Dims dims = widget_text_line_get_dims(widget);
	SDL_Rect r = {x, y, dims.w, dims.h};
	return r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h;
}

/* *** Multiple Top Left Top To Bottom widget section *** */

struct WidgetMultipleTopLeftTopToBottom {
	Widget base;
	Widget** sub_wgs;
	int sub_wgs_count;
	int spacing;
	int offset_x, offset_y;
};
typedef struct WidgetMultipleTopLeftTopToBottom WidgetMultipleTopLeftTopToBottom;

Widget* new_widget_mtlttb(int spacing, int offset_x, int offset_y) {
	WidgetMultipleTopLeftTopToBottom* widget = malloc(sizeof(WidgetMultipleTopLeftTopToBottom));
	*widget = (WidgetMultipleTopLeftTopToBottom){
		.base = {
			.type = WIDGET_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM,
		},
		.sub_wgs = NULL,
		.sub_wgs_count = 0,
		.spacing = spacing,
		.offset_x = offset_x,
		.offset_y = offset_y,
	};
	return (Widget*)widget;
}

void widget_mtlttb_add_sub(Widget* widget_, Widget* sub) {
	assert(widget_->type == WIDGET_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM);
	WidgetMultipleTopLeftTopToBottom* widget = (WidgetMultipleTopLeftTopToBottom*)widget_;
	if (widget->sub_wgs_count == 0) {
		assert(widget->sub_wgs == NULL);
		widget->sub_wgs_count = 1;
		widget->sub_wgs = malloc(widget->sub_wgs_count * sizeof(Widget*));
	} else {
		assert(widget->sub_wgs != NULL);
		widget->sub_wgs_count++;
		widget->sub_wgs = realloc(widget->sub_wgs, widget->sub_wgs_count * sizeof(Widget*));
	}
	widget->sub_wgs[widget->sub_wgs_count-1] = sub;
}

static Dims widget_multiple_top_left_top_to_bottom_get_dims(WidgetMultipleTopLeftTopToBottom const* widget) {
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

static void widget_multiple_top_left_top_to_bottom_render(WidgetMultipleTopLeftTopToBottom const* widget, int x, int y) {
	x += widget->offset_x;
	y += widget->offset_y;
	for (int i = 0; i < widget->sub_wgs_count; i++) {
		widget_render(widget->sub_wgs[i], x, y);
		Dims sub_dims = widget_get_dims(widget->sub_wgs[i]);
		y += sub_dims.h + widget->spacing;
	}
}

static bool widget_multiple_top_left_top_to_bottom_click(WidgetMultipleTopLeftTopToBottom const* widget, int x, int y, int cx, int cy) {
	x += widget->offset_x;
	y += widget->offset_y;
	for (int i = 0; i < widget->sub_wgs_count; i++) {
		Dims sub_dims = widget_get_dims(widget->sub_wgs[i]);
		SDL_Rect r = {x, y, sub_dims.w, sub_dims.h};
		if (r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h) {
			return widget_click(widget->sub_wgs[i], x, y, cx, cy);
		}
		y += sub_dims.h + widget->spacing;
	}
	return false;
}

/* *** Button widget section *** */

struct WidgetButton {
	Widget base;
	Widget* sub_wg;
	void* whatever;
	void (*left_click_callback)(void* whatever);
};
typedef struct WidgetButton WidgetButton;

Widget* new_widget_button(Widget* sub_wg, void* whatever, void (*left_click_callback)(void* whatever)) {
	WidgetButton* widget = malloc(sizeof(WidgetButton));
	*widget = (WidgetButton){
		.base = {
			.type = WIDGET_BUTTON,
		},
		.sub_wg = sub_wg,
		.whatever = whatever,
		.left_click_callback = left_click_callback,
	};
	return (Widget*)widget;
}

static Dims widget_button_get_dims(WidgetButton const* widget) {
	return widget_get_dims(widget->sub_wg);
}

static void widget_button_render(WidgetButton const* widget, int x, int y) {
	widget_render(widget->sub_wg, x, y);
}

static bool widget_button_click(WidgetButton const* widget, int x, int y, int cx, int cy) {
	Dims sub_dims = widget_get_dims(widget->sub_wg);
	SDL_Rect r = {x, y, sub_dims.w, sub_dims.h};
	if (r.x <= cx && cx < r.x + r.w && r.y <= cy && cy < r.y + r.h) {
		widget->left_click_callback(widget->whatever);
		return true;
	}
	return false;
}

/* *** Dynamic dispatch section *** */

Dims widget_get_dims(Widget const* widget) {
	switch (widget->type) {
		case WIDGET_TEXT_LINE:
			return widget_text_line_get_dims((WidgetTextLine const*)widget);
		break;
		case WIDGET_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM:
			return widget_multiple_top_left_top_to_bottom_get_dims((WidgetMultipleTopLeftTopToBottom const*)widget);
		break;
		case WIDGET_BUTTON:
			return widget_button_get_dims((WidgetButton const*)widget);
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
		case WIDGET_BUTTON:
			widget_button_render((WidgetButton const*)widget, x, y);
		break;
		default: assert(false);
	}
}

bool widget_click(Widget const* widget, int x, int y, int cx, int cy) {
	switch (widget->type) {
		case WIDGET_TEXT_LINE:
			return widget_text_line_click((WidgetTextLine const*)widget, x, y, cx, cy);
		break;
		case WIDGET_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM:
			return widget_multiple_top_left_top_to_bottom_click((WidgetMultipleTopLeftTopToBottom const*)widget, x, y, cx, cy);
		break;
		case WIDGET_BUTTON:
			return widget_button_click((WidgetButton const*)widget, x, y, cx, cy);
		break;
		default: assert(false);
	}
}
