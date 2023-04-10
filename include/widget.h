#ifndef WHEN_THE_FACTORY_WIDGET_
#define WHEN_THE_FACTORY_WIDGET_

#include <stdbool.h>
#include <SDL2/SDL.h>

/* TODO: Move to some "utils.h" header file. */
struct Dims {
	int w, h;
};
typedef struct Dims Dims;

/* TODO: Move to some "utils.h" header file. */
struct CallbackWithData {
	void (*func)(void* whatever);
	void* whatever;
};
typedef struct CallbackWithData CallbackWithData;


enum WgType {
	WG_TEXT_LINE,
	WG_MULTIPLE_TOP_LEFT,
	WG_BUTTON,
	WG_BOX,
	WG_SPRITE,
};
typedef enum WgType WgType;

enum Orientation {
	ORIENTATION_TOP_TO_BOTTOM,
	ORIENTATION_LEFT_TO_RIGHT,
};
typedef enum Orientation Orientation;

/* Widget, a UI component that may be part of a tree of widgets. */
typedef struct Wg Wg;
struct Wg {
	WgType type;
	union {
		struct WgDataTextLine {
			char* string;
			SDL_Color fg_color;
		} text_line;
		struct WgDataMulTopLeft {
			Wg** sub_wgs;
			int sub_wgs_count;
			int spacing;
			int offset_x, offset_y;
			Orientation orientation;
		} multl;
		struct WgDataButton {
			Wg* sub_wg;
			CallbackWithData left_click_callback;
		} button;
		struct WgDataBox {
			Wg* sub_wg;
			int margin_x, margin_y;
			int line_thickness;
			SDL_Color line_color, bg_color;
		} box;
		struct WgDataSprite {
			SDL_Rect rect_in_spritesheet;
			int w, h;
		} sprite;
	};
};

/* The root of the whole widget tree. */
extern Wg* g_wg_root;

Dims wg_get_dims(Wg const* wg);
void wg_render(Wg const* wg, int x, int y);
bool wg_click(Wg const* wg, int x, int y, int cx, int cy);

void wg_delete(Wg* wg);

Wg* new_wg_text_line(char* string, SDL_Color fg_color);

Wg* new_wg_multopleft(int spacing, int offset_x, int offset_y, Orientation orientation);
void wg_multopleft_add_sub(Wg* wg, Wg* sub);
void wg_multopleft_empty(Wg* wg);

Wg* new_wg_button(Wg* sub_wg, CallbackWithData left_click_callback);

Wg* new_wg_box(Wg* sub_wg, int margin_x, int margin_y, int line_thickness,
	SDL_Color line_color, SDL_Color bg_color);

Wg* new_wg_sprite(SDL_Rect rect_in_spritesheet, int w, int h);

#endif /* WHEN_THE_FACTORY_WIDGET_ */
