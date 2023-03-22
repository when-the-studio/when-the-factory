#ifndef WHEN_THE_FACTORY_WIDGET_
#define WHEN_THE_FACTORY_WIDGET_

#include <stdbool.h>
#include <SDL2/SDL.h>

struct Dims {
	int w, h;
};
typedef struct Dims Dims;

enum WidgetType {
	WIDGET_TEXT_LINE,
	WIDGET_MULTIPLE_TOP_LEFT_TOP_TO_BOTTOM,
	WIDGET_BUTTON,
};
typedef enum WidgetType WidgetType;

struct Widget {
	WidgetType type;
};
typedef struct Widget Widget;

/* The root of the whole widget tree. */
extern Widget* g_wg_root;

Dims widget_get_dims(Widget const* widget);
void widget_render(Widget const* widget, int x, int y);
bool widget_click(Widget const* widget, int x, int y, int cx, int cy);

Widget* new_widget_text_line(char* string, SDL_Color fg_color);

Widget* new_widget_mtlttb(int spacing, int offset_x, int offset_y);
void widget_mtlttb_add_sub(Widget* widget, Widget* sub);

Widget* new_widget_button(Widget* sub_wg, void* whatever, void (*left_click_callback)(void* whatever));

#endif /* WHEN_THE_FACTORY_WIDGET_ */
