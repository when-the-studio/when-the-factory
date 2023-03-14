#ifndef WHEN_THE_FACTORY_RENDERER_
#define WHEN_THE_FACTORY_RENDERER_

#include <SDL2/SDL.h>

#define WINDOW_W 1400
#define WINDOW_H 800

extern SDL_Window*   g_window;
extern SDL_Renderer* g_renderer;
extern SDL_Texture*  g_spritesheet;

/* Initialises the rendering.
 * TODO: Discuss ?*/
void renderer_init(void);

/* Coordinates of a point on the window. */
struct WinCoords {
	int x, y;
};
typedef struct WinCoords WinCoords;

/* Specifies which point of something to draw (e.g. its top-left corner, its center, etc.)
 * should end up at the given window coordinates when rendering it. */
struct PinPoint {
	float x, y;
};
typedef struct PinPoint PinPoint;

#define PP_TOP_LEFT      ((PinPoint){.x = 0.0f, .y = 0.0f})
#define PP_TOP_CENTER    ((PinPoint){.x = 0.5f, .y = 0.0f})
#define PP_TOP_RIGHT     ((PinPoint){.x = 1.0f, .y = 0.0f})
#define PP_CENTER_LEFT   ((PinPoint){.x = 0.0f, .y = 0.5f})
#define PP_CENTER_CENTER ((PinPoint){.x = 0.5f, .y = 0.5f})
#define PP_CENTER_RIGHT  ((PinPoint){.x = 1.0f, .y = 0.5f})
#define PP_BOTTOM_LEFT   ((PinPoint){.x = 0.0f, .y = 1.0f})
#define PP_BOTTOM_CENTER ((PinPoint){.x = 0.5f, .y = 1.0f})
#define PP_BOTTOM_RIGHT  ((PinPoint){.x = 1.0f, .y = 1.0f})

/* Renders the given string so that its pin point is as the given window coordinates. */
void render_string(char const* string, WinCoords wc, PinPoint pp, SDL_Color color);

#endif /* WHEN_THE_FACTORY_RENDERER_ */
