#ifndef WHEN_THE_FACTORY_RENDERER_
#define WHEN_THE_FACTORY_RENDERER_

#include <SDL2/SDL.h>

#define WINDOW_W 1400
#define WINDOW_H 800

extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;

void renderer_init(void);
#endif // WHEN_THE_FACTORY_RENDERER_