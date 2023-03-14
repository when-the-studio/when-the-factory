#ifndef WHEN_THE_FACTORY_RENDERER_
#define WHEN_THE_FACTORY_RENDERER_

#include <SDL2/SDL.h>

#define WINDOW_W 1400
#define WINDOW_H 800

extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;
extern SDL_Texture* g_spritesheet;

void renderer_init(void);

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

/* Renders the given text so that its pin point is as the given window coordinates. */
void render_text(char const* text, int x, int y, SDL_Color color, PinPoint pin_point);

#endif // WHEN_THE_FACTORY_RENDERER_